#include "enhancedpropertyadditionengine.h"
#include "itemparser.h"
#include "itemdatabase.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>
#include <QBuffer>
#include <QDataStream>
#include <algorithm>

// Constants
const QString EnhancedPropertyAdditionEngine::END_MARKER = "111111111";
const QByteArray EnhancedPropertyAdditionEngine::ITEM_HEADER = QByteArray("JM");

EnhancedPropertyAdditionEngine::EnhancedPropertyAdditionEngine(QObject *parent)
    : QObject(parent)
{
    initializeDefaultProperties();
    
    // Try to load external props.tsv database
    loadPropertyDatabase();
}

bool EnhancedPropertyAdditionEngine::addPropertyToItem(ItemInfo *item, int propertyId, int value, quint32 parameter)
{
    clearError();
    
    if (!item) {
        setError(tr("Invalid item"));
        return false;
    }
    
    emit statusChanged(tr("Validating property..."));
    
    // Validate property
    QString validationError;
    if (!validateProperty(propertyId, value, &validationError)) {
        setError(validationError);
        return false;
    }
    
    // Check if property already exists
    if (item->props.contains(propertyId)) {
        setError(tr("Property already exists. Remove it first or modify its value."));
        return false;
    }
    
    emit statusChanged(tr("Creating standalone item data..."));
    
    // SMART APPROACH: Create standalone item file from ItemInfo
    // This mimics our successful CLI tool approach
    QByteArray standaloneItemData = createStandaloneItemFile(item);
    if (standaloneItemData.isEmpty()) {
        setError(tr("Failed to create standalone item data"));
        return false;
    }
    
    emit statusChanged(tr("Processing standalone item..."));
    
    // Apply property addition to standalone data
    QByteArray modifiedItemData = addPropertyToStandaloneItem(standaloneItemData, propertyId, value, parameter);
    if (modifiedItemData.isEmpty()) {
        setError(tr("Failed to add property to standalone item"));
        return false;
    }
    
    emit statusChanged(tr("Integrating changes back..."));
    
    // Parse modified data back to update original ItemInfo
    if (!updateItemFromStandaloneData(item, modifiedItemData)) {
        setError(tr("Failed to integrate changes back to item"));
        return false;
    }
    
    qDebug() << "EnhancedPropertyAdditionEngine: Successfully completed extract→modify→reintegrate";
    emit statusChanged(tr("Property added successfully"));
    return true;
}

bool EnhancedPropertyAdditionEngine::addPropertiesToItem(ItemInfo *item, const QMap<int, QPair<int, quint32>> &properties)
{
    clearError();
    
    if (properties.isEmpty()) {
        setError(tr("No properties to add"));
        return false;
    }
    
    // For now, add properties one by one
    // TODO: Optimize by building all property bits at once
    for (auto it = properties.constBegin(); it != properties.constEnd(); ++it) {
        int propertyId = it.key();
        int value = it.value().first;
        quint32 parameter = it.value().second;
        
        if (!addPropertyToItem(item, propertyId, value, parameter)) {
            return false;
        }
    }
    
    return true;
}

bool EnhancedPropertyAdditionEngine::validateProperty(int propertyId, int value, QString *error)
{
    if (!isPropertySupported(propertyId)) {
        if (error) *error = tr("Property ID %1 is not supported").arg(propertyId);
        return false;
    }
    
    PropertySpec spec = getPropertySpec(propertyId);
    
    // Calculate value range
    int maxRawValue = (1 << spec.bits) - 1;
    int maxValue = maxRawValue - spec.add;
    int minValue = 0 - spec.add;
    
    if (value < minValue || value > maxValue) {
        if (error) *error = tr("Value %1 is out of range [%2, %3] for property %4")
                                   .arg(value).arg(minValue).arg(maxValue).arg(spec.name);
        return false;
    }
    
    return true;
}

bool EnhancedPropertyAdditionEngine::isPropertySupported(int propertyId) const
{
    return _propertyDatabase.contains(propertyId);
}

EnhancedPropertyAdditionEngine::PropertySpec EnhancedPropertyAdditionEngine::getPropertySpec(int propertyId) const
{
    return _propertyDatabase.value(propertyId, PropertySpec{});
}

void EnhancedPropertyAdditionEngine::loadPropertyDatabase(const QString &propsPath)
{
    QString path = propsPath;
    if (path.isEmpty()) {
        // Try default locations
        QStringList candidates = {
            QCoreApplication::applicationDirPath() + "/props.tsv",
            QDir::currentPath() + "/props.tsv",
            ":/resources/props.tsv"
        };
        
        for (const QString &candidate : candidates) {
            if (QFile::exists(candidate)) {
                path = candidate;
                break;
            }
        }
    }
    
    if (path.isEmpty() || !QFile::exists(path)) {
        qWarning() << "EnhancedPropertyAdditionEngine: props.tsv not found, using defaults";
        return;
    }
    
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "EnhancedPropertyAdditionEngine: Failed to open" << path;
        return;
    }
    
    QTextStream in(&file);
    QString line;
    
    // Skip header line
    if (!in.atEnd()) {
        in.readLine();
    }
    
    _propertyDatabase.clear();
    
    while (!in.atEnd()) {
        line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;
        
        QStringList parts = line.split('\t');
        if (parts.size() < 5) continue;
        
        bool ok;
        PropertySpec spec;
        spec.id = parts[0].toInt(&ok);
        if (!ok) continue;
        
        spec.name = parts[1];
        spec.add = parts[2].toInt(&ok);
        if (!ok) continue;
        
        spec.bits = parts[3].toInt(&ok);
        if (!ok) continue;
        
        spec.paramBits = parts[4].toInt(&ok);
        if (!ok) continue;
        
        _propertyDatabase[spec.id] = spec;
    }
    
    qDebug() << "EnhancedPropertyAdditionEngine: Loaded" << _propertyDatabase.size() << "properties from" << path;
}

QVector<EnhancedPropertyAdditionEngine::PropertySpec> EnhancedPropertyAdditionEngine::getSupportedProperties() const
{
    QVector<PropertySpec> result;
    for (auto it = _propertyDatabase.constBegin(); it != _propertyDatabase.constEnd(); ++it) {
        result.append(it.value());
    }
    return result;
}

// Private Methods

QString EnhancedPropertyAdditionEngine::byteToBinary_LSB(uint8_t byte) const
{
    QString result;
    result.reserve(8);
    for (int i = 0; i < 8; i++) { // LSB-first: bit 0 first, bit 7 last
        result += ((byte >> i) & 1) ? '1' : '0';
    }
    return result;
}

uint8_t EnhancedPropertyAdditionEngine::binaryToByte_LSB(const QString &binary) const
{
    uint8_t result = 0;
    for (int i = 0; i < 8 && i < binary.length(); i++) {
        if (binary[i] == '1') {
            result |= (1 << i); // LSB-first: bit 0 at position 0
        }
    }
    return result;
}

QString EnhancedPropertyAdditionEngine::numberToBinary(int number, int bits) const
{
    QString result;
    result.reserve(bits);
    // Use MSB-first for numbers (like CLI tool), but LSB-first for bytes
    for (int i = bits - 1; i >= 0; i--) {
        result += ((number >> i) & 1) ? '1' : '0';
    }
    return result;
}

int EnhancedPropertyAdditionEngine::binaryToNumber(const QString &binary) const
{
    int result = 0;
    // Use MSB-first for numbers (matching numberToBinary)
    for (int i = 0; i < binary.length(); i++) {
        if (binary[i] == '1') {
            result |= (1 << (binary.length() - 1 - i));
        }
    }
    return result;
}

bool EnhancedPropertyAdditionEngine::parseFileStructure(const QString &bitString, QString &content, QString &padding, int &endMarkerPos)
{
    // Find all occurrences of end marker
    QVector<int> endMarkerPositions;
    int pos = 0;
    while ((pos = bitString.indexOf(END_MARKER, pos)) != -1) {
        endMarkerPositions.append(pos);
        pos++;
    }
    
    if (endMarkerPositions.isEmpty()) {
        setError(tr("No end marker found in bit string"));
        return false;
    }
    
    // Use the last occurrence as the real end marker
    endMarkerPos = endMarkerPositions.last();
    
    // Structure: [content][end marker][padding]
    int contentBits = endMarkerPos;
    int paddingBits = bitString.length() - endMarkerPos - 9;
    
    if (contentBits < 0 || paddingBits < 0) {
        setError(tr("Invalid file structure: negative content or padding"));
        return false;
    }
    
    content = bitString.mid(0, contentBits);
    padding = bitString.mid(endMarkerPos + 9, paddingBits);
    
    return true;
}

QString EnhancedPropertyAdditionEngine::createPropertyBits(int propertyId, int value, quint32 parameter)
{
    if (!isPropertySupported(propertyId)) {
        setError(tr("Unsupported property ID: %1").arg(propertyId));
        return QString();
    }
    
    PropertySpec spec = getPropertySpec(propertyId);
    
    // Calculate raw value
    int rawValue = value + spec.add;
    
    // Create bits using correct format (MSB-first then reverse to LSB-first like CLI tool)
    // Build bits in correct order: ID (9 bits, LSB-first), then parameter (if present, LSB-first), then value (LSB-first)
    QString idBits = numberToBinary(propertyId, 9);
    std::reverse(idBits.begin(), idBits.end()); // Convert to LSB-first

    QString paramBits;
    if (spec.paramBits > 0) {
        paramBits = numberToBinary(parameter, spec.paramBits);
        std::reverse(paramBits.begin(), paramBits.end()); // Convert to LSB-first
    }

    QString valueBits = numberToBinary(rawValue, spec.bits);
    std::reverse(valueBits.begin(), valueBits.end()); // Convert to LSB-first

    QString result = idBits + paramBits + valueBits;
    
    return result;
}

QString EnhancedPropertyAdditionEngine::calculateNewPadding(int totalContentBits) const
{
    int paddingNeeded = (8 - (totalContentBits % 8)) % 8;
    return QString(paddingNeeded, '0');
}

QString EnhancedPropertyAdditionEngine::rebuildBitString(const QString &content, const QString &newPropertyBits, const QString &endMarker)
{
    // New content = original content + new property
    QString newContent = content + newPropertyBits;
    
    // Calculate new padding for byte alignment
    int totalContentBits = newContent.length() + endMarker.length();
    QString newPadding = calculateNewPadding(totalContentBits);
    
    // Final structure: [content + property][end marker][padding]
    QString result = newContent + endMarker + newPadding;
    
    return result;
}

QByteArray EnhancedPropertyAdditionEngine::bitStringToBytes(const QString &bitString) const
{
    QByteArray result;
    // Convert bitString (MSB-per-byte, chunk order matching ItemParser representation) into bytes
    // Split into 8-bit chunks (MSB-first per chunk), convert to numbers, then reverse chunk order to match file layout
    QVector<uint8_t> vals;
    for (int i = 0; i < bitString.length(); i += 8) {
        QString chunk = bitString.mid(i, 8);
        if (chunk.length() < 8) {
            chunk += QString(8 - chunk.length(), '0');
        }
        int v = binaryToNumber(chunk); // binaryToNumber expects MSB-first
        vals.append(static_cast<uint8_t>(v));
    }

    // Reverse order of byte chunks to match ItemParser/CLI behavior (they prepend bytes)
    std::reverse(vals.begin(), vals.end());

    for (uint8_t v : vals)
        result.append(static_cast<char>(v));

    return result;
}

QString EnhancedPropertyAdditionEngine::bytesToBitString(const QByteArray &bytes) const
{
    QString result;
    result.reserve(bytes.size() * 8);
    // Skip JM header if present
    int startIndex = (bytes.size() >= 2 && bytes.left(2) == ITEM_HEADER) ? 2 : 0;

    // Build bitstring by prepending MSB-first byte strings (this matches ItemParser::parseItem behavior)
    for (int i = startIndex; i < bytes.size(); i++) {
        uint8_t b = static_cast<uint8_t>(bytes[i]);
        QString s = numberToBinary(b, 8); // MSB-first
        result.prepend(s);
    }

    return result;
}

bool EnhancedPropertyAdditionEngine::validateBitStringIntegrity(const QString &bitString) const
{
    // Check if bit string contains only 0s and 1s
    for (const QChar &c : bitString) {
        if (c != '0' && c != '1') {
            qDebug() << "EnhancedPropertyAdditionEngine: Invalid character in bitString:" << c;
            return false;
        }
    }
    
    // Check if length is reasonable (very lenient - accept any reasonable D2I bitString)
    if (bitString.length() < 100 || bitString.length() > 50000) {
        qDebug() << "EnhancedPropertyAdditionEngine: BitString length out of range:" << bitString.length();
        return false;
    }
    
    return true;
}

bool EnhancedPropertyAdditionEngine::validateFileStructure(ItemInfo *item) const
{
    if (!item) return false;
    
    return validateBitStringIntegrity(item->bitString) && 
           item->bitString.contains(END_MARKER);
}

void EnhancedPropertyAdditionEngine::initializeDefaultProperties()
{
    // Initialize with common properties based on our research
    _propertyDatabase.clear();
    
    // Core stats
    _propertyDatabase[0] = {0, 200, 11, 0, "Strength"};
    _propertyDatabase[1] = {1, 200, 11, 0, "Energy"};
    _propertyDatabase[2] = {2, 200, 11, 0, "Dexterity"};
    _propertyDatabase[3] = {3, 200, 11, 0, "Vitality"};
    
    // Life and Mana
    _propertyDatabase[7] = {7, 500, 12, 0, "Life"};
    _propertyDatabase[9] = {9, 500, 12, 0, "Mana"};
    
    // Resistances
    _propertyDatabase[39] = {39, 200, 11, 0, "Fire Resistance"};
    _propertyDatabase[43] = {43, 200, 11, 0, "Lightning Resistance"};
    _propertyDatabase[41] = {41, 200, 11, 0, "Cold Resistance"};
    _propertyDatabase[45] = {45, 200, 11, 0, "Poison Resistance"};
    
    // Magic Find and Gold Find - TESTED SUCCESSFULLY!
    _propertyDatabase[79] = {79, 100, 9, 0, "Gold Find"};
    _propertyDatabase[80] = {80, 100, 9, 0, "Magic Find"};
    
    // Attack modifiers
    _propertyDatabase[93] = {93, 150, 9, 0, "Attack Speed"};
    _propertyDatabase[99] = {99, 150, 9, 0, "Attack Rating"};
    
    // Skills
    _propertyDatabase[127] = {127, 0, 5, 0, "All Skills"};
    
    qDebug() << "EnhancedPropertyAdditionEngine: Initialized" << _propertyDatabase.size() << "default properties";
}

QByteArray EnhancedPropertyAdditionEngine::createStandaloneItemFile(ItemInfo *item) const
{
    if (!item) return QByteArray();
    
    // Create a standalone D2I file from ItemInfo bitString
    // This mimics our successful CLI tool approach
    
    qDebug() << "EnhancedPropertyAdditionEngine: Creating standalone item file";
    qDebug() << "  - Original bitString length:" << item->bitString.length();
    
    // Convert bitString to bytes using ItemParser logic (chunk-prepend)
    QByteArray itemBytes;
    for (int i = 0, n = item->bitString.length(); i < n; i += 8) {
        QString chunk = item->bitString.mid(i, 8);
        if (chunk.length() < 8) break;
        
        bool ok = false;
        quint8 val = static_cast<quint8>(chunk.toShort(&ok, 2));
        if (!ok) {
            qDebug() << "EnhancedPropertyAdditionEngine: Failed to convert chunk:" << chunk;
            break;
        }
        itemBytes.prepend(static_cast<char>(val)); // ItemParser uses prepend
    }
    
    // Create full standalone file with JM header
    QByteArray standaloneFile = ITEM_HEADER + itemBytes;
    
    qDebug() << "EnhancedPropertyAdditionEngine: Created standalone file:";
    qDebug() << "  - File size:" << standaloneFile.size() << "bytes";
    qDebug() << "  - Item bytes:" << itemBytes.size() << "bytes";
    
    return standaloneFile;
}

QByteArray EnhancedPropertyAdditionEngine::addPropertyToStandaloneItem(const QByteArray &itemData, int propertyId, int value, quint32 parameter) const
{
    if (itemData.size() < 3) return QByteArray();
    
    qDebug() << "EnhancedPropertyAdditionEngine: Processing standalone item";
    qDebug() << "  - Input size:" << itemData.size() << "bytes";
    
    // Create bitString from standalone file using centralized helper
    QString bitString = bytesToBitString(itemData);
    
    qDebug() << "  - BitString length:" << bitString.length() << "bits";
    
    // Apply our proven property addition algorithm
    QString content, padding;
    int endMarkerPos;
    if (!const_cast<EnhancedPropertyAdditionEngine*>(this)->parseFileStructure(bitString, content, padding, endMarkerPos)) {
        qDebug() << "EnhancedPropertyAdditionEngine: Failed to parse standalone file structure";
        return QByteArray();
    }
    
    // Create property bits
    QString newPropertyBits = const_cast<EnhancedPropertyAdditionEngine*>(this)->createPropertyBits(propertyId, value, parameter);
    if (newPropertyBits.isEmpty()) {
        qDebug() << "EnhancedPropertyAdditionEngine: Failed to create property bits";
        return QByteArray();
    }
    
    // Rebuild bitString
    QString newBitString = const_cast<EnhancedPropertyAdditionEngine*>(this)->rebuildBitString(content, newPropertyBits);
    if (newBitString.isEmpty()) {
        qDebug() << "EnhancedPropertyAdditionEngine: Failed to rebuild bitString";
        return QByteArray();
    }
    
    qDebug() << "  - New bitString length:" << newBitString.length() << "bits";
    
    // Convert back to bytes using centralized helper (handles chunk order & MSB/LSB semantics)
    QByteArray newItemBytes = bitStringToBytes(newBitString);

    // Create new standalone file with header
    QByteArray result = ITEM_HEADER + newItemBytes;
    
    qDebug() << "  - Result size:" << result.size() << "bytes";
    
    return result;
}

bool EnhancedPropertyAdditionEngine::updateItemFromStandaloneData(ItemInfo *item, const QByteArray &modifiedData)
{
    if (!item || modifiedData.size() < 3) return false;
    
    qDebug() << "EnhancedPropertyAdditionEngine: Updating item from standalone data";
    
    // Parse the modified standalone file to get new bitString
    QBuffer buffer(const_cast<QByteArray*>(&modifiedData));
    if (!buffer.open(QIODevice::ReadOnly)) {
        qDebug() << "EnhancedPropertyAdditionEngine: Failed to open buffer";
        return false;
    }
    
    QDataStream stream(&buffer);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    // Parse the modified item
    ItemInfo *parsedItem = ItemParser::parseItem(stream, modifiedData);
    if (!parsedItem) {
        qDebug() << "EnhancedPropertyAdditionEngine: Failed to parse modified item";
        return false;
    }
    
    // Verify essential properties
    if (parsedItem->itemType != item->itemType) {
        qDebug() << "EnhancedPropertyAdditionEngine: Item type changed during processing";
        delete parsedItem;
        return false;
    }
    
    // Update original item with new data
    item->bitString = parsedItem->bitString;
    item->hasChanged = true;
    
    // Update properties map
    // Clear existing properties and add new ones
    qDeleteAll(item->props);
    item->props.clear();
    
    for (auto it = parsedItem->props.constBegin(); it != parsedItem->props.constEnd(); ++it) {
        item->props.insert(it.key(), new ItemProperty(*it.value()));
    }
    
    qDebug() << "EnhancedPropertyAdditionEngine: Successfully updated item";
    qDebug() << "  - New bitString length:" << item->bitString.length();
    qDebug() << "  - New properties count:" << item->props.size();
    
    delete parsedItem;
    return true;
}

void EnhancedPropertyAdditionEngine::setError(const QString &error)
{
    _lastError = error;
    emit errorOccurred(error);
    qWarning() << "EnhancedPropertyAdditionEngine:" << error;
}