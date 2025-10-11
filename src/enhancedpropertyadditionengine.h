#ifndef ENHANCEDPROPERTYADDITIONENGINE_H
#define ENHANCEDPROPERTYADDITIONENGINE_H

#include "structs.h"
#include <QObject>
#include <QMap>
#include <QString>
#include <QVector>

class ItemInfo;

/**
 * Enhanced Property Addition Engine using LSB-first bit encoding
 * 
 * This engine correctly handles D2I file structure with proper bit encoding:
 * - LSB-first encoding within each byte (bit 0 first, bit 7 last)
 * - File structure: [content][end marker][padding]
 * - Direct byte manipulation without chunk-prepend corruption
 * 
 * Based on breakthrough research from property_adder_logic.cpp
 */
class EnhancedPropertyAdditionEngine : public QObject
{
    Q_OBJECT

public:
    struct PropertySpec {
        int id;
        int add;        // Add value from props.tsv
        int bits;       // Number of bits for value encoding
        int paramBits;  // Number of bits for parameter (usually 0)
        QString name;   // Property display name
    };

    explicit EnhancedPropertyAdditionEngine(QObject *parent = nullptr);
    
    // Main property addition function
    bool addPropertyToItem(ItemInfo *item, int propertyId, int value, quint32 parameter = 0);
    
    // Batch property addition
    bool addPropertiesToItem(ItemInfo *item, const QMap<int, QPair<int, quint32>> &properties);
    
    // Validation functions
    bool validateProperty(int propertyId, int value, QString *error = nullptr);
    bool isPropertySupported(int propertyId) const;
    PropertySpec getPropertySpec(int propertyId) const;
    
    // Property database functions
    void loadPropertyDatabase(const QString &propsPath = QString());
    QVector<PropertySpec> getSupportedProperties() const;
    
    // Error handling
    QString lastError() const { return _lastError; }
    bool hasError() const { return !_lastError.isEmpty(); }
    void clearError() { _lastError.clear(); }

signals:
    void statusChanged(const QString &status);
    void errorOccurred(const QString &error);
    void propertyAdded(int propertyId, const QString &propertyName, int value);

private:
    // Core bit manipulation functions (LSB-first)
    QString byteToBinary_LSB(uint8_t byte) const;
    uint8_t binaryToByte_LSB(const QString &binary) const;
    QString numberToBinary(int number, int bits) const;
    int binaryToNumber(const QString &binary) const;
    
    // D2I file structure manipulation  
    bool parseFileStructure(const QString &bitString, QString &content, QString &padding, int &endMarkerPos);
    QString createPropertyBits(int propertyId, int value, quint32 parameter = 0);
    QString calculateNewPadding(int totalContentBits) const;
    QString rebuildBitString(const QString &content, const QString &newPropertyBits, const QString &endMarker = "111111111");
    
    // File I/O and conversion
    QByteArray bitStringToBytes(const QString &bitString) const;
    QString bytesToBitString(const QByteArray &bytes) const;
    
    // Standalone item processing (new smart approach)
    QByteArray createStandaloneItemFile(ItemInfo *item) const;
    QByteArray addPropertyToStandaloneItem(const QByteArray &itemData, int propertyId, int value, quint32 parameter) const;
    bool updateItemFromStandaloneData(ItemInfo *item, const QByteArray &modifiedData);
    
    // Validation helpers
    bool validateBitStringIntegrity(const QString &bitString) const;
    bool validateFileStructure(ItemInfo *item) const;
    
    // Property database
    void initializeDefaultProperties();
    QMap<int, PropertySpec> _propertyDatabase;
    
    // Error handling
    void setError(const QString &error);
    QString _lastError;
    
    // Constants
    static const QString END_MARKER;
    static const QByteArray ITEM_HEADER;
};

#endif // ENHANCEDPROPERTYADDITIONENGINE_H