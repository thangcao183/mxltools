#include "lengthwareserializer.h"
#include <QDebug>

LengthAwareSerializer::LengthAwareSerializer()
{
}

QByteArray LengthAwareSerializer::createD2iFile(const QString &bitString)
{
    qDebug() << "LengthAwareSerializer: Processing bitString with length" << bitString.length();
    
    if (!validateBitString(bitString)) {
        qDebug() << "LengthAwareSerializer: Invalid bitString format";
        return QByteArray();
    }
    
    // CRITICAL: bitString represents ITEM DATA ONLY (no header)
    // The header must be created separately and combined
    
    // Create file header (standard d2i header)
    QByteArray header = createFileHeader();
    qDebug() << "LengthAwareSerializer: Created header" << header.size() << "bytes:" << header.toHex(' ');
    
    // Convert bitString to item bytes using sequential method
    QByteArray itemBytes = convertBitStringToBytes(bitString);
    qDebug() << "LengthAwareSerializer: Converted bitString to" << itemBytes.size() << "bytes";
    qDebug() << "LengthAwareSerializer: First item bytes:" << itemBytes.left(8).toHex(' ');
    
    // Combine header + item
    QByteArray result = header + itemBytes;
    qDebug() << "LengthAwareSerializer: Final file size:" << result.size() << "bytes";
    qDebug() << "LengthAwareSerializer: Final first bytes:" << result.left(8).toHex(' ');
    
    return result;
}

QByteArray LengthAwareSerializer::convertBitStringToBytes(const QString &bitString)
{
    // Pad bitString to byte boundary
    int paddedLength = (bitString.length() + 7) / 8 * 8;
    QString paddedBits = bitString.leftJustified(paddedLength, '0');
    qDebug() << "LengthAwareSerializer: Padded" << bitString.length() << "bits to" << paddedLength << "bits";
    
    QByteArray result;
    
    // CRITICAL DIFFERENCE: Sequential conversion instead of chunk-prepend
    // This maintains proper byte alignment throughout the file
    for (int i = 0; i < paddedBits.length(); i += 8) {
        QString byteBits = paddedBits.mid(i, 8);
        quint8 byteValue = convertBitsToByteSequential(byteBits);
        result.append(byteValue);
        
        // Debug first few bytes to verify correct conversion
        if (i < 32) {
            qDebug() << "LengthAwareSerializer: Byte" << (i/8) << "bits:" << byteBits 
                     << "value:" << byteValue << "hex:" << QString::number(byteValue, 16);
        }
    }
    
    qDebug() << "LengthAwareSerializer: Sequential conversion completed," << result.size() << "bytes generated";
    return result;
}

QByteArray LengthAwareSerializer::createFileHeader()
{
    // Standard d2i header: "JM" (0x4A, 0x4D)
    QByteArray header;
    header.append(0x4A); // 'J'
    header.append(0x4D); // 'M'
    return header;
}

bool LengthAwareSerializer::validateBitString(const QString &bitString)
{
    if (bitString.isEmpty()) {
        qDebug() << "LengthAwareSerializer: Empty bitString";
        return false;
    }
    
    // Check that bitString contains only '0' and '1'
    for (QChar c : bitString) {
        if (c != '0' && c != '1') {
            qDebug() << "LengthAwareSerializer: Invalid character in bitString:" << c;
            return false;
        }
    }
    
    // Check minimum reasonable length (should have at least item header + some properties)
    if (bitString.length() < 200) {
        qDebug() << "LengthAwareSerializer: BitString too short:" << bitString.length();
        return false;
    }
    
    return true;
}

quint8 LengthAwareSerializer::convertBitsToByteSequential(const QString &bits)
{
    // Sequential LSB-first conversion
    // bits[0] = LSB (rightmost bit), bits[7] = MSB (leftmost bit)
    
    quint8 result = 0;
    for (int i = 0; i < qMin(8, bits.length()); i++) {
        if (bits[i] == '1') {
            result |= (1 << i);  // Set bit i (LSB-first)
        }
    }
    
    return result;
}