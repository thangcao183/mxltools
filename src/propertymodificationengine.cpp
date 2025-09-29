#include "propertymodificationengine.h"
#include "itemparser.h"
#include "itemdatabase.h"
#include "reversebitwriter.h"
#include "reversebitreader.h"
#include "enums.h"
#include "helpers.h"

#include <QApplication>
#include <QDebug>

PropertyModificationEngine::PropertyModificationEngine(QObject *parent)
    : QObject(parent)
{
}

bool PropertyModificationEngine::modifyItemProperties(ItemInfo *item, const PropertiesMultiMap &newProperties)
{
    if (!item) {
        setError(tr("Invalid item"));
        return false;
    }
    
    emit statusChanged(tr("Validating properties..."));
    emit progressChanged(10);
    
    // Validate all properties
    QString validationError;
    if (!validatePropertyCombination(newProperties, &validationError)) {
        setError(validationError);
        return false;
    }
    
    emit statusChanged(tr("Backing up original properties..."));
    emit progressChanged(20);
    
    // Backup original properties
    PropertiesMultiMap originalProps;
    for (auto it = item->props.constBegin(); it != item->props.constEnd(); ++it) {
        originalProps.insert(it.key(), new ItemProperty(*it.value()));
    }
    
    try {
        emit statusChanged(tr("Clearing existing properties..."));
        emit progressChanged(30);
        
        // Clear existing properties
        qDeleteAll(item->props);
        item->props.clear();
        
        emit statusChanged(tr("Adding new properties..."));
        emit progressChanged(50);
        
        // Add new properties
        for (auto it = newProperties.constBegin(); it != newProperties.constEnd(); ++it) {
            item->props.insert(it.key(), new ItemProperty(*it.value()));
        }
        
        emit statusChanged(tr("Reconstructing item bit string..."));
        emit progressChanged(70);
        
        // Reconstruct the item's bit string
        if (!reconstructItemBitString(item)) {
            // Restore original properties on failure
            qDeleteAll(item->props);
            item->props = originalProps;
            return false;
        }
        
        emit statusChanged(tr("Updating display strings..."));
        emit progressChanged(90);
        
        // Update display strings for all properties
        for (auto it = item->props.begin(); it != item->props.end(); ++it) {
            ItemParser::createDisplayStringForPropertyWithId(it.key(), it.value());
        }
        
        // Clean up backup
        qDeleteAll(originalProps);
        
        item->hasChanged = true;
        
        emit statusChanged(tr("Properties updated successfully"));
        emit progressChanged(100);
        
        return true;
        
    } catch (const std::exception &e) {
        // Restore original properties
        qDeleteAll(item->props);
        item->props = originalProps;
        setError(QString("Exception during property modification: %1").arg(e.what()));
        return false;
    } catch (...) {
        // Restore original properties
        qDeleteAll(item->props);
        item->props = originalProps;
        setError(tr("Unknown error during property modification"));
        return false;
    }
}

bool PropertyModificationEngine::reconstructItemBitString(ItemInfo *item)
{
    if (!item) return false;
    
    try {
        // Use ItemParser logic to calculate exact properties start position
        int propertiesStart = calculatePropertiesStartPosition(item);
        
        if (propertiesStart < 0) {
            // Fallback: estimate properties start based on typical item structure
            // This is safer than failing completely
            qDebug() << "PropertyModificationEngine: Using fallback estimation for properties start";
            
            // Typical minimum item size before properties: ~200-400 bits
            // Look for a reasonable position by scanning backwards from end
            propertiesStart = item->bitString.length() - 200; // Conservative estimate
            
            if (propertiesStart < 100) propertiesStart = 100; // Ensure some minimum space for item data
            
            qDebug() << "PropertyModificationEngine: Fallback properties start:" << propertiesStart;
        }
        
        // Keep everything before properties (item header + basic data)
        QString beforeProperties = item->bitString.left(propertiesStart);
        qDebug() << "PropertyModificationEngine: Before properties section length:" << beforeProperties.length();
        
        // Check if there's data after properties that we need to preserve
        QString afterProperties;
        if (!item->props.isEmpty()) {
            // Find where current properties end by looking for end marker (511)
            QString currentBitString = item->bitString;
            int endMarkerPos = -1;
            
            // Search for property end marker (511 = 111111111 in 9 bits)
            QString endMarkerPattern = "111111111";
            for (int i = propertiesStart; i <= currentBitString.length() - 9; i++) {
                if (currentBitString.mid(i, 9) == endMarkerPattern) {
                    endMarkerPos = i + 9;
                    break;
                }
            }
            
            if (endMarkerPos > 0 && endMarkerPos < currentBitString.length()) {
                afterProperties = currentBitString.mid(endMarkerPos);
                qDebug() << "PropertyModificationEngine: Found data after properties, length:" << afterProperties.length();
            }
        }
        
        // Build new properties section using ItemParser's property writing logic
        QString propertiesBits = buildPropertiesBitString(item->props);
        qDebug() << "PropertyModificationEngine: New properties section length:" << propertiesBits.length();
        
        // Assemble complete bit string
        QString newBitString = beforeProperties + propertiesBits + afterProperties;
        
        qDebug() << "PropertyModificationEngine: Before reconstruction:" 
                 << "total=" << item->bitString.length()
                 << "before_props=" << beforeProperties.length() 
                 << "after_props=" << afterProperties.length();
        
        // Byte align the result (same as ItemParser does)
        ReverseBitWriter::byteAlignBits(newBitString);
        
        qDebug() << "PropertyModificationEngine: After reconstruction:" 
                 << "total=" << newBitString.length() << "(was:" << item->bitString.length() << ")"
                 << "new_props=" << propertiesBits.length();
        
        // Validate we didn't lose critical data
        if (newBitString.length() < item->bitString.length() - 50) { // Allow some difference for property changes
            qDebug() << "PropertyModificationEngine: WARNING - Significant bit loss detected!";
            qDebug() << "Original bit string:" << item->bitString.left(100) << "...";
            qDebug() << "New bit string:" << newBitString.left(100) << "...";
            // Don't apply the change if too much data is lost
            setError(tr("Reconstruction would lose too much data"));
            return false;
        }
        
        // Update the item's bit string
        item->bitString = newBitString;
        
        return true;
        
    } catch (const std::exception &e) {
        setError(QString("Failed to reconstruct bit string: %1").arg(e.what()));
        return false;
    } catch (int errorCode) {
        setError(QString("Bit manipulation error: %1").arg(errorCode));
        return false;
    } catch (...) {
        setError(tr("Unknown error during bit string reconstruction"));
        return false;
    }
}

QString PropertyModificationEngine::buildPropertiesBitString(const PropertiesMultiMap &properties)
{
    QString bitString;
    
    qDebug() << "PropertyModificationEngine: Building properties bit string for" << properties.size() << "properties";
    
    try {
        // Get ordered list of properties
        QList<QPair<int, ItemProperty*>> orderedProps = getOrderedProperties(properties);
        
        qDebug() << "PropertyModificationEngine: Got" << orderedProps.size() << "ordered properties";
        
        // Write each property
        for (const auto &pair : orderedProps) {
            qDebug() << "PropertyModificationEngine: Writing property ID" << pair.first 
                     << "value" << pair.second->value << "param" << pair.second->param;
            writePropertyToBits(bitString, pair.first, pair.second);
        }
        
        // Add end marker
        insertEndMarker(bitString);
        
    } catch (...) {
        qDebug() << "PropertyModificationEngine: Exception in buildPropertiesBitString";
        throw;
    }
    
    return bitString;
}

void PropertyModificationEngine::writePropertyToBits(QString &bitString, int propertyId, const ItemProperty *property)
{
    if (!property) return;
    
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) return;
    
    // Handle special properties
    if (isSpecialProperty(propertyId)) {
        switch (propertyId) {
            case Enums::ItemProperties::EnhancedDamage:
                writeEnhancedDamageProperty(bitString, property->value);
                return;
                
            case Enums::ItemProperties::MinimumDamageFire:
            case Enums::ItemProperties::MinimumDamageLightning:
            case Enums::ItemProperties::MinimumDamageMagic:
            case Enums::ItemProperties::MinimumDamageCold:
            case Enums::ItemProperties::MinimumDamagePoison:
                // Elemental damage requires special handling - skip for now
                // This should be handled by the calling function to write min/max/length together
                break;
        }
    }
    
    // Standard property writing
    
    // Write property ID (9 bits)
    insertPropertyId(bitString, propertyId);
    
    // Write parameter if present
    if (propTxt->paramBits > 0) {
        appendBits(bitString, property->param, propTxt->paramBits);
    }
    
    // Write value with add correction
    int adjustedValue = property->value + propTxt->add;
    appendBits(bitString, adjustedValue, propTxt->bits);
}

void PropertyModificationEngine::writeEnhancedDamageProperty(QString &bitString, int value)
{
    insertPropertyId(bitString, Enums::ItemProperties::EnhancedDamage);
    
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(Enums::ItemProperties::EnhancedDamage);
    if (!propTxt) return;
    
    int adjustedValue = value + propTxt->add;
    
    // Enhanced damage is stored twice
    appendBits(bitString, adjustedValue, propTxt->bits);
    appendBits(bitString, adjustedValue, propTxt->bits);
}

void PropertyModificationEngine::writeElementalDamageProperty(QString &bitString, int baseId, int minValue, int maxValue, int length)
{
    ItemPropertyTxt *minPropTxt = ItemDataBase::Properties()->value(baseId);
    ItemPropertyTxt *maxPropTxt = ItemDataBase::Properties()->value(baseId + 1);
    
    if (!minPropTxt || !maxPropTxt) return;
    
    // Write min damage
    insertPropertyId(bitString, baseId);
    appendBits(bitString, minValue + minPropTxt->add, minPropTxt->bits);
    
    // Write max damage
    insertPropertyId(bitString, baseId + 1);
    appendBits(bitString, maxValue + maxPropTxt->add, maxPropTxt->bits);
    
    // Write length for cold/poison
    if (length > 0 && (baseId == Enums::ItemProperties::MinimumDamageCold || 
                       baseId == Enums::ItemProperties::MinimumDamagePoison)) {
        ItemPropertyTxt *lengthPropTxt = ItemDataBase::Properties()->value(baseId + 2);
        if (lengthPropTxt) {
            insertPropertyId(bitString, baseId + 2);
            appendBits(bitString, length + lengthPropTxt->add, lengthPropTxt->bits);
        }
    }
}

void PropertyModificationEngine::writeSkillProperty(QString &bitString, int propertyId, int value, quint32 skillId)
{
    insertPropertyId(bitString, propertyId);
    
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) return;
    
    // Write skill ID parameter
    if (propTxt->paramBits > 0) {
        appendBits(bitString, skillId, propTxt->paramBits);
    }
    
    // Write skill level
    appendBits(bitString, value + propTxt->add, propTxt->bits);
}

void PropertyModificationEngine::writeBooleanProperty(QString &bitString, int propertyId, bool value)
{
    insertPropertyId(bitString, propertyId);
    
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) return;
    
    appendBits(bitString, value ? 1 : 0, propTxt->bits);
}

bool PropertyModificationEngine::validateProperty(int propertyId, int value, quint32 parameter, QString *error)
{
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) {
        if (error) *error = tr("Unknown property ID: %1").arg(propertyId);
        return false;
    }
    
    // Check bit limits
    int maxValue = (1 << propTxt->bits) - 1 - propTxt->add;
    int minValue = -propTxt->add;
    
    if (value < minValue || value > maxValue) {
        if (error) *error = tr("Property %1: Value %2 out of range [%3, %4]")
                               .arg(propertyId).arg(value).arg(minValue).arg(maxValue);
        return false;
    }
    
    // Parameter validation
    if (propTxt->paramBits > 0) {
        quint32 maxParam = (1 << propTxt->paramBits) - 1;
        if (parameter > maxParam) {
            if (error) *error = tr("Property %1: Parameter %2 out of range [0, %3]")
                                   .arg(propertyId).arg(parameter).arg(maxParam);
            return false;
        }
    }
    
    // Special validation - only for specific properties that need it
    switch (propertyId) {
        case Enums::ItemProperties::EnhancedDamage:
            return validateEnhancedDamage(value, error);
            
        case Enums::ItemProperties::Defence: // Only base defense, not enhanced defense
            return validateDefense(value, error);
            
        // Validate all damage-related properties
        case Enums::ItemProperties::MinimumDamage:
        case Enums::ItemProperties::MaximumDamage:
        case Enums::ItemProperties::MinimumDamageSecondary:
        case Enums::ItemProperties::MaximumDamageSecondary:
        case Enums::ItemProperties::MinimumDamageFire:
        case Enums::ItemProperties::MaximumDamageFire:
        case Enums::ItemProperties::MinimumDamageLightning:
        case Enums::ItemProperties::MaximumDamageLightning:
        case Enums::ItemProperties::MinimumDamageMagic:
        case Enums::ItemProperties::MaximumDamageMagic:
        case Enums::ItemProperties::MinimumDamageCold:
        case Enums::ItemProperties::MaximumDamageCold:
        case Enums::ItemProperties::MinimumDamagePoison:
        case Enums::ItemProperties::MaximumDamagePoison:
            return validateElementalDamage(propertyId, value, error);
    }
    
    // For all other properties (stats, resistances, etc.), 
    // only validate bit range limits - allow negative values if in range
    
    return true;
}

bool PropertyModificationEngine::validatePropertyCombination(const PropertiesMultiMap &properties, QString *error)
{
    QMap<int, int> propertyCount;
    
    // Count occurrences of each property
    for (auto it = properties.constBegin(); it != properties.constEnd(); ++it) {
        propertyCount[it.key()]++;
        
        // Validate individual property
        if (!validateProperty(it.key(), it.value()->value, it.value()->param, error)) {
            return false;
        }
    }
    
    // Check for duplicate properties that shouldn't be duplicated
    for (auto it = propertyCount.constBegin(); it != propertyCount.constEnd(); ++it) {
        if (it.value() > 1) {
            ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(it.key());
            QString propName = propTxt ? QString::fromUtf8(propTxt->stat) : QString::number(it.key());
            
            if (error) *error = tr("Duplicate property not allowed: %1 (appears %2 times)")
                                   .arg(propName).arg(it.value());
            return false;
        }
    }
    
    return true;
}

QList<int> PropertyModificationEngine::getDependentProperties(int propertyId) const
{
    QList<int> dependents;
    
    switch (propertyId) {
        case Enums::ItemProperties::MinimumDamageFire:
            dependents << Enums::ItemProperties::MaximumDamageFire;
            break;
            
        case Enums::ItemProperties::MinimumDamageLightning:
            dependents << Enums::ItemProperties::MaximumDamageLightning;
            break;
            
        case Enums::ItemProperties::MinimumDamageMagic:
            dependents << Enums::ItemProperties::MaximumDamageMagic;
            break;
            
        case Enums::ItemProperties::MinimumDamageCold:
            dependents << Enums::ItemProperties::MaximumDamageCold 
                      << Enums::ItemProperties::DurationCold;
            break;
            
        case Enums::ItemProperties::MinimumDamagePoison:
            dependents << Enums::ItemProperties::MaximumDamagePoison 
                      << Enums::ItemProperties::DurationPoison;
            break;
    }
    
    return dependents;
}

bool PropertyModificationEngine::isSpecialProperty(int propertyId) const
{
    switch (propertyId) {
        case Enums::ItemProperties::EnhancedDamage:
        case Enums::ItemProperties::MinimumDamageFire:
        case Enums::ItemProperties::MinimumDamageLightning:
        case Enums::ItemProperties::MinimumDamageMagic:
        case Enums::ItemProperties::MinimumDamageCold:
        case Enums::ItemProperties::MinimumDamagePoison:
            return true;
        default:
            return false;
    }
}

int PropertyModificationEngine::calculateBitLength(int propertyId, int value, quint32 parameter) const
{
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) return 0;
    
    int totalBits = 9; // Property ID
    totalBits += propTxt->paramBits; // Parameter
    totalBits += propTxt->bits; // Value
    
    // Special cases
    if (propertyId == Enums::ItemProperties::EnhancedDamage) {
        totalBits += propTxt->bits; // Enhanced damage stores value twice
    }
    
    return totalBits;
}

void PropertyModificationEngine::appendBits(QString &bitString, quint64 value, int bitCount)
{
    if (bitCount <= 0) return;
    
    QString bits = QString::number(value, 2).rightJustified(bitCount, '0');
    
    // Reverse the bits (LSB first)
    std::reverse(bits.begin(), bits.end());
    
    bitString.prepend(bits);
}

void PropertyModificationEngine::insertPropertyId(QString &bitString, int propertyId)
{
    appendBits(bitString, propertyId, 9);
}

void PropertyModificationEngine::insertEndMarker(QString &bitString)
{
    insertPropertyId(bitString, 511); // End marker
}

QList<QPair<int, ItemProperty*>> PropertyModificationEngine::getOrderedProperties(const PropertiesMultiMap &properties)
{
    QList<QPair<int, ItemProperty*>> orderedList;
    
    qDebug() << "PropertyModificationEngine: getOrderedProperties input size:" << properties.size();
    
    // Convert multimap to list
    for (auto it = properties.constBegin(); it != properties.constEnd(); ++it) {
        if (!it.value()) {
            qDebug() << "PropertyModificationEngine: Null property value for ID" << it.key();
            continue;
        }
        orderedList.append(QPair<int, ItemProperty*>(it.key(), it.value()));
    }
    
    qDebug() << "PropertyModificationEngine: After filtering, list size:" << orderedList.size();
    
    // Sort by property ID
    std::sort(orderedList.begin(), orderedList.end(), 
              [](const QPair<int, ItemProperty*> &a, const QPair<int, ItemProperty*> &b) {
                  return a.first < b.first;
              });
    
    return orderedList;
}

bool PropertyModificationEngine::validateEnhancedDamage(int value, QString *error) const
{
    if (value < 0 || value > 32767) {
        if (error) *error = tr("Enhanced Damage must be between 0 and 32767");
        return false;
    }
    return true;
}

bool PropertyModificationEngine::validateElementalDamage(int propertyId, int value, QString *error) const
{
    // Only validate that actual damage values are non-negative
    // Don't block negative values for properties that might have penalties
    switch (propertyId) {
        case Enums::ItemProperties::MinimumDamageFire:
        case Enums::ItemProperties::MaximumDamageFire:
        case Enums::ItemProperties::MinimumDamageLightning:
        case Enums::ItemProperties::MaximumDamageLightning:
        case Enums::ItemProperties::MinimumDamageMagic:
        case Enums::ItemProperties::MaximumDamageMagic:
        case Enums::ItemProperties::MinimumDamageCold:
        case Enums::ItemProperties::MaximumDamageCold:
        case Enums::ItemProperties::MinimumDamagePoison:
        case Enums::ItemProperties::MaximumDamagePoison:
            if (value < 0) {
                if (error) *error = tr("Damage values cannot be negative");
                return false;
            }
            break;
        default:
            // For other properties, allow negative values (could be penalties/resistances)
            break;
    }
    
    return true;
}

bool PropertyModificationEngine::validateDefense(int value, QString *error) const
{
    // Only actual defense values cannot be negative
    // Other defense-related properties might allow negative values (penalties)
    if (value < 0) {
        if (error) *error = tr("Defense value cannot be negative");
        return false;
    }
    return true;
}

bool PropertyModificationEngine::validateSkillProperty(int propertyId, int value, quint32 skillId, QString *error) const
{
    if (value < 0 || value > 99) {
        if (error) *error = tr("Skill level must be between 0 and 99");
        return false;
    }
    
    // Could validate skill ID against skill database
    return true;
}

void PropertyModificationEngine::setError(const QString &error)
{
    _lastError = error;
    emit errorOccurred(error);
}

int PropertyModificationEngine::calculatePropertiesStartPosition(ItemInfo *item)
{
    if (!item) return -1;
    
    // Try to use existing bitStringOffset from properties if available
    // This is more reliable than re-parsing
    if (!item->props.isEmpty()) {
        auto firstProp = item->props.constBegin();
        int offset = firstProp.value()->bitStringOffset;
        if (offset > 16) { // Must be after JM header
            // Convert from ItemParser offset (which includes JM) to actual bit position
            // ItemParser offset points to parameter+value start, we need to go back to property ID
            ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(firstProp.key());
            if (propTxt) {
                // Go back: value bits + param bits + property ID bits = total property size
                int propertyTotalBits = 9 + propTxt->paramBits + propTxt->bits;
                int propertiesStart = item->bitString.length() - (offset - 16) - propTxt->paramBits - propTxt->bits;
                
                qDebug() << "PropertyModificationEngine: Using existing offset method, properties start at:" << propertiesStart;
                
                if (propertiesStart >= 0 && propertiesStart < item->bitString.length()) {
                    return propertiesStart;
                }
            }
        }
    }
    
    // Fallback: try re-parsing (but catch exceptions gracefully)
    ReverseBitReader bitReader(item->bitString);
    
    try {
        qDebug() << "PropertyModificationEngine: Attempting to re-parse to find properties start";
        qDebug() << "BitString length:" << item->bitString.length() << "isEar:" << item->isEar << "isExtended:" << item->isExtended;
        
        // Skip all the same fields that ItemParser skips before reaching properties
        skipItemBasicData(bitReader, item);
        
        // Return current position - this is where properties start
        int pos = bitReader.pos();
        
        qDebug() << "PropertyModificationEngine: Re-parse successful, properties start at:" << pos;
        
        // Validate position is reasonable
        if (pos < 0 || pos >= item->bitString.length()) {
            qDebug() << "PropertyModificationEngine: Invalid properties start position:" << pos;
            return -1;
        }
        
        return pos;
        
    } catch (int errorCode) {
        qDebug() << "PropertyModificationEngine: Exception" << errorCode << "while calculating properties position";
        qDebug() << "BitReader position when exception occurred:" << bitReader.pos();
        return -1;
    } catch (...) {
        qDebug() << "PropertyModificationEngine: Unknown exception while calculating properties position";
        return -1;
    }
}

void PropertyModificationEngine::skipItemBasicData(ReverseBitReader &bitReader, ItemInfo *item)
{
    // This mirrors the exact parsing logic in ItemParser::parseItem
    // Skip all fields before properties section
    
    qDebug() << "PropertyModificationEngine: Starting skipItemBasicData, absolutePos:" << bitReader.absolutePos();
    
    // Add validation to prevent reading past bitstring end
    // ReverseBitReader uses internal _pos that counts from end, not from beginning
    auto safeSkip = [&bitReader](int bits, const char* field = "") {
        if (bitReader.absolutePos() - bits < 0) {
            qDebug() << "PropertyModificationEngine: Cannot skip" << bits << "bits for" << field 
                     << "at position" << bitReader.pos() << "(absolute:" << bitReader.absolutePos() << ")";
            throw 3; // Same exception code as ReverseBitReader
        }
        bitReader.skip(bits);
    };
    
    auto safeReadNumber = [&bitReader](int bits, const char* field = "") -> int {
        if (bitReader.absolutePos() - bits < 0) {
            qDebug() << "PropertyModificationEngine: Cannot read" << bits << "bits for" << field 
                     << "at position" << bitReader.pos() << "(absolute:" << bitReader.absolutePos() << ")";
            throw 3; // Same exception code as ReverseBitReader
        }
        return bitReader.readNumber(bits);
    };
    
    auto safeReadBool = [&bitReader](const char* field = "") -> bool {
        if (bitReader.absolutePos() - 1 < 0) {
            qDebug() << "PropertyModificationEngine: Cannot read bool for" << field 
                     << "at position" << bitReader.pos() << "(absolute:" << bitReader.absolutePos() << ")";
            throw 3; // Same exception code as ReverseBitReader  
        }
        return bitReader.readBool();
    };
    
    // Skip item flags and basic info
    safeSkip(1, "isQuest"); 
    safeSkip(3, "unknown1");
    safeSkip(1, "isIdentified"); 
    safeSkip(5, "unknown2");
    safeSkip(1, "isDuped"); 
    safeSkip(1, "isSocketed"); 
    safeSkip(2, "unknown3");
    safeSkip(2, "illegal+unk"); 
    safeSkip(1, "isEar"); 
    safeSkip(1, "isStarter"); 
    safeSkip(2, "unknown4");
    safeSkip(1, "unknown5");
    safeSkip(1, "isExtended"); 
    safeSkip(1, "isEthereal"); 
    safeSkip(1, "unknown6");
    safeSkip(1, "isPersonalized"); 
    safeSkip(1, "unknown7");
    safeSkip(1, "isRW"); 
    safeSkip(5, "unknown8");
    safeSkip(8, "version"); 
    safeSkip(2, "unknown9");
    safeSkip(3, "location"); 
    safeSkip(4, "whereEquipped"); 
    safeSkip(4, "column"); 
    safeSkip(4, "row"); 
    safeSkip(3, "storage");

    if (item->isEar) {
        safeSkip(3, "earClassCode");
        safeSkip(7, "earLevel");
        for (int i = 0; i < 18; ++i) {
            if (safeReadNumber(7, "earNameChar") == 0) break;
        }
        return; // Ears don't have properties
    }

    // Skip item type (4 bytes)
    safeSkip(32, "itemType");

    if (item->isExtended) {
        qDebug() << "PropertyModificationEngine: Processing extended item, pos:" << bitReader.pos();
        safeSkip(3); // socketablesNumber
        safeSkip(32); // guid
        safeSkip(7); // ilvl
        safeSkip(4); // quality
        
        if (safeReadBool()) // has variable graphic
            safeSkip(3);
        if (safeReadBool()) // autoprefix
            safeSkip(11);

        ItemBase *itemBase = ItemDataBase::Items()->value(item->itemType);
        if (!itemBase) return;
        
        switch (item->quality) {
        case Enums::ItemQuality::Normal:
            break;
        case Enums::ItemQuality::LowQuality: 
        case Enums::ItemQuality::HighQuality:
            safeSkip(3); // nonMagicType
            break;
        case Enums::ItemQuality::Magic:
            safeSkip(22); // prefix & suffix
            break;
        case Enums::ItemQuality::Set: 
        case Enums::ItemQuality::Unique:
            safeSkip(15); // setOrUniqueId
            break;
        case Enums::ItemQuality::Rare: 
        case Enums::ItemQuality::Crafted:
            safeSkip(16); // first & second names
            for (int i = 0; i < 6; ++i)
                if (safeReadBool())
                    safeSkip(11); // prefix or suffix (1-3)
            break;
        case Enums::ItemQuality::Honorific:
            safeSkip(16);
            break;
        }

        if (item->isRW)
            safeSkip(16); // RW code

        // Skip inscribed name
        if (item->isPersonalized) {
            for (int i = 0; i < 16; ++i) {
                if (safeReadNumber(ItemParser::kInscribedNameCharacterLength) == 0) break;
            }
        }

        safeSkip(1); // tome of ID bit
        if (ItemDataBase::isTomeWithScrolls(item))
            safeSkip(5); // book ID

        // Handle armor/weapon specific data - mirror ItemParser logic exactly
        const bool isArmor = ItemParser::itemTypesInheritFromType(itemBase->types, "armo");
        if (isArmor) {
            ItemPropertyTxt *defenceProp = ItemDataBase::Properties()->value(Enums::ItemProperties::Defence);
            if (defenceProp) safeReadNumber(defenceProp->bits); // Read defense but don't store
        }
        if (isArmor || ItemParser::itemTypesInheritFromType(itemBase->types, "weap")) {
            ItemPropertyTxt *maxDurabilityProp = ItemDataBase::Properties()->value(Enums::ItemProperties::DurabilityMax);
            if (maxDurabilityProp) {
                // Read maxDurability to check if we need to read current durability too
                int maxDurability = safeReadNumber(maxDurabilityProp->bits) - maxDurabilityProp->add;
                if (maxDurability > 0) {
                    ItemPropertyTxt *durabilityProp = ItemDataBase::Properties()->value(Enums::ItemProperties::Durability);
                    if (durabilityProp) safeReadNumber(durabilityProp->bits); // Read current durability but don't store
                }
            }
        }

        // Handle quantity for stackable items - must read, not skip
        if (itemBase->isStackable)
            safeReadNumber(9); // Read quantity but don't store
        
        // Handle sockets number - must read, not skip
        if (item->isSocketed)
            safeReadNumber(4); // Read sockets number but don't store

        // Handle set lists flags - must read each bool, not bulk skip
        if (item->quality == Enums::ItemQuality::Set) {
            const int kSetListsNumber = 5;
            for (int i = 0; i < kSetListsNumber; ++i)
                safeReadBool(); // Read each set list flag but don't store
        }
    }
    
    // Now bitReader is positioned right at the start of properties
}