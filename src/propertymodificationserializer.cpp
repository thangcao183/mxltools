#include "propertymodificationserializer.h"
#include "itemparser.h"
#include <QDebug>

PropertyModificationSerializer::PropertyModificationSerializer()
{
}

QByteArray PropertyModificationSerializer::serializeBitStringToBytes(const QString &bitString)
{
    // Pad to byte boundary
    QString paddedBits = padToByteBoundary(bitString);
    
    qDebug() << "PropertyModificationSerializer: Converting bitString length" 
             << bitString.length() << "padded to" << paddedBits.length();
    
    // Convert to bytes using FORWARD order (not ItemParser's prepend approach)
    return bitsToBytes(paddedBits);
}

QByteArray PropertyModificationSerializer::createD2iFile(const QString &bitString)
{
    // Create complete .d2i file with "JM" header + item bytes
    QByteArray itemBytes = serializeBitStringToBytes(bitString);
    QByteArray fullFile = ItemParser::kItemHeader + itemBytes;
    
    qDebug() << "PropertyModificationSerializer: Created .d2i file:"
             << "header" << ItemParser::kItemHeader.length() << "bytes"
             << "item" << itemBytes.length() << "bytes"
             << "total" << fullFile.length() << "bytes";
    
    return fullFile;
}

bool PropertyModificationSerializer::writeItemDirect(ItemInfo *item, QDataStream &stream)
{
    if (!item) return false;
    
    // Write header
    stream.writeRawData(ItemParser::kItemHeader.data(), ItemParser::kItemHeader.length());
    
    // Convert bitString to bytes using our direct approach
    QByteArray itemBytes = serializeBitStringToBytes(item->bitString);
    
    // Write item bytes
    stream.writeRawData(itemBytes.data(), itemBytes.length());
    
    qDebug() << "PropertyModificationSerializer: Wrote item directly:"
             << "bitString" << item->bitString.length() << "bits"
             << "itemBytes" << itemBytes.length() << "bytes";
    
    return true;
}

QString PropertyModificationSerializer::deserializeBytesToBitString(const QByteArray &bytes)
{
    QString bitString;
    
    // Convert each byte to 8-bit string in FORWARD order
    for (int i = 0; i < bytes.length(); ++i) {
        quint8 byte = static_cast<quint8>(bytes.at(i));
        QString byteBits = QString::number(byte, 2).rightJustified(8, '0');
        bitString += byteBits;
    }
    
    qDebug() << "PropertyModificationSerializer: Deserialized" 
             << bytes.length() << "bytes to" << bitString.length() << "bits";
    
    return bitString;
}

QByteArray PropertyModificationSerializer::bitsToBytes(const QString &bits)
{
    QByteArray bytes;
    
    // CRITICAL UNDERSTANDING: ItemParser uses chunk-based approach where bitString is processed 
    // in 8-bit chunks, then each chunk is PREPENDED to build the final byte array.
    // This means: bitString[0-7] becomes bytes[N-1], bitString[8-15] becomes bytes[N-2], etc.
    // We must replicate this exact behavior to avoid corruption.
    
    // Process bits in 8-bit chunks from LEFT to RIGHT
    for (int i = 0; i < bits.length(); i += 8) {
        QString chunk = bits.mid(i, 8);
        
        // Pad last chunk if needed
        if (chunk.length() < 8) {
            chunk = chunk.leftJustified(8, '0');
        }
        
        // Convert chunk to byte
        bool ok = false;
        quint8 byteValue = static_cast<quint8>(chunk.toInt(&ok, 2));
        
        if (ok) {
            // PREPEND each chunk to match ItemParser::writeItems behavior exactly
            // This preserves the chunk mapping that ItemParser expects during reading
            bytes.prepend(static_cast<char>(byteValue));
        } else {
            qDebug() << "PropertyModificationSerializer: ERROR - Invalid bit chunk:" << chunk;
            break;
        }
    }
    
    qDebug() << "PropertyModificationSerializer: Converted" << bits.length() 
             << "bits to" << bytes.length() << "bytes (chunk-prepend order matching ItemParser)";
    
    return bytes;
}

QString PropertyModificationSerializer::padToByteBoundary(const QString &bits)
{
    int remainder = bits.length() % 8;
    if (remainder == 0) {
        return bits; // Already byte-aligned
    }
    
    // Pad with zeros at the END (not at the beginning)
    int padAmount = 8 - remainder;
    QString paddedBits = bits + QString(padAmount, '0');
    
    qDebug() << "PropertyModificationSerializer: Padded" << bits.length()
             << "bits with" << padAmount << "zeros to" << paddedBits.length() << "bits";
    
    return paddedBits;
}