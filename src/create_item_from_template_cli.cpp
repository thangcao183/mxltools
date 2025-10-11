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

// Usage: create_item_from_template_cli <template_name> <out.d2i> <row> <col> <storage> <propId> <propValue>
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    LanguageManager &langManager = LanguageManager::instance();
    langManager.currentLocale = "en";
    langManager.setResourcesPath(app.applicationDirPath() + "/../resources");

    if (argc < 8) { qDebug() << "Usage: create_item_from_template_cli <template_name> <out.d2i> <row> <col> <storage> <propId> <propValue>"; return 2; }
    QString tpl = argv[1];
    QString outPath = argv[2];
    int row = QString(argv[3]).toInt();
    int col = QString(argv[4]).toInt();
    int storage = QString(argv[5]).toInt();
    int propId = QString(argv[6]).toInt();
    int propValue = QString(argv[7]).toInt();

    ItemInfo *tplItem = ItemDataBase::loadItemFromFile(tpl);
    if (!tplItem) { qDebug() << "Failed to load template" << tpl; return 3; }

    ItemInfo *item = new ItemInfo(*tplItem);
    item->move(row, col, 0);
    item->storage = storage;

    // insert property
    item->props.insert(propId, new ItemProperty(propValue));

    PropertyModificationEngine engine;
    if (!engine.reconstructItemBitString(item)) {
        qDebug() << "Engine failed:" << engine.lastError();
        return 4;
    }
    ReverseBitWriter::byteAlignBits(item->bitString);

    // Serialize
    QByteArray outData; QBuffer ob(&outData); ob.open(QIODevice::ReadWrite);
    QDataStream ods(&ob); ods.setByteOrder(QDataStream::LittleEndian);
    ItemsList items; items.append(item);
    ItemParser::writeItems(items, ods);

    QFile outF(outPath);
    if (!outF.open(QIODevice::WriteOnly)) { qDebug() << "Failed to open out" << outPath; return 5; }
    outF.write(outData); outF.close();

    qDebug() << "Wrote" << outPath << "based on template" << tpl;
    return 0;
}
