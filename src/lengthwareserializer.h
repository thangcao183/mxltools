#ifndef LENGTHAWARE_SERIALIZER_H
#define LENGTHAWARE_SERIALIZER_H

#include <QString>
#include <QByteArray>

/**
 * LengthAwareSerializer - Advanced serializer designed to handle bitString length changes
 * 
 * Unlike PropertyModificationSerializer which uses chunk-prepend order and fails on length changes,
 * this serializer uses a different approach that maintains file structure integrity:
 * 
 * 1. Preserves exact byte alignment for itemType and other critical sections
 * 2. Handles variable-length bitString properly
 * 3. Uses sequential bit-to-byte conversion instead of chunk-prepend
 * 4. Maintains compatibility with ItemParser expectations
 */
class LengthAwareSerializer
{
public:
    LengthAwareSerializer();
    
    /**
     * Create d2i file from modified bitString with length changes support
     * @param bitString The modified bitString (can be different length than original)
     * @return Complete d2i file bytes ready for writing
     */
    QByteArray createD2iFile(const QString &bitString);
    
    /**
     * Convert bitString to bytes using sequential order (NOT chunk-prepend)
     * This maintains proper byte alignment for all sections
     * @param bitString Input bit string in LSB-first format
     * @return Byte array with proper bit-to-byte mapping
     */
    QByteArray convertBitStringToBytes(const QString &bitString);
    
    /**
     * Create file header (currently just standard 2-byte header)
     * @return File header bytes
     */
    QByteArray createFileHeader();
    
private:
    /**
     * Validate bitString format and content
     * @param bitString Input bit string to validate
     * @return true if bitString is valid
     */
    bool validateBitString(const QString &bitString);
    
    /**
     * Convert 8-bit chunk to byte using LSB-first order
     * @param bits 8-character bit string (LSB-first)
     * @return Converted byte value
     */
    quint8 convertBitsToByteSequential(const QString &bits);
};

#endif // LENGTHAWARE_SERIALIZER_H