#ifndef PROPERTYMODIFICATIONSERIALIZER_H
#define PROPERTYMODIFICATIONSERIALIZER_H

#include <QString>
#include <QByteArray>
#include <QDataStream>
#include "structs.h"

class PropertyModificationSerializer
{
public:
    PropertyModificationSerializer();
    
    // Convert bitString directly to bytes WITHOUT using ItemParser::writeItems
    static QByteArray serializeBitStringToBytes(const QString &bitString);
    
    // Create complete .d2i file from bitString
    static QByteArray createD2iFile(const QString &bitString);
    
    // Write item to stream using direct serialization (bypass ItemParser::writeItems)
    static bool writeItemDirect(ItemInfo *item, QDataStream &stream);
    
    // Convert bytes back to bitString for verification
    static QString deserializeBytesToBitString(const QByteArray &bytes);
    
private:
    // Helper: convert 8-bit chunks to bytes in FORWARD order (not prepend)
    static QByteArray bitsToBytes(const QString &bits);
    
    // Helper: pad bitString to byte boundary
    static QString padToByteBoundary(const QString &bits);
};

#endif // PROPERTYMODIFICATIONSERIALIZER_H