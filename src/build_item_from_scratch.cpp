#include "structs.h"
#include "reversebitwriter.h"
#include "propertymodificationengine.h"
#include "itemparser.h"
#include "languagemanager.hpp"
#include "resourcepathmanager.hpp"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QBuffer>
#include <QDataStream>

// Usage: build_item_from_scratch <input.d2i> <output.d2i>
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    LanguageManager &langManager = LanguageManager::instance();
    langManager.currentLocale = "en";
    langManager.setResourcesPath(app.applicationDirPath() + "/../resources");

    if (argc < 3) { qDebug() << "Usage: build_item_from_scratch <input.d2i> <output.d2i>"; return 2; }
    QString inPath = argv[1];
    QString outPath = argv[2];

    // Parse original item to extract authoritative fields and bitString
    QFile f(inPath);
    if (!f.open(QIODevice::ReadOnly)) { qDebug() << "Failed to open" << inPath; return 3; }
    QByteArray bytes = f.readAll(); f.close();

    QBuffer buffer(&bytes); buffer.open(QIODevice::ReadOnly);
    QDataStream ds(&buffer); ds.setByteOrder(QDataStream::LittleEndian);
    ItemsList parsed;
    QString corrupted = ItemParser::parseItemsToBuffer(1, ds, bytes, QString(), &parsed);
    if (parsed.isEmpty()) { qDebug() << "No items parsed from" << inPath; return 4; }
    ItemInfo *src = parsed.first();

    // Create fresh ItemInfo and copy fields and bitString
    ItemInfo *item = new ItemInfo();
    item->itemType = src->itemType;
    item->isQuest = src->isQuest;
    item->isIdentified = src->isIdentified;
    item->isSocketed = src->isSocketed;
    item->isEar = src->isEar;
    item->isStarter = src->isStarter;
    item->isExtended = src->isExtended;
    item->isEthereal = src->isEthereal;
    item->isPersonalized = src->isPersonalized;
    item->isRW = src->isRW;

    item->location = src->location;
    item->whereEquipped = src->whereEquipped;
    item->row = src->row;
    item->column = src->column;
    item->storage = src->storage;

    item->guid = src->guid;
    item->ilvl = src->ilvl;
    item->quality = src->quality;
    item->variableGraphicIndex = src->variableGraphicIndex;

    // copy properties (shallow new copies)
    for (auto it = src->props.begin(); it != src->props.end(); ++it) {
        ItemProperty *p = it.value();
        item->props.insert(it.key(), new ItemProperty(p->value, p->param));
    }

    // Use the authoritative bitString from the parsed item so serialization matches
    item->bitString = src->bitString;

    // Serialize
    QByteArray outData;
    QBuffer outBuf(&outData); outBuf.open(QIODevice::ReadWrite);
    QDataStream ods(&outBuf); ods.setByteOrder(QDataStream::LittleEndian);
    ItemsList items; items.append(item);
    ItemParser::writeItems(items, ods);

    QFile outF(outPath);
    if (!outF.open(QIODevice::WriteOnly)) { qDebug() << "Failed to open out" << outPath; return 5; }
    outF.write(outData); outF.close();

    qDebug() << "Wrote" << outPath << "(copied from" << inPath << ")";
    return 0;
}
