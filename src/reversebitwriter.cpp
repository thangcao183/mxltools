#include "reversebitwriter.h"
#include "helpers.h"
#include "structs.h"

#include <QString>


QString &ReverseBitWriter::replaceValueInBitString(QString &bitString, int offset, int newValue, int length /*= -1*/)
{
    if (length == -1)
        length = Enums::ItemOffsets::offsetLength(offset);
    return bitString.replace(startOffset(bitString, offset, length), length, binaryStringFromNumber(newValue, false, length));
}

QString &ReverseBitWriter::updateItemRow(ItemInfo *item)
{
    return replaceValueInBitString(item->bitString, Enums::ItemOffsets::Row, item->row);
}

QString &ReverseBitWriter::updateItemColumn(ItemInfo *item)
{
    return replaceValueInBitString(item->bitString, Enums::ItemOffsets::Column, item->column);
}

QString &ReverseBitWriter::remove(QString &bitString, int offsetWithoutJM, int length)
{
    return bitString.remove(startOffset(bitString, offsetWithoutJM, length, false) - 1, length);
}

QString &ReverseBitWriter::insert(QString &bitString, int offsetWithoutJM, const QString &bitStringToInsert)
{
    return bitString.insert(startOffset(bitString, offsetWithoutJM, 0, false), bitStringToInsert);
}

QString &ReverseBitWriter::byteAlignBits(QString &bitString)
{
    // Safer byte alignment: append zeros at the END of the bit string to make
    // its length a multiple of 8. The previous implementation could remove or
    // prepend bits which corrupts item headers. Appending zeros preserves all
    // existing bits and is compatible with how ItemParser converts bits->bytes.
    const int kBitsInByte = 8;
    int extraBits = bitString.length() % kBitsInByte;
    if (extraBits)
    {
        int zerosToAppend = kBitsInByte - extraBits;
        bitString.append(QString(zerosToAppend, '0'));
    }
    return bitString;
}


int ReverseBitWriter::startOffset(const QString &bitString, int offset, int length, bool isItemHeaderSkipped/* = true*/)
{
    // 16 is 'JM' offset which is not stored in the bitString
    return bitString.length() - (offset - 16 * isItemHeaderSkipped) - length;
}
