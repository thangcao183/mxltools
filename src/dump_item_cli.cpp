#include "itemparser.h"
#include "helpers.h"
#include "itemdatabase.h"
#include "structs.h"
#include "propertymodificationengine.h"
#include "languagemanager.hpp"
#include "resourcepathmanager.hpp"

#include <QApplication>
#include <QFile>
#include <QBuffer>
#include <QDataStream>
#include <QDebug>

// Usage: dump_item_cli <input.d2i>
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    LanguageManager &langManager = LanguageManager::instance();
    langManager.currentLocale = "en";
    langManager.setResourcesPath(app.applicationDirPath() + "/../resources");

    if (argc < 2) { qDebug() << "Usage: dump_item_cli <input.d2i>"; return 2; }
    QString inPath = argv[1];
    QFile f(inPath);
    if (!f.open(QIODevice::ReadOnly)) { qDebug() << "Failed to open" << inPath; return 3; }
    QByteArray bytes = f.readAll(); f.close();

    QBuffer buffer(&bytes); buffer.open(QIODevice::ReadOnly);
    QDataStream ds(&buffer); ds.setByteOrder(QDataStream::LittleEndian);

    ItemsList items;
    QString corrupted = ItemParser::parseItemsToBuffer(1, ds, bytes, QString(), &items);
    if (items.isEmpty()) { qDebug() << "No items parsed"; return 4; }
    ItemInfo *it = items.first();

    qDebug() << "itemType:" << it->itemType;
    qDebug() << "isQuest:" << it->isQuest << "isIdentified:" << it->isIdentified << "isSocketed:" << it->isSocketed
             << "isEar:" << it->isEar << "isStarter:" << it->isStarter << "isExtended:" << it->isExtended
             << "isEthereal:" << it->isEthereal << "isPersonalized:" << it->isPersonalized << "isRW:" << it->isRW;
    qDebug() << "location:" << it->location << "whereEquipped:" << it->whereEquipped << "row:" << it->row << "column:" << it->column << "storage:" << it->storage;
    qDebug() << "guid:" << it->guid << "ilvl:" << it->ilvl << "quality:" << it->quality << "variableGraphicIndex:" << it->variableGraphicIndex;
    qDebug() << "inscribedName:" << it->inscribedName << "defense:" << it->defense << "currentDurability:" << it->currentDurability << "maxDurability:" << it->maxDurability << "quantity:" << it->quantity << "socketsNumber:" << it->socketsNumber;
    qDebug() << "bitString length:" << it->bitString.length();

    qDebug() << "Properties (id -> value, param, bitStringOffset):";
    for (auto iter = it->props.begin(); iter != it->props.end(); ++iter) {
        ItemProperty *p = iter.value();
        qDebug() << iter.key() << "->" << p->value << ", param=" << p->param << ", offset=" << p->bitStringOffset << ", display=" << p->displayString;
    }

    qDebug() << "RW props:";
    for (auto iter = it->rwProps.begin(); iter != it->rwProps.end(); ++iter) {
        ItemProperty *p = iter.value();
        qDebug() << iter.key() << "->" << p->value << ", param=" << p->param;
    }

    qDebug() << "socketables count:" << it->socketablesInfo.size();
    return 0;
}
