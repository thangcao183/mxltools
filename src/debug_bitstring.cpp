#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QDataStream>
#include "itemparser.h"
#include "itemdatabase.h"
#include "reversebitreader.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    if (argc < 2) {
        qDebug() << "Usage: debug_bitstring <file.d2i>";
        return 1;
    }
    
    QString fileName = argv[1];
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open" << fileName;
        return 2;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    ItemDataBase db;
    if (!db.initialize("../d2_items.db")) {
        qDebug() << "Failed to initialize database";
        return 3;
    }
    
    QDataStream stream(data);
    ItemInfo *item = ItemParser::parseItem(stream, data);
    if (!item) {
        qDebug() << "Failed to parse item";
        return 4;
    }
    
    qDebug() << "=== BitString Analysis ===";
    qDebug() << "Total length:" << item->bitString.length();
    qDebug() << "Item type:" << item->itemType;
    
    // Look for end marker
    QString endMarker = "111111111";
    int endPos = item->bitString.lastIndexOf(endMarker);
    qDebug() << "End marker position:" << endPos;
    
    // Show bit sections
    qDebug() << "\n=== First 50 bits ===";
    qDebug() << item->bitString.left(50);
    qDebug() << "\n=== Around properties (220-250) ===";
    if (item->bitString.length() > 250) {
        qDebug() << item->bitString.mid(220, 30);
    }
    
    qDebug() << "\n=== Around end marker ===";
    if (endPos >= 0 && endPos < item->bitString.length()) {
        int start = qMax(0, endPos - 20);
        int len = qMin(40, item->bitString.length() - start);
        qDebug() << "Bits" << start << "to" << (start+len-1) << ":";
        qDebug() << item->bitString.mid(start, len);
    }
    
    // Try to find property ID=7
    ReverseBitReader reader(item->bitString);
    reader.setPos(232); // Properties start
    
    qDebug() << "\n=== Reading properties from position 232 ===";
    try {
        int propCount = 0;
        while (reader.pos() + 9 <= item->bitString.length() && propCount < 20) {
            int currentPos = reader.pos();
            int propId = reader.readNumber(9);
            
            qDebug() << "At pos" << currentPos << ": read propId =" << propId;
            
            if (propId == 511) {
                qDebug() << "Found end marker at position" << currentPos;
                break;
            }
            
            if (propId == 7) {
                qDebug() << "*** Found existing property ID=7 at position" << currentPos << "***";
                ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(7);
                if (propTxt) {
                    int paramValue = 0;
                    if (propTxt->paramBits > 0) {
                        paramValue = reader.readNumber(propTxt->paramBits);
                    }
                    int rawValue = reader.readNumber(propTxt->bits);
                    int finalValue = rawValue - propTxt->add;
                    qDebug() << "Property 7: param=" << paramValue << "raw=" << rawValue << "final=" << finalValue;
                }
                break;
            }
            
            // Skip this property
            ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propId);
            if (propTxt && propId >= 0 && propId < 512) {
                reader.skip(propTxt->paramBits + propTxt->bits);
            } else {
                qDebug() << "Unknown property ID" << propId << ", stopping";
                break;
            }
            
            propCount++;
        }
    } catch (...) {
        qDebug() << "Exception during property reading";
    }
    
    delete item;
    return 0;
}