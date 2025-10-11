#ifndef PROPERTYMODIFICATIONENGINE_H
#define PROPERTYMODIFICATIONENGINE_H

#include "structs.h"
#include <QObject>
#include <QMap>
#include <QPair>

class ItemInfo;
class ReverseBitReader;
class ReverseBitWriter;

class PropertyModificationEngine : public QObject
{
    Q_OBJECT

public:
    explicit PropertyModificationEngine(QObject *parent = nullptr);
    
    // Main functions for property modification
    bool modifyItemProperties(ItemInfo *item, const PropertiesMultiMap &newProperties);
    bool reconstructItemBitString(ItemInfo *item);
    
    // Validation functions
    bool validateProperty(int propertyId, int value, quint32 parameter, QString *error = nullptr);
    bool validatePropertyCombination(const PropertiesMultiMap &properties, QString *error = nullptr);
    
    // Error handling
    QString lastError() const { return _lastError; }
    
    // Utility functions
    QList<int> getDependentProperties(int propertyId) const;
    bool isSpecialProperty(int propertyId) const;
    int calculateBitLength(int propertyId, int value, quint32 parameter) const;
    
signals:
    void progressChanged(int percentage);
    void statusChanged(const QString &status);
    void errorOccurred(const QString &error);

private:
    // Core bit manipulation
    QString buildPropertiesBitString(const PropertiesMultiMap &properties);
    void writePropertyToBits(QString &bitString, int propertyId, const ItemProperty *property);
    
    // Special property handlers
    void writeEnhancedDamageProperty(QString &bitString, int value);
    void writeElementalDamageProperty(QString &bitString, int baseId, int minValue, int maxValue, int length = 0);
    void writeSkillProperty(QString &bitString, int propertyId, int value, quint32 skillId);
    void writeBooleanProperty(QString &bitString, int propertyId, bool value);
    
    // Validation helpers
    bool validateEnhancedDamage(int value, QString *error) const;
    bool validateElementalDamage(int propertyId, int value, QString *error) const;
    bool validateSkillProperty(int propertyId, int value, quint32 skillId, QString *error) const;
    bool validateDefense(int value, QString *error) const;
    
    // Bit manipulation utilities
    void appendBits(QString &bitString, quint64 value, int bitCount);
    void insertPropertyId(QString &bitString, int propertyId);
    void insertEndMarker(QString &bitString);
    
    // Item structure parsing utilities - reuse ItemParser logic
    int calculatePropertiesStartPosition(ItemInfo *item);
    void skipItemBasicData(class ReverseBitReader &bitReader, ItemInfo *item);
    // In-place property insertion helper (safe, verifies via round-trip)
    bool insertPropertyInPlace(ItemInfo *item, int propertyId, const ItemProperty *property);
    
    // Options B and C: Advanced reconstruction methods
    bool tryChunkAwareInsertion(ItemInfo *item, const QString &beforeProperties, const QString &propertiesBits, const QString &afterProperties);
    bool tryCompleteItemRecreation(ItemInfo *item);
    
    // Option E: Smart bit-level reconstruction with corruption prevention
    bool trySmartBitReconstruction(ItemInfo *item, const QString &beforeProperties, const QString &propertiesBits, const QString &afterProperties);
    
    // Option F: Property replacement (modify existing property instead of adding new one)
    bool tryPropertyReplacement(ItemInfo *item, int newPropertyId, int newValue);
    
    // Property ordering and dependencies
    QList<QPair<int, ItemProperty*>> getOrderedProperties(const PropertiesMultiMap &properties);
    void handlePropertyDependencies(const PropertiesMultiMap &properties, QList<QPair<int, ItemProperty*>> &orderedList);
    
    // Error handling
    void setError(const QString &error);
    QString _lastError;
};

#endif // PROPERTYMODIFICATIONENGINE_H