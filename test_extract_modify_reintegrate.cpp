#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include "src/enhancedpropertyadditionengine.h"
#include "src/itemparser.h"
#include "src/itemdatabase.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set up paths
    QDir::setCurrent("/home/wolf/CODE/C/mxltools");
    
    // Initialize database
    ItemDataBase::initDatabase("d2_items.db");
    
    // Load test item file
    QFile testFile("/home/wolf/CODE/C/mxltools/save/ring_test.d2i");
    if (!testFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open test file";
        return 1;
    }
    
    QByteArray fileData = testFile.readAll();
    testFile.close();
    
    qDebug() << "Test file size:" << fileData.size() << "bytes";
    
    // Parse the item
    QBuffer buffer(&fileData);
    buffer.open(QIODevice::ReadOnly);
    QDataStream stream(&buffer);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    ItemInfo *item = ItemParser::parseItem(stream, fileData);
    if (!item) {
        qDebug() << "Failed to parse item";
        return 1;
    }
    
    qDebug() << "Parsed item successfully:";
    qDebug() << "  - Type:" << item->itemType;
    qDebug() << "  - BitString length:" << item->bitString.length();
    qDebug() << "  - Properties count:" << item->props.size();
    
    // Initialize Enhanced Engine
    EnhancedPropertyAdditionEngine engine;
    if (!engine.loadPropertyDatabase("/home/wolf/CODE/C/mxltools/props.tsv")) {
        qDebug() << "Failed to load properties database";
        return 1;
    }
    
    qDebug() << "Enhanced Engine initialized successfully";
    
    // Test adding Life property (+50)
    qDebug() << "\n=== Testing Extract→Modify→Reintegrate Approach ===";
    
    bool success = engine.addPropertyToItem(item, 7, 50, 0); // Life +50
    
    if (success) {
        qDebug() << "SUCCESS: Property added successfully!";
        qDebug() << "  - New bitString length:" << item->bitString.length();
        qDebug() << "  - New properties count:" << item->props.size();
        
        // Show properties
        for (auto it = item->props.constBegin(); it != item->props.constEnd(); ++it) {
            qDebug() << "  - Property" << it.key() << ":" << it.value()->value;
        }
    } else {
        qDebug() << "FAILED: Property addition failed";
        qDebug() << "Error:" << engine.lastError();
    }
    
    delete item;
    return 0;
}