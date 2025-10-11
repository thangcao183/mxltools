#include "itemparser.h"
#include "helpers.h"
#include "itemdatabase.h"
#include "structs.h"
#include "reversebitreader.h"
#include "languagemanager.hpp"
#include "resourcepathmanager.hpp"

#include <QApplication>
#include <QFile>
#include <QBuffer>
#include <QDataStream>
#include <QDebug>

// RESEARCH TOOL: Study d2i file structure and property storage
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    LanguageManager &langManager = LanguageManager::instance();  
    langManager.currentLocale = "en";
    langManager.setResourcesPath(app.applicationDirPath() + "/../resources");

    if (argc < 2) {
        qDebug() << "Usage: research_d2i_structure <input.d2i>";
        return 2;
    }

    QString inPath = argv[1];

    QFile f(inPath);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open" << inPath;
        return 3;
    }
    QByteArray inputData = f.readAll();
    f.close();

    qDebug() << "=== FILE ANALYSIS ===";
    qDebug() << "File size:" << inputData.size() << "bytes";
    qDebug() << "Header (first 2 bytes):" << inputData.left(2).toHex(' ');
    qDebug() << "Item data size:" << (inputData.size() - 2) << "bytes";
    
    // Show raw bytes for analysis
    qDebug() << "\n=== RAW BYTES (first 20) ===";
    for (int i = 0; i < qMin(20, inputData.size()); i++) {
        qDebug() << "Byte" << i << ":" << QString("0x%1").arg((quint8)inputData[i], 2, 16, QChar('0')) 
                 << "=" << (quint8)inputData[i] << "binary:" << QString::number((quint8)inputData[i], 2).rightJustified(8, '0');
    }

    QBuffer buffer(&inputData);
    buffer.open(QIODevice::ReadOnly);
    QDataStream ds(&buffer);
    ds.setByteOrder(QDataStream::LittleEndian);

    ItemsList items;
    QString corrupted = ItemParser::parseItemsToBuffer(1, ds, inputData, QString(), &items);
    if (items.isEmpty()) {
        qDebug() << "Failed to parse item from" << inPath;
        return 4;
    }

    ItemInfo *item = items.first();
    
    qDebug() << "\n=== PARSED ITEM INFO ===";
    qDebug() << "Item type:" << item->itemType;
    qDebug() << "BitString length:" << item->bitString.length() << "bits";
    qDebug() << "BitString in bytes:" << (item->bitString.length() + 7) / 8 << "bytes";
    qDebug() << "Properties count:" << item->props.size();
    qDebug() << "Location:" << item->location << "Row:" << item->row << "Col:" << item->column;
    qDebug() << "Is Extended:" << item->isExtended;
    qDebug() << "Is Identified:" << item->isIdentified;
    qDebug() << "Quality:" << item->quality;

    qDebug() << "\n=== BITSTRING STRUCTURE ===";
    qDebug() << "Full bitString:";
    qDebug() << item->bitString;
    
    // Break down bitString into sections
    qDebug() << "\n=== BITSTRING SECTIONS ===";
    qDebug() << "First 32 bits (header area):" << item->bitString.left(32);
    qDebug() << "Bits 32-64:" << item->bitString.mid(32, 32);
    qDebug() << "Bits 64-96:" << item->bitString.mid(64, 32);
    qDebug() << "Bits 200-232 (before properties):" << item->bitString.mid(200, 32);
    qDebug() << "Bits 232-264 (properties start):" << item->bitString.mid(232, 32);

    qDebug() << "\n=== PROPERTIES ANALYSIS ===";
    int propertiesStart = 232;
    ReverseBitReader reader(item->bitString);
    reader.setPos(propertiesStart);
    
    qDebug() << "Reading properties from bit position" << propertiesStart << ":";
    
    try {
        int propCount = 0;
        while (reader.pos() + 9 <= item->bitString.length() && propCount < 20) {
            int startPos = reader.pos();
            int propId = reader.readNumber(9);
            
            if (propId == 511) {
                qDebug() << "Property" << propCount << ": END MARKER at bit" << startPos;
                break;
            }
            
            ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propId);
            if (!propTxt) {
                qDebug() << "Property" << propCount << ": UNKNOWN ID" << propId << "at bit" << startPos;
                break;
            }
            
            int paramValue = 0;
            if (propTxt->paramBits > 0) {
                paramValue = reader.readNumber(propTxt->paramBits);
            }
            
            int rawValue = reader.readNumber(propTxt->bits);
            int finalValue = rawValue - propTxt->add;
            int endPos = reader.pos();
            
            qDebug() << "Property" << propCount << ": ID=" << propId 
                     << "start=" << startPos << "end=" << endPos << "size=" << (endPos - startPos) << "bits"
                     << "param=" << paramValue << "raw=" << rawValue << "final=" << finalValue;
            
            // Show the exact bits for this property
            QString propBits = item->bitString.mid(startPos, endPos - startPos);
            qDebug() << "  Bits:" << propBits;
            
            propCount++;
        }
    } catch (...) {
        qDebug() << "Exception while analyzing properties";
    }

    qDebug() << "\n=== PROPERTIES MAP ===";
    QMapIterator<int, ItemProperty*> it(item->props);
    while (it.hasNext()) {
        it.next();
        ItemProperty *prop = it.value();
        qDebug() << "Property ID=" << it.key() 
                 << "value=" << prop->value 
                 << "param=" << prop->param
                 << "bitStringOffset=" << prop->bitStringOffset;
    }

    qDebug() << "\n=== ADDITION SPACE ANALYSIS ===";
    
    // Calculate total bits used by existing properties
    int totalPropBits = 0;
    QMapIterator<int, ItemProperty*> bitIt(item->props);
    while (bitIt.hasNext()) {
        bitIt.next();
        int propId = bitIt.key();
        ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propId);
        if (propTxt) {
            int propSize = 9 + propTxt->paramBits + propTxt->bits; // ID + param + value
            totalPropBits += propSize;
            qDebug() << "Property" << propId << "uses" << propSize << "bits";
        }
    }
    totalPropBits += 9; // End marker
    
    qDebug() << "Total properties bits:" << totalPropBits;
    qDebug() << "Properties start at bit:" << propertiesStart;
    qDebug() << "Properties should end at bit:" << (propertiesStart + totalPropBits);
    qDebug() << "Actual bitString ends at bit:" << item->bitString.length();
    qDebug() << "Available space:" << (item->bitString.length() - propertiesStart - totalPropBits) << "bits";

    qDebug() << "\n=== NEW PROPERTY REQUIREMENTS ===";
    int newPropId = 7;
    ItemPropertyTxt *newPropTxt = ItemDataBase::Properties()->value(newPropId);
    if (newPropTxt) {
        int requiredBits = 9 + newPropTxt->paramBits + newPropTxt->bits;
        qDebug() << "Property" << newPropId << "would need:" << requiredBits << "bits";
        qDebug() << "  - ID: 9 bits";
        qDebug() << "  - Param:" << newPropTxt->paramBits << "bits";
        qDebug() << "  - Value:" << newPropTxt->bits << "bits";
        qDebug() << "  - Add value:" << newPropTxt->add;
        
        // Check if we have space
        int availableSpace = item->bitString.length() - propertiesStart - totalPropBits;
        if (requiredBits <= availableSpace) {
            qDebug() << "GOOD: We have" << availableSpace << "bits available," << requiredBits << "bits needed";
        } else {
            qDebug() << "PROBLEM: We need" << requiredBits << "bits but only have" << availableSpace << "available";
            qDebug() << "Need to extend bitString by" << (requiredBits - availableSpace) << "bits";
        }
    }

    qDebug() << "\n=== PADDING BITS ANALYSIS ===";
    
    // Calculate where meaningful content ends
    int meaningfulBits = propertiesStart + totalPropBits;
    int totalBits = item->bitString.length();
    int paddingBits = totalBits - meaningfulBits;
    
    qDebug() << "Meaningful content ends at bit:" << meaningfulBits;
    qDebug() << "Total bitString length:" << totalBits << "bits";
    qDebug() << "Padding bits:" << paddingBits << "bits";
    
    if (paddingBits > 0) {
        QString paddingPattern = item->bitString.left(paddingBits);
        qDebug() << "Padding pattern:" << paddingPattern;
        
        // Verify padding is all zeros at the start
        bool allZeros = true;
        for (QChar c : paddingPattern) {
            if (c != '0') {
                allZeros = false;
                break;
            }
        }
        qDebug() << "Padding is all zeros:" << (allZeros ? "YES" : "NO");
    }
    
    // Byte alignment check
    int remainder = totalBits % 8;
    qDebug() << "Byte alignment: totalBits % 8 =" << remainder;
    if (remainder == 0) {
        qDebug() << "File is perfectly byte-aligned";
    } else {
        qDebug() << "File needs" << (8 - remainder) << "more bits for byte alignment";
    }
    
    // Show how padding works for property addition
    qDebug() << "\n=== PADDING FOR PROPERTY ADDITION ===";
    if (newPropTxt) {
        int requiredBits = 9 + newPropTxt->paramBits + newPropTxt->bits;
        int newTotalBits = meaningfulBits + requiredBits;
        int newPaddingNeeded = (8 - (newTotalBits % 8)) % 8;
        int newFileBits = newTotalBits + newPaddingNeeded;
        int newFileSize = (newFileBits / 8) + 2; // +2 for JM header
        
        qDebug() << "After adding property" << newPropId << ":";
        qDebug() << "  - New content bits:" << newTotalBits;
        qDebug() << "  - New padding needed:" << newPaddingNeeded << "bits";
        qDebug() << "  - New total bits:" << newFileBits;
        qDebug() << "  - New file size:" << newFileSize << "bytes";
        qDebug() << "  - Size increase:" << (newFileSize - inputData.size()) << "bytes";
        
        if (newPaddingNeeded > 0) {
            QString newPaddingPattern = QString("0").repeated(newPaddingNeeded);
            qDebug() << "  - New padding pattern:" << newPaddingPattern;
        }
    }

    qDebug() << "\n=== SUCCESSFUL MODIFICATION COMPARISON ===";
    qDebug() << "Property modification works because:";
    qDebug() << "  - Same bitString length (448 bits)";
    qDebug() << "  - Same file size (58 bytes)";
    qDebug() << "  - Only changes bit values, not structure";
    qDebug() << "  - Padding remains unchanged";
    qDebug() << "Property addition challenge:";
    qDebug() << "  - Changes bitString length (448 -> new length)";
    qDebug() << "  - Changes file size (58 -> new size)";  
    qDebug() << "  - Requires new padding calculation";
    qDebug() << "  - PropertyModificationSerializer can't handle length changes";
    qDebug() << "Solution: Direct byte manipulation with proper padding";

    // Clean up
    qDeleteAll(items);
    return 0;
}