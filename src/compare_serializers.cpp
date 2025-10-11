#include "itemparser.h"
#include "helpers.h"
#include "itemdatabase.h"
#include "structs.h"
#include "propertymodificationserializer.h"
#include "reversebitreader.h"
#include "reversebitwriter.h"
#include "languagemanager.hpp"
#include "resourcepathmanager.hpp"

#include <QApplication>
#include <QFile>
#include <QBuffer>
#include <QDataStream>
#include <QDebug>

// COMPARISON TEST: Compare PropertyModificationSerializer vs our manual approach
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    LanguageManager &langManager = LanguageManager::instance();  
    langManager.currentLocale = "en";
    langManager.setResourcesPath(app.applicationDirPath() + "/../resources");

    QString inPath = "amu_2482595313.d2i";

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
    
    QString originalBitString = item->bitString;
    
    qDebug() << "\n=== COMPARISON TEST ===";
    qDebug() << "Original bitString first 50 chars:" << originalBitString.left(50);
    qDebug() << "Original bitString last 50 chars:" << originalBitString.right(50);
    
    // Test 1: PropertyModificationSerializer approach (known to work)
    PropertyModificationSerializer serializer;
    QByteArray propertyModBytes = serializer.serializeBitStringToBytes(originalBitString);
    
    qDebug() << "\n=== PropertyModificationSerializer ===";
    qDebug() << "Input bitString length:" << originalBitString.length();
    qDebug() << "Output bytes length:" << propertyModBytes.length();
    qDebug() << "First 10 bytes (hex):" << propertyModBytes.left(10).toHex();
    qDebug() << "Last 10 bytes (hex):" << propertyModBytes.right(10).toHex();
    
    // Test 2: Our manual approach
    int contentBits = originalBitString.length();
    int paddingNeeded = (8 - (contentBits % 8)) % 8;
    int totalBits = contentBits + paddingNeeded;
    int totalBytes = totalBits / 8;
    
    qDebug() << "\n=== Our Manual Approach ===";
    qDebug() << "Content bits:" << contentBits;
    qDebug() << "Padding needed:" << paddingNeeded;
    qDebug() << "Total bits:" << totalBits;
    qDebug() << "Total bytes:" << totalBytes;
    
    // Pad at the end (like PropertyModificationSerializer)
    QString paddedBitString = originalBitString + QString('0').repeated(paddingNeeded);
    qDebug() << "Padded bitString length:" << paddedBitString.length();
    
    // Manual conversion with chunk-prepend
    QByteArray manualBytes;
    manualBytes.resize(totalBytes);
    
    // Step 1: Convert to natural byte order
    QByteArray naturalBytes;
    for (int byteIndex = 0; byteIndex < totalBytes; byteIndex++) {
        quint8 byteValue = 0;
        for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
            int bitPos = byteIndex * 8 + bitIndex;
            if (bitPos < paddedBitString.length() && paddedBitString[bitPos] == '1') {
                byteValue |= (1 << (7 - bitIndex)); // MSB first within each byte
            }
        }
        naturalBytes.append(byteValue);
    }
    
    // Step 2: Apply chunk-prepend order
    for (int i = 0; i < totalBytes; i++) {
        manualBytes[i] = naturalBytes[totalBytes - 1 - i];
    }
    
    qDebug() << "Manual bytes length:" << manualBytes.length();
    qDebug() << "First 10 bytes (hex):" << manualBytes.left(10).toHex();
    qDebug() << "Last 10 bytes (hex):" << manualBytes.right(10).toHex();
    
    // Compare results
    qDebug() << "\n=== COMPARISON ===";
    qDebug() << "Lengths match:" << (propertyModBytes.length() == manualBytes.length());
    qDebug() << "Contents match:" << (propertyModBytes == manualBytes);
    
    if (propertyModBytes != manualBytes) {
        qDebug() << "MISMATCH DETAILS:";
        for (int i = 0; i < qMin(propertyModBytes.length(), manualBytes.length()); i++) {
            if (propertyModBytes[i] != manualBytes[i]) {
                qDebug() << "  Byte" << i << ": PropertyMod=" << QString::number((quint8)propertyModBytes[i], 16)
                         << "Manual=" << QString::number((quint8)manualBytes[i], 16);
                if (i > 10) break; // Don't flood output
            }
        }
    }
    
    // Test reconstruction
    QByteArray testFile1 = QByteArray("JM") + propertyModBytes;
    QByteArray testFile2 = QByteArray("JM") + manualBytes;
    
    // Write test files
    QFile f1("test_propertymod.d2i");
    f1.open(QIODevice::WriteOnly);
    f1.write(testFile1);
    f1.close();
    
    QFile f2("test_manual.d2i");
    f2.open(QIODevice::WriteOnly);
    f2.write(testFile2);
    f2.close();
    
    qDebug() << "Test files written: test_propertymod.d2i and test_manual.d2i";
    
    qDeleteAll(items);
    return 0;
}