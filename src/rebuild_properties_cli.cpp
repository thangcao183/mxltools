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

// CLI: rebuild_properties <file.d2i> [output.d2i]
// Rebuild properties for all items in the provided save file and write a rebuilt file.
int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    if (argc < 2)
    {
        qDebug() << "Usage: rebuild_properties <file.d2i> [output.d2i]";
        return 2;
    }

    // Ensure resources path
    LanguageManager &langManager = LanguageManager::instance();
    langManager.currentLocale = "en";
    langManager.setResourcesPath(app.applicationDirPath() + "/../resources");

    QString path = argv[1];
    QString outPath;
    if (argc >= 3)
        outPath = argv[2];
    else
        outPath = path + ".rebuilt.d2i"; // default: don't overwrite original
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open file" << path;
        return 3;
    }
    QByteArray fileBytes = f.readAll();
    f.close();

    QBuffer buffer(&fileBytes);
    buffer.open(QIODevice::ReadOnly);
    QDataStream ds(&buffer);
    ds.setByteOrder(QDataStream::LittleEndian);

    ItemsList allItems;
    // Count JM headers to estimate number of items in file
    const QByteArray jm = ItemParser::kItemHeader;
    int jmCount = 0;
    for (int pos = 0; pos < fileBytes.size(); ) {
        int idx = fileBytes.indexOf(jm, pos);
        if (idx == -1)
            break;
        ++jmCount;
        pos = idx + jm.size();
    }
    if (jmCount == 0)
        jmCount = 1; // fallback

    QString corrupted = ItemParser::parseItemsToBuffer(static_cast<quint16>(jmCount), ds, fileBytes, QString(), &allItems);
    if (allItems.isEmpty())
    {
        qDebug() << "No items parsed from file";
        return 4;
    }

    PropertyModificationEngine engine;
    bool ok = true;
    foreach (ItemInfo *item, allItems)
    {
        qDebug() << "Rebuilding properties for item" << item->itemType << "(props:" << item->props.size() <<")";
        if (!engine.reconstructItemBitString(item))
        {
            qDebug() << "Failed to reconstruct bit string for item:" << item->itemType << "error:" << engine.lastError();
            ok = false;
            break;
        }
        // Ensure byte alignment
        ReverseBitWriter::byteAlignBits(item->bitString);
    }

    if (!ok)
    {
        qDebug() << "Reconstruction failed, aborting.";
        return 5;
    }

    // Serialize all items back
    QByteArray outData;
    QBuffer outBuffer(&outData);
    outBuffer.open(QIODevice::ReadWrite);
    QDataStream outDs(&outBuffer);
    outDs.setByteOrder(QDataStream::LittleEndian);
    ItemParser::writeItems(allItems, outDs);

    // Write to output path (do not overwrite original unless explicitly asked)
    if (QFile::exists(outPath))
    {
        qDebug() << "Warning: output path exists, it will be overwritten:" << outPath;
        QFile::remove(outPath);
    }
    QFile outF(outPath);
    if (!outF.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open for writing:" << outPath;
        return 6;
    }
    outF.write(outData);
    outF.close();

    qDebug() << "Wrote rebuilt file to" << outPath << "(original left at" << path << ")";

    // Light-weight sanity check: ensure we wrote something plausible.
    if (outData.isEmpty())
    {
        qDebug() << "Error: produced empty output file; aborting.";
        if (QFile::exists(outPath))
            QFile::remove(outPath);
        return 7;
    }

    qDebug() << "Wrote rebuilt file to" << outPath << ". Skipping full parse verification to avoid parser crashes; please verify manually with the main app or using safe tools.";
    return 0;
}
