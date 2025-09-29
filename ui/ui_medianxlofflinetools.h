/********************************************************************************
** Form generated from reading UI file 'medianxlofflinetools.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MEDIANXLOFFLINETOOLS_H
#define UI_MEDIANXLOFFLINETOOLS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MedianXLOfflineToolsClass
{
public:
    QAction *actionLoadCharacter;
    QAction *actionSaveCharacter;
    QAction *actionExit;
    QAction *actionAbout;
    QAction *actionAboutQt;
    QAction *actionRespecStats;
    QAction *actionRespecSkills;
    QAction *actionActivateWaypoints;
    QAction *actionRename;
    QAction *actionReloadCharacter;
    QAction *actionConvertToSoftcore;
    QAction *actionResurrect;
    QAction *actionToolbar;
    QAction *actionShowItems;
    QAction *actionLoadLastUsedCharacter;
    QAction *actionGiveCube;
    QAction *actionOpenItemsAutomatically;
    QAction *actionFind;
    QAction *actionFindNext;
    QAction *actionFindPrevious;
    QAction *actionReloadSharedStashes;
    QAction *actionAutoOpenPersonalStash;
    QAction *actionAutoOpenSharedStash;
    QAction *actionAutoOpenHCShared;
    QAction *actionBackup;
    QAction *actionWarnWhenColoredName;
    QAction *actionFirstPage;
    QAction *actionPrevious100;
    QAction *actionPrevious10;
    QAction *actionPreviousPage;
    QAction *actionNextPage;
    QAction *actionNext10;
    QAction *actionNext100;
    QAction *actionLastPage;
    QAction *actionShowAllStats;
    QAction *actionCheckFileAssociations;
    QAction *actionAssociate;
    QAction *actionCheckForUpdateOnStart;
    QAction *actionCheckForUpdate;
    QAction *actionBackups1;
    QAction *actionBackups2;
    QAction *actionBackups5;
    QAction *actionBackups10;
    QAction *actionBackupsUnlimited;
    QAction *actionBackupFormatReadable;
    QAction *actionBackupFormatTimestamp;
    QAction *actionPreviewDisenchantAlways;
    QAction *actionPreviewDisenchantForSinglePage;
    QAction *actionPreviewDisenchantNever;
    QAction *actionOpenFileAssociationUI;
    QAction *actionSkillTree;
    QWidget *centralWidget;
    QGroupBox *statsGroupBox;
    QLineEdit *freeStatPointsLineEdit;
    QLineEdit *freeSkillPointsLineEdit;
    QTableWidget *statsTableWidget;
    QLineEdit *inventoryGoldLineEdit;
    QLineEdit *stashGoldLineEdit;
    QLineEdit *signetsOfLearningEatenLineEdit;
    QCheckBox *respecSkillsCheckBox;
    QSpinBox *energySpinBox;
    QSpinBox *dexteritySpinBox;
    QSpinBox *strengthSpinBox;
    QSpinBox *vitalitySpinBox;
    QPushButton *respecStatsButton;
    QPushButton *showAllStatsButton;
    QGroupBox *characterGroupBox;
    QPushButton *renameButton;
    QLineEdit *classLineEdit;
    QSpinBox *levelSpinBox;
    QLineEdit *titleLineEdit;
    QGroupBox *hardcoreGroupBox;
    QCheckBox *convertToSoftcoreCheckBox;
    QPushButton *resurrectButton;
    QLabel *charNamePreviewLabel;
    QGroupBox *mercGroupBox;
    QComboBox *mercTypeComboBox;
    QComboBox *mercNameComboBox;
    QLineEdit *mercLevelLineEdit;
    QGroupBox *waypointsGroupBox;
    QCheckBox *activateWaypointsCheckBox;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuRecentCharacters;
    QMenu *menuHelp;
    QMenu *menuEdit;
    QMenu *menuHardcore;
    QMenu *menuWaypoints;
    QMenu *menuRespec;
    QMenu *menuOptions;
    QMenu *menuAuto_open_shared_stashes;
    QMenu *menuBackupsLimit;
    QMenu *menuBackupNameFormat;
    QMenu *menuShow_disenchant_preview_dialog;
    QMenu *menuItems;
    QMenu *menuGoToPage;
    QMenu *menuExport;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MedianXLOfflineToolsClass)
    {
        if (MedianXLOfflineToolsClass->objectName().isEmpty())
            MedianXLOfflineToolsClass->setObjectName("MedianXLOfflineToolsClass");
        MedianXLOfflineToolsClass->resize(686, 390);
        MedianXLOfflineToolsClass->setAcceptDrops(true);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/MedianXLOfflineTools/icons/icon.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        MedianXLOfflineToolsClass->setWindowIcon(icon);
        MedianXLOfflineToolsClass->setUnifiedTitleAndToolBarOnMac(true);
        actionLoadCharacter = new QAction(MedianXLOfflineToolsClass);
        actionLoadCharacter->setObjectName("actionLoadCharacter");
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/MedianXLOfflineTools/icons/open.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionLoadCharacter->setIcon(icon1);
        actionLoadCharacter->setShortcutContext(Qt::ApplicationShortcut);
        actionSaveCharacter = new QAction(MedianXLOfflineToolsClass);
        actionSaveCharacter->setObjectName("actionSaveCharacter");
        actionSaveCharacter->setEnabled(false);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/MedianXLOfflineTools/icons/save.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionSaveCharacter->setIcon(icon2);
        actionSaveCharacter->setShortcutContext(Qt::ApplicationShortcut);
        actionExit = new QAction(MedianXLOfflineToolsClass);
        actionExit->setObjectName("actionExit");
        actionExit->setMenuRole(QAction::QuitRole);
        actionAbout = new QAction(MedianXLOfflineToolsClass);
        actionAbout->setObjectName("actionAbout");
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/MedianXLOfflineTools/icons/about.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionAbout->setIcon(icon3);
        actionAbout->setShortcutContext(Qt::ApplicationShortcut);
        actionAbout->setMenuRole(QAction::AboutRole);
        actionAboutQt = new QAction(MedianXLOfflineToolsClass);
        actionAboutQt->setObjectName("actionAboutQt");
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/MedianXLOfflineTools/icons/qt.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionAboutQt->setIcon(icon4);
        actionAboutQt->setMenuRole(QAction::AboutQtRole);
        actionRespecStats = new QAction(MedianXLOfflineToolsClass);
        actionRespecStats->setObjectName("actionRespecStats");
        actionRespecStats->setCheckable(false);
        actionRespecStats->setEnabled(false);
        actionRespecSkills = new QAction(MedianXLOfflineToolsClass);
        actionRespecSkills->setObjectName("actionRespecSkills");
        actionRespecSkills->setCheckable(true);
        actionRespecSkills->setEnabled(false);
        actionActivateWaypoints = new QAction(MedianXLOfflineToolsClass);
        actionActivateWaypoints->setObjectName("actionActivateWaypoints");
        actionActivateWaypoints->setCheckable(true);
        actionActivateWaypoints->setEnabled(false);
        actionRename = new QAction(MedianXLOfflineToolsClass);
        actionRename->setObjectName("actionRename");
        actionRename->setEnabled(false);
        actionReloadCharacter = new QAction(MedianXLOfflineToolsClass);
        actionReloadCharacter->setObjectName("actionReloadCharacter");
        actionReloadCharacter->setEnabled(false);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/MedianXLOfflineTools/icons/reload.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionReloadCharacter->setIcon(icon5);
        actionReloadCharacter->setShortcutContext(Qt::ApplicationShortcut);
        actionConvertToSoftcore = new QAction(MedianXLOfflineToolsClass);
        actionConvertToSoftcore->setObjectName("actionConvertToSoftcore");
        actionConvertToSoftcore->setCheckable(true);
        actionConvertToSoftcore->setEnabled(false);
        actionResurrect = new QAction(MedianXLOfflineToolsClass);
        actionResurrect->setObjectName("actionResurrect");
        actionResurrect->setEnabled(false);
        actionToolbar = new QAction(MedianXLOfflineToolsClass);
        actionToolbar->setObjectName("actionToolbar");
        actionToolbar->setCheckable(true);
        actionShowItems = new QAction(MedianXLOfflineToolsClass);
        actionShowItems->setObjectName("actionShowItems");
        actionShowItems->setEnabled(false);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/MedianXLOfflineTools/icons/items.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionShowItems->setIcon(icon6);
        actionShowItems->setShortcutContext(Qt::ApplicationShortcut);
        actionLoadLastUsedCharacter = new QAction(MedianXLOfflineToolsClass);
        actionLoadLastUsedCharacter->setObjectName("actionLoadLastUsedCharacter");
        actionLoadLastUsedCharacter->setCheckable(true);
        actionLoadLastUsedCharacter->setChecked(true);
        actionGiveCube = new QAction(MedianXLOfflineToolsClass);
        actionGiveCube->setObjectName("actionGiveCube");
        actionGiveCube->setEnabled(false);
        actionOpenItemsAutomatically = new QAction(MedianXLOfflineToolsClass);
        actionOpenItemsAutomatically->setObjectName("actionOpenItemsAutomatically");
        actionOpenItemsAutomatically->setCheckable(true);
        actionFind = new QAction(MedianXLOfflineToolsClass);
        actionFind->setObjectName("actionFind");
        actionFind->setEnabled(false);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/MedianXLOfflineTools/icons/find.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionFind->setIcon(icon7);
        actionFind->setShortcutContext(Qt::ApplicationShortcut);
        actionFindNext = new QAction(MedianXLOfflineToolsClass);
        actionFindNext->setObjectName("actionFindNext");
        actionFindNext->setEnabled(false);
        actionFindNext->setShortcutContext(Qt::ApplicationShortcut);
        actionFindPrevious = new QAction(MedianXLOfflineToolsClass);
        actionFindPrevious->setObjectName("actionFindPrevious");
        actionFindPrevious->setEnabled(false);
        actionFindPrevious->setShortcutContext(Qt::ApplicationShortcut);
        actionReloadSharedStashes = new QAction(MedianXLOfflineToolsClass);
        actionReloadSharedStashes->setObjectName("actionReloadSharedStashes");
        actionReloadSharedStashes->setCheckable(true);
        actionAutoOpenPersonalStash = new QAction(MedianXLOfflineToolsClass);
        actionAutoOpenPersonalStash->setObjectName("actionAutoOpenPersonalStash");
        actionAutoOpenPersonalStash->setCheckable(true);
        actionAutoOpenPersonalStash->setChecked(true);
        actionAutoOpenSharedStash = new QAction(MedianXLOfflineToolsClass);
        actionAutoOpenSharedStash->setObjectName("actionAutoOpenSharedStash");
        actionAutoOpenSharedStash->setCheckable(true);
        actionAutoOpenSharedStash->setChecked(true);
        actionAutoOpenHCShared = new QAction(MedianXLOfflineToolsClass);
        actionAutoOpenHCShared->setObjectName("actionAutoOpenHCShared");
        actionAutoOpenHCShared->setCheckable(true);
        actionAutoOpenHCShared->setChecked(true);
        actionBackup = new QAction(MedianXLOfflineToolsClass);
        actionBackup->setObjectName("actionBackup");
        actionBackup->setCheckable(true);
        actionBackup->setChecked(true);
        actionWarnWhenColoredName = new QAction(MedianXLOfflineToolsClass);
        actionWarnWhenColoredName->setObjectName("actionWarnWhenColoredName");
        actionWarnWhenColoredName->setCheckable(true);
        actionWarnWhenColoredName->setChecked(true);
        actionFirstPage = new QAction(MedianXLOfflineToolsClass);
        actionFirstPage->setObjectName("actionFirstPage");
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/PlugyArrows/icons/plugy/first.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionFirstPage->setIcon(icon8);
#if QT_CONFIG(shortcut)
        actionFirstPage->setShortcut(QString::fromUtf8("Ctrl+Shift+Left"));
#endif // QT_CONFIG(shortcut)
        actionFirstPage->setShortcutContext(Qt::ApplicationShortcut);
        actionPrevious100 = new QAction(MedianXLOfflineToolsClass);
        actionPrevious100->setObjectName("actionPrevious100");
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/PlugyArrows/icons/plugy/left100.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionPrevious100->setIcon(icon9);
#if QT_CONFIG(shortcut)
        actionPrevious100->setShortcut(QString::fromUtf8("Alt+Shift+Left"));
#endif // QT_CONFIG(shortcut)
        actionPrevious100->setShortcutContext(Qt::ApplicationShortcut);
        actionPrevious10 = new QAction(MedianXLOfflineToolsClass);
        actionPrevious10->setObjectName("actionPrevious10");
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/PlugyArrows/icons/plugy/left10.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionPrevious10->setIcon(icon10);
#if QT_CONFIG(shortcut)
        actionPrevious10->setShortcut(QString::fromUtf8("Alt+Left"));
#endif // QT_CONFIG(shortcut)
        actionPrevious10->setShortcutContext(Qt::ApplicationShortcut);
        actionPreviousPage = new QAction(MedianXLOfflineToolsClass);
        actionPreviousPage->setObjectName("actionPreviousPage");
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/PlugyArrows/icons/plugy/left.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionPreviousPage->setIcon(icon11);
#if QT_CONFIG(shortcut)
        actionPreviousPage->setShortcut(QString::fromUtf8("Ctrl+Left"));
#endif // QT_CONFIG(shortcut)
        actionPreviousPage->setShortcutContext(Qt::ApplicationShortcut);
        actionNextPage = new QAction(MedianXLOfflineToolsClass);
        actionNextPage->setObjectName("actionNextPage");
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/PlugyArrows/icons/plugy/right.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionNextPage->setIcon(icon12);
#if QT_CONFIG(shortcut)
        actionNextPage->setShortcut(QString::fromUtf8("Ctrl+Right"));
#endif // QT_CONFIG(shortcut)
        actionNextPage->setShortcutContext(Qt::ApplicationShortcut);
        actionNext10 = new QAction(MedianXLOfflineToolsClass);
        actionNext10->setObjectName("actionNext10");
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/PlugyArrows/icons/plugy/right10.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionNext10->setIcon(icon13);
#if QT_CONFIG(shortcut)
        actionNext10->setShortcut(QString::fromUtf8("Alt+Right"));
#endif // QT_CONFIG(shortcut)
        actionNext10->setShortcutContext(Qt::ApplicationShortcut);
        actionNext100 = new QAction(MedianXLOfflineToolsClass);
        actionNext100->setObjectName("actionNext100");
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/PlugyArrows/icons/plugy/right100.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionNext100->setIcon(icon14);
#if QT_CONFIG(shortcut)
        actionNext100->setShortcut(QString::fromUtf8("Alt+Shift+Right"));
#endif // QT_CONFIG(shortcut)
        actionNext100->setShortcutContext(Qt::ApplicationShortcut);
        actionLastPage = new QAction(MedianXLOfflineToolsClass);
        actionLastPage->setObjectName("actionLastPage");
        QIcon icon15;
        icon15.addFile(QString::fromUtf8(":/PlugyArrows/icons/plugy/last.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionLastPage->setIcon(icon15);
#if QT_CONFIG(shortcut)
        actionLastPage->setShortcut(QString::fromUtf8("Ctrl+Shift+Right"));
#endif // QT_CONFIG(shortcut)
        actionLastPage->setShortcutContext(Qt::ApplicationShortcut);
        actionShowAllStats = new QAction(MedianXLOfflineToolsClass);
        actionShowAllStats->setObjectName("actionShowAllStats");
        actionShowAllStats->setEnabled(false);
        actionCheckFileAssociations = new QAction(MedianXLOfflineToolsClass);
        actionCheckFileAssociations->setObjectName("actionCheckFileAssociations");
        actionCheckFileAssociations->setCheckable(true);
        actionCheckFileAssociations->setChecked(true);
        actionAssociate = new QAction(MedianXLOfflineToolsClass);
        actionAssociate->setObjectName("actionAssociate");
        actionCheckForUpdateOnStart = new QAction(MedianXLOfflineToolsClass);
        actionCheckForUpdateOnStart->setObjectName("actionCheckForUpdateOnStart");
        actionCheckForUpdateOnStart->setCheckable(true);
        actionCheckForUpdateOnStart->setChecked(true);
        actionCheckForUpdate = new QAction(MedianXLOfflineToolsClass);
        actionCheckForUpdate->setObjectName("actionCheckForUpdate");
        actionBackups1 = new QAction(MedianXLOfflineToolsClass);
        actionBackups1->setObjectName("actionBackups1");
        actionBackups1->setCheckable(true);
        actionBackups2 = new QAction(MedianXLOfflineToolsClass);
        actionBackups2->setObjectName("actionBackups2");
        actionBackups2->setCheckable(true);
        actionBackups5 = new QAction(MedianXLOfflineToolsClass);
        actionBackups5->setObjectName("actionBackups5");
        actionBackups5->setCheckable(true);
        actionBackups5->setChecked(true);
        actionBackups10 = new QAction(MedianXLOfflineToolsClass);
        actionBackups10->setObjectName("actionBackups10");
        actionBackups10->setCheckable(true);
        actionBackupsUnlimited = new QAction(MedianXLOfflineToolsClass);
        actionBackupsUnlimited->setObjectName("actionBackupsUnlimited");
        actionBackupsUnlimited->setCheckable(true);
        actionBackupFormatReadable = new QAction(MedianXLOfflineToolsClass);
        actionBackupFormatReadable->setObjectName("actionBackupFormatReadable");
        actionBackupFormatReadable->setCheckable(true);
        actionBackupFormatReadable->setChecked(true);
        actionBackupFormatTimestamp = new QAction(MedianXLOfflineToolsClass);
        actionBackupFormatTimestamp->setObjectName("actionBackupFormatTimestamp");
        actionBackupFormatTimestamp->setCheckable(true);
        actionPreviewDisenchantAlways = new QAction(MedianXLOfflineToolsClass);
        actionPreviewDisenchantAlways->setObjectName("actionPreviewDisenchantAlways");
        actionPreviewDisenchantAlways->setCheckable(true);
        actionPreviewDisenchantAlways->setChecked(true);
        actionPreviewDisenchantForSinglePage = new QAction(MedianXLOfflineToolsClass);
        actionPreviewDisenchantForSinglePage->setObjectName("actionPreviewDisenchantForSinglePage");
        actionPreviewDisenchantForSinglePage->setCheckable(true);
        actionPreviewDisenchantNever = new QAction(MedianXLOfflineToolsClass);
        actionPreviewDisenchantNever->setObjectName("actionPreviewDisenchantNever");
        actionPreviewDisenchantNever->setCheckable(true);
        actionOpenFileAssociationUI = new QAction(MedianXLOfflineToolsClass);
        actionOpenFileAssociationUI->setObjectName("actionOpenFileAssociationUI");
        actionSkillTree = new QAction(MedianXLOfflineToolsClass);
        actionSkillTree->setObjectName("actionSkillTree");
        actionSkillTree->setEnabled(false);
        centralWidget = new QWidget(MedianXLOfflineToolsClass);
        centralWidget->setObjectName("centralWidget");
        statsGroupBox = new QGroupBox(centralWidget);
        statsGroupBox->setObjectName("statsGroupBox");
        statsGroupBox->setEnabled(false);
        statsGroupBox->setGeometry(QRect(320, 10, 351, 271));
        freeStatPointsLineEdit = new QLineEdit(statsGroupBox);
        freeStatPointsLineEdit->setObjectName("freeStatPointsLineEdit");
        freeStatPointsLineEdit->setGeometry(QRect(10, 230, 60, 20));
        freeStatPointsLineEdit->setMaxLength(4);
        freeStatPointsLineEdit->setReadOnly(true);
        freeSkillPointsLineEdit = new QLineEdit(statsGroupBox);
        freeSkillPointsLineEdit->setObjectName("freeSkillPointsLineEdit");
        freeSkillPointsLineEdit->setGeometry(QRect(10, 50, 60, 20));
        freeSkillPointsLineEdit->setReadOnly(true);
        statsTableWidget = new QTableWidget(statsGroupBox);
        if (statsTableWidget->columnCount() < 2)
            statsTableWidget->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        statsTableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        statsTableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        if (statsTableWidget->rowCount() < 3)
            statsTableWidget->setRowCount(3);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        statsTableWidget->setVerticalHeaderItem(0, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        statsTableWidget->setVerticalHeaderItem(1, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        statsTableWidget->setVerticalHeaderItem(2, __qtablewidgetitem4);
        statsTableWidget->setObjectName("statsTableWidget");
        statsTableWidget->setGeometry(QRect(90, 90, 251, 115));
        QSizePolicy sizePolicy(QSizePolicy::Policy::Maximum, QSizePolicy::Policy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(statsTableWidget->sizePolicy().hasHeightForWidth());
        statsTableWidget->setSizePolicy(sizePolicy);
        statsTableWidget->setFrameShape(QFrame::StyledPanel);
        statsTableWidget->setFrameShadow(QFrame::Sunken);
        statsTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        statsTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        statsTableWidget->setAutoScroll(true);
        statsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        statsTableWidget->setTabKeyNavigation(false);
        statsTableWidget->setProperty("showDropIndicator", QVariant(false));
        statsTableWidget->setAlternatingRowColors(false);
        statsTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
        statsTableWidget->setGridStyle(Qt::SolidLine);
        statsTableWidget->setCornerButtonEnabled(false);
        statsTableWidget->horizontalHeader()->setCascadingSectionResizes(false);
        statsTableWidget->horizontalHeader()->setDefaultSectionSize(100);
        statsTableWidget->verticalHeader()->setCascadingSectionResizes(false);
        inventoryGoldLineEdit = new QLineEdit(statsGroupBox);
        inventoryGoldLineEdit->setObjectName("inventoryGoldLineEdit");
        inventoryGoldLineEdit->setGeometry(QRect(10, 20, 60, 20));
        inventoryGoldLineEdit->setReadOnly(true);
        stashGoldLineEdit = new QLineEdit(statsGroupBox);
        stashGoldLineEdit->setObjectName("stashGoldLineEdit");
        stashGoldLineEdit->setGeometry(QRect(200, 20, 65, 20));
        stashGoldLineEdit->setReadOnly(true);
        signetsOfLearningEatenLineEdit = new QLineEdit(statsGroupBox);
        signetsOfLearningEatenLineEdit->setObjectName("signetsOfLearningEatenLineEdit");
        signetsOfLearningEatenLineEdit->setGeometry(QRect(200, 230, 65, 20));
        signetsOfLearningEatenLineEdit->setReadOnly(true);
        respecSkillsCheckBox = new QCheckBox(statsGroupBox);
        respecSkillsCheckBox->setObjectName("respecSkillsCheckBox");
        respecSkillsCheckBox->setGeometry(QRect(80, 50, 83, 17));
        energySpinBox = new QSpinBox(statsGroupBox);
        energySpinBox->setObjectName("energySpinBox");
        energySpinBox->setGeometry(QRect(10, 170, 60, 20));
        energySpinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
        dexteritySpinBox = new QSpinBox(statsGroupBox);
        dexteritySpinBox->setObjectName("dexteritySpinBox");
        dexteritySpinBox->setGeometry(QRect(10, 118, 60, 20));
        dexteritySpinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
        strengthSpinBox = new QSpinBox(statsGroupBox);
        strengthSpinBox->setObjectName("strengthSpinBox");
        strengthSpinBox->setGeometry(QRect(10, 92, 60, 20));
        strengthSpinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
        vitalitySpinBox = new QSpinBox(statsGroupBox);
        vitalitySpinBox->setObjectName("vitalitySpinBox");
        vitalitySpinBox->setGeometry(QRect(10, 144, 60, 20));
        vitalitySpinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);
        respecStatsButton = new QPushButton(statsGroupBox);
        respecStatsButton->setObjectName("respecStatsButton");
        respecStatsButton->setGeometry(QRect(80, 230, 75, 23));
        respecStatsButton->setChecked(false);
        showAllStatsButton = new QPushButton(statsGroupBox);
        showAllStatsButton->setObjectName("showAllStatsButton");
        showAllStatsButton->setGeometry(QRect(80, 20, 75, 23));
        characterGroupBox = new QGroupBox(centralWidget);
        characterGroupBox->setObjectName("characterGroupBox");
        characterGroupBox->setEnabled(false);
        characterGroupBox->setGeometry(QRect(20, 10, 291, 171));
        renameButton = new QPushButton(characterGroupBox);
        renameButton->setObjectName("renameButton");
        renameButton->setGeometry(QRect(200, 20, 75, 23));
        classLineEdit = new QLineEdit(characterGroupBox);
        classLineEdit->setObjectName("classLineEdit");
        classLineEdit->setGeometry(QRect(53, 50, 135, 20));
        QSizePolicy sizePolicy1(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(classLineEdit->sizePolicy().hasHeightForWidth());
        classLineEdit->setSizePolicy(sizePolicy1);
        classLineEdit->setMinimumSize(QSize(0, 0));
        classLineEdit->setMaximumSize(QSize(16777215, 16777215));
        classLineEdit->setReadOnly(true);
        levelSpinBox = new QSpinBox(characterGroupBox);
        levelSpinBox->setObjectName("levelSpinBox");
        levelSpinBox->setGeometry(QRect(200, 80, 51, 20));
        levelSpinBox->setMinimum(1);
        levelSpinBox->setMaximum(120);
        levelSpinBox->setValue(1);
        titleLineEdit = new QLineEdit(characterGroupBox);
        titleLineEdit->setObjectName("titleLineEdit");
        titleLineEdit->setGeometry(QRect(53, 80, 135, 20));
        sizePolicy1.setHeightForWidth(titleLineEdit->sizePolicy().hasHeightForWidth());
        titleLineEdit->setSizePolicy(sizePolicy1);
        titleLineEdit->setMinimumSize(QSize(0, 0));
        titleLineEdit->setMaximumSize(QSize(16777215, 16777215));
        titleLineEdit->setReadOnly(true);
        hardcoreGroupBox = new QGroupBox(characterGroupBox);
        hardcoreGroupBox->setObjectName("hardcoreGroupBox");
        hardcoreGroupBox->setGeometry(QRect(10, 110, 275, 47));
        convertToSoftcoreCheckBox = new QCheckBox(hardcoreGroupBox);
        convertToSoftcoreCheckBox->setObjectName("convertToSoftcoreCheckBox");
        convertToSoftcoreCheckBox->setGeometry(QRect(10, 20, 119, 17));
        resurrectButton = new QPushButton(hardcoreGroupBox);
        resurrectButton->setObjectName("resurrectButton");
        resurrectButton->setEnabled(false);
        resurrectButton->setGeometry(QRect(190, 14, 75, 23));
        charNamePreviewLabel = new QLabel(characterGroupBox);
        charNamePreviewLabel->setObjectName("charNamePreviewLabel");
        charNamePreviewLabel->setGeometry(QRect(53, 20, 135, 20));
        charNamePreviewLabel->setText(QString::fromUtf8("name preview goes here"));
        mercGroupBox = new QGroupBox(centralWidget);
        mercGroupBox->setObjectName("mercGroupBox");
        mercGroupBox->setEnabled(false);
        mercGroupBox->setGeometry(QRect(20, 250, 291, 62));
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(mercGroupBox->sizePolicy().hasHeightForWidth());
        mercGroupBox->setSizePolicy(sizePolicy2);
        mercTypeComboBox = new QComboBox(mercGroupBox);
        mercTypeComboBox->setObjectName("mercTypeComboBox");
        mercTypeComboBox->setGeometry(QRect(10, 20, 91, 32));
        mercTypeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        mercNameComboBox = new QComboBox(mercGroupBox);
        mercNameComboBox->setObjectName("mercNameComboBox");
        mercNameComboBox->setGeometry(QRect(188, 20, 91, 32));
        mercNameComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        mercLevelLineEdit = new QLineEdit(mercGroupBox);
        mercLevelLineEdit->setObjectName("mercLevelLineEdit");
        mercLevelLineEdit->setGeometry(QRect(130, 20, 40, 20));
        mercLevelLineEdit->setReadOnly(true);
        waypointsGroupBox = new QGroupBox(centralWidget);
        waypointsGroupBox->setObjectName("waypointsGroupBox");
        waypointsGroupBox->setEnabled(false);
        waypointsGroupBox->setGeometry(QRect(20, 190, 291, 51));
        activateWaypointsCheckBox = new QCheckBox(waypointsGroupBox);
        activateWaypointsCheckBox->setObjectName("activateWaypointsCheckBox");
        activateWaypointsCheckBox->setGeometry(QRect(10, 20, 63, 17));
        MedianXLOfflineToolsClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MedianXLOfflineToolsClass);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 686, 24));
        menuBar->setNativeMenuBar(true);
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName("menuFile");
        menuFile->setSeparatorsCollapsible(true);
        menuRecentCharacters = new QMenu(menuFile);
        menuRecentCharacters->setObjectName("menuRecentCharacters");
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName("menuHelp");
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName("menuEdit");
        menuHardcore = new QMenu(menuEdit);
        menuHardcore->setObjectName("menuHardcore");
        menuWaypoints = new QMenu(menuEdit);
        menuWaypoints->setObjectName("menuWaypoints");
        menuRespec = new QMenu(menuEdit);
        menuRespec->setObjectName("menuRespec");
        menuOptions = new QMenu(menuBar);
        menuOptions->setObjectName("menuOptions");
        menuAuto_open_shared_stashes = new QMenu(menuOptions);
        menuAuto_open_shared_stashes->setObjectName("menuAuto_open_shared_stashes");
        menuBackupsLimit = new QMenu(menuOptions);
        menuBackupsLimit->setObjectName("menuBackupsLimit");
        menuBackupNameFormat = new QMenu(menuOptions);
        menuBackupNameFormat->setObjectName("menuBackupNameFormat");
        menuShow_disenchant_preview_dialog = new QMenu(menuOptions);
        menuShow_disenchant_preview_dialog->setObjectName("menuShow_disenchant_preview_dialog");
        menuItems = new QMenu(menuBar);
        menuItems->setObjectName("menuItems");
        menuGoToPage = new QMenu(menuItems);
        menuGoToPage->setObjectName("menuGoToPage");
        menuGoToPage->setEnabled(false);
        menuExport = new QMenu(menuBar);
        menuExport->setObjectName("menuExport");
        MedianXLOfflineToolsClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MedianXLOfflineToolsClass);
        mainToolBar->setObjectName("mainToolBar");
        mainToolBar->setEnabled(true);
        mainToolBar->setMovable(false);
        mainToolBar->setFloatable(false);
        MedianXLOfflineToolsClass->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MedianXLOfflineToolsClass);
        statusBar->setObjectName("statusBar");
        MedianXLOfflineToolsClass->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuItems->menuAction());
        menuBar->addAction(menuExport->menuAction());
        menuBar->addAction(menuOptions->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionLoadCharacter);
        menuFile->addAction(menuRecentCharacters->menuAction());
        menuFile->addAction(actionReloadCharacter);
        menuFile->addSeparator();
        menuFile->addAction(actionSaveCharacter);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuHelp->addAction(actionCheckForUpdate);
        menuHelp->addSeparator();
        menuHelp->addAction(actionAbout);
        menuHelp->addAction(actionAboutQt);
        menuEdit->addAction(actionRename);
        menuEdit->addAction(menuRespec->menuAction());
        menuEdit->addAction(menuWaypoints->menuAction());
        menuEdit->addAction(menuHardcore->menuAction());
        menuHardcore->addAction(actionConvertToSoftcore);
        menuHardcore->addAction(actionResurrect);
        menuWaypoints->addAction(actionActivateWaypoints);
        menuRespec->addAction(actionRespecStats);
        menuRespec->addAction(actionRespecSkills);
        menuOptions->addAction(actionLoadLastUsedCharacter);
        menuOptions->addAction(actionWarnWhenColoredName);
        menuOptions->addSeparator();
        menuOptions->addAction(actionBackup);
        menuOptions->addAction(menuBackupsLimit->menuAction());
        menuOptions->addAction(menuBackupNameFormat->menuAction());
        menuOptions->addSeparator();
        menuOptions->addAction(actionOpenItemsAutomatically);
        menuOptions->addAction(menuShow_disenchant_preview_dialog->menuAction());
        menuOptions->addSeparator();
        menuOptions->addAction(menuAuto_open_shared_stashes->menuAction());
        menuOptions->addAction(actionReloadSharedStashes);
        menuOptions->addSeparator();
        menuOptions->addAction(actionCheckFileAssociations);
        menuOptions->addAction(actionAssociate);
        menuOptions->addAction(actionOpenFileAssociationUI);
        menuOptions->addSeparator();
        menuOptions->addAction(actionCheckForUpdateOnStart);
        menuAuto_open_shared_stashes->addAction(actionAutoOpenPersonalStash);
        menuAuto_open_shared_stashes->addAction(actionAutoOpenSharedStash);
        menuAuto_open_shared_stashes->addAction(actionAutoOpenHCShared);
        menuBackupsLimit->addAction(actionBackups1);
        menuBackupsLimit->addAction(actionBackups2);
        menuBackupsLimit->addAction(actionBackups5);
        menuBackupsLimit->addAction(actionBackups10);
        menuBackupsLimit->addAction(actionBackupsUnlimited);
        menuBackupNameFormat->addAction(actionBackupFormatReadable);
        menuBackupNameFormat->addAction(actionBackupFormatTimestamp);
        menuShow_disenchant_preview_dialog->addAction(actionPreviewDisenchantAlways);
        menuShow_disenchant_preview_dialog->addAction(actionPreviewDisenchantForSinglePage);
        menuShow_disenchant_preview_dialog->addAction(actionPreviewDisenchantNever);
        menuItems->addAction(actionShowItems);
        menuItems->addAction(menuGoToPage->menuAction());
        menuItems->addSeparator();
        menuItems->addAction(actionFind);
        menuItems->addAction(actionFindNext);
        menuItems->addAction(actionFindPrevious);
        menuItems->addSeparator();
        menuItems->addAction(actionGiveCube);
        menuGoToPage->addAction(actionFirstPage);
        menuGoToPage->addAction(actionPrevious100);
        menuGoToPage->addAction(actionPrevious10);
        menuGoToPage->addAction(actionPreviousPage);
        menuGoToPage->addAction(actionNextPage);
        menuGoToPage->addAction(actionNext10);
        menuGoToPage->addAction(actionNext100);
        menuGoToPage->addAction(actionLastPage);
        menuExport->addAction(actionShowAllStats);
        mainToolBar->addAction(actionLoadCharacter);
        mainToolBar->addAction(actionReloadCharacter);
        mainToolBar->addAction(actionSaveCharacter);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionShowItems);
        mainToolBar->addAction(actionFind);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionSkillTree);
        mainToolBar->addAction(actionShowAllStats);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionAbout);

        retranslateUi(MedianXLOfflineToolsClass);
        QObject::connect(actionExit, &QAction::triggered, MedianXLOfflineToolsClass, qOverload<>(&QMainWindow::close));
        QObject::connect(actionRespecSkills, &QAction::triggered, respecSkillsCheckBox, &QCheckBox::setChecked);
        QObject::connect(actionActivateWaypoints, &QAction::triggered, activateWaypointsCheckBox, &QCheckBox::setChecked);
        QObject::connect(respecSkillsCheckBox, &QCheckBox::toggled, actionRespecSkills, &QAction::setChecked);
        QObject::connect(activateWaypointsCheckBox, &QCheckBox::toggled, actionActivateWaypoints, &QAction::setChecked);
        QObject::connect(actionRespecStats, &QAction::triggered, respecStatsButton, qOverload<>(&QPushButton::click));
        QObject::connect(actionResurrect, &QAction::triggered, resurrectButton, qOverload<>(&QPushButton::click));
        QObject::connect(actionConvertToSoftcore, &QAction::triggered, convertToSoftcoreCheckBox, &QCheckBox::setChecked);
        QObject::connect(convertToSoftcoreCheckBox, &QCheckBox::clicked, actionConvertToSoftcore, &QAction::setChecked);
        QObject::connect(actionShowAllStats, &QAction::triggered, showAllStatsButton, qOverload<>(&QPushButton::click));

        QMetaObject::connectSlotsByName(MedianXLOfflineToolsClass);
    } // setupUi

    void retranslateUi(QMainWindow *MedianXLOfflineToolsClass)
    {
        actionLoadCharacter->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "L&oad Character...", nullptr));
#if QT_CONFIG(shortcut)
        actionLoadCharacter->setShortcut(QCoreApplication::translate("MedianXLOfflineToolsClass", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSaveCharacter->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "&Save Character", nullptr));
#if QT_CONFIG(shortcut)
        actionSaveCharacter->setShortcut(QCoreApplication::translate("MedianXLOfflineToolsClass", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionExit->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Exit", nullptr));
        actionAbout->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "About", nullptr));
#if QT_CONFIG(shortcut)
        actionAbout->setShortcut(QCoreApplication::translate("MedianXLOfflineToolsClass", "F1", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAboutQt->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "About Qt", nullptr));
        actionAboutQt->setIconText(QCoreApplication::translate("MedianXLOfflineToolsClass", "About Qt", nullptr));
#if QT_CONFIG(tooltip)
        actionAboutQt->setToolTip(QCoreApplication::translate("MedianXLOfflineToolsClass", "About Qt", nullptr));
#endif // QT_CONFIG(tooltip)
        actionRespecStats->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Respec Stats", nullptr));
#if QT_CONFIG(shortcut)
        actionRespecStats->setShortcut(QCoreApplication::translate("MedianXLOfflineToolsClass", "F5", nullptr));
#endif // QT_CONFIG(shortcut)
        actionRespecSkills->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Respec Skills", nullptr));
#if QT_CONFIG(shortcut)
        actionRespecSkills->setShortcut(QCoreApplication::translate("MedianXLOfflineToolsClass", "F6", nullptr));
#endif // QT_CONFIG(shortcut)
        actionActivateWaypoints->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Activate Waypoints", nullptr));
#if QT_CONFIG(shortcut)
        actionActivateWaypoints->setShortcut(QCoreApplication::translate("MedianXLOfflineToolsClass", "F7", nullptr));
#endif // QT_CONFIG(shortcut)
        actionRename->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Rename", nullptr));
#if QT_CONFIG(shortcut)
        actionRename->setShortcut(QCoreApplication::translate("MedianXLOfflineToolsClass", "Alt+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionReloadCharacter->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "&Reload Character", nullptr));
#if QT_CONFIG(shortcut)
        actionReloadCharacter->setShortcut(QCoreApplication::translate("MedianXLOfflineToolsClass", "Ctrl+R", nullptr));
#endif // QT_CONFIG(shortcut)
        actionConvertToSoftcore->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Convert to Softcore", nullptr));
        actionResurrect->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Resurrect", nullptr));
        actionToolbar->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Toolbar", nullptr));
        actionShowItems->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Show Items", nullptr));
#if QT_CONFIG(tooltip)
        actionShowItems->setToolTip(QCoreApplication::translate("MedianXLOfflineToolsClass", "Show Items", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionShowItems->setShortcut(QCoreApplication::translate("MedianXLOfflineToolsClass", "Ctrl+I", nullptr));
#endif // QT_CONFIG(shortcut)
        actionLoadLastUsedCharacter->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Load last used character", nullptr));
        actionGiveCube->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Give me the Cube", nullptr));
#if QT_CONFIG(statustip)
        actionGiveCube->setStatusTip(QCoreApplication::translate("MedianXLOfflineToolsClass", "Present your character a new Horadric Cube", nullptr));
#endif // QT_CONFIG(statustip)
        actionOpenItemsAutomatically->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Auto-open items window", nullptr));
#if QT_CONFIG(statustip)
        actionOpenItemsAutomatically->setStatusTip(QCoreApplication::translate("MedianXLOfflineToolsClass", "Open items window when a character is loaded", nullptr));
#endif // QT_CONFIG(statustip)
        actionFind->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Find...", nullptr));
#if QT_CONFIG(tooltip)
        actionFind->setToolTip(QCoreApplication::translate("MedianXLOfflineToolsClass", "Find Items", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(shortcut)
        actionFind->setShortcut(QCoreApplication::translate("MedianXLOfflineToolsClass", "Ctrl+F", nullptr));
#endif // QT_CONFIG(shortcut)
        actionFindNext->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Find next", nullptr));
        actionFindPrevious->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Find previous", nullptr));
        actionReloadSharedStashes->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Always reload shared stashes", nullptr));
#if QT_CONFIG(statustip)
        actionReloadSharedStashes->setStatusTip(QCoreApplication::translate("MedianXLOfflineToolsClass", "Reload shared stashes when loading a character (may be slow)", nullptr));
#endif // QT_CONFIG(statustip)
        actionAutoOpenPersonalStash->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Personal", nullptr));
        actionAutoOpenSharedStash->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Shared", nullptr));
        actionAutoOpenHCShared->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Hardcore", nullptr));
#if QT_CONFIG(tooltip)
        actionAutoOpenHCShared->setToolTip(QCoreApplication::translate("MedianXLOfflineToolsClass", "Hardcore", nullptr));
#endif // QT_CONFIG(tooltip)
        actionBackup->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Make backups before saving", nullptr));
        actionWarnWhenColoredName->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Warn when new name has color", nullptr));
#if QT_CONFIG(statustip)
        actionWarnWhenColoredName->setStatusTip(QCoreApplication::translate("MedianXLOfflineToolsClass", "Show confirmation dialog when using color in new name", nullptr));
#endif // QT_CONFIG(statustip)
        actionFirstPage->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "First", nullptr));
        actionPrevious100->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Previous 100", nullptr));
        actionPrevious10->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Previous 10", nullptr));
        actionPreviousPage->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Previous", nullptr));
        actionNextPage->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Next", nullptr));
        actionNext10->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Next 10", nullptr));
        actionNext100->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Next 100", nullptr));
        actionLastPage->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Last", nullptr));
        actionShowAllStats->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "All character stats", nullptr));
        actionShowAllStats->setIconText(QCoreApplication::translate("MedianXLOfflineToolsClass", "All stats", nullptr));
#if QT_CONFIG(shortcut)
        actionShowAllStats->setShortcut(QCoreApplication::translate("MedianXLOfflineToolsClass", "Ctrl+A", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCheckFileAssociations->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Check file associations on start", nullptr));
        actionAssociate->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Associate with .d2s files", nullptr));
        actionCheckForUpdateOnStart->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Check for update on start", nullptr));
        actionCheckForUpdate->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Check for Update", nullptr));
        actionBackups1->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "1", nullptr));
        actionBackups2->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "2", nullptr));
        actionBackups5->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "5", nullptr));
        actionBackups10->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "10", nullptr));
        actionBackupsUnlimited->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Unlimited", nullptr));
        actionPreviewDisenchantAlways->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Always", nullptr));
        actionPreviewDisenchantForSinglePage->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Only for single page", nullptr));
        actionPreviewDisenchantNever->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Never", nullptr));
        actionOpenFileAssociationUI->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Open file association UI", nullptr));
        actionSkillTree->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Skill Tree", nullptr));
#if QT_CONFIG(shortcut)
        actionSkillTree->setShortcut(QCoreApplication::translate("MedianXLOfflineToolsClass", "Ctrl+T", nullptr));
#endif // QT_CONFIG(shortcut)
        statsGroupBox->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Stats", nullptr));
        QTableWidgetItem *___qtablewidgetitem = statsTableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Current", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = statsTableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Base", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = statsTableWidget->verticalHeaderItem(0);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Life", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = statsTableWidget->verticalHeaderItem(1);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Mana", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = statsTableWidget->verticalHeaderItem(2);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Stamina", nullptr));
        respecSkillsCheckBox->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Respec Skills", nullptr));
        energySpinBox->setSpecialValueText(QString());
        dexteritySpinBox->setSpecialValueText(QString());
        strengthSpinBox->setSpecialValueText(QString());
        vitalitySpinBox->setSpecialValueText(QString());
        respecStatsButton->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Respec Stats", nullptr));
        showAllStatsButton->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Show all stats", nullptr));
        characterGroupBox->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Character", nullptr));
        renameButton->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Rename", nullptr));
        hardcoreGroupBox->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Hardcore", nullptr));
        convertToSoftcoreCheckBox->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Convert to Softcore", nullptr));
        resurrectButton->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Resurrect", nullptr));
        mercGroupBox->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Mercenary", nullptr));
        waypointsGroupBox->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Waypoints", nullptr));
        activateWaypointsCheckBox->setText(QCoreApplication::translate("MedianXLOfflineToolsClass", "Activate", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "&File", nullptr));
        menuRecentCharacters->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Recent Characters", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "&Help", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "&Edit", nullptr));
        menuHardcore->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Hardcore", nullptr));
        menuWaypoints->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Waypoints", nullptr));
        menuRespec->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Respec", nullptr));
        menuOptions->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "&Options", nullptr));
        menuAuto_open_shared_stashes->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Auto-open shared stashes", nullptr));
        menuBackupsLimit->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Backups limit", nullptr));
        menuBackupNameFormat->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Backup name format", nullptr));
        menuShow_disenchant_preview_dialog->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Show disenchant preview dialog", nullptr));
        menuItems->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "&Items", nullptr));
        menuGoToPage->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Go to page", nullptr));
        menuExport->setTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "E&xport", nullptr));
        mainToolBar->setWindowTitle(QCoreApplication::translate("MedianXLOfflineToolsClass", "Toolbar", nullptr));
        (void)MedianXLOfflineToolsClass;
    } // retranslateUi

};

namespace Ui {
    class MedianXLOfflineToolsClass: public Ui_MedianXLOfflineToolsClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MEDIANXLOFFLINETOOLS_H
