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

// DEEP RESEARCH: Study ItemParser internals and bit construction logic
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    LanguageManager &langManager = LanguageManager::instance();  
    langManager.currentLocale = "en";
    langManager.setResourcesPath(app.applicationDirPath() + "/../resources");

    if (argc < 2) {
        qDebug() << "Usage: deep_itemparser_research <input.d2i>";
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

    qDebug() << "=== DEEP ITEMPARSER RESEARCH ===";
    qDebug() << "File size:" << inputData.size() << "bytes";
    
    // Show all bytes in detail
    qDebug() << "\n=== COMPLETE BYTE ANALYSIS ===";
    qDebug() << "Header:" << inputData.left(2).toHex(' ');
    
    for (int i = 2; i < inputData.size(); i++) {
        quint8 byte = inputData[i];  
        QString binary = QString::number(byte, 2).rightJustified(8, '0');
        qDebug() << "Byte" << (i-2) << "(file pos" << i << "):"
                 << QString("0x%1").arg(byte, 2, 16, QChar('0'))
                 << "=" << byte << "binary:" << binary;
    }

    // Parse with ItemParser and trace
    QBuffer buffer(&inputData);
    buffer.open(QIODevice::ReadOnly);
    QDataStream ds(&buffer);
    ds.setByteOrder(QDataStream::LittleEndian);

    qDebug() << "\n=== ITEMPARSER PROCESSING ===";
    ItemsList items;
    QString corrupted = ItemParser::parseItemsToBuffer(1, ds, inputData, QString(), &items);
    
    if (items.isEmpty()) {
        qDebug() << "Failed to parse item";
        return 4;
    }

    ItemInfo *item = items.first();
    
    qDebug() << "\n=== RESULTING BITSTRING ANALYSIS ===";
    qDebug() << "BitString length:" << item->bitString.length() << "bits";
    qDebug() << "Expected bytes:" << (item->bitString.length() + 7) / 8;
    qDebug() << "Actual item bytes:" << (inputData.size() - 2);
    
    // Break bitString into 8-bit chunks and analyze
    qDebug() << "\n=== BITSTRING TO BYTE MAPPING ===";
    for (int i = 0; i < item->bitString.length(); i += 8) {
        int byteIndex = i / 8;
        QString chunk = item->bitString.mid(i, 8);
        
        // Try different interpretations
        quint8 lsbFirst = 0, msbFirst = 0;
        
        // LSB-first interpretation
        for (int j = 0; j < chunk.length(); j++) {
            if (chunk[j] == '1') {
                lsbFirst |= (1 << j);
            }
        }
        
        // MSB-first interpretation  
        for (int j = 0; j < chunk.length(); j++) {
            if (chunk[j] == '1') {
                msbFirst |= (1 << (7 - j));
            }
        }
        
        quint8 actualByte = (byteIndex + 2 < inputData.size()) ? inputData[byteIndex + 2] : 0;
        
        qDebug() << "Bit chunk" << byteIndex << ":" << chunk
                 << "LSB-first:" << QString("0x%1").arg(lsbFirst, 2, 16, QChar('0'))
                 << "MSB-first:" << QString("0x%1").arg(msbFirst, 2, 16, QChar('0'))
                 << "Actual:" << QString("0x%1").arg(actualByte, 2, 16, QChar('0'))
                 << ((lsbFirst == actualByte) ? "MATCH-LSB" : 
                     (msbFirst == actualByte) ? "MATCH-MSB" : "NO-MATCH");
    }

    qDebug() << "\n=== ITEMPARSER INTERNAL LOGIC STUDY ===";
    qDebug() << "Let's study how ItemParser actually constructs bitString...";
    
    // Try to understand ItemParser's bit reading pattern
    // by studying the source code structure
    
    qDebug() << "Item fields that affect bitString construction:";
    qDebug() << "- isExtended:" << item->isExtended;
    qDebug() << "- isIdentified:" << item->isIdentified; 
    qDebug() << "- isSocketed:" << item->isSocketed;
    qDebug() << "- quality:" << item->quality;
    qDebug() << "- ilvl:" << item->ilvl;
    
    // Let's trace the exact bit positions
    qDebug() << "\n=== BIT POSITION TRACING ===";
    qDebug() << "Properties section starts at bit 232";
    qDebug() << "Let's see what's in bits 0-232 (item header/body):";
    
    QString headerBits = item->bitString.left(232);
    qDebug() << "Header bits (0-232):" << headerBits;
    
    // Convert header bits back to bytes to see if we can match
    qDebug() << "\n=== HEADER BITS TO BYTES CONVERSION TEST ===";
    for (int method = 1; method <= 2; method++) {
        qDebug() << "Method" << method << ":";
        QByteArray reconstructed;
        
        for (int i = 0; i < headerBits.length(); i += 8) {
            QString chunk = headerBits.mid(i, 8);
            if (chunk.length() < 8) {
                chunk = chunk.leftJustified(8, '0'); // Pad with zeros
            }
            
            quint8 byte = 0;
            if (method == 1) {
                // LSB-first
                for (int j = 0; j < 8; j++) {
                    if (j < chunk.length() && chunk[j] == '1') {
                        byte |= (1 << j);
                    }
                }
            } else {
                // MSB-first  
                for (int j = 0; j < 8; j++) {
                    if (j < chunk.length() && chunk[j] == '1') {
                        byte |= (1 << (7 - j));
                    }
                }
            }
            reconstructed.append(byte);
        }
        
        QByteArray expectedHeader = inputData.mid(2, reconstructed.size());
        qDebug() << "Reconstructed:" << reconstructed.left(8).toHex(' ');
        qDebug() << "Expected:     " << expectedHeader.left(8).toHex(' ');
        qDebug() << "Match:" << (reconstructed == expectedHeader ? "YES" : "NO");
    }

    // Clean up
    qDeleteAll(items);
    return 0;
}