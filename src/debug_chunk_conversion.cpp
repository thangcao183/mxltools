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

// DEBUG CHUNK CONVERSION: Compare different approaches
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    LanguageManager &langManager = LanguageManager::instance();  
    langManager.currentLocale = "en";
    langManager.setResourcesPath(app.applicationDirPath() + "/../resources");

    QString testBitString = "00011111111100010111101100000000"; // First 32 bits for testing
    
    qDebug() << "Test bitString:" << testBitString;
    qDebug() << "Length:" << testBitString.length() << "bits";
    
    // Method 1: ItemParser exact logic (our current approach)
    qDebug() << "\n=== METHOD 1: ItemParser exact logic ===";
    QByteArray method1Bytes;
    for (int i = 0, n = testBitString.length(); i < n; i += 8) {
        QString chunk = testBitString.mid(i, 8);
        qDebug() << "Chunk at" << i << ":" << chunk;
        
        bool ok = false;
        quint8 byteValue = static_cast<quint8>(chunk.toShort(&ok, 2));
        qDebug() << "  Converted to byte:" << byteValue << "hex:" << QString::number(byteValue, 16);
        
        if (ok) {
            method1Bytes.prepend(static_cast<char>(byteValue));
            qDebug() << "  Current bytes (hex):" << method1Bytes.toHex();
        }
    }
    qDebug() << "Final method1 bytes:" << method1Bytes.toHex();
    qDebug() << "Final method1 length:" << method1Bytes.length();
    
    // Method 2: Natural order then reverse (our old approach)  
    qDebug() << "\n=== METHOD 2: Natural then reverse ===";
    QByteArray method2Bytes;
    QByteArray naturalBytes;
    
    for (int i = 0, n = testBitString.length(); i < n; i += 8) {
        QString chunk = testBitString.mid(i, 8);
        
        bool ok = false;
        quint8 byteValue = static_cast<quint8>(chunk.toShort(&ok, 2));
        
        if (ok) {
            naturalBytes.append(static_cast<char>(byteValue));
        }
    }
    qDebug() << "Natural bytes:" << naturalBytes.toHex();
    
    // Reverse byte order
    for (int i = naturalBytes.length() - 1; i >= 0; i--) {
        method2Bytes.append(naturalBytes[i]);
    }
    qDebug() << "Final method2 bytes:" << method2Bytes.toHex();
    qDebug() << "Final method2 length:" << method2Bytes.length();
    
    // Method 3: Bit-by-bit conversion
    qDebug() << "\n=== METHOD 3: Bit-by-bit then reverse ===";
    QByteArray method3Bytes;
    int totalBytes = testBitString.length() / 8;
    
    QByteArray naturalBytes3;
    for (int byteIndex = 0; byteIndex < totalBytes; byteIndex++) {
        quint8 byteValue = 0;
        for (int bitIndex = 0; bitIndex < 8; bitIndex++) {
            int bitPos = byteIndex * 8 + bitIndex;
            if (bitPos < testBitString.length() && testBitString[bitPos] == '1') {
                byteValue |= (1 << (7 - bitIndex)); // MSB first within each byte
            }
        }
        naturalBytes3.append(byteValue);
    }
    qDebug() << "Natural bytes3:" << naturalBytes3.toHex();
    
    // Reverse byte order
    for (int i = naturalBytes3.length() - 1; i >= 0; i--) {
        method3Bytes.append(naturalBytes3[i]);
    }
    qDebug() << "Final method3 bytes:" << method3Bytes.toHex();
    qDebug() << "Final method3 length:" << method3Bytes.length();
    
    // Compare results
    qDebug() << "\n=== COMPARISON ===";
    qDebug() << "Method1 == Method2:" << (method1Bytes == method2Bytes);
    qDebug() << "Method1 == Method3:" << (method1Bytes == method3Bytes);
    qDebug() << "Method2 == Method3:" << (method2Bytes == method3Bytes);
    
    return 0;
}