#include "itemspropertiessplitter.h"
#include "propertiesviewerwidget.h"
#include "propertyeditor.h"
#include "itemdatabase.h"
#include "itemstoragetableview.h"
#include "itemstoragetablemodel.h"
#include "itemparser.h"
#include "resourcepathmanager.hpp"
#include "itemsviewerdialog.h"
#include "plugyitemssplitter.h"
#include "reversebitwriter.h"
#include "characterinfo.hpp"
#include "progressbarmodal.hpp"
#include "qd2charrenamer.h"
#include "runecreationwidget.h"
#include "itemcreationwidget.h"
#include "gemcreationwidget.h"
#include "arcanecrystalcreationwidget.h"
#include "oilcreationwidget.h"
#include "mysticorbcreationwidget.h"

#include <QMenu>
#include <QInputDialog>
#include <QApplication>
#include <QClipboard>
#include <QVBoxLayout>
#include <QPushButton>
#include <QBuffer>

#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>

#ifdef DUMP_INFO_ACTION
#include <QTextCodec>
#endif


static const int kShardsPerCrystal = 5;
static const QRegExp kRuneRegExp("r(\\d\\d)");
static const quint8 HighestRuneKey = 33, kPerfectGrade = 4;


ItemsPropertiesSplitter::ItemsPropertiesSplitter(ItemStorageTableView *itemsView, QWidget *parent /*= 0*/) : QSplitter(Qt::Horizontal, parent), _itemsView(itemsView), _propertiesWidget(new PropertiesViewerWidget(parent))
{
    QWidget *leftWidget = new QWidget(this);

    QPushButton *copyBBCodesButton = new QPushButton(tr("Copy BBCode of items here"), leftWidget);
    QMenu *copyBBCodesMenu = createCopyBBCodeMenu(false);
    copyBBCodesButton->setMenu(copyBBCodesMenu);

    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->addWidget(_itemsView);
    leftLayout->addWidget(copyBBCodesButton);

    addWidget(leftWidget);
    addWidget(_propertiesWidget);

    createItemActions();

    setChildrenCollapsible(false);
    setStretchFactor(1, 5);

    connect(_itemsView, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(showContextMenu(const QPoint &)));
    connect(_itemsView, SIGNAL(pressed(QModelIndex)), SLOT(moveBetweenStashes()));
    connect(copyBBCodesMenu, SIGNAL(triggered(QAction*)), SLOT(copyAllItemsBBCode(QAction*)));
    
    // Connect PropertiesViewerWidget signals to forward to ItemsViewerDialog and then MainWindow
    connect(_propertiesWidget, SIGNAL(itemsChanged()), SIGNAL(itemsChanged()));
    connect(_propertiesWidget, SIGNAL(itemsChanged(bool)), SIGNAL(itemsChanged(bool)));
}

void ItemsPropertiesSplitter::setModel(ItemStorageTableModel *model)
{
    _itemsModel = model;
    _itemsView->setModel(model);
    // TODO: [0.5] change signal to selectionChanged
    connect(_itemsView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), SLOT(itemSelected(const QModelIndex &)));
    connect(_itemsModel, SIGNAL(itemMoved(const QModelIndex &, const QModelIndex &)), SLOT(moveItem(const QModelIndex &, const QModelIndex &)));
}

void ItemsPropertiesSplitter::itemSelected(const QModelIndex &index, bool display /*= true*/)
{
    ItemInfo *item = _itemsModel->itemAtIndex(index);
    if (display)
        _propertiesWidget->showItem(item);

    // correctly disable hotkeys
    _itemActions[DisenchantShards]->setEnabled(ItemDataBase::canDisenchantIntoArcaneShards(item));
    _itemActions[DisenchantSignet]->setEnabled(ItemDataBase::canDisenchantIntoSignetOfLearning(item));
    _itemActions[RemoveMO]->setEnabled(_propertiesWidget->hasMysticOrbs() && !ItemDataBase::isUberCharm(item));

    // item bbcode
    QAction *bbcodeAction = _itemActions[CopyItemBBCode];
    bbcodeAction->setObjectName(itemNameBBCode(item));
    foreach (QAction *action, bbcodeAction->menu()->actions() + QList<QAction *>() << bbcodeAction)
        action->setDisabled(bbcodeAction->objectName().isEmpty());

    // eat signet of learning
    quint8 statsFromSignet = 0;
    if (item)
    {
        QRegExp customSignetRegExp("(\\d\\d)\\^");
        if (isSignetOfLearning(item))
            statsFromSignet = 1;
        else if (customSignetRegExp.exactMatch(item->itemType))
            statsFromSignet = customSignetRegExp.cap(1).toUShort();
        else if (item->itemType == "zk#")
            statsFromSignet = 5;
        else if (item->itemType == "zke" || item->itemType == "zky")
            statsFromSignet = 25;
    }
    QAction *eatSignetsAction = _itemActions[EatSignetOfLearning];
    eatSignetsAction->setData(statsFromSignet);
    eatSignetsAction->setText(tr("Eat signet [%n free stat(s)]", 0, statsFromSignet));
    eatSignetsAction->setEnabled(statsFromSignet > 0 && CharacterInfo::instance().valueOfStatistic(Enums::CharacterStats::SignetsOfLearningEaten) < Enums::CharacterStats::SignetsOfLearningMax);
}

void ItemsPropertiesSplitter::moveItem(const QModelIndex &newIndex, const QModelIndex &oldIndex)
{
    ItemInfo *item = _itemsModel->itemAtIndex(newIndex);
    ReverseBitWriter::updateItemRow(item);
    ReverseBitWriter::updateItemColumn(item);
    item->hasChanged = true;

    int oldRow = oldIndex.row(), oldCol = oldIndex.column();
    if (_itemsView->rowSpan(oldRow, oldCol) > 1 || _itemsView->columnSpan(oldRow, oldCol) > 1)
        _itemsView->setSpan(oldRow, oldCol, 1, 1);
    setCellSpanForItem(item);
    _itemsView->setCurrentIndex(newIndex);

    emit itemsChanged();
}

void ItemsPropertiesSplitter::showItem(ItemInfo *item)
{
    if (item)
    {
        _itemsView->setCurrentIndex(_itemsModel->index(item->row, item->column));
        //dumpInfo(item, false);
    }
}

void ItemsPropertiesSplitter::showFirstItem()
{
    _itemsView->setFocus(); // otherwise
    // sometimes item is selected, but there's no visual selection
    if (_itemsView->selectionModel()->selectedIndexes().isEmpty() || !selectedItem(false))
        showItem(_itemsModel->firstItem());
}

void ItemsPropertiesSplitter::setItems(const ItemsList &newItems)
{
    _allItems = newItems;
    updateItems(_allItems);
}

QPair<bool, bool> ItemsPropertiesSplitter::updateDisenchantButtonsState(bool includeUniques, bool includeSets, bool toCrystals, ItemsList *items /*= 0*/)
{
    bool allowShards = false, allowSignets = false;
    int shards = 0;
    foreach (ItemInfo *item, items ? *items : _allItems)
    {
        if (toCrystals)
        {
            if (isArcaneShard(item))
                ++shards;
            else if (isArcaneShard2(item))
                shards += 2;
            else if (isArcaneShard3(item))
                shards += 3;
            else if (isArcaneShard4(item))
                shards += 4;
        }

        if ((includeUniques && item->quality == Enums::ItemQuality::Unique) || (includeSets && item->quality == Enums::ItemQuality::Set))
        {
            if (!allowShards)
                allowShards = ItemDataBase::canDisenchantIntoArcaneShards(item);
            if (!allowSignets)
                allowSignets = ItemDataBase::canDisenchantIntoSignetOfLearning(item);
            if (allowShards && allowSignets)
                break;
        }
    }
    // allow just upgrading shards to crystals
    if (toCrystals && !allowShards && shards >= kShardsPerCrystal)
        allowShards = true;
    return qMakePair(allowShards, allowSignets);
}

bool ItemsPropertiesSplitter::canSocketableMapBeUpgraded(const UpgradableItemsMultiMap &socketableMap)
{
    foreach (quint8 key, socketableMap.uniqueKeys())
        if (socketableMap.count(key) > 1)
            return true;
    return false;
}

QPair<bool, bool> ItemsPropertiesSplitter::updateUpgradeButtonsState(int reserveRunes, ItemsList *pItems /*= 0*/)
{
    const ItemsList &items = pItems ? *pItems : _allItems;

    bool enableGemsButton = false;
    QHash<QByteArray, UpgradableItemsMultiMap> gemsMapsHash = gemsMapsFromItems(items);
    foreach (const UpgradableItemsMultiMap &gemsMap, gemsMapsHash)
        if ((enableGemsButton = canSocketableMapBeUpgraded(gemsMap)))
            break;

    return qMakePair(enableGemsButton, canSocketableMapBeUpgraded(runesMapFromItems(items, reserveRunes)));
}

void ItemsPropertiesSplitter::updateItems(const ItemsList &newItems)
{
    _propertiesWidget->clear();
    _itemsModel->setItems(newItems);

    _itemsView->clearSpans();
    foreach (ItemInfo *item, newItems)
        setCellSpanForItem(item);

    showFirstItem();
}

void ItemsPropertiesSplitter::showContextMenu(const QPoint &pos)
{
    ItemInfo *item = selectedItem(false);
    QModelIndex clickedIndex = _itemsView->indexAt(pos);
    
    if (item)
    {
        QList<QAction *> actions;

        // Add edit properties action
        QAction *editPropsAction = new QAction(tr("Edit Properties"), this);
        connect(editPropsAction, SIGNAL(triggered()), SLOT(editItemProperties()));
        actions << editPropsAction << separatorAction();

        if (shouldAddMoveItemAction())
        {
            QAction *moveBetweenStashesAction = new QAction(moveItemActionText() + QString(" (%1)").arg(tr("Alt+Click")), this);
            connect(moveBetweenStashesAction, SIGNAL(triggered()), SLOT(moveBetweenStashes()));
            actions << moveBetweenStashesAction << separatorAction();
        }

        if (_itemActions[CopyItemBBCode]->isEnabled())
            actions << _itemActions[CopyItemBBCode] << separatorAction();

        // TODO: 0.5
        //QMenu *menuExport = new QMenu(tr("Export as"), _itemsView);
        //menuExport->addActions(QList<QAction *>() << _itemActions[ExportBbCode] << _itemActions[ExportHtml]);
        //actions << menuExport->menuAction() << separator();

        if (_itemActions[DisenchantShards]->isEnabled() || _itemActions[DisenchantSignet]->isEnabled())
        {
            QMenu *menuDisenchant = new QMenu(tr("Disenchant into"), _itemsView);
            if (_itemActions[DisenchantSignet]->isEnabled())
                menuDisenchant->addAction(_itemActions[DisenchantSignet]);
            if (_itemActions[DisenchantShards]->isEnabled())
                menuDisenchant->addAction(_itemActions[DisenchantShards]);
            actions << menuDisenchant->menuAction();
        }

        // TODO: 0.5
//        if (item->isSocketed && item->socketablesNumber)
//            actions << _itemActions[Unsocket];

        if (_itemActions[RemoveMO]->isEnabled())
        {
            QAction *actionToAdd = 0;
            if (_propertiesWidget->mysticOrbsTotal() > 1)
            {
                QMenu *menuMO = new QMenu(_itemsView);
                _itemActions[RemoveMO]->setText(tr("All"));
                menuMO->addActions(QList<QAction *>() << _itemActions[RemoveMO] << separatorAction());

                createActionsForMysticOrbs(menuMO, true, item);
                menuMO->addAction(separatorAction());
                createActionsForMysticOrbs(menuMO, false, item);

                actionToAdd = menuMO->menuAction();
            }
            else
                actionToAdd = _itemActions[RemoveMO];

            actionToAdd->setText(tr("Remove Mystic Orbs"));
            actions << actionToAdd;
        }

        // downgrade a rune
        if (kRuneRegExp.exactMatch(item->itemType))
        {
            quint8 runeCode = kRuneRegExp.cap(1).toUShort();
            if (runeCode > 1 && runeCode <= HighestRuneKey)
            {
                QMenu *menuDowngrade = new QMenu(tr("Downgrade to"), _itemsView);
                while (--runeCode)
                {
                    QByteArray runeKey = QString("r%1").arg(runeCode, 2, 10, kZeroChar).toLatin1();
                    ItemBase *base = ItemDataBase::Items()->value(runeKey);
                    QAction *actionRune = new QAction(QIcon(ResourcePathManager::pathForItemImageName(base->imageName)), QString("(%1) %2").arg(base->rlvl).arg(QString(base->name).remove("\\purple;")), _itemsView);
                    actionRune->setData(runeCode);
                    actionRune->setIconVisibleInMenu(true); // explicitly show icon on Mac OS X
                    connect(actionRune, SIGNAL(triggered()), SLOT(downgradeSelectedRune()));
                    menuDowngrade->addAction(actionRune);
                }
                actions << menuDowngrade->menuAction();
            }
        }

        if (_itemActions[EatSignetOfLearning]->data().toUInt() > 0)
            actions << _itemActions[EatSignetOfLearning];

        if (isShrineVessel(item))
        {
            QAction *collectAction = new QAction(tr("Collect Shrines"), this);
            connect(collectAction, SIGNAL(triggered()), SLOT(collectShrinesToVessel()));
            actions << collectAction;

            ItemProperty *prop = item->props.value(Enums::ItemProperties::ShrineVesselCounter);
            if (prop->value > 0)
            {
                QAction *extractAction = new QAction(tr("Extract all Shrines"), this);
                connect(extractAction, SIGNAL(triggered()), SLOT(extractShrinesFromVessel()));
                actions << extractAction;

                if (prop->value > 1)
                {
                    extractAction = new QAction(tr("Extract Shrines..."), this);
                    extractAction->setObjectName("input");
                    connect(extractAction, SIGNAL(triggered()), SLOT(extractShrinesFromVessel()));
                    actions << extractAction;
                }
                else
                    extractAction->setText(tr("Extract the only Shrine"));
            }
            actions << separatorAction();
        }

        if (item->isPersonalized)
        {
            QAction *depersonalizeAction = new QAction(tr("Depersonalize"), this);
            connect(depersonalizeAction, SIGNAL(triggered()), SLOT(depersonalize()));
            actions << depersonalizeAction;
        }
        else if (item->isExtended) // allow personalization of any item
        {
            QAction *personalizeAction = new QAction(tr("Personalize"), this);
            connect(personalizeAction, SIGNAL(triggered()), SLOT(personalize()));
            actions << personalizeAction;

            personalizeAction = new QAction(tr("Personalize with name..."), this);
            personalizeAction->setObjectName("setName");
            connect(personalizeAction, SIGNAL(triggered()), SLOT(personalize()));
            actions << personalizeAction;
        }

        actions << separatorAction() << _itemActions[Delete];
#if 1
    QAction *exportD2iAction = new QAction(tr("Export as .d2i"), this);
    connect(exportD2iAction, SIGNAL(triggered()), SLOT(exportItemAsD2i()));
    actions << separatorAction() << exportD2iAction;

    QAction *importD2iAction = new QAction(tr("Import from .d2i"), this);
    connect(importD2iAction, SIGNAL(triggered()), SLOT(importItemFromD2i()));
    actions << importD2iAction;

    QAction *editD2iAction = new QAction(tr("Edit .d2i file..."), this);
    connect(editD2iAction, SIGNAL(triggered()), SLOT(editD2iFile()));
    actions << editD2iAction;
#endif
#ifdef DUMP_INFO_ACTION
        QAction *dumpInfoAction = new QAction("Dump info", this);
        connect(dumpInfoAction, SIGNAL(triggered()), SLOT(dumpInfo()));
        actions << separatorAction() << dumpInfoAction;
#endif
        QMenu::exec(actions, _itemsView->mapToGlobal(pos));
    }
    else if (clickedIndex.isValid())
    {
        // Right-clicked on empty cell - show item creation menu
        QList<QAction *> actions;
        
        QAction *createRuneAction = new QAction(QString("Create Rune..."), this);
        if (createRuneAction) {
            createRuneAction->setData(QPoint(clickedIndex.row(), clickedIndex.column()));
            connect(createRuneAction, SIGNAL(triggered()), this, SLOT(createRuneAt()));
            actions << createRuneAction;
        }
        
        QAction *createGemAction = new QAction(QString("Create Gem..."), this);
        if (createGemAction) {
            createGemAction->setData(QPoint(clickedIndex.row(), clickedIndex.column()));
            connect(createGemAction, SIGNAL(triggered()), this, SLOT(createGemAt()));
            actions << createGemAction;
        }

        QAction *createOilAction = new QAction(QString("Create Oil..."), this);
        if (createOilAction) {
            createOilAction->setData(QPoint(clickedIndex.row(), clickedIndex.column()));
            connect(createOilAction, SIGNAL(triggered()), this, SLOT(createOilAt()));
            actions << createOilAction;
        }
        
        QAction *createCrystalAction = new QAction(QString("Create Arcane Crystal..."), this);
        if (createCrystalAction) {
            createCrystalAction->setData(QPoint(clickedIndex.row(), clickedIndex.column()));
            connect(createCrystalAction, SIGNAL(triggered()), this, SLOT(createArcaneCrystalAt()));
            actions << createCrystalAction;
        }

        QAction *createShrineAction = new QAction(QString("Create Shrine..."), this);
        if (createShrineAction) {
            createShrineAction->setData(QPoint(clickedIndex.row(), clickedIndex.column()));
            connect(createShrineAction, SIGNAL(triggered()), this, SLOT(createShrineAt()));
            actions << createShrineAction;
        }

        QAction *createMysticOrbAction = new QAction(QString("Create Mystic Orb..."), this);
        if (createMysticOrbAction) {
            createMysticOrbAction->setData(QPoint(clickedIndex.row(), clickedIndex.column()));
            connect(createMysticOrbAction, SIGNAL(triggered()), this, SLOT(createMysticOrbAt()));
            actions << createMysticOrbAction;
        }
        
        QAction *createItemAction = new QAction(QString("Create Items..."), this);
        if (createItemAction) {
            createItemAction->setData(QPoint(clickedIndex.row(), clickedIndex.column()));
            connect(createItemAction, SIGNAL(triggered()), this, SLOT(createItemAt()));
            actions << createItemAction;
        }
        // Add Import .d2i action for empty cell (place imported item at clicked coordinates)
        QAction *importEmptyD2iAction = new QAction(QString("Import from .d2i..."), this);
        if (importEmptyD2iAction) {
            importEmptyD2iAction->setData(QPoint(clickedIndex.row(), clickedIndex.column()));
            connect(importEmptyD2iAction, SIGNAL(triggered()), this, SLOT(importItemFromD2iAt()));
            actions << importEmptyD2iAction;
        }
        
        if (!actions.isEmpty()) {
            QMenu::exec(actions, _itemsView->mapToGlobal(pos));
        }
    }
}

void ItemsPropertiesSplitter::exportItemAsD2i()
{
    ItemInfo *item = selectedItem(true);
    if (!item)
        return;

    QString suggested = QString("%1_%2.d2i").arg(QString::fromLatin1(item->itemType)).arg(QString::number(item->guid));
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export item as .d2i"), suggested, tr("Diablo II Item (*.d2i);;All files (*)"));
    if (fileName.isEmpty())
        return;

    // Serialize item using canonical writer to ensure header and byte ordering match ItemParser
    QByteArray out;
    QBuffer buffer(&out);
    buffer.open(QIODevice::WriteOnly);
    QDataStream ds(&buffer);
    ds.setByteOrder(QDataStream::LittleEndian);

    ItemsList list;
    list.append(item);
    ItemParser::writeItems(list, ds);
    buffer.close();

    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, tr("Export failed"), tr("Unable to open file for writing: %1").arg(fileName));
        return;
    }

    // Debug: log bitString and hex of written bytes to help diagnose mismatches
#ifndef QT_NO_DEBUG
    qDebug() << "Export .d2i: item->bitString (len)" << item->bitString.length() << item->bitString.left(128);
    qDebug() << "Export .d2i: written bytes hex:" << out.toHex(' ');
#endif

    qint64 written = f.write(out);
    f.close();
    if (written != out.size())
    {
        QMessageBox::warning(this, tr("Export incomplete"), tr("Wrote %1 bytes out of %2").arg(written).arg(out.size()));
        return;
    }
    QMessageBox::information(this, tr("Export successful"), tr("Item exported to %1").arg(fileName));
}

void ItemsPropertiesSplitter::importItemFromD2i()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import item from .d2i"), QString(), tr("Diablo II Item (*.d2i);;All files (*)"));
    if (fileName.isEmpty())
        return;

    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
    {
        ERROR_BOX(QString("Unable to open %1: %2").arg(fileName).arg(f.errorString()));
        return;
    }
    QByteArray bytes = f.readAll();
    f.close();

    QDataStream ds(bytes);
    ds.setByteOrder(QDataStream::LittleEndian);

    ItemInfo *item = ItemParser::parseItem(ds, bytes);
    if (!item)
    {
        ERROR_BOX(tr("Failed to parse item from file %1").arg(fileName));
        return;
    }

    // default placement: add to selected storage, otherwise use active ItemsViewerDialog tab
    ItemInfo *sel = selectedItem(false);
    int storage = -1;
    if (sel)
        storage = sel->storage;
    else
    {
        ItemsViewerDialog *viewer = qobject_cast<ItemsViewerDialog *>(this->window());
        if (!viewer)
        {
            // fallback to walking up the parent chain (older approach)
            QWidget *p = parentWidget();
            while (p) { viewer = qobject_cast<ItemsViewerDialog *>(p); if (viewer) break; p = p->parentWidget(); }
        }

        if (viewer)
        {
            int tab = viewer->tabWidget()->currentIndex();
            if (tab <= ItemsViewerDialog::InventoryIndex)
                storage = tab;
            else if (tab == ItemsViewerDialog::CubeIndex)
                storage = tab + 2;
            else
                storage = tab + 3;
        }
        else
            storage = Enums::ItemStorage::Inventory;
    }
    // ask how many copies to import
    bool ok = false;
    int copies = QInputDialog::getInt(this, tr("Import copies"), tr("Number of copies:"), 1, 1, 999, 1, &ok);
    if (!ok)
    {
        delete item;
        return;
    }

    item->storage = storage;
    item->row = -1; item->column = -1;
    item->hasChanged = true;

    int rows = ItemsViewerDialog::rowsInStorageAtIndex(storage);
    int cols = ItemsViewerDialog::colsInStorageAtIndex(storage);
    int successCount = 0;

    // Build placementItems list similar to createRuneAt so we fill row-major across the storage/page
    PlugyItemsSplitter *plugy = dynamic_cast<PlugyItemsSplitter *>(this);
    ItemsList placementItems;
    if (plugy) {
        quint32 page = plugy->currentPage();
        placementItems = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored, &page);
    } else {
        placementItems = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored);
    }

    // Row-major fill across the storage/page
    for (int r = 0; r < rows && successCount < copies; ++r) {
        for (int c = 0; c < cols && successCount < copies; ++c) {
            if (ItemDataBase::canStoreItemAt(r, c, item->itemType, placementItems, rows, cols)) {
                ItemInfo *newItem = new ItemInfo(*item);
                newItem->row = r; newItem->column = c; newItem->storage = storage;
                newItem->move(r, c, plugy ? plugy->currentPage() : newItem->plugyPage, true);
                newItem->location = Enums::ItemLocation::Stored;
                ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newItem) ? Enums::ItemStorage::Stash : newItem->storage);
                ReverseBitWriter::replaceValueInBitString(newItem->bitString, Enums::ItemOffsets::Location, newItem->location);
                addItemToList(newItem);
                placementItems.append(newItem);
                setCurrentStorageHasChanged();
                emit itemsChanged();
                ++successCount;
            }
        }
    }

    // Fallback placement for remaining copies: try storeItemIn or add without coords
    int remaining = copies - successCount;
    for (int i = 0; i < remaining; ++i)
    {
        ItemInfo *newItem = new ItemInfo(*item);
        newItem->hasChanged = true;
        newItem->storage = storage;
        newItem->row = -1; newItem->column = -1;

        bool stored = ItemDataBase::storeItemIn(newItem, static_cast<Enums::ItemStorage::ItemStorageEnum>(storage), rows, cols);
        if (!stored)
        {
            qDebug() << "[DEBUG] importItemFromD2i fallback: storeItemIn failed for storage" << storage << "for remaining copy" << i+1;
            addItemToList(newItem);
            WARNING_BOX(tr("Item imported but could not be placed in storage %1. Added to list without coordinates.").arg(storage));
        }
        else
        {
            addItemToList(newItem);
            ++successCount;
        }
    }

    delete item;
    if (successCount > 0)
        INFO_BOX(tr("Imported %1 item(s) from %2").arg(successCount).arg(fileName));
}

void ItemsPropertiesSplitter::importItemFromD2iAt()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;

    QPoint pos = action->data().toPoint();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import item from .d2i"), QString(), tr("Diablo II Item (*.d2i);;All files (*)"));
    if (fileName.isEmpty())
        return;

    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
    {
        ERROR_BOX(QString("Unable to open %1: %2").arg(fileName).arg(f.errorString()));
        return;
    }
    QByteArray bytes = f.readAll();
    f.close();

    QDataStream ds(bytes);
    ds.setByteOrder(QDataStream::LittleEndian);

    ItemInfo *item = ItemParser::parseItem(ds, bytes);
    if (!item)
    {
        ERROR_BOX(tr("Failed to parse item from file %1").arg(fileName));
        return;
    }

    // place at clicked coordinates inside current storage (try exact, then nearest free, then fallback)
    ItemInfo *sel2 = selectedItem(false);
    int storage2 = -1;
    if (sel2)
        storage2 = sel2->storage;
    else
    {
        ItemsViewerDialog *viewer = qobject_cast<ItemsViewerDialog *>(this->window());
        if (!viewer)
        {
            QWidget *p = parentWidget();
            while (p) { viewer = qobject_cast<ItemsViewerDialog *>(p); if (viewer) break; p = p->parentWidget(); }
        }

        if (viewer)
        {
            int tab = viewer->tabWidget()->currentIndex();
            if (tab <= ItemsViewerDialog::InventoryIndex)
                storage2 = tab;
            else if (tab == ItemsViewerDialog::CubeIndex)
                storage2 = tab + 2;
            else
                storage2 = tab + 3;
        }
        else
            storage2 = Enums::ItemStorage::Inventory;
    }
    item->storage = storage2;
    item->row = pos.x(); item->column = pos.y();
    item->hasChanged = true;

    int rows = ItemsViewerDialog::rowsInStorageAtIndex(storage2);
    int cols = ItemsViewerDialog::colsInStorageAtIndex(storage2);
    {
        int tabIdx = ItemsViewerDialog::tabIndexFromItemStorage(storage2);
        QString tabName = ItemsViewerDialog::tabNameAtIndex(tabIdx);
        qDebug() << "[DEBUG] importItemFromD2iAt: chosen storage=" << storage2 << "(" << tabName << ") pos=" << item->row << item->column << "rows=" << rows << "cols=" << cols;
    }

    // ask how many copies to import
    bool ok = false;
    int copies = QInputDialog::getInt(this, tr("Import copies"), tr("Number of copies:"), 1, 1, 999, 1, &ok);
    if (!ok)
    {
        delete item;
        return;
    }

    int successCount = 0;

    // Try nearest free slot (search border-by-border around requested coordinates)
    auto findNearestFree = [&](int desiredRow, int desiredCol, const ItemsList &items) -> QPair<int,int>
    {
        int maxRadius = qMax(rows, cols);
        for (int r = 0; r <= maxRadius; ++r)
        {
            for (int dy = -r; dy <= r; ++dy)
            {
                for (int dx = -r; dx <= r; ++dx)
                {
                    if (qAbs(dy) != r && qAbs(dx) != r) continue;
                    int rr = desiredRow + dy;
                    int cc = desiredCol + dx;
                    if (rr < 0 || cc < 0 || rr >= rows || cc >= cols) continue;
                    if (ItemDataBase::canStoreItemAt(rr, cc, item->itemType, items, rows, cols))
                        return qMakePair(rr, cc);
                }
            }
        }
        return qMakePair(-1, -1);
    };

    auto tryPlaceOne = [&](ItemInfo *it) -> bool {
        // exact placement
        bool canExactLocal = ItemDataBase::canStoreItemAt(it->row, it->column, it->itemType, _allItems, rows, cols);
        qDebug() << "[DEBUG] importItemFromD2iAt: exact placement check at" << it->row << it->column << "->" << canExactLocal;
        if (canExactLocal)
        {
            it->move(it->row, it->column, it->plugyPage);
            it->storage = storage2;
            ReverseBitWriter::replaceValueInBitString(it->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(it) ? Enums::ItemStorage::Stash : it->storage);
            ReverseBitWriter::replaceValueInBitString(it->bitString, Enums::ItemOffsets::Location, it->location);
            addItemToList(it);
            setCurrentStorageHasChanged();
            emit itemsChanged();
            return true;
        }

        // nearest
        PlugyItemsSplitter *plugyLocal = dynamic_cast<PlugyItemsSplitter *>(this);
        if (plugyLocal)
        {
            quint32 page = plugyLocal->currentPage();
            ItemsList itemsOnPage = ItemDataBase::itemsStoredIn(storage2, Enums::ItemLocation::Stored, &page);
            QPair<int,int> p = findNearestFree(it->row, it->column, itemsOnPage);
            qDebug() << "[DEBUG] importItemFromD2iAt: plugy itemsOnPage count=" << itemsOnPage.size() << "nearest->" << p.first << p.second;
            if (p.first >= 0)
            {
                it->move(p.first, p.second, page);
                it->storage = storage2;
                ReverseBitWriter::replaceValueInBitString(it->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(it) ? Enums::ItemStorage::Stash : it->storage);
                ReverseBitWriter::replaceValueInBitString(it->bitString, Enums::ItemOffsets::Location, it->location);
                addItemToList(it);
                setCurrentStorageHasChanged();
                emit itemsChanged();
                return true;
            }
        }
        else
        {
            ItemsList items = ItemDataBase::itemsStoredIn(storage2, Enums::ItemLocation::Stored);
            QPair<int,int> p = findNearestFree(it->row, it->column, items);
            qDebug() << "[DEBUG] importItemFromD2iAt: non-plugy items count=" << items.size() << "nearest->" << p.first << p.second;
            if (p.first >= 0)
            {
                it->move(p.first, p.second, it->plugyPage);
                it->storage = storage2;
                ReverseBitWriter::replaceValueInBitString(it->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(it) ? Enums::ItemStorage::Stash : it->storage);
                ReverseBitWriter::replaceValueInBitString(it->bitString, Enums::ItemOffsets::Location, it->location);
                addItemToList(it);
                setCurrentStorageHasChanged();
                emit itemsChanged();
                return true;
            }
        }

        // fallback storeItemIn
        if (ItemDataBase::storeItemIn(it, static_cast<Enums::ItemStorage::ItemStorageEnum>(storage2), rows, cols))
        {
            addItemToList(it);
            return true;
        }

        // couldn't place
        addItemToList(it);
        WARNING_BOX(tr("Item imported but could not be placed in storage %1 at coordinates (%2,%3). Added to list without coordinates.").arg(storage2).arg(it->row).arg(it->column));
        return false;
    };

    // loop copies
    for (int i = 0; i < copies; ++i)
    {
        ItemInfo *newItem = new ItemInfo(*item);
        newItem->hasChanged = true;
        newItem->row = item->row; newItem->column = item->column; newItem->storage = storage2;
        if (tryPlaceOne(newItem))
            ++successCount;
    }

    delete item;
    if (successCount > 0)
        INFO_BOX(tr("Imported %1 item(s) from %2").arg(successCount).arg(fileName));
    return;
}

void ItemsPropertiesSplitter::editD2iFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open .d2i file to edit"), QString(), tr("Diablo II Item (*.d2i);;All files (*)"));
    if (fileName.isEmpty())
        return;

    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
    {
        ERROR_BOX(QString("Unable to open %1: %2").arg(fileName).arg(f.errorString()));
        return;
    }
    QByteArray bytes = f.readAll();
    f.close();

    QDataStream ds(bytes);
    ds.setByteOrder(QDataStream::LittleEndian);

    ItemInfo *item = ItemParser::parseItem(ds, bytes);
    if (!item)
    {
        ERROR_BOX(tr("Failed to parse item from file %1").arg(fileName));
        return;
    }

    // Show property editor for the parsed item
    if (!_propertiesWidget)
        return;

    // Reuse the existing PropertyEditor by opening it and setting the item
    _propertiesWidget->openPropertyEditor();
    PropertyEditor *editor = _propertiesWidget->findChild<PropertyEditor *>();
    if (!editor)
    {
        ERROR_BOX(tr("Property editor not available"));
        delete item;
        return;
    }

    // Set the parsed item into the editor
    editor->setItem(item);

    // Connect to itemChanged signal to save back to file when user applies changes
    connect(editor, &PropertyEditor::itemChanged, this, [this, item, fileName]() {
        // Serialize the item back to bytes using ItemParser::writeItems
        QByteArray out;
        QBuffer buffer(&out);
        buffer.open(QIODevice::WriteOnly);
        QDataStream dsOut(&buffer);
        dsOut.setByteOrder(QDataStream::LittleEndian);

        ItemsList list;
        list.append(item);
        ItemParser::writeItems(list, dsOut);
        buffer.close();

        // Create a backup before overwriting
        QString backupName = fileName + ".bak";
        QFile backupFile(backupName);
        bool backupOk = true;
        if (!backupFile.exists()) {
            // write original bytes to backup
            QFile orig(fileName);
            if (orig.open(QIODevice::ReadOnly)) {
                QByteArray origData = orig.readAll();
                orig.close();
                if (!backupFile.open(QIODevice::WriteOnly)) {
                    backupOk = false;
                } else {
                    backupFile.write(origData);
                    backupFile.close();
                }
            } else {
                backupOk = false;
            }
        }

        if (!backupOk) {
            if (QUESTION_BOX_YESNO(tr("Failed to create backup '%1'. Continue without backup?").arg(backupName), QMessageBox::No) == QMessageBox::No) {
                WARNING_BOX(tr("Aborted save due to backup failure"));
                return;
            }
        }

        QFile outf(fileName);
        if (!outf.open(QIODevice::WriteOnly))
        {
            ERROR_BOX(tr("Unable to open %1 for writing: %2").arg(fileName).arg(outf.errorString()));
            return;
        }
        // out already contains JM+item bytes because writeItems writes header and bytes
        qint64 written = outf.write(out);
        outf.close();
        if (written != out.size())
        {
            WARNING_BOX(tr("Wrote %1 bytes out of %2").arg(written).arg(out.size()));
            return;
        }

        // Round-trip verification: attempt to parse the file we just wrote
        QFile verifyF(fileName);
        if (!verifyF.open(QIODevice::ReadOnly)) {
            ERROR_BOX(tr("Saved file but cannot open %1 for verification: %2").arg(fileName).arg(verifyF.errorString()));
            return;
        }
        QByteArray writtenBytes = verifyF.readAll();
        verifyF.close();

        QDataStream verifyDs(writtenBytes);
        verifyDs.setByteOrder(QDataStream::LittleEndian);
        ItemInfo *verifiedItem = ItemParser::parseItem(verifyDs, writtenBytes);
        if (!verifiedItem) {
            // Run the inspector script if available to provide diagnostics
            QString inspector = QCoreApplication::applicationDirPath() + "/../tools/inspect_d2i.py";
            QString inspectorOutput;
            if (QFile::exists(inspector)) {
                QProcess proc;
                proc.start("/usr/bin/env", QStringList() << "python3" << inspector << fileName);
                if (proc.waitForFinished(3000)) {
                    inspectorOutput = proc.readAllStandardOutput();
                    inspectorOutput += "\n";
                    inspectorOutput += proc.readAllStandardError();
                } else {
                    inspectorOutput = tr("Inspector timed out after 3s");
                }
            } else {
                inspectorOutput = tr("Inspector not found at %1").arg(inspector);
            }

            // Restore backup if available
            if (QFile::exists(backupName)) {
                QFile::remove(fileName);
                QFile::rename(backupName, fileName);
            }

            // Show diagnostics in a dialog
            QDialog dlg(this);
            dlg.setWindowTitle(tr("Verification failed - diagnostics"));
            QVBoxLayout *layout = new QVBoxLayout(&dlg);
            QTextEdit *out = new QTextEdit(&dlg);
            out->setReadOnly(true);
            out->setPlainText(inspectorOutput);
            layout->addWidget(out);
            QPushButton *closeBtn = new QPushButton(tr("Close"), &dlg);
            connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
            layout->addWidget(closeBtn);
            dlg.resize(800, 600);
            dlg.exec();

            ERROR_BOX(tr("Verification failed: written file cannot be parsed. Restored backup. See diagnostics for details."));
            return;
        }
        delete verifiedItem;

        // For detailed verification, re-parse the saved bytes into an ItemInfo and display properties
        QDataStream finalDs(writtenBytes);
        finalDs.setByteOrder(QDataStream::LittleEndian);
        ItemInfo *finalItem = ItemParser::parseItem(finalDs, writtenBytes);
        if (finalItem) {
            // Build a human-readable report of parsed properties
            QStringList lines;
            lines << tr("Parsed properties:");
            for (auto it = finalItem->props.constBegin(); it != finalItem->props.constEnd(); ++it) {
                ItemProperty *p = it.value();
                int id = it.key();
                ItemPropertyTxt *txt = ItemDataBase::Properties()->value(id);
                QString display = p->displayString.isEmpty() ? (txt ? QString::fromUtf8(txt->stat) : tr("Unknown(%1)").arg(id)) : p->displayString;
                lines << QString("ID=%1 param=%2 value=%3 -> %4").arg(id).arg(p->param).arg(p->value).arg(display);
            }

            // Show report in dialog
            QDialog reportDlg(this);
            reportDlg.setWindowTitle(tr("Verification report - %1").arg(fileName));
            QVBoxLayout *reportLayout = new QVBoxLayout(&reportDlg);
            QTextEdit *reportOut = new QTextEdit(&reportDlg);
            reportOut->setReadOnly(true);
            reportOut->setPlainText(lines.join('\n'));
            reportLayout->addWidget(reportOut);
            QPushButton *closeBtn2 = new QPushButton(tr("Close"), &reportDlg);
            connect(closeBtn2, &QPushButton::clicked, &reportDlg, &QDialog::accept);
            reportLayout->addWidget(closeBtn2);
            reportDlg.resize(700, 400);
            reportDlg.exec();

            delete finalItem;
        }

        INFO_BOX(tr("Saved edited .d2i to %1 and verification passed").arg(fileName));
    });
}

ItemInfo *ItemsPropertiesSplitter::selectedItem(bool showError /*= true*/)
{
    //QModelIndexList selectedIndexes = _itemsView->selectionModel()->selectedIndexes();
    ItemInfo *item = _itemsModel->itemAtIndex(_itemsView->selectionModel()->currentIndex());
    if (!item && showError)
        ERROR_BOX("TROLOLOL no item selection found");
    return item;
}

bool ItemsPropertiesSplitter::moveBetweenStashes()
{
    ItemInfo *item = selectedItem(false);
    if (sender() == _itemsView && !(item && qApp->mouseButtons() == Qt::LeftButton && qApp->keyboardModifiers() == Qt::AltModifier)) // Alt+Click
        return false;

    removeItemFromModel(item);
    _itemsView->selectionModel()->clearSelection();
    emit itemCountChanged(_itemsModel->itemCount());

    emit itemMovingBetweenStashes(item);
    return true;
}

void ItemsPropertiesSplitter::exportText()
{

}

void ItemsPropertiesSplitter::editItemProperties()
{
    ItemInfo *item = selectedItem();
    if (item) {
        _propertiesWidget->openPropertyEditor();
    }
}

void ItemsPropertiesSplitter::copyAllItemsBBCode(QAction *action)
{
    QStringList codes;
    foreach (ItemInfo *item, _itemsModel->items())
    {
        QString name = itemNameBBCode(item);
        if (!name.isEmpty())
            codes << itemBBCode(name, action->objectName());
    }
    if (!codes.isEmpty())
        qApp->clipboard()->setText(codes.join(QChar('\n')));
}

void ItemsPropertiesSplitter::copyItemBBCode(QAction *action)
{
    QAction *menuAction = qobject_cast<QMenu *>(sender())->menuAction();
    qApp->clipboard()->setText(itemBBCode(menuAction->objectName(), action->objectName()));
}

void ItemsPropertiesSplitter::disenchantSelectedItem()
{
    QAction *action = qobject_cast<QAction *>(sender());
    ItemInfo *item = selectedItem();
    if (!action || !item)
    {
        ERROR_BOX("TROLOLOL I can't disenchant the item");
        return;
    }

    ItemInfo *newItem = ItemDataBase::loadItemFromFile(action->objectName() == "signet" ? "signet_of_learning" : "arcane_shard");
    ItemInfo *newItemStored = disenchantItemIntoItem(item, newItem);
    delete newItem;

    if (newItemStored) // let's be safe
    {
        _itemsView->setCurrentIndex(_itemsModel->index(newItemStored->row, newItemStored->column));
        itemSelected(_itemsView->currentIndex(), false);
    }
}

void ItemsPropertiesSplitter::downgradeSelectedRune()
{
    QAction *senderAction = qobject_cast<QAction *>(sender());
    if (!senderAction)
    {
        ERROR_BOX("EPIC PHAIL WHEN DOWNGRADING RUNE");
        return;
    }

    quint8 newRuneCode = senderAction->data().toUInt();
    ItemInfo *item = selectedItem();
    item->hasChanged = true;
    item->itemType = QString("r%1").arg(newRuneCode, 2, 10, kZeroChar).toLatin1();
    for (int i = 1; i <= 2; ++i)
        ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Type + i*8, item->itemType.at(i));

    _propertiesWidget->showItem(item);
    emit itemsChanged();
}

void ItemsPropertiesSplitter::eatSelectedSignet()
{
    QAction *senderAction = qobject_cast<QAction *>(sender());
    if (!senderAction)
    {
        ERROR_BOX("EPIC PHAIL WHEN EATING SIGNET");
        return;
    }

    uint signetsToEat = senderAction->data().toUInt(), signetsEaten = CharacterInfo::instance().valueOfStatistic(Enums::CharacterStats::SignetsOfLearningEaten);
    const uint signetsMax = Enums::CharacterStats::SignetsOfLearningMax;
    if (signetsEaten + signetsToEat > signetsMax)
        if (QUESTION_BOX_YESNO(tr("You're going to eat %n signet(s), which is beyond the limit (%1) by %2.\nDo you really want to do it?", 0, signetsToEat).arg(signetsMax).arg(signetsEaten + signetsToEat - signetsMax), QMessageBox::No) == QMessageBox::No)
            return;
    deleteItem(selectedItem());
    emit signetsOfLearningEaten(signetsToEat);
}

void ItemsPropertiesSplitter::collectShrinesToVessel()
{
    ItemInfo *vesselItem = selectedItem();
    QByteArray shrineType = QByteArray::fromRawData(vesselItem->itemType.constData(), vesselItem->itemType.length() - 1);
    int shrines = 0;
    foreach (ItemInfo *item, _allItems)
    {
        if (item->itemType == shrineType)
        {
            ++shrines;
            performDeleteItem(item, false);
        }
    }

    if (shrines)
    {
        ItemProperty *vesselProp = vesselItem->props.value(Enums::ItemProperties::ShrineVesselCounter);
        if (vesselProp->value < 0)
            vesselProp->value = 0;
        vesselProp->value += shrines;

        ItemPropertyTxt *txtProp = ItemDataBase::Properties()->value(Enums::ItemProperties::ShrineVesselCounter);
        ReverseBitWriter::replaceValueInBitString(vesselItem->bitString, vesselProp->bitStringOffset, vesselProp->value + txtProp->add, txtProp->bits);

        vesselItem->hasChanged = true;
        _propertiesWidget->showItem(vesselItem);
        emit itemsChanged();

        INFO_BOX(tr("%n Shrine(s) inserted in the Vessel", 0, shrines));
    }
    else
        INFO_BOX(tr("No Shrines of selected type found"));
}

void ItemsPropertiesSplitter::extractShrinesFromVessel()
{
    ItemInfo *vesselItem = selectedItem();
    ItemProperty *vesselProp = vesselItem->props.value(Enums::ItemProperties::ShrineVesselCounter);
    int n = vesselProp->value;
    if (sender()->objectName() == "input")
    {
        bool ok;
        n = QInputDialog::getInt(this, tr("Shrine Vessel"), tr("Extract Shrines (1-%1)").arg(n), 1, 1, n, 1, &ok);
        if (!ok)
            return;
    }

    ItemInfo *shrine = ItemDataBase::loadItemFromFile("shrine");
    if (shrine->itemType.at(0) != vesselItem->itemType.at(0))
    {
        shrine->itemType = QByteArray(vesselItem->itemType.constData(), vesselItem->itemType.length() - 1); // e.g. B0+S => B0+
        ReverseBitWriter::replaceValueInBitString(shrine->bitString, Enums::ItemOffsets::Type, shrine->itemType.at(0)); // shrines differ only by the first type letter
    }

    bool notEnoughSpace = false;
    int stored = 0;
    for (; stored < n; ++stored)
    {
        ItemInfo *shrineCopy = new ItemInfo(*shrine);
        if (!storeItemInStorage(shrineCopy, vesselItem->storage))
        {
            notEnoughSpace = true;
            delete shrineCopy;
            break;
        }
        setCellSpanForItem(shrineCopy);
    }
    delete shrine;

    if (!stored)
    {
        ERROR_BOX(tr("No free space for Shrines in current storage"));
        return;
    }

    vesselProp->value -= stored;
    ItemPropertyTxt *txtProp = ItemDataBase::Properties()->value(Enums::ItemProperties::ShrineVesselCounter);
    ReverseBitWriter::replaceValueInBitString(vesselItem->bitString, vesselProp->bitStringOffset, vesselProp->value + txtProp->add, txtProp->bits);

    vesselItem->hasChanged = true;
    _propertiesWidget->showItem(vesselItem);
    emit itemsChanged();

    if (notEnoughSpace)
        ERROR_BOX(tr("Not enough space to store %1 Shrines in current storage, extracted only %2 pieces").arg(n, stored));
}

void ItemsPropertiesSplitter::depersonalize()
{
    ItemInfo *item = selectedItem();

    ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::IsPersonalized, 0);
    item->isPersonalized = false;

    ReverseBitWriter::remove(item->bitString, item->inscribedNameOffset, (item->inscribedName.length() + 1) * ItemParser::kInscribedNameCharacterLength); // also remove trailing \0
    item->inscribedName.clear();

    ReverseBitWriter::byteAlignBits(item->bitString);
    item->hasChanged = true;

    _propertiesWidget->showItem(item);
    emit itemsChanged();
}

void ItemsPropertiesSplitter::personalize()
{
    ItemInfo *item = selectedItem();
    QString personalizationName = CharacterInfo::instance().basicInfo.originalName;
    // remove all colors
    for (int i = 0; i < ColorsManager::colorCodes().size(); ++i)
        personalizationName.remove(ColorsManager::unicodeColorHeader() + ColorsManager::colorCodes().at(i));

    if (sender()->objectName() == "setName") // set arbitrary name
    {
        QD2CharRenamer renameWidget(personalizationName, false, this, false);
        renameWidget.setLineToolTip(tr("Colors don't work in personalized name"));
        if (!renameWidget.exec())
            return;
        personalizationName = renameWidget.name();
    }

    ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::IsPersonalized, 1);
    item->isPersonalized = true;

    item->inscribedName = personalizationName.toLatin1();
    const char *personalizationNameCstr = item->inscribedName.constData();
    QString personalizationNameBitString;
    // Build bitstring by appending each character's bits in order (non-destructive)
    for (quint8 i = 0; i < personalizationName.length() + 1; ++i) // trailing \0 must also be written
        personalizationNameBitString.append(binaryStringFromNumber(personalizationNameCstr[i], false, ItemParser::kInscribedNameCharacterLength));
    ReverseBitWriter::insert(item->bitString, item->inscribedNameOffset, personalizationNameBitString);

    ReverseBitWriter::byteAlignBits(item->bitString);
    item->hasChanged = true;

    _propertiesWidget->showItem(item);
    emit itemsChanged();
}

//void ItemsPropertiesSplitter::unsocketItem()
//{

//}

void ItemsPropertiesSplitter::createRuneAt()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) return;

    QPoint position = action->data().toPoint();
    if (position.x() < 0 || position.y() < 0) return;

    try {
        RuneCreationWidget *dialog = new RuneCreationWidget(this);
        if (!dialog) return;

        dialog->setItemPosition(position.x(), position.y());

        if (dialog->exec() == QDialog::Accepted)
        {
            int copies = dialog->copies();
            int successCount = 0;

            // Determine target storage: use selected item's storage; if none selected, use active ItemsViewerDialog tab
            ItemInfo *sel = selectedItem(false);
            int storage = -1;
            if (sel)
                storage = sel->storage;
            else
            {
                ItemsViewerDialog *viewer = nullptr;
                QWidget *p = parentWidget();
                while (p) { viewer = qobject_cast<ItemsViewerDialog *>(p); if (viewer) break; p = p->parentWidget(); }
                if (viewer)
                {
                    int tab = viewer->tabWidget()->currentIndex();
                    if (tab <= ItemsViewerDialog::InventoryIndex)
                        storage = tab;
                    else if (tab == ItemsViewerDialog::CubeIndex)
                        storage = tab + 2; // inverse of tabIndexFromItemStorage
                    else
                        storage = tab + 3;
                }
                else
                    storage = Enums::ItemStorage::Inventory;
            }

            PlugyItemsSplitter *plugy = dynamic_cast<PlugyItemsSplitter *>(this);
            int rows = ItemsViewerDialog::rowsInStorageAtIndex(storage);
            int cols = ItemsViewerDialog::colsInStorageAtIndex(storage);

            ItemsList placementItems;
            if (plugy) {
                quint32 page = plugy->currentPage();
                placementItems = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored, &page);
            } else {
                placementItems = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored);
            }

            // Row-major fill across the storage/page
            for (int r = 0; r < rows && successCount < copies; ++r) {
                for (int c = 0; c < cols && successCount < copies; ++c) {
                    if (ItemDataBase::canStoreItemAt(r, c, dialog->getCreatedRune()->itemType, placementItems, rows, cols)) {
                        ItemInfo *newRune = new ItemInfo(*dialog->getCreatedRune());
                        newRune->row = r; newRune->column = c; newRune->storage = storage;
                        newRune->move(r, c, plugy ? plugy->currentPage() : newRune->plugyPage, true);
                        newRune->location = Enums::ItemLocation::Stored;
                        ReverseBitWriter::replaceValueInBitString(newRune->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newRune) ? Enums::ItemStorage::Stash : newRune->storage);
                        ReverseBitWriter::replaceValueInBitString(newRune->bitString, Enums::ItemOffsets::Location, newRune->location);
                        addItemToList(newRune, true);
                        placementItems.append(newRune);
                        setCurrentStorageHasChanged();
                        emit itemsChanged();
                        ++successCount;
                    }
                }
            }

            // Fallback placement for remaining copies
            int remaining = copies - successCount;
            for (int i = 0; i < remaining; ++i) {
                ItemInfo *newRune = new ItemInfo(*dialog->getCreatedRune());
                newRune->row = dialog->requestedRow();
                newRune->column = dialog->requestedColumn();
                newRune->storage = storage;

                // try exact
                if (ItemDataBase::canStoreItemAt(newRune->row, newRune->column, newRune->itemType, _allItems, rows, cols)) {
                    newRune->move(newRune->row, newRune->column, newRune->plugyPage);
                    newRune->storage = storage;
                    ReverseBitWriter::replaceValueInBitString(newRune->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newRune) ? Enums::ItemStorage::Stash : newRune->storage);
                    ReverseBitWriter::replaceValueInBitString(newRune->bitString, Enums::ItemOffsets::Location, newRune->location);
                    addItemToList(newRune, true);
                    setCurrentStorageHasChanged();
                    emit itemsChanged();
                    ++successCount;
                    continue;
                }

                // nearest search
                bool placed = false;
                if (plugy) {
                    quint32 page = plugy->currentPage();
                    ItemsList itemsOnPage = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored, &page);
                    auto findNearestFree = [&](int desiredRow, int desiredCol, const ItemsList &items) -> QPair<int,int> {
                        int maxRadius = qMax(rows, cols);
                        for (int r = 0; r <= maxRadius; ++r) {
                            for (int dy = -r; dy <= r; ++dy) {
                                for (int dx = -r; dx <= r; ++dx) {
                                    if (qAbs(dy) != r && qAbs(dx) != r) continue;
                                    int rr = desiredRow + dy;
                                    int cc = desiredCol + dx;
                                    if (rr < 0 || cc < 0 || rr >= rows || cc >= cols) continue;
                                    if (ItemDataBase::canStoreItemAt(rr, cc, newRune->itemType, items, rows, cols))
                                        return qMakePair(rr, cc);
                                }
                            }
                        }
                        return qMakePair(-1, -1);
                    };

                    QPair<int,int> p = findNearestFree(dialog->requestedRow(), dialog->requestedColumn(), itemsOnPage);
                    if (p.first >= 0) {
                        newRune->move(p.first, p.second, page, true);
                        newRune->storage = storage;
                        newRune->location = Enums::ItemLocation::Stored;
                        ReverseBitWriter::replaceValueInBitString(newRune->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newRune) ? Enums::ItemStorage::Stash : newRune->storage);
                        ReverseBitWriter::replaceValueInBitString(newRune->bitString, Enums::ItemOffsets::Location, newRune->location);
                        addItemToList(newRune, true);
                        setCurrentStorageHasChanged();
                        emit itemsChanged();
                        ++successCount;
                        placed = true;
                    }
                } else {
                    ItemsList items = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored);
                    auto findNearestFree = [&](int desiredRow, int desiredCol, const ItemsList &itemsLocal) -> QPair<int,int> {
                        int maxRadius = qMax(rows, cols);
                        for (int r = 0; r <= maxRadius; ++r) {
                            for (int dy = -r; dy <= r; ++dy) {
                                for (int dx = -r; dx <= r; ++dx) {
                                    if (qAbs(dy) != r && qAbs(dx) != r) continue;
                                    int rr = desiredRow + dy;
                                    int cc = desiredCol + dx;
                                    if (rr < 0 || cc < 0 || rr >= rows || cc >= cols) continue;
                                    if (ItemDataBase::canStoreItemAt(rr, cc, newRune->itemType, itemsLocal, rows, cols))
                                        return qMakePair(rr, cc);
                                }
                            }
                        }
                        return qMakePair(-1, -1);
                    };

                    QPair<int,int> p = findNearestFree(dialog->requestedRow(), dialog->requestedColumn(), items);
                    if (p.first >= 0) {
                        newRune->move(p.first, p.second, 0, true);
                        newRune->storage = storage;
                        newRune->location = Enums::ItemLocation::Stored;
                        ReverseBitWriter::replaceValueInBitString(newRune->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newRune) ? Enums::ItemStorage::Stash : newRune->storage);
                        ReverseBitWriter::replaceValueInBitString(newRune->bitString, Enums::ItemOffsets::Location, newRune->location);
                        addItemToList(newRune, true);
                        setCurrentStorageHasChanged();
                        emit itemsChanged();
                        ++successCount;
                        placed = true;
                    }
                }

                if (!placed) {
                    if (ItemDataBase::storeItemIn(newRune, static_cast<Enums::ItemStorage::ItemStorageEnum>(storage), rows, cols)) {
                        addItemToList(newRune, true);
                        ++successCount;
                    } else {
                        addItemToList(newRune, true); // still add to list without coords
                    }
                }
            }

            if (successCount > 0)
                QMessageBox::information(this, tr("Rune(s) Created"), tr("Created %1 rune(s). Remember to save the character (Ctrl+S) to keep the changes!").arg(successCount));
        }

        dialog->deleteLater();
    } catch (...) {
        // Silently handle exceptions
    }
}

void ItemsPropertiesSplitter::createGemAt()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) {
        return;
    }
    
    QPoint position = action->data().toPoint();
    if (position.x() < 0 || position.y() < 0) {
        return;
    }
    
    try {
        GemCreationWidget *dialog = new GemCreationWidget(this);
        if (!dialog) return;
        
        dialog->setItemPosition(position.x(), position.y());
        
        if (dialog->exec() == QDialog::Accepted)
        {
            int copies = dialog->copies();
            int successCount = 0;

            // Determine target storage: use selected item's storage; if none selected, use active ItemsViewerDialog tab
            ItemInfo *sel = selectedItem(false);
            int storage = -1;
            if (sel)
                storage = sel->storage;
            else
            {
                ItemsViewerDialog *viewer = nullptr;
                QWidget *p = parentWidget();
                while (p) { viewer = qobject_cast<ItemsViewerDialog *>(p); if (viewer) break; p = p->parentWidget(); }
                if (viewer)
                {
                    int tab = viewer->tabWidget()->currentIndex();
                    if (tab <= ItemsViewerDialog::InventoryIndex)
                        storage = tab;
                    else if (tab == ItemsViewerDialog::CubeIndex)
                        storage = tab + 2; // inverse of tabIndexFromItemStorage
                    else
                        storage = tab + 3;
                }
                else
                    storage = Enums::ItemStorage::Inventory;
            }

            PlugyItemsSplitter *plugy = dynamic_cast<PlugyItemsSplitter *>(this);
            int rows = ItemsViewerDialog::rowsInStorageAtIndex(storage);
            int cols = ItemsViewerDialog::colsInStorageAtIndex(storage);

            ItemsList placementItems;
            if (plugy) {
                quint32 page = plugy->currentPage();
                placementItems = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored, &page);
            } else {
                placementItems = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored);
            }

            // Row-major fill across the storage/page
            for (int r = 0; r < rows && successCount < copies; ++r) {
                for (int c = 0; c < cols && successCount < copies; ++c) {
                    if (ItemDataBase::canStoreItemAt(r, c, dialog->getCreatedGem()->itemType, placementItems, rows, cols)) {
                        ItemInfo *newGem = new ItemInfo(*dialog->getCreatedGem());
                        newGem->row = r; newGem->column = c; newGem->storage = storage;
                        newGem->move(r, c, plugy ? plugy->currentPage() : newGem->plugyPage, true);
                        newGem->location = Enums::ItemLocation::Stored;
                        ReverseBitWriter::replaceValueInBitString(newGem->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newGem) ? Enums::ItemStorage::Stash : newGem->storage);
                        ReverseBitWriter::replaceValueInBitString(newGem->bitString, Enums::ItemOffsets::Location, newGem->location);
                        addItemToList(newGem, true);
                        placementItems.append(newGem);
                        setCurrentStorageHasChanged();
                        emit itemsChanged();
                        ++successCount;
                    }
                }
            }

            // Fallback placement for remaining copies
            int remaining = copies - successCount;
            for (int i = 0; i < remaining; ++i) {
                ItemInfo *newGem = new ItemInfo(*dialog->getCreatedGem());
                newGem->row = dialog->requestedRow();
                newGem->column = dialog->requestedColumn();
                newGem->storage = storage;

                // try exact
                if (ItemDataBase::canStoreItemAt(newGem->row, newGem->column, newGem->itemType, _allItems, rows, cols)) {
                    newGem->move(newGem->row, newGem->column, newGem->plugyPage);
                    newGem->storage = storage;
                    ReverseBitWriter::replaceValueInBitString(newGem->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newGem) ? Enums::ItemStorage::Stash : newGem->storage);
                    ReverseBitWriter::replaceValueInBitString(newGem->bitString, Enums::ItemOffsets::Location, newGem->location);
                    addItemToList(newGem, true);
                    setCurrentStorageHasChanged();
                    emit itemsChanged();
                    ++successCount;
                    continue;
                }

                // nearest search
                bool placed = false;
                if (plugy) {
                    quint32 page = plugy->currentPage();
                    ItemsList itemsOnPage = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored, &page);
                    auto findNearestFree = [&](int desiredRow, int desiredCol, const ItemsList &items) -> QPair<int,int> {
                        int maxRadius = qMax(rows, cols);
                        for (int r = 0; r <= maxRadius; ++r) {
                            for (int dy = -r; dy <= r; ++dy) {
                                for (int dx = -r; dx <= r; ++dx) {
                                    if (qAbs(dy) != r && qAbs(dx) != r) continue;
                                    int rr = desiredRow + dy;
                                    int cc = desiredCol + dx;
                                    if (rr < 0 || cc < 0 || rr >= rows || cc >= cols) continue;
                                    if (ItemDataBase::canStoreItemAt(rr, cc, newGem->itemType, items, rows, cols))
                                        return qMakePair(rr, cc);
                                }
                            }
                        }
                        return qMakePair(-1, -1);
                    };

                    QPair<int,int> p = findNearestFree(dialog->requestedRow(), dialog->requestedColumn(), itemsOnPage);
                    if (p.first >= 0) {
                        newGem->move(p.first, p.second, page, true);
                        newGem->storage = storage;
                        newGem->location = Enums::ItemLocation::Stored;
                        ReverseBitWriter::replaceValueInBitString(newGem->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newGem) ? Enums::ItemStorage::Stash : newGem->storage);
                        ReverseBitWriter::replaceValueInBitString(newGem->bitString, Enums::ItemOffsets::Location, newGem->location);
                        addItemToList(newGem, true);
                        setCurrentStorageHasChanged();
                        emit itemsChanged();
                        ++successCount;
                        placed = true;
                    }
                } else {
                    ItemsList items = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored);
                    auto findNearestFree = [&](int desiredRow, int desiredCol, const ItemsList &itemsLocal) -> QPair<int,int> {
                        int maxRadius = qMax(rows, cols);
                        for (int r = 0; r <= maxRadius; ++r) {
                            for (int dy = -r; dy <= r; ++dy) {
                                for (int dx = -r; dx <= r; ++dx) {
                                    if (qAbs(dy) != r && qAbs(dx) != r) continue;
                                    int rr = desiredRow + dy;
                                    int cc = desiredCol + dx;
                                    if (rr < 0 || cc < 0 || rr >= rows || cc >= cols) continue;
                                    if (ItemDataBase::canStoreItemAt(rr, cc, newGem->itemType, itemsLocal, rows, cols))
                                        return qMakePair(rr, cc);
                                }
                            }
                        }
                        return qMakePair(-1, -1);
                    };

                    QPair<int,int> p = findNearestFree(dialog->requestedRow(), dialog->requestedColumn(), items);
                    if (p.first >= 0) {
                        newGem->move(p.first, p.second, 0, true);
                        newGem->storage = storage;
                        newGem->location = Enums::ItemLocation::Stored;
                        ReverseBitWriter::replaceValueInBitString(newGem->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newGem) ? Enums::ItemStorage::Stash : newGem->storage);
                        ReverseBitWriter::replaceValueInBitString(newGem->bitString, Enums::ItemOffsets::Location, newGem->location);
                        addItemToList(newGem, true);
                        setCurrentStorageHasChanged();
                        emit itemsChanged();
                        ++successCount;
                        placed = true;
                    }
                }

                if (!placed) {
                    if (storeItemInStorage(newGem, storage, true)) {
                        ++successCount;
                    } else {
                        addItemToList(newGem, true); // still add to list without coords
                    }
                }
            }

            if (successCount > 0)
                QMessageBox::information(this, tr("Gem(s) Created"), tr("Created %1 gem(s). Remember to save the character (Ctrl+S) to keep the changes!").arg(successCount));
        }

        dialog->deleteLater();
    } catch (...) {
        // Silently handle exceptions
    }
}

void ItemsPropertiesSplitter::createOilAt()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) return;

    QPoint position = action->data().toPoint();
    if (position.x() < 0 || position.y() < 0) return;

    try {
        OilCreationWidget *dialog = new OilCreationWidget(this);
        if (!dialog) return;

        dialog->setItemPosition(position.x(), position.y());

        if (dialog->exec() == QDialog::Accepted)
        {
            int copies = dialog->copies();
            int successCount = 0;

            // Determine target storage: use selected item's storage; if none selected, use active ItemsViewerDialog tab
            ItemInfo *sel = selectedItem(false);
            int storage = -1;
            if (sel)
                storage = sel->storage;
            else
            {
                ItemsViewerDialog *viewer = nullptr;
                QWidget *p = parentWidget();
                while (p) { viewer = qobject_cast<ItemsViewerDialog *>(p); if (viewer) break; p = p->parentWidget(); }
                if (viewer)
                {
                    int tab = viewer->tabWidget()->currentIndex();
                    if (tab <= ItemsViewerDialog::InventoryIndex)
                        storage = tab;
                    else if (tab == ItemsViewerDialog::CubeIndex)
                        storage = tab + 2; // inverse of tabIndexFromItemStorage
                    else
                        storage = tab + 3;
                }
                else
                    storage = Enums::ItemStorage::Inventory;
            }

            PlugyItemsSplitter *plugy = dynamic_cast<PlugyItemsSplitter *>(this);
            int rows = ItemsViewerDialog::rowsInStorageAtIndex(storage);
            int cols = ItemsViewerDialog::colsInStorageAtIndex(storage);

            ItemsList placementItems;
            if (plugy) {
                quint32 page = plugy->currentPage();
                placementItems = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored, &page);
            } else {
                placementItems = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored);
            }

            // Row-major fill across the storage/page
            for (int r = 0; r < rows && successCount < copies; ++r) {
                for (int c = 0; c < cols && successCount < copies; ++c) {
                    if (ItemDataBase::canStoreItemAt(r, c, dialog->getCreatedOil()->itemType, placementItems, rows, cols)) {
                        ItemInfo *newOil = new ItemInfo(*dialog->getCreatedOil());
                        newOil->row = r; newOil->column = c; newOil->storage = storage;
                        newOil->move(r, c, plugy ? plugy->currentPage() : newOil->plugyPage, true);
                        newOil->location = Enums::ItemLocation::Stored;
                        ReverseBitWriter::replaceValueInBitString(newOil->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newOil) ? Enums::ItemStorage::Stash : newOil->storage);
                        ReverseBitWriter::replaceValueInBitString(newOil->bitString, Enums::ItemOffsets::Location, newOil->location);
                        addItemToList(newOil, true);
                        placementItems.append(newOil);
                        setCurrentStorageHasChanged();
                        emit itemsChanged();
                        ++successCount;
                    }
                }
            }

            // Fallback placement for remaining copies
            int remaining = copies - successCount;
            for (int i = 0; i < remaining; ++i) {
                ItemInfo *newOil = new ItemInfo(*dialog->getCreatedOil());
                newOil->row = dialog->requestedRow();
                newOil->column = dialog->requestedColumn();
                newOil->storage = storage;

                // try exact
                if (ItemDataBase::canStoreItemAt(newOil->row, newOil->column, newOil->itemType, _allItems, rows, cols)) {
                    newOil->move(newOil->row, newOil->column, newOil->plugyPage);
                    newOil->storage = storage;
                    ReverseBitWriter::replaceValueInBitString(newOil->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newOil) ? Enums::ItemStorage::Stash : newOil->storage);
                    ReverseBitWriter::replaceValueInBitString(newOil->bitString, Enums::ItemOffsets::Location, newOil->location);
                    addItemToList(newOil, true);
                    setCurrentStorageHasChanged();
                    emit itemsChanged();
                    ++successCount;
                    continue;
                }

                // nearest search
                bool placed = false;
                if (plugy) {
                    quint32 page = plugy->currentPage();
                    ItemsList itemsOnPage = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored, &page);
                    auto findNearestFree = [&](int desiredRow, int desiredCol, const ItemsList &items) -> QPair<int,int> {
                        int maxRadius = qMax(rows, cols);
                        for (int r = 0; r <= maxRadius; ++r) {
                            for (int dy = -r; dy <= r; ++dy) {
                                for (int dx = -r; dx <= r; ++dx) {
                                    if (qAbs(dy) != r && qAbs(dx) != r) continue;
                                    int rr = desiredRow + dy;
                                    int cc = desiredCol + dx;
                                    if (rr < 0 || cc < 0 || rr >= rows || cc >= cols) continue;
                                    if (ItemDataBase::canStoreItemAt(rr, cc, newOil->itemType, items, rows, cols))
                                        return qMakePair(rr, cc);
                                }
                            }
                        }
                        return qMakePair(-1, -1);
                    };

                    QPair<int,int> p = findNearestFree(dialog->requestedRow(), dialog->requestedColumn(), itemsOnPage);
                    if (p.first >= 0) {
                        newOil->move(p.first, p.second, page, true);
                        newOil->storage = storage;
                        newOil->location = Enums::ItemLocation::Stored;
                        ReverseBitWriter::replaceValueInBitString(newOil->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newOil) ? Enums::ItemStorage::Stash : newOil->storage);
                        ReverseBitWriter::replaceValueInBitString(newOil->bitString, Enums::ItemOffsets::Location, newOil->location);
                        addItemToList(newOil, true);
                        setCurrentStorageHasChanged();
                        emit itemsChanged();
                        ++successCount;
                        placed = true;
                    }
                } else {
                    ItemsList items = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored);
                    auto findNearestFree = [&](int desiredRow, int desiredCol, const ItemsList &itemsLocal) -> QPair<int,int> {
                        int maxRadius = qMax(rows, cols);
                        for (int r = 0; r <= maxRadius; ++r) {
                            for (int dy = -r; dy <= r; ++dy) {
                                for (int dx = -r; dx <= r; ++dx) {
                                    if (qAbs(dy) != r && qAbs(dx) != r) continue;
                                    int rr = desiredRow + dy;
                                    int cc = desiredCol + dx;
                                    if (rr < 0 || cc < 0 || rr >= rows || cc >= cols) continue;
                                    if (ItemDataBase::canStoreItemAt(rr, cc, newOil->itemType, itemsLocal, rows, cols))
                                        return qMakePair(rr, cc);
                                }
                            }
                        }
                        return qMakePair(-1, -1);
                    };

                    QPair<int,int> p = findNearestFree(dialog->requestedRow(), dialog->requestedColumn(), items);
                    if (p.first >= 0) {
                        newOil->move(p.first, p.second, 0, true);
                        newOil->storage = storage;
                        newOil->location = Enums::ItemLocation::Stored;
                        ReverseBitWriter::replaceValueInBitString(newOil->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newOil) ? Enums::ItemStorage::Stash : newOil->storage);
                        ReverseBitWriter::replaceValueInBitString(newOil->bitString, Enums::ItemOffsets::Location, newOil->location);
                        addItemToList(newOil, true);
                        setCurrentStorageHasChanged();
                        emit itemsChanged();
                        ++successCount;
                        placed = true;
                    }
                }

                if (!placed) {
                    if (ItemDataBase::storeItemIn(newOil, static_cast<Enums::ItemStorage::ItemStorageEnum>(storage), rows, cols)) {
                        addItemToList(newOil, true);
                        ++successCount;
                    } else {
                        addItemToList(newOil, true); // still add to list without coords
                    }
                }
            }

            if (successCount > 0)
                QMessageBox::information(this, tr("Oil(s) Created"), tr("Created %1 oil(s). Remember to save the character (Ctrl+S) to keep the changes!").arg(successCount));
        }

        dialog->deleteLater();
    } catch (...) {
        // Silently handle exceptions
    }
}

void ItemsPropertiesSplitter::createArcaneCrystalAt()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) {
        return;
    }
    
    QPoint position = action->data().toPoint();
    if (position.x() < 0 || position.y() < 0) {
        return;
    }
    
    try {
        ArcaneCrystalCreationWidget *dialog = new ArcaneCrystalCreationWidget(this);
        if (!dialog) {
            qWarning() << "Failed to create ArcaneCrystalCreationWidget";
            return;
        }
        
        dialog->setStartPosition(position.x(), position.y());
        
        if (dialog->exec() == QDialog::Accepted)
        {
            QList<ItemInfo*> newCrystals = dialog->getCreatedCrystals();
            if (!newCrystals.isEmpty())
            {
                qDebug() << "Creating" << newCrystals.size() << "Arcane Crystals";
                
                int successCount = 0;
                QList<ItemInfo*> successfulCrystals;
                
                // Determine target storage: use selected item's storage; if none selected, use active ItemsViewerDialog tab
                ItemInfo *sel = selectedItem(false);
                int storage = -1;
                if (sel)
                    storage = sel->storage;
                else
                {
                    ItemsViewerDialog *viewer = nullptr;
                    QWidget *p = parentWidget();
                    while (p) { viewer = qobject_cast<ItemsViewerDialog *>(p); if (viewer) break; p = p->parentWidget(); }
                    if (viewer)
                    {
                        int tab = viewer->tabWidget()->currentIndex();
                        if (tab <= ItemsViewerDialog::InventoryIndex)
                            storage = tab;
                        else if (tab == ItemsViewerDialog::CubeIndex)
                            storage = tab + 2;
                        else
                            storage = tab + 3;
                    }
                    else
                        storage = Enums::ItemStorage::Inventory;
                }

                PlugyItemsSplitter *plugy = dynamic_cast<PlugyItemsSplitter *>(this);
                int rows = ItemsViewerDialog::rowsInStorageAtIndex(storage);
                int cols = ItemsViewerDialog::colsInStorageAtIndex(storage);

                auto findNearestFree = [&](int desiredRow, int desiredCol, quint32 plugyPage, ItemsList items, const QByteArray &storeItemType) -> QPair<int,int>
                {
                    int maxRadius = qMax(rows, cols);
                    for (int r = 0; r <= maxRadius; ++r)
                    {
                        for (int dy = -r; dy <= r; ++dy)
                        {
                            for (int dx = -r; dx <= r; ++dx)
                            {
                                if (qAbs(dy) != r && qAbs(dx) != r) continue;
                                int rr = desiredRow + dy;
                                int cc = desiredCol + dx;
                                if (rr < 0 || cc < 0 || rr >= rows || cc >= cols) continue;
                                if (ItemDataBase::canStoreItemAt(rr, cc, storeItemType, items, rows, cols))
                                    return qMakePair(rr, cc);
                            }
                        }
                    }
                    return qMakePair(-1, -1);
                };

                for (int i = 0; i < newCrystals.size(); i++) {
                    ItemInfo* crystal = newCrystals.at(i);
                    if (!crystal) continue;
                    qDebug() << "[DEBUG] Storing Arcane Crystal" << (i+1) << "initial pos:" << crystal->row << crystal->column << "target storage:" << storage;

                    bool placed = false;
                    if (plugy)
                    {
                        // guard: ensure plugy is valid and currentPage() accessible
                        quint32 page = 0;
                        try {
                            page = plugy->currentPage();
                        } catch (...) { qWarning() << "[DEBUG] plugy->currentPage() threw"; }

                        qDebug() << "[DEBUG] PlugY path: page=" << page;
                        ItemsList itemsOnPage = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored, &page);
                        qDebug() << "[DEBUG] itemsOnPage count:" << itemsOnPage.size();
                        QPair<int,int> p = findNearestFree(crystal->row, crystal->column, page, itemsOnPage, crystal->itemType);
                        qDebug() << "[DEBUG] findNearestFree returned:" << p.first << p.second;
                        if (p.first >= 0)
                        {
                            crystal->move(p.first, p.second, page, true);
                            crystal->storage = storage;
                            crystal->location = Enums::ItemLocation::Stored;
                            ReverseBitWriter::replaceValueInBitString(crystal->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(crystal) ? Enums::ItemStorage::Stash : crystal->storage);
                            ReverseBitWriter::replaceValueInBitString(crystal->bitString, Enums::ItemOffsets::Location, crystal->location);
                            addItemToList(crystal, true);
                            placed = true;
                        }
                        else
                        {
                            qDebug() << "[DEBUG] PlugY branch: no nearest free slot found on page" << page;
                        }
                    }
                    else
                    {
                        ItemsList items = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored);
                        qDebug() << "[DEBUG] Non-PlugY path: items count:" << items.size();
                        QPair<int,int> p = findNearestFree(crystal->row, crystal->column, 0, items, crystal->itemType);
                        qDebug() << "[DEBUG] findNearestFree returned:" << p.first << p.second;
                        if (p.first >= 0)
                        {
                            crystal->move(p.first, p.second, 0, true);
                            crystal->storage = storage;
                            crystal->location = Enums::ItemLocation::Stored;
                            ReverseBitWriter::replaceValueInBitString(crystal->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(crystal) ? Enums::ItemStorage::Stash : crystal->storage);
                            ReverseBitWriter::replaceValueInBitString(crystal->bitString, Enums::ItemOffsets::Location, crystal->location);
                            addItemToList(crystal, true);
                            placed = true;
                        }
                        else
                        {
                            qDebug() << "[DEBUG] Non-PlugY branch: no nearest free slot found";
                        }
                    }

                    if (!placed)
                    {
                        if (storeItemInStorage(crystal, storage, true))
                        {
                            successCount++;
                            successfulCrystals.append(crystal);
                        }
                        else
                        {
                            qWarning() << "Failed to store Arcane Crystal at position" << crystal->row << crystal->column;
                            delete crystal;
                        }
                    }
                    else
                    {
                        successCount++;
                        successfulCrystals.append(crystal);
                    }
                }
                
                if (successCount > 0) {
                    // Mark as changed for save
                    setCurrentStorageHasChanged();
                    
                    // Emit signal to notify that items have changed
                    emit itemsChanged();
                    
                    qDebug() << "Added" << successCount << "Arcane Crystals to character items. Total items now:" 
                             << CharacterInfo::instance().items.character.size();
                    
                    // Prompt user to save immediately
                    QString message;
                    if (successCount == 1) {
                        message = tr("Arcane Crystal created successfully at position (%1, %2).\n\n"
                                   "Remember to save the character (Ctrl+S) to keep the changes!")
                                .arg(newCrystals.first()->row + 1).arg(newCrystals.first()->column + 1);
                    } else {
                        message = tr("%1 Arcane Crystals created successfully starting at position (%2, %3).\n\n"
                                   "Remember to save the character (Ctrl+S) to keep the changes!")
                                .arg(successCount).arg(position.x() + 1).arg(position.y() + 1);
                    }
                    
                    QMessageBox::information(this, tr("Arcane Crystals Created"), message);
                    
                    // Show the first crystal in the properties viewer
                    if (!successfulCrystals.isEmpty()) {
                        showItem(successfulCrystals.first());
                    }
                } else {
                    QMessageBox::warning(this, tr("Storage Failed"), 
                        tr("Failed to store any Arcane Crystals in inventory!"));
                }
            }
        }
        
        dialog->deleteLater();
    } catch (...) {
        // Silently handle exceptions
    }
}

void ItemsPropertiesSplitter::createShrineAt()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) return;

    QPoint position = action->data().toPoint();
    if (position.x() < 0 || position.y() < 0) return;

    // Gather shrine types from misc.tsv (try multiple candidate paths)
    QString miscPath;
    // 1) generated locale path under resources
    QString genPath = QString("%1/txt_parser/generated/%2/misc.tsv").arg(LanguageManager::instance().resourcesPath).arg(LanguageManager::instance().modLocalization());
    if (QFile::exists(genPath)) miscPath = genPath;
    // 2) common path under resources
    if (miscPath.isEmpty()) {
        QString p = QString("%1/txt/misc.tsv").arg(LanguageManager::instance().resourcesPath);
        if (QFile::exists(p)) miscPath = p;
    }
    // 3) repo utils fallback
    if (miscPath.isEmpty()) {
        // Try relative to application dir (useful when running from build/)
        QString appDir = QCoreApplication::applicationDirPath();
        QString repoPath1 = QDir(appDir).absoluteFilePath("../utils/txt_parser/txt/misc.tsv");
        QString repoPath2 = QDir(appDir).absoluteFilePath("../../utils/txt_parser/txt/misc.tsv");
        if (QFile::exists(repoPath1)) miscPath = repoPath1;
        else if (QFile::exists(repoPath2)) miscPath = repoPath2;
    }
    // 4) resource path via ResourcePathManager (rare)
    if (miscPath.isEmpty()) {
        QString p = ResourcePathManager::dataPathForFileName("txt/misc.tsv");
        if (QFile::exists(p)) miscPath = p;
    }

    QFile f(miscPath);
    QList<QPair<QString, QString>> shrineEntries; // pair<displayName, code>
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&f);
        QString header = in.readLine(); // skip header
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (line.trimmed().isEmpty()) continue;
            QStringList cols = line.split('\t');
            if (cols.size() < 6) continue;
            QString name = cols.at(0).trimmed();
            QString code = cols.at(5).trimmed();
            if (name.isEmpty() || code.isEmpty()) continue;
            // filter by lines that contain 'Shrine' in name
            if (name.contains("Shrine", Qt::CaseInsensitive))
                shrineEntries.append(qMakePair(name, code));
        }
        f.close();
    }

    if (shrineEntries.isEmpty())
    {
        ERROR_BOX(tr("No shrine types found in misc.tsv"));
        return;
    }

    // Let user pick shrine type (simple combo)
    QStringList options;
    for (auto &p : shrineEntries)
        options << QString("%1 (%2)").arg(p.first).arg(p.second);

    bool ok = false;
    QString choice = QInputDialog::getItem(this, tr("Select Shrine type"), tr("Shrine:"), options, 0, false, &ok);
    if (!ok || choice.isEmpty()) return;

    int idx = options.indexOf(choice);
    if (idx < 0) return;
    QString chosenCode = shrineEntries.at(idx).second;

    // ask quantity
    int copies = QInputDialog::getInt(this, tr("Number of Shrines"), tr("Number of shrines to create:"), 1, 1, 999, 1, &ok);
    if (!ok) return;

    // Load shrine template - prefer specific shrine.d2i name if exists, otherwise try generic "shrine"
    QString shrineTemplate = QString("shrine");
    ItemInfo *templateItem = ItemDataBase::loadItemFromFile(shrineTemplate);
    if (!templateItem)
    {
        ERROR_BOX(tr("Failed to load shrine template (shrine.d2i)"));
        return;
    }

    // override template itemType to chosen code
    templateItem->itemType = chosenCode.toLatin1();
    ReverseBitWriter::replaceValueInBitString(templateItem->bitString, Enums::ItemOffsets::Type, templateItem->itemType.at(0));

    // Prepare storage resolution similar to other create* handlers
    ItemInfo *sel = selectedItem(false);
    int storage = -1;
    if (sel)
        storage = sel->storage;
    else
    {
        ItemsViewerDialog *viewer = nullptr;
        QWidget *p = parentWidget();
        while (p) { viewer = qobject_cast<ItemsViewerDialog *>(p); if (viewer) break; p = p->parentWidget(); }
        if (viewer)
        {
            int tab = viewer->tabWidget()->currentIndex();
            if (tab <= ItemsViewerDialog::InventoryIndex) storage = tab;
            else if (tab == ItemsViewerDialog::CubeIndex) storage = tab + 2;
            else storage = tab + 3;
        }
        else storage = Enums::ItemStorage::Inventory;
    }

    PlugyItemsSplitter *plugy = dynamic_cast<PlugyItemsSplitter *>(this);
    int rows = ItemsViewerDialog::rowsInStorageAtIndex(storage);
    int cols = ItemsViewerDialog::colsInStorageAtIndex(storage);

    auto findNearestFree = [&](int desiredRow, int desiredCol, const ItemsList &items, const QByteArray &storeItemType) -> QPair<int,int>
    {
        int maxRadius = qMax(rows, cols);
        for (int r = 0; r <= maxRadius; ++r)
        {
            for (int dy = -r; dy <= r; ++dy)
            {
                for (int dx = -r; dx <= r; ++dx)
                {
                    if (qAbs(dy) != r && qAbs(dx) != r) continue;
                    int rr = desiredRow + dy;
                    int cc = desiredCol + dx;
                    if (rr < 0 || cc < 0 || rr >= rows || cc >= cols) continue;
                    if (ItemDataBase::canStoreItemAt(rr, cc, storeItemType, items, rows, cols))
                        return qMakePair(rr, cc);
                }
            }
        }
        return qMakePair(-1, -1);
    };

    int success = 0;
    for (int i = 0; i < copies; ++i)
    {
        ItemInfo *it = new ItemInfo(*templateItem);
        it->hasChanged = true;
        it->row = position.x(); it->column = position.y(); it->storage = storage;

        bool placed = false;
        if (plugy)
        {
            quint32 page = plugy->currentPage();
            ItemsList itemsOnPage = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored, &page);
            QPair<int,int> p = findNearestFree(it->row, it->column, itemsOnPage, it->itemType);
            if (p.first >= 0)
            {
                it->move(p.first, p.second, page);
                it->storage = storage;
                ReverseBitWriter::replaceValueInBitString(it->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(it) ? Enums::ItemStorage::Stash : it->storage);
                ReverseBitWriter::replaceValueInBitString(it->bitString, Enums::ItemOffsets::Location, it->location);
                addItemToList(it);
                placed = true;
            }
        }
        else
        {
            ItemsList items = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored);
            QPair<int,int> p = findNearestFree(it->row, it->column, items, it->itemType);
            if (p.first >= 0)
            {
                it->move(p.first, p.second, it->plugyPage);
                it->storage = storage;
                ReverseBitWriter::replaceValueInBitString(it->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(it) ? Enums::ItemStorage::Stash : it->storage);
                ReverseBitWriter::replaceValueInBitString(it->bitString, Enums::ItemOffsets::Location, it->location);
                addItemToList(it);
                placed = true;
            }
        }

        if (!placed)
        {
            if (!storeItemInStorage(it, storage, true))
            {
                addItemToList(it);
                WARNING_BOX(tr("Shrine created but could not be placed in storage %1. Added to list without coordinates.").arg(storage));
            }
            else ++success;
        }
        else ++success;
    }

    delete templateItem;
    if (success > 0)
    {
        setCurrentStorageHasChanged();
        emit itemsChanged();
        INFO_BOX(tr("Created %1 shrine(s)").arg(success));
    }
}

void ItemsPropertiesSplitter::createMysticOrbAt()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) return;

    QPoint position = action->data().toPoint();
    if (position.x() < 0 || position.y() < 0) return;

    try {
        MysticOrbCreationWidget *dialog = new MysticOrbCreationWidget(this);
        if (!dialog) return;

        dialog->setItemPosition(position.x(), position.y());

        if (dialog->exec() == QDialog::Accepted)
        {
            int copies = dialog->copies();
            int successCount = 0;

            ItemInfo *sel = selectedItem(false);
            int storage = -1;
            if (sel)
                storage = sel->storage;
            else
            {
                ItemsViewerDialog *viewer = nullptr;
                QWidget *p = parentWidget();
                while (p) { viewer = qobject_cast<ItemsViewerDialog *>(p); if (viewer) break; p = p->parentWidget(); }
                if (viewer)
                {
                    int tab = viewer->tabWidget()->currentIndex();
                    if (tab <= ItemsViewerDialog::InventoryIndex)
                        storage = tab;
                    else if (tab == ItemsViewerDialog::CubeIndex)
                        storage = tab + 2;
                    else
                        storage = tab + 3;
                }
                else
                    storage = Enums::ItemStorage::Inventory;
            }

            PlugyItemsSplitter *plugy = dynamic_cast<PlugyItemsSplitter *>(this);
            int rows = ItemsViewerDialog::rowsInStorageAtIndex(storage);
            int cols = ItemsViewerDialog::colsInStorageAtIndex(storage);

            ItemsList placementItems;
            if (plugy) { quint32 page = plugy->currentPage(); placementItems = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored, &page); }
            else placementItems = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored);

            for (int r = 0; r < rows && successCount < copies; ++r) {
                for (int c = 0; c < cols && successCount < copies; ++c) {
                    if (ItemDataBase::canStoreItemAt(r, c, dialog->getCreatedOrb()->itemType, placementItems, rows, cols)) {
                        ItemInfo *newOrb = new ItemInfo(*dialog->getCreatedOrb());
                        newOrb->row = r; newOrb->column = c; newOrb->storage = storage;
                        newOrb->move(r, c, plugy ? plugy->currentPage() : newOrb->plugyPage, true);
                        newOrb->location = Enums::ItemLocation::Stored;
                        ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newOrb) ? Enums::ItemStorage::Stash : newOrb->storage);
                        ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Location, newOrb->location);
                        addItemToList(newOrb, true);
                        placementItems.append(newOrb);
                        setCurrentStorageHasChanged(); emit itemsChanged(); ++successCount;
                    }
                }
            }

            int remaining = copies - successCount;
            for (int i = 0; i < remaining; ++i) {
                ItemInfo *newOrb = new ItemInfo(*dialog->getCreatedOrb());
                newOrb->row = dialog->requestedRow(); newOrb->column = dialog->requestedColumn(); newOrb->storage = storage;

                if (ItemDataBase::canStoreItemAt(newOrb->row, newOrb->column, newOrb->itemType, _allItems, rows, cols)) {
                    newOrb->move(newOrb->row, newOrb->column, newOrb->plugyPage);
                    newOrb->storage = storage;
                    ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newOrb) ? Enums::ItemStorage::Stash : newOrb->storage);
                    ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Location, newOrb->location);
                    addItemToList(newOrb, true); setCurrentStorageHasChanged(); emit itemsChanged(); ++successCount; continue;
                }

                bool placed = false;
                if (plugy) {
                    quint32 page = plugy->currentPage();
                    ItemsList itemsOnPage = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored, &page);
                    auto findNearestFree = [&](int desiredRow, int desiredCol, const ItemsList &items) -> QPair<int,int> {
                        int maxRadius = qMax(rows, cols);
                        for (int r = 0; r <= maxRadius; ++r) for (int dy = -r; dy <= r; ++dy) for (int dx = -r; dx <= r; ++dx) {
                            if (qAbs(dy) != r && qAbs(dx) != r) continue; int rr = desiredRow + dy; int cc = desiredCol + dx;
                            if (rr < 0 || cc < 0 || rr >= rows || cc >= cols) continue; if (ItemDataBase::canStoreItemAt(rr, cc, newOrb->itemType, items, rows, cols)) return qMakePair(rr, cc);
                        }
                        return qMakePair(-1, -1);
                    };
                    QPair<int,int> p = findNearestFree(dialog->requestedRow(), dialog->requestedColumn(), itemsOnPage);
                    if (p.first >= 0) { newOrb->move(p.first, p.second, page, true); newOrb->storage = storage; newOrb->location = Enums::ItemLocation::Stored; ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newOrb) ? Enums::ItemStorage::Stash : newOrb->storage); ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Location, newOrb->location); addItemToList(newOrb, true); setCurrentStorageHasChanged(); emit itemsChanged(); ++successCount; placed = true; }
                } else {
                    ItemsList items = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored);
                    auto findNearestFree = [&](int desiredRow, int desiredCol, const ItemsList &itemsLocal) -> QPair<int,int> {
                        int maxRadius = qMax(rows, cols);
                        for (int r = 0; r <= maxRadius; ++r) for (int dy = -r; dy <= r; ++dy) for (int dx = -r; dx <= r; ++dx) {
                            if (qAbs(dy) != r && qAbs(dx) != r) continue; int rr = desiredRow + dy; int cc = desiredCol + dx;
                            if (rr < 0 || cc < 0 || rr >= rows || cc >= cols) continue; if (ItemDataBase::canStoreItemAt(rr, cc, newOrb->itemType, itemsLocal, rows, cols)) return qMakePair(rr, cc);
                        }
                        return qMakePair(-1, -1);
                    };
                    QPair<int,int> p = findNearestFree(dialog->requestedRow(), dialog->requestedColumn(), items);
                    if (p.first >= 0) { newOrb->move(p.first, p.second, 0, true); newOrb->storage = storage; newOrb->location = Enums::ItemLocation::Stored; ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newOrb) ? Enums::ItemStorage::Stash : newOrb->storage); ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Location, newOrb->location); addItemToList(newOrb, true); setCurrentStorageHasChanged(); emit itemsChanged(); ++successCount; placed = true; }
                }

                if (!placed) {
                    if (storeItemInStorage(newOrb, storage, true)) { ++successCount; } else { addItemToList(newOrb, true); }
                }
            }

            if (successCount > 0) QMessageBox::information(this, tr("Mystic Orb(s) Created"), tr("Created %1 Mystic Orb(s). Remember to save the character (Ctrl+S) to keep the changes!").arg(successCount));
        }

        dialog->deleteLater();
    } catch (...) {
        // Silently handle exceptions
    }
}

void ItemsPropertiesSplitter::createItemAt()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) return;

    QPoint position = action->data().toPoint();
    if (position.x() < 0 || position.y() < 0) return;

    try {
        ItemCreationWidget *dialog = new ItemCreationWidget(this);
        if (!dialog) return;

        dialog->setItemPosition(position.x(), position.y());

        if (dialog->exec() == QDialog::Accepted)
        {
            int copies = dialog->copies();
            int successCount = 0;

            ItemInfo *sel = selectedItem(false);
            int storage = -1;
            if (sel)
                storage = sel->storage;
            else
            {
                ItemsViewerDialog *viewer = nullptr;
                QWidget *p = parentWidget();
                while (p) { viewer = qobject_cast<ItemsViewerDialog *>(p); if (viewer) break; p = p->parentWidget(); }
                if (viewer)
                {
                    int tab = viewer->tabWidget()->currentIndex();
                    if (tab <= ItemsViewerDialog::InventoryIndex)
                        storage = tab;
                    else if (tab == ItemsViewerDialog::CubeIndex)
                        storage = tab + 2;
                    else
                        storage = tab + 3;
                }
                else
                    storage = Enums::ItemStorage::Inventory;
            }

            PlugyItemsSplitter *plugy = dynamic_cast<PlugyItemsSplitter *>(this);
            int rows = ItemsViewerDialog::rowsInStorageAtIndex(storage);
            int cols = ItemsViewerDialog::colsInStorageAtIndex(storage);

            ItemsList placementItems;
            if (plugy) { quint32 page = plugy->currentPage(); placementItems = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored, &page); }
            else placementItems = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored);

            for (int r = 0; r < rows && successCount < copies; ++r) {
                for (int c = 0; c < cols && successCount < copies; ++c) {
                    if (ItemDataBase::canStoreItemAt(r, c, dialog->getCreatedOrb()->itemType, placementItems, rows, cols)) {
                        ItemInfo *newOrb = new ItemInfo(*dialog->getCreatedOrb());
                        newOrb->row = r; newOrb->column = c; newOrb->storage = storage;
                        newOrb->move(r, c, plugy ? plugy->currentPage() : newOrb->plugyPage, true);
                        newOrb->location = Enums::ItemLocation::Stored;
                        ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newOrb) ? Enums::ItemStorage::Stash : newOrb->storage);
                        ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Location, newOrb->location);
                        addItemToList(newOrb, true);
                        placementItems.append(newOrb);
                        setCurrentStorageHasChanged(); emit itemsChanged(); ++successCount;
                    }
                }
            }

            int remaining = copies - successCount;
            for (int i = 0; i < remaining; ++i) {
                ItemInfo *newOrb = new ItemInfo(*dialog->getCreatedOrb());
                newOrb->row = dialog->requestedRow(); newOrb->column = dialog->requestedColumn(); newOrb->storage = storage;

                if (ItemDataBase::canStoreItemAt(newOrb->row, newOrb->column, newOrb->itemType, _allItems, rows, cols)) {
                    newOrb->move(newOrb->row, newOrb->column, newOrb->plugyPage);
                    newOrb->storage = storage;
                    ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newOrb) ? Enums::ItemStorage::Stash : newOrb->storage);
                    ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Location, newOrb->location);
                    addItemToList(newOrb, true); setCurrentStorageHasChanged(); emit itemsChanged(); ++successCount; continue;
                }

                bool placed = false;
                if (plugy) {
                    quint32 page = plugy->currentPage();
                    ItemsList itemsOnPage = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored, &page);
                    auto findNearestFree = [&](int desiredRow, int desiredCol, const ItemsList &items) -> QPair<int,int> {
                        int maxRadius = qMax(rows, cols);
                        for (int r = 0; r <= maxRadius; ++r) for (int dy = -r; dy <= r; ++dy) for (int dx = -r; dx <= r; ++dx) {
                            if (qAbs(dy) != r && qAbs(dx) != r) continue; int rr = desiredRow + dy; int cc = desiredCol + dx;
                            if (rr < 0 || cc < 0 || rr >= rows || cc >= cols) continue; if (ItemDataBase::canStoreItemAt(rr, cc, newOrb->itemType, items, rows, cols)) return qMakePair(rr, cc);
                        }
                        return qMakePair(-1, -1);
                    };
                    QPair<int,int> p = findNearestFree(dialog->requestedRow(), dialog->requestedColumn(), itemsOnPage);
                    if (p.first >= 0) { newOrb->move(p.first, p.second, page, true); newOrb->storage = storage; newOrb->location = Enums::ItemLocation::Stored; ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newOrb) ? Enums::ItemStorage::Stash : newOrb->storage); ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Location, newOrb->location); addItemToList(newOrb, true); setCurrentStorageHasChanged(); emit itemsChanged(); ++successCount; placed = true; }
                } else {
                    ItemsList items = ItemDataBase::itemsStoredIn(storage, Enums::ItemLocation::Stored);
                    auto findNearestFree = [&](int desiredRow, int desiredCol, const ItemsList &itemsLocal) -> QPair<int,int> {
                        int maxRadius = qMax(rows, cols);
                        for (int r = 0; r <= maxRadius; ++r) for (int dy = -r; dy <= r; ++dy) for (int dx = -r; dx <= r; ++dx) {
                            if (qAbs(dy) != r && qAbs(dx) != r) continue; int rr = desiredRow + dy; int cc = desiredCol + dx;
                            if (rr < 0 || cc < 0 || rr >= rows || cc >= cols) continue; if (ItemDataBase::canStoreItemAt(rr, cc, newOrb->itemType, itemsLocal, rows, cols)) return qMakePair(rr, cc);
                        }
                        return qMakePair(-1, -1);
                    };
                    QPair<int,int> p = findNearestFree(dialog->requestedRow(), dialog->requestedColumn(), items);
                    if (p.first >= 0) { newOrb->move(p.first, p.second, 0, true); newOrb->storage = storage; newOrb->location = Enums::ItemLocation::Stored; ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newOrb) ? Enums::ItemStorage::Stash : newOrb->storage); ReverseBitWriter::replaceValueInBitString(newOrb->bitString, Enums::ItemOffsets::Location, newOrb->location); addItemToList(newOrb, true); setCurrentStorageHasChanged(); emit itemsChanged(); ++successCount; placed = true; }
                }

                if (!placed) {
                    if (storeItemInStorage(newOrb, storage, true)) { ++successCount; } else { addItemToList(newOrb, true); }
                }
            }

            if (successCount > 0) QMessageBox::information(this, tr("Item(s) Created"), tr("Created %1 Item(s). Remember to save the character (Ctrl+S) to keep the changes!").arg(successCount));
        }

        dialog->deleteLater();
    } catch (...) {
        // Silently handle exceptions
    }
}


void ItemsPropertiesSplitter::deleteItemTriggered()
{
    ItemInfo *item = selectedItem(false);
    if (item && QUESTION_BOX_YESNO(tr("Are you sure you want to delete this item?"), QMessageBox::Yes) == QMessageBox::Yes)
    {
        bool isCube = ItemDataBase::isCube(item);
        if (isCube && !ItemDataBase::itemsStoredIn(Enums::ItemStorage::Cube).isEmpty())
            if (QUESTION_BOX_YESNO(tr("Cube is not empty. Do you really want to delete it?\nNote: items inside will be preserved. You can recover them by getting new Cube."), QMessageBox::No) == QMessageBox::Yes)
                return;

        deleteItem(item);
        if (isCube)
            emit cubeDeleted();
    }
}

#ifdef DUMP_INFO_ACTION
void ItemsPropertiesSplitter::dumpInfo(ItemInfo *item /*= 0*/, bool shouldShowMsgBox /*= true*/)
{
    if (!item)
        item = selectedItem(false);
    ItemBase *base = ItemDataBase::Items()->value(item->itemType);
    const char *quality = metaEnumFromName<Enums::ItemQuality>("ItemQualityEnum").valueToKey(item->quality);
    bool isSetOrUnique = areBothItemsSetOrUnique(item, item); // hacky code :)

    qDebug() << ItemParser::itemStorageAndCoordinatesString("location %1, row %2, col %3, equipped in %4", item) << "quality" << quality << "code" << item->itemType << "types" << base->types << "image" << base->imageName << "vargfx" << item->variableGraphicIndex << "quest ID" << base->questId;
    if (isSetOrUnique)
        qDebug() << "set/unique ID" << item->setOrUniqueId;
    for (PropertiesMultiMap::const_iterator it = item->props.constBegin(); it != item->props.constEnd(); ++it)
        qDebug() << "prop" << it.key() << it.value()->value << it.value()->param;
    qDebug("--------------------");

    if (shouldShowMsgBox)
    {
        QString types;
        foreach (const QByteArray &type, base->types)
            types += type + ", ";
        INFO_BOX(QString("%1\nquality %2, set/unique ID %3\ncode %4, types: %5\nimage %6, vargfx %7, quest ID %8").arg(ItemParser::itemStorageAndCoordinatesString("location %1, row %2, col %3, equipped in %4", item))
            .arg(quality).arg(isSetOrUnique ? item->setOrUniqueId : 0).arg(QTextCodec::codecForName("Windows-1252")->toUnicode(item->itemType)).arg(types).arg(base->imageName.constData()).arg(item->variableGraphicIndex).arg(base->questId));
    }
}
#endif

void ItemsPropertiesSplitter::setCurrentStorageHasChanged()
{
    // a hack to make stash modified
    if (!_allItems.isEmpty())
        _allItems.first()->hasChanged = true;
}

void ItemsPropertiesSplitter::deleteItem(ItemInfo *item)
{
    performDeleteItem(item);
    showFirstItem();

    emit itemCountChanged(_allItems.size());
    emit itemDeleted();

    setCurrentStorageHasChanged();
}

void ItemsPropertiesSplitter::performDeleteItem(ItemInfo *item, bool emitSignal /*= true*/)
{
    // TODO: [0.5] add option to unsocket at first
    removeItemFromList(item, emitSignal);
    delete item;
}

void ItemsPropertiesSplitter::addItemToList(ItemInfo *item, bool emitSignal /*= true*/)
{
    if (!CharacterInfo::instance().items.character.contains(item))
        CharacterInfo::instance().items.character.append(item);

    if (!_allItems.contains(item))
        _allItems.append(item);
    if (isItemInCurrentStorage(item))
    {
        _itemsModel->addItem(item);
        if (selectedItem(false) == item) // signal is emitted only when single item is disenchanted (through context menu), so we don't need extra parameter for just another name
            _propertiesWidget->showItem(item);
    }

    if (emitSignal)
        emit itemsChanged();
}

void ItemsPropertiesSplitter::removeItemFromList(ItemInfo *item, bool emitSignal /*= true*/)
{
    CharacterInfo::instance().items.character.removeOne(item);

    _allItems.removeOne(item);
    if (selectedItem(false) == item)
        _propertiesWidget->clear();
    if (isItemInCurrentStorage(item))
        removeItemFromModel(item);

    if (emitSignal)
        emit itemsChanged();
}

void ItemsPropertiesSplitter::removeItemFromModel(ItemInfo *item)
{
    _itemsModel->removeItem(item);
    if (_itemsView->rowSpan(item->row, item->column) > 1 || _itemsView->columnSpan(item->row, item->column) > 1) // hides warning in console
        _itemsView->setSpan(item->row, item->column, 1, 1);
}

ItemsList ItemsPropertiesSplitter::disenchantAllItems(bool toShards, bool upgradeToCrystals, bool eatSignets, ItemsList *pItems /*= 0*/)
{
    ProgressBarModal progressBar;
    progressBar.centerInWidget(this);
    progressBar.show();

    ItemsList &items = pItems ? *pItems : _allItems, disenchantedItems;
    quint32 disenchantedItemsNumber = items.size(), signetsEaten = 0, signetsEatenTotal = CharacterInfo::instance().valueOfStatistic(Enums::CharacterStats::SignetsOfLearningEaten);
    ItemInfo *disenchantedItem = ItemDataBase::loadItemFromFile(toShards ? "arcane_shard" : "signet_of_learning");
    foreach (ItemInfo *item, items)
    {
        qApp->processEvents();

        bool shouldDisenchant = true;
        if (!toShards && eatSignets)
        {
            if (signetsEatenTotal + signetsEaten < Enums::CharacterStats::SignetsOfLearningMax)
            {
                removeItemFromList(item, false);
                shouldDisenchant = false;
                ++signetsEaten;
            }
        }
        if (shouldDisenchant)
            disenchantedItems += disenchantItemIntoItem(item, disenchantedItem, false);
    }

    QString text;
    QString baseTextFormat = tr("You've received %1", "number of Arcane Crystals, Arcane Shards, or Signets of Learning");
    if (toShards)
    {
        if (upgradeToCrystals)
        {
            quint32 shards = 0;
            foreach (ItemInfo *item, disenchantedItems)
            {
                qApp->processEvents();
                if (isArcaneShard(item))
                    ++shards;
                else if (isArcaneShard2(item))
                    shards += 2;
                else if (isArcaneShard3(item))
                    shards += 3;
                else if (isArcaneShard4(item))
                    shards += 4;
            }

            quint32 crystals = shards / kShardsPerCrystal;
            if (crystals)
            {
                int storage = disenchantedItems.first()->storage;
                foreach (ItemInfo *item, disenchantedItems)
                {
                    if (isArcaneShard(item) || isArcaneShard2(item) || isArcaneShard3(item) || isArcaneShard4(item))
                    {
                        qApp->processEvents();
                        performDeleteItem(item, false);
                    }
                }

                ItemInfo *crystal = ItemDataBase::loadItemFromFile("arcane_crystal");
                for (quint32 i = 0; i < crystals; ++i)
                {
                    qApp->processEvents();

                    ItemInfo *crystalCopy = new ItemInfo(*crystal);
                    storeItemInStorage(crystalCopy, storage);
                }
                delete crystal;

                quint8 shardsLeft = shards - crystals * kShardsPerCrystal;
                for (int i = 0; i < shardsLeft; ++i)
                {
                    qApp->processEvents();

                    ItemInfo *shard = new ItemInfo(*disenchantedItem);
                    storeItemInStorage(shard, storage);
                }

                QString crystalsText = tr("%n Arcane Crystal(s)", 0, crystals);
                text = baseTextFormat.arg(crystalsText);
                if (shardsLeft)
                    text += QString(" %1 %2").arg(tr("and"), tr("%n Arcane Shard(s)", 0, shardsLeft));

                emit itemCountChanged(_allItems.size());
            }
        }

        if (text.isEmpty())
            text = baseTextFormat.arg(tr("%n Arcane Shard(s)", 0, disenchantedItemsNumber));
    }
    else
    {
        QString signetsText = tr("%n Signet(s) of Learning", 0, disenchantedItemsNumber);
        QString baseSignetsTextFormat = tr("You've eaten %1", "number of Signets of Learning");
        if (eatSignets && signetsEaten)
        {
            emit signetsOfLearningEaten(signetsEaten);

            if (static_cast<quint32>(signetsEaten) == disenchantedItemsNumber)
                text = baseSignetsTextFormat.arg(signetsText);
            else
            {
                signetsText = tr("%n Signet(s) of Learning", 0, signetsEaten);
                if (signetsEaten != Enums::CharacterStats::SignetsOfLearningMax)
                    text = tr("%1 (now you have %2) and received %3").arg(baseSignetsTextFormat.arg(signetsText)).arg(QString::number(Enums::CharacterStats::SignetsOfLearningMax))
                                                                     .arg(tr("%n Signet(s) of Learning", 0, disenchantedItemsNumber - signetsEaten));
                else
                    text = tr("%1 and received %2").arg(baseSignetsTextFormat.arg(signetsText)).arg(disenchantedItemsNumber - signetsEaten);
            }
        }
        else
            text = baseTextFormat.arg(signetsText);
    }
    progressBar.hide();

    delete disenchantedItem;
    emit itemsChanged();
    _itemsView->viewport()->update();

    INFO_BOX(text);
    return disenchantedItems;
}

ItemInfo *ItemsPropertiesSplitter::disenchantItemIntoItem(ItemInfo *oldItem, ItemInfo *newItem, bool emitSignal /*= true*/)
{
    ItemsList items = ItemDataBase::itemsStoredIn(oldItem->storage, oldItem->location, oldItem->plugyPage ? &oldItem->plugyPage : 0);
    items.removeOne(oldItem);
    ItemInfo *newItemCopy = new ItemInfo(*newItem); // it's safe because there're no properties and no socketables
    if (!ItemDataBase::canStoreItemAt(oldItem->row, oldItem->column, newItemCopy->itemType, items, ItemsViewerDialog::rowsInStorageAtIndex(oldItem->storage), ItemsViewerDialog::colsInStorageAtIndex(oldItem->storage)))
    {
        ERROR_BOX("If you see this text (which you shouldn't), please tell me which item you've just tried to disenchant");
        delete newItemCopy;
        return 0;
    }

    newItemCopy->move(oldItem->row, oldItem->column, oldItem->plugyPage);
    newItemCopy->storage = oldItem->storage;
    newItemCopy->whereEquipped = oldItem->whereEquipped;

    // update bits
    ReverseBitWriter::replaceValueInBitString(newItemCopy->bitString, Enums::ItemOffsets::Storage, isInExternalStorage(newItemCopy) ? Enums::ItemStorage::Stash : newItemCopy->storage);

    performDeleteItem(oldItem, emitSignal);
    addItemToList(newItemCopy, emitSignal);

    return newItemCopy;
}

bool ItemsPropertiesSplitter::storeItemInStorage(ItemInfo *item, int storage, bool emitSignal /*= false*/)
{
    bool result = ItemDataBase::storeItemIn(item, static_cast<Enums::ItemStorage::ItemStorageEnum>(storage), ItemsViewerDialog::rowsInStorageAtIndex(storage), ItemsViewerDialog::colsInStorageAtIndex(storage));
    if (result)
        addItemToList(item, emitSignal);
    return result;
}

void ItemsPropertiesSplitter::setCellSpanForItem(ItemInfo *item)
{
    _itemsView->setCellSpanForItem(item);
}

bool ItemsPropertiesSplitter::upgradeItemsInMap(UpgradableItemsMultiMap &itemsMap, quint8 maxKey, const QString &itemNameFormat)
{
    QList<quint8> keys = itemsMap.uniqueKeys();
    if (keys.isEmpty())
        return false;

    int currentStorage = itemsMap.value(keys.at(0))->storage;
    for (int i = 0; i < keys.size(); ++i)
    {
        quint8 key = keys.at(i);
        int sameItemsSize = itemsMap.values(key).size(), upgradedItemsSize = sameItemsSize / 2, leftItemsSize = sameItemsSize - upgradedItemsSize * 2;
        if (upgradedItemsSize)
        {
            for (UpgradableItemsMultiMap::iterator iter = itemsMap.begin(); sameItemsSize > leftItemsSize && iter != itemsMap.end();)
            {
                if (iter.key() == key)
                {
                    qApp->processEvents();
                    removeItemFromList(iter.value(), false);
                    --sameItemsSize;
                    iter = itemsMap.erase(iter);
                }
                else
                    ++iter;
            }

            quint8 newKey = key + 1;
            ItemsList higherItems = itemsMap.values(newKey);
            ItemInfo *newItem = higherItems.isEmpty() ? ItemDataBase::loadItemFromFile(itemNameFormat.arg(newKey)) : higherItems.first();
            for (int j = 0; j < upgradedItemsSize; ++j)
            {
                qApp->processEvents();
                ItemInfo *itemCopy = new ItemInfo(*newItem);
                storeItemInStorage(itemCopy, currentStorage);
                itemsMap.insert(newKey, itemCopy);
            }

            if (higherItems.isEmpty())
            {
                delete newItem;

                if (newKey == maxKey)
                    break;
                keys.insert(i + 1, newKey);
            }
        }
    }
    return true;
}

void ItemsPropertiesSplitter::upgradeGems(ItemsList *pItems /*= 0*/)
{
    QHash<QByteArray, UpgradableItemsMultiMap> gemsMapsHash = gemsMapsFromItems(pItems ? *pItems : _allItems);
    for (QHash<QByteArray, UpgradableItemsMultiMap>::iterator iter = gemsMapsHash.begin(); iter != gemsMapsHash.end(); ++iter)
        upgradeItemsInMap(iter.value(), kPerfectGrade, QString("gems/%1%2").arg(iter.key().constData()).arg("%1"));

    emit itemsChanged();
    emit itemCountChanged(_allItems.size());
}

void ItemsPropertiesSplitter::upgradeRunes(int reserveRunes, ItemsList *pItems /*= 0*/)
{
    UpgradableItemsMultiMap runesMap = runesMapFromItems(pItems ? *pItems : _allItems, reserveRunes);
    if (upgradeItemsInMap(runesMap, HighestRuneKey, "runes/r%1"))
    {
        emit itemsChanged();
        emit itemCountChanged(_allItems.size());
    }
}

QHash<QByteArray, UpgradableItemsMultiMap> ItemsPropertiesSplitter::gemsMapsFromItems(const ItemsList &items)
{
    const QByteArray kPerfectGradeBytes = QByteArray::number(kPerfectGrade);

    QMultiHash<QByteArray, ItemInfo *> allGems;
    foreach (ItemInfo *item, items)
    {
        QList<QByteArray> types = ItemDataBase::Items()->value(item->itemType)->types;  // first element is gem type, second element is gem grade
        if (types.at(0).startsWith("gem") && !types.at(1).endsWith(kPerfectGradeBytes)) // exclude prefect gems
            allGems.insert(types.at(0), item);
    }

    QHash<QByteArray, UpgradableItemsMultiMap> gemsMapsHash;
    foreach (const QByteArray &gemType, allGems.uniqueKeys())
    {
        UpgradableItemsMultiMap gemsMap;
        foreach (ItemInfo *gem, allGems.values(gemType))
            gemsMap.insert(ItemDataBase::Items()->value(gem->itemType)->types.at(1).right(1).toUShort(), gem);
        gemsMapsHash[gemType] = gemsMap;
    }
    return gemsMapsHash;
}

UpgradableItemsMultiMap ItemsPropertiesSplitter::runesMapFromItems(const ItemsList &items, int reserveRunes)
{
    UpgradableItemsMultiMap runesMap;
    QHash<quint8, quint8> reserveHash;
    foreach (ItemInfo *item, items)
    {
        if (kRuneRegExp.exactMatch(item->itemType))
        {
            quint8 runeKey = kRuneRegExp.cap(1).toUShort();
            if (runeKey < HighestRuneKey) // don't include 'On' rune, Great runes and Ultimative runes
            {
                quint8 &reserve = reserveHash[runeKey];
                if (reserve < reserveRunes)
                    ++reserve;
                else
                    runesMap.insert(runeKey, item);
            }
        }
    }
    return runesMap;
}

QString ItemsPropertiesSplitter::itemBBCode(const QString &name, const QString &codeType)
{
    return QString("[item%1]%2[/item]").arg(codeType.isEmpty() ? QString() : (QLatin1String("=") + codeType)).arg(name);
}

QString ItemsPropertiesSplitter::itemNameBBCode(ItemInfo *item)
{
    if (!item)
        return QString();

    QString name;
    if (item->quality == Enums::ItemQuality::Set)
    {
        if (SetItemInfo *setItem = ItemDataBase::Sets()->value(item->setOrUniqueId))
            name = setItem->itemName;
        else
            return QString();
    }
	else if (item->quality == Enums::ItemQuality::Unique)
	{
		if (UniqueItemInfo *uniqueInfo = ItemDataBase::Uniques()->value(item->setOrUniqueId))
			name = uniqueInfo->name;
		else
			return QString();
	}
    else if (item->isRW)
    {
        name = item->rwName;
        foreach (ItemInfo *socketable, item->socketablesInfo)
        {
            if (socketable->itemType.startsWith("rx"))
            {
                name += QLatin1String(" (Xis)");
                break;
            }
        }
    }
    else
        return QString();

    ItemDataBase::removeColorCodesFromString(name);
    return name.split(QLatin1String("\\n"),
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        Qt::SkipEmptyParts
#else
        QString::SkipEmptyParts
#endif
    ).last();
}

QMenu *ItemsPropertiesSplitter::createCopyBBCodeMenu(bool addShortcut)
{
    QMenu *menu = new QMenu(tr("Copy item BBCode"), _itemsView);

    QAction *actionBBCodeText = new QAction(tr("Text", "BBCode type"), menu);
    if (addShortcut)
        actionBBCodeText->setShortcut(QKeySequence("Ctrl+Alt+C"));

    QAction *actionBBCodeImage = new QAction(tr("Image", "BBCode type"), menu);
    actionBBCodeImage->setObjectName(QLatin1String("image"));

    QAction *actionBBCodeFull = new QAction(tr("Full", "BBCode type"), menu);
    actionBBCodeFull->setObjectName(QLatin1String("full"));

    menu->addActions(QList<QAction *>() << actionBBCodeText << actionBBCodeImage << actionBBCodeFull);
    return menu;
}

void ItemsPropertiesSplitter::createItemActions()
{
    //QAction *actionBbCode = new QAction("BBCode", _itemsView);
    //actionBbCode->setShortcut(QKeySequence("Ctrl+E"));
    //actionBbCode->setObjectName("bbcode");
    //connect(actionBbCode, SIGNAL(triggered()), SLOT(exportText()));
    //_itemsView->addAction(actionBbCode);
    //_itemActions[ExportBbCode] = actionBbCode;

    //QAction *actionHtml = new QAction("HTML", _itemsView);
    //actionHtml->setShortcut(QKeySequence("Alt+E"));
    //actionHtml->setObjectName("html");
    //connect(actionHtml, SIGNAL(triggered()), SLOT(exportText()));
    //_itemsView->addAction(actionHtml);
    //_itemActions[ExportHtml] = actionHtml;

    QMenu *menuCopyItemBBCode = createCopyBBCodeMenu(true);
    connect(menuCopyItemBBCode, SIGNAL(triggered(QAction*)), SLOT(copyItemBBCode(QAction*)));
    _itemsView->addActions(menuCopyItemBBCode->actions());
    _itemActions[CopyItemBBCode] = menuCopyItemBBCode->menuAction();

    QAction *actionSol = new QAction(QIcon(ResourcePathManager::pathForItemImageName("sigil1b")), tr("Signet of Learning"), _itemsView);
    actionSol->setShortcut(QKeySequence("Ctrl+D"));
    actionSol->setObjectName("signet");
    actionSol->setIconVisibleInMenu(true); // explicitly show icon on Mac OS X
    connect(actionSol, SIGNAL(triggered()), SLOT(disenchantSelectedItem()));
    _itemsView->addAction(actionSol);
    _itemActions[DisenchantSignet] = actionSol;

    QAction *actionShards = new QAction(QIcon(ResourcePathManager::pathForItemImageName("invfary4")), tr("Arcane Shards"), _itemsView);
    actionShards->setShortcut(QKeySequence("Alt+D"));
    actionShards->setObjectName("shards");
    actionShards->setIconVisibleInMenu(true); // explicitly show icon on Mac OS X
    connect(actionShards, SIGNAL(triggered()), SLOT(disenchantSelectedItem()));
    _itemsView->addAction(actionShards);
    _itemActions[DisenchantShards] = actionShards;

    // TODO: [0.5] unsocket
//    QAction *actionUnsocket = new QAction(tr("Unsocket"), _itemsView);
//    connect(actionUnsocket, SIGNAL(triggered()), SLOT(unsocketItem()));
//    _itemsView->addAction(actionUnsocket);
//    _itemActions[Unsocket] = actionUnsocket;

    QAction *actionRemoveMO = new QAction(_itemsView);
    actionRemoveMO->setShortcut(QKeySequence("Ctrl+M"));
    connect(actionRemoveMO, SIGNAL(triggered()), _propertiesWidget, SLOT(removeAllMysticOrbs()));
    connect(actionRemoveMO, SIGNAL(triggered()), SIGNAL(itemsChanged()));
    _itemsView->addAction(actionRemoveMO);
    _itemActions[RemoveMO] = actionRemoveMO;

    QAction *actionEatSignetOfLearning = new QAction(_itemsView);
    actionEatSignetOfLearning->setShortcut(QKeySequence("Ctrl+L"));
    connect(actionEatSignetOfLearning, SIGNAL(triggered()), SLOT(eatSelectedSignet()));
    _itemsView->addAction(actionEatSignetOfLearning);
    _itemActions[EatSignetOfLearning] = actionEatSignetOfLearning;

    QAction *actionDelete = new QAction(tr("Delete"), _itemsView);
    actionDelete->setShortcut(
#ifdef Q_OS_MAC
                Qt::Key_Backspace
#else
                QKeySequence::Delete
#endif
                );
    connect(actionDelete, SIGNAL(triggered()), SLOT(deleteItemTriggered()));
    _itemsView->addAction(actionDelete);
    _itemActions[Delete] = actionDelete;
}

QAction *ItemsPropertiesSplitter::separatorAction()
{
    QAction *sep = new QAction(_itemsView);
    sep->setSeparator(true);
    return sep;
}

void ItemsPropertiesSplitter::createActionsForMysticOrbs(QMenu *parentMenu, bool isItemMO, ItemInfo *item)
{
    foreach (int moCode, _propertiesWidget->mysticOrbs(isItemMO))
    {
        QAction *moAction = new QAction((isItemMO ? item->props : item->rwProps).value(moCode)->displayString, _itemsView);
        moAction->setProperty("isItemMO", isItemMO);
        moAction->setProperty("moCode", moCode);
        connect(moAction, SIGNAL(triggered()), _propertiesWidget, SLOT(removeMysticOrb()));
        connect(moAction, SIGNAL(triggered()), SIGNAL(itemsChanged()));
        parentMenu->addAction(moAction);
    }
}

bool ItemsPropertiesSplitter::shouldAddMoveItemAction() const { return true; }
QString ItemsPropertiesSplitter::moveItemActionText() const { return tr("Move to shared infinite stash"); }
