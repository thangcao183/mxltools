#include "itemdatabase.h"
#include "propertymodificationengine.h"
#include "itemparser.h"
#include "languagemanager.hpp"
#include "resourcepathmanager.hpp"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QBuffer>
#include <QDataStream>

// Usage: create_special_item_cli <item_code> <out.d2i> <row> <col> <storage> <propId> <propValue>
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    LanguageManager &langManager = LanguageManager::instance();
    langManager.currentLocale = "en";
    langManager.setResourcesPath(app.applicationDirPath() + "/../resources");

    if (argc < 8) { qDebug() << "Usage: create_special_item_cli <item_code> <out.d2i> <row> <col> <storage> <propId> <propValue>"; return 2; }
    QByteArray code = QByteArray::fromRawData(argv[1], strlen(argv[1]));
    QString outPath = argv[2];
    int row = QString(argv[3]).toInt();
    int col = QString(argv[4]).toInt();
    int storage = QString(argv[5]).toInt();
    int propId = QString(argv[6]).toInt();
    int propValue = QString(argv[7]).toInt();

    // Lookup base item by code
    ItemBase *base = ItemDataBase::Items()->value(code, 0);
    if (!base)
    {
        qDebug() << "Item code not found in items database:" << code;
        qDebug() << "Make sure the code is exactly the key used in items.tsv (4-byte code).";
        return 3;
    }

    // Create minimal ItemInfo and set as Unique (special)
    ItemInfo *item = new ItemInfo();
    item->itemType = code;
    item->quality = Enums::ItemQuality::Unique;
    item->isIdentified = true;
    item->isExtended = true;
    item->ilvl = base->rlvl ? base->rlvl : 1;
    item->variableGraphicIndex = 0;
    item->move(row, col, 0, false);
    item->storage = storage;

    // Insert provided property
    item->props.insert(propId, new ItemProperty(propValue));

    PropertyModificationEngine engine;
    if (!engine.reconstructItemBitString(item)) {
        qDebug() << "Engine failed:" << engine.lastError();
        return 4;
    }
    ReverseBitWriter::byteAlignBits(item->bitString);

    // Serialize to .d2i
    QByteArray outData; QBuffer ob(&outData); ob.open(QIODevice::ReadWrite);
    QDataStream ods(&ob); ods.setByteOrder(QDataStream::LittleEndian);
    ItemsList items; items.append(item);
    ItemParser::writeItems(items, ods);

    QFile outF(outPath);
    if (!outF.open(QIODevice::WriteOnly)) { qDebug() << "Failed to open out" << outPath; return 5; }
    outF.write(outData); outF.close();

    qDebug() << "Wrote special item" << outPath << "based on code" << code;
    return 0;
}
