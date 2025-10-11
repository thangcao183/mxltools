#include "itemparser.h"
#include "helpers.h"
#include "itemdatabase.h"
#include "structs.h"
#include "reversebitreader.h"
#include "reversebitwriter.h"
#include "languagemanager.hpp"
#include "resourcepathmanager.hpp"

#include <QApplication>
#include <QFile>
#include <QBuffer>
#include <QDataStream>
#include <QDebug>

// DIRECT PROPERTY ADDITION: Direct byte manipulation, NOT using PropertyModificationSerializer
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    LanguageManager &langManager = LanguageManager::instance();  
    langManager.currentLocale = "en";
    langManager.setResourcesPath(app.applicationDirPath() + "/../resources");

    if (argc < 4) {
        qDebug() << "Usage: direct_property_adder <input.d2i> <propId> <value> [output.d2i]";
        return 2;
    }

    QString inPath = argv[1];
    int propId = QString(argv[2]).toInt();
    int value = QString(argv[3]).toInt();
    QString outPath = (argc >= 5) ? argv[4] : "direct_added.d2i";

    QFile f(inPath);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open" << inPath;
        return 3;
    }
    QByteArray inputData = f.readAll();
    f.close();

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
    qDebug() << "Item type:" << item->itemType << "bitString length:" << item->bitString.length();
    qDebug() << "Current properties count:" << item->props.size();

    // Check if property already exists
    auto propIt = item->props.find(propId);
    if (propIt != item->props.end()) {
        qDebug() << "Property" << propId << "already exists with value:" << propIt.value()->value;
        qDebug() << "Use modification tool instead of addition tool";
        return 8;
    }

    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propId);
    if (!propTxt) {
        qDebug() << "Property" << propId << "not found in database";
        return 6;  
    }

    qDebug() << "Adding new property" << propId << "with value:" << value;
    qDebug() << "Property definition: add=" << propTxt->add << "bits=" << propTxt->bits 
             << "paramBits=" << propTxt->paramBits;

    // Use exact known working value from successful property modification
    int propertiesStart = 232;
    qDebug() << "Properties section starts at bit:" << propertiesStart;

    // Find end marker position
    ReverseBitReader endMarkerReader(item->bitString);
    endMarkerReader.setPos(propertiesStart);
    
    int endMarkerPos = -1;
    try {
        int propCount = 0;
        while (endMarkerReader.pos() + 9 <= item->bitString.length() && propCount < 50) {
            int currentPos = endMarkerReader.pos();
            int readId = endMarkerReader.readNumber(9);
            
            if (readId == 511) {
                endMarkerPos = currentPos;
                qDebug() << "Found end marker at position" << endMarkerPos;
                break;
            }
            
            // Skip this property's param and value bits
            if (readId >= 0 && readId < 512) {
                ItemPropertyTxt *currentPropTxt = ItemDataBase::Properties()->value(readId);
                if (currentPropTxt) {
                    endMarkerReader.skip(currentPropTxt->paramBits + currentPropTxt->bits);
                }
            }
            propCount++;
        }
    } catch (...) {
        qDebug() << "Exception while finding end marker";
    }
    
    if (endMarkerPos < 0) {
        qDebug() << "Could not find properties end marker";
        return 7;
    }

    // Build new property in exact same format as existing properties
    int adjustedValue = value + propTxt->add;
    
    // Property ID in LSB-first binary  
    QString propIdBits = QString::number(propId, 2).rightJustified(9, '0');
    std::reverse(propIdBits.begin(), propIdBits.end());
    
    // Parameter bits (if any)
    QString paramBits;
    if (propTxt->paramBits > 0) {
        paramBits = QString::number(0, 2).rightJustified(propTxt->paramBits, '0'); // Default param = 0
        std::reverse(paramBits.begin(), paramBits.end());
    }
    
    // Value bits in LSB-first
    QString valueBits = QString::number(adjustedValue, 2).rightJustified(propTxt->bits, '0');
    std::reverse(valueBits.begin(), valueBits.end());
    
    QString newPropertyBits = propIdBits + paramBits + valueBits;
    
    qDebug() << "New property bit string (LSB-first format):";
    qDebug() << "  - propId:" << propId << "bits:" << propIdBits;
    qDebug() << "  - param: 0 bits:" << paramBits;
    qDebug() << "  - value:" << value << "adjusted:" << adjustedValue << "bits:" << valueBits;
    qDebug() << "  - complete:" << newPropertyBits << "(" << newPropertyBits.length() << "bits)";

    // Replace end marker with [new property + end marker]
    QString modifiedBitString = item->bitString;
    QString endMarkerBits = "111111111"; // End marker (511 in LSB-first)
    
    // Replace end marker at endMarkerPos with: newPropertyBits + endMarkerBits
    QString replacementBits = newPropertyBits + endMarkerBits;
    
    // Remove old end marker (9 bits) and insert replacement
    modifiedBitString.remove(endMarkerPos, 9);
    modifiedBitString.insert(endMarkerPos, replacementBits);
    
    qDebug() << "Replaced end marker at position" << endMarkerPos;
    qDebug() << "BitString length: original=" << item->bitString.length() 
             << "modified=" << modifiedBitString.length() 
             << "difference=" << (modifiedBitString.length() - item->bitString.length());

    // Now we need to create the D2I file manually with proper padding
    // Instead of using PropertyModificationSerializer (which corrupts data)
    
    // Calculate padding needed for byte alignment
    int contentBits = modifiedBitString.length();
    int paddingNeeded = (8 - (contentBits % 8)) % 8;
    int totalBits = contentBits + paddingNeeded;
    int totalBytes = totalBits / 8;
    
    qDebug() << "Direct byte creation:";
    qDebug() << "  - Content bits:" << contentBits;
    qDebug() << "  - Padding needed:" << paddingNeeded;
    qDebug() << "  - Total bits:" << totalBits;
    qDebug() << "  - Total bytes:" << totalBytes;
    
    // Pad the bitString at the END (matching PropertyModificationSerializer behavior)
    QString paddedBitString = modifiedBitString + QString('0').repeated(paddingNeeded);
    
    // Convert padded bitString to bytes using ItemParser's CHUNK-PREPEND logic
    // CRITICAL: ItemParser uses chunk-prepend order, NOT simple MSB conversion!
    
    QByteArray itemBytes; // Start with EMPTY byte array, NO pre-allocation!
    
    // Use EXACT ItemParser::writeItems logic:
    // for (int i = 0, n = item->bitString.length(); i < n; i += 8)
    //     itemBytes.prepend(item->bitString.mid(i, 8).toShort(0, 2));
    
    qDebug() << "Converting padded bitString to bytes:";
    qDebug() << "  - Padded bitString length:" << paddedBitString.length();
    qDebug() << "  - Expected chunks:" << (paddedBitString.length() / 8);
    
    for (int i = 0, n = paddedBitString.length(); i < n; i += 8) {
        QString chunk = paddedBitString.mid(i, 8);
        
        // Convert 8-bit binary string to byte value
        bool ok = false;
        quint8 byteValue = static_cast<quint8>(chunk.toShort(&ok, 2));
        
        if (ok) {
            itemBytes.prepend(static_cast<char>(byteValue));
            if (i < 32) { // Debug first few chunks
                qDebug() << "  - Chunk" << (i/8) << ":" << chunk << "→" << byteValue << "hex:" << QString::number(byteValue, 16);
                qDebug() << "    Current itemBytes length:" << itemBytes.length() << "hex:" << itemBytes.toHex();
            }
        } else {
            qDebug() << "ERROR: Invalid bit chunk at position" << i << ":" << chunk;
            break;
        }
    }
    
    qDebug() << "Final itemBytes:";
    qDebug() << "  - Length:" << itemBytes.length();
    qDebug() << "  - First 20 bytes (hex):" << itemBytes.left(20).toHex();
    qDebug() << "  - Last 10 bytes (hex):" << itemBytes.right(10).toHex();
    
    // Create final D2I file: JM header + item bytes
    QByteArray outData;
    outData.append("JM"); // Header
    outData.append(itemBytes);
    
    qDebug() << "Direct creation result:";
    qDebug() << "  - Item bytes:" << totalBytes;
    qDebug() << "  - Total file size:" << outData.length() << "bytes";
    qDebug() << "  - Expected size change:" << (outData.length() - inputData.length()) << "bytes";

    QFile outFile(outPath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to create output file" << outPath;
        return 9;
    }
    outFile.write(outData);
    outFile.close();

    qDebug() << "Successfully wrote" << outPath << "using direct property addition approach";

    // Verification
    QFile verifyFile(outPath);
    if (!verifyFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open for verification" << outPath;
        return 10;
    }
    QByteArray verifyData = verifyFile.readAll();
    verifyFile.close();

    QBuffer verifyBuffer(&verifyData);
    verifyBuffer.open(QIODevice::ReadOnly);
    QDataStream verifyStream(&verifyBuffer);
    verifyStream.setByteOrder(QDataStream::LittleEndian);

    ItemsList verifyItems;
    QString verifyCorrupted = ItemParser::parseItemsToBuffer(1, verifyStream, verifyData, QString(), &verifyItems);
    
    if (verifyItems.isEmpty()) {
        qDebug() << "Verification failed: Could not parse result file";
        return 11;
    }

    ItemInfo *verifyItem = verifyItems.first();
    qDebug() << "Verification: itemType:" << verifyItem->itemType;
    qDebug() << "Verification: bitString length:" << verifyItem->bitString.length();
    qDebug() << "Verification: properties count:" << verifyItem->props.size();
    
    auto newPropIt = verifyItem->props.find(propId);
    if (newPropIt != verifyItem->props.end()) {
        qDebug() << "SUCCESS: New property" << propId << "added with value:" << newPropIt.value()->value;
        
        // Clean up
        qDeleteAll(verifyItems);
        qDeleteAll(items);
        return 0;
    } else {
        qDebug() << "WARNING: Property" << propId << "not found in verification";
        
        // Print all found properties for debugging
        qDebug() << "Found properties:";
        for (auto it = verifyItem->props.begin(); it != verifyItem->props.end(); ++it) {
            qDebug() << "  - Property" << it.key() << "=" << it.value()->value;
        }
        
        qDeleteAll(verifyItems);
        qDeleteAll(items);
        return 12;
    }
}