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
        // Find the position where properties start in the bit string
        // Properties come after the item header and basic item data
        
        // Parse the existing bit string to find where properties begin
        ReverseBitReader reader(item->bitString);
        QString baseItemBits = item->bitString;
        
        // Find the properties section by looking for the first property ID or end marker
        int propertiesStart = -1;
        
        // Scan through the bit string looking for property IDs
        for (int pos = 0; pos < item->bitString.length() - 9; pos++) {
            QString segment = item->bitString.mid(pos, 9);
            bool ok;
            int value = segment.toInt(&ok, 2);
            
            if (ok && ((value > 0 && value < 512) || value == 511)) { // Valid property ID or end marker
                // Check if this looks like a valid property position
                ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(value);
                if (propTxt || value == 511) {
                    propertiesStart = pos;
                    break;
                }
            }
        }
        
        if (propertiesStart < 0) {
            // If we can't find properties, assume they start after basic item data
            // This is a fallback - in practice we should calculate the exact position
            propertiesStart = item->bitString.length() - 100; // Rough estimate
            if (propertiesStart < 0) propertiesStart = 0;
        }
        
        // Keep everything before properties
        QString newBitString = item->bitString.left(propertiesStart);
        
        // Build new properties section
        QString propertiesBits = buildPropertiesBitString(item->props);
        
        // Append the new properties
        newBitString += propertiesBits;
        
        // Byte align the result
        ReverseBitWriter::byteAlignBits(newBitString);
        
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
    
    // Get ordered list of properties
    QList<QPair<int, ItemProperty*>> orderedProps = getOrderedProperties(properties);
    
    // Write each property
    for (const auto &pair : orderedProps) {
        writePropertyToBits(bitString, pair.first, pair.second);
    }
    
    // Add end marker
    insertEndMarker(bitString);
    
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
    
    // Convert multimap to list
    for (auto it = properties.constBegin(); it != properties.constEnd(); ++it) {
        orderedList.append(QPair<int, ItemProperty*>(it.key(), it.value()));
    }
    
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