#include "specialitemcreationwidget.h"
#include "structs.h"
#include "itemdatabase.h"
#include "reversebitwriter.h"
#include "propertymodificationengine.h"
#include "languagemanager.hpp"
#include "resourcepathmanager.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QListWidget>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>

SpecialItemCreationWidget::SpecialItemCreationWidget(QWidget *parent)
    : QDialog(parent), _createdItem(nullptr), _targetRow(0), _targetCol(0)
{
    setupUI();
    populateItemList();

        _list->clear();

        // Fixed list of oil item types provided by the user
        const QStringList oilCodes = {
            "1p[","2o[","2p[","3p[","4o[","4p[","5o[","6o[","7o[","9o[",
            "ebs1","ebs2","ebs3","ebt1","ebt2","ebt3","ooc"
        };

        for (const QString &code : oilCodes) {
            ItemBase *b = ItemDataBase::Items()->value(code.toUtf8());
            QString label = b ? QString("%1 - %2").arg(b->name).arg(code) : QString("%1").arg(code);
            QListWidgetItem *it = new QListWidgetItem(label);
            it->setData(Qt::UserRole, code);
            it->setData(Qt::UserRole + 1, true);
            _list->addItem(it);
        }
                    _list->addItem(it);
                }
            }
            f.close();
            return; // done
        }
    }

    // Fallback to existing misc.tsv discovery if items.tsv not found or failed to open
    QString miscPath;
    QString genPath = QString("%1/txt_parser/generated/%2/misc.tsv").arg(LanguageManager::instance().resourcesPath).arg(LanguageManager::instance().modLocalization());
    if (QFile::exists(genPath)) miscPath = genPath;
    if (miscPath.isEmpty()) {
        QString p = QString("%1/txt/misc.tsv").arg(LanguageManager::instance().resourcesPath);
        if (QFile::exists(p)) miscPath = p;
    }
    if (miscPath.isEmpty()) {
        QString appDir = QCoreApplication::applicationDirPath();
        QString repoPath1 = QDir(appDir).absoluteFilePath("../utils/txt_parser/txt/misc.tsv");
        QString repoPath2 = QDir(appDir).absoluteFilePath("../../utils/txt_parser/txt/misc.tsv");
        if (QFile::exists(repoPath1)) miscPath = repoPath1;
        else if (QFile::exists(repoPath2)) miscPath = repoPath2;
    }
    if (miscPath.isEmpty()) {
        QString p = ResourcePathManager::dataPathForFileName("txt/misc.tsv");
        if (QFile::exists(p)) miscPath = p;
    }

    if (!miscPath.isEmpty()) {
        QFile f(miscPath);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&f);
            QString header = in.readLine(); // skip header
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.trimmed().isEmpty()) continue;
                QStringList cols = line.split('\t');
                if (cols.size() < 6) continue;
                QString name = cols.at(0).trimmed();
                QString code = cols.at(5).trimmed();
                if (name.isEmpty() || code.isEmpty()) continue;
                // filter oils by code prefix 'oil' or name contains 'Oil'
                if (code.startsWith("oil") || name.contains("Oil", Qt::CaseInsensitive)) {
                    QString label = QString("%1 - %2").arg(name).arg(code);
                    QListWidgetItem *it = new QListWidgetItem(label);
                    it->setData(Qt::UserRole, code);
                    // mark as oil entry for later detection
                    it->setData(Qt::UserRole + 1, true);
                    _list->addItem(it);
                }
            }
            f.close();
        }
    }
}

void SpecialItemCreationWidget::setItemPosition(int row, int column)
{
    _targetRow = row; _targetCol = column;
    _rowSpin->setValue(row); _colSpin->setValue(column);
}

void SpecialItemCreationWidget::onSelectionChanged()
{
    QListWidgetItem *it = _list->currentItem();
    if (!it) { _previewLabel->setText(tr("No base selected.")); return; }
    QString code = it->data(Qt::UserRole).toString();
    ItemBase *b = ItemDataBase::Items()->value(code.toUtf8());
    QString txt = QString("<b>%1</b><br>Code: %2<br>Size: %3x%4<br>Req Lvl: %5")
            .arg(b ? b->name : QString("<unknown>"))
            .arg(code)
            .arg(b ? b->width : 0)
            .arg(b ? b->height : 0)
            .arg(b ? b->rlvl : 0);
    _previewLabel->setText(txt);
}

void SpecialItemCreationWidget::onCreateClicked()
{
    QListWidgetItem *it = _list->currentItem();
    if (!it) { QMessageBox::warning(this, tr("Warning"), tr("Please select a base item.")); return; }
    QString code = it->data(Qt::UserRole).toString();

    // For oil items we have a stable template. Create special oils by loading a
    // canonical oil template and only replacing the 4-byte itemType to avoid
    // complex bitstring insertion logic which can fail.
    bool isOil = it->data(Qt::UserRole + 1).toBool();

    if (isOil) {
        // Try multiple template name candidates so we pick the existing resource
        const QStringList tplCandidates = { QStringLiteral("4o["), QStringLiteral("4o[.d2i"), QStringLiteral("oil"), QStringLiteral("oil.d2i") };
        ItemInfo *tpl = nullptr;
        for (const QString &cand : tplCandidates) {
            tpl = ItemDataBase::loadItemFromFile(cand);
            if (tpl) break;
        }

        // try explicit resource path for items/4o[.d2i as last resort
        if (!tpl) {
            QString explicitPath = ResourcePathManager::dataPathForFileName("items/4o[.d2i");
            if (!explicitPath.isEmpty() && QFile::exists(explicitPath))
                tpl = ItemDataBase::loadItemFromFile(explicitPath);
        }

        if (!tpl) {
            QMessageBox::critical(this, tr("Error"), tr("Oil template not found (tried 4o[.d2i / oil.d2i)."));
            _createdItem = nullptr;
            return;
        }

        ItemInfo *item = new ItemInfo(*tpl);

        // Update itemType bytes directly in the bitString (first 4 chars)
        QByteArray newType = code.toUtf8();
        // pad/trim to 4
        while (newType.length() < 4) newType.append('\0');
        for (int i = 0; i < 4; ++i) {
            ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Type + i * 8, newType.at(i));
        }

        // set coordinates and minimal metadata
        item->move(_rowSpin->value(), _colSpin->value(), 0, true);
        item->storage = -1;
        item->hasChanged = true;

        // Keep itemType field consistent with the modified bitString
        item->itemType = code.toUtf8();

        // Optionally attach a simple property if provided
        int propId = _propIdSpin->value();
        int propVal = _propValueSpin->value();
        if (propId > 0) item->props.insert(propId, new ItemProperty(propVal));

        ReverseBitWriter::byteAlignBits(item->bitString);
        _createdItem = item;
        accept();
        return;
    }

    // Non-oil fall back: try to reconstruct normally (existing behaviour)
    QString filePath = QString("items/%1").arg(code);
    ItemInfo *tpl = ItemDataBase::loadItemFromFile(filePath);
    ItemInfo *item = nullptr;
    if (tpl) {
        item = new ItemInfo(*tpl);
    } else {
        // Minimal construction
        item = new ItemInfo();
        item->itemType = code.toUtf8();
        item->isExtended = true;
        ItemBase *base = ItemDataBase::Items()->value(code.toUtf8());
        if (base) item->ilvl = base->rlvl;
        item->quality = Enums::ItemQuality::Unique;
    }

    item->move(_rowSpin->value(), _colSpin->value(), 0, true);
    item->storage = -1; // caller will decide

    int propId = _propIdSpin->value();
    int propVal = _propValueSpin->value();
    if (propId > 0) item->props.insert(propId, new ItemProperty(propVal));

    PropertyModificationEngine engine;
    if (!engine.reconstructItemBitString(item)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to build item bitstring: %1").arg(engine.lastError()));
        delete item; _createdItem = nullptr; return;
    }
    ReverseBitWriter::byteAlignBits(item->bitString);

    item->hasChanged = true;
    _createdItem = item;
    accept();
}

void SpecialItemCreationWidget::onCancelClicked()
{
    _createdItem = nullptr;
    reject();
}
