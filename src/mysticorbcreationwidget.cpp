#include "mysticorbcreationwidget.h"
#include "structs.h"
#include "itemdatabase.h"
#include "reversebitwriter.h"
#include "resourcepathmanager.hpp"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <QComboBox>
#include <QDir>
#include <QTextStream>
#ifdef HAS_QTSQL
#include <QtSql>
#endif

MysticOrbCreationWidget::MysticOrbCreationWidget(QWidget *parent)
    : QDialog(parent), _createdOrb(nullptr), _targetRow(0), _targetCol(0), _copies(1)
{
    setupUI();
    setWindowTitle(tr("Create Mystic Orb"));
    setModal(true);
    resize(420, 260);
}

MysticOrbCreationWidget::~MysticOrbCreationWidget()
{
}

void MysticOrbCreationWidget::setupUI()
{
    QVBoxLayout *main = new QVBoxLayout(this);

    QLabel *title = new QLabel(tr("Create Mystic Orb (using template 47+)"));
    title->setStyleSheet("font-weight:bold; font-size:13px;");
    main->addWidget(title);

    QGridLayout *grid = new QGridLayout();
    grid->addWidget(new QLabel(tr("Row: ")), 0, 0);
    _rowSpin = new QSpinBox(); _rowSpin->setRange(0, 9); grid->addWidget(_rowSpin, 0, 1);
    grid->addWidget(new QLabel(tr("Column: ")), 1, 0);
    _colSpin = new QSpinBox(); _colSpin->setRange(0, 9); grid->addWidget(_colSpin, 1, 1);
    grid->addWidget(new QLabel(tr("Copies: ")), 2, 0);
    _copiesSpin = new QSpinBox(); _copiesSpin->setRange(1, 999); _copiesSpin->setValue(1); grid->addWidget(_copiesSpin, 2, 1);

    // Orb selection combo (loaded from items.tsv)
    grid->addWidget(new QLabel(tr("Orb type: ")), 3, 0);
    _orbCombo = new QComboBox(); grid->addWidget(_orbCombo, 3, 1);

    main->addLayout(grid);

    _previewLabel = new QLabel(); _previewLabel->setFrameStyle(QFrame::Box);
    _previewLabel->setMinimumHeight(60); _previewLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    _previewLabel->setStyleSheet("background:#f7f7f7; padding:8px;");
    main->addWidget(_previewLabel);

    QHBoxLayout *buttons = new QHBoxLayout(); buttons->addStretch();
    _createButton = new QPushButton(tr("Create Mystic Orb")); _createButton->setDefault(true);
    _cancelButton = new QPushButton(tr("Cancel"));
    buttons->addWidget(_createButton); buttons->addWidget(_cancelButton);
    main->addLayout(buttons);

    connect(_createButton, SIGNAL(clicked()), this, SLOT(onCreateClicked()));
    connect(_cancelButton, SIGNAL(clicked()), this, SLOT(onCancelClicked()));

    _previewLabel->setText(tr("Template: resources/items/47+.d2i"));

    // First try to load orbs from a generated SQLite DB (resources/data/items.db)
    bool loaded = false;
#ifdef HAS_QTSQL
    QString dbPath = QCoreApplication::applicationDirPath() + QStringLiteral("/../resources/data/items.db");
    if (QFile::exists(dbPath)) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "itemsdb_conn");
        db.setDatabaseName(dbPath);
        if (db.open()) {
            QSqlQuery q(db);
            if (q.exec("SELECT code, name FROM items WHERE name LIKE '%Mystic Orb%' OR name LIKE '%Mystic%' OR tags LIKE '%myst%';")) {
                while (q.next()) {
                    QString code = q.value(0).toString();
                    QString name = q.value(1).toString();
                    if (!code.isEmpty()) {
                        _orbCombo->addItem(code + " - " + name, code);
                    }
                }
                if (_orbCombo->count() > 0) loaded = true;
            }
            db.close();
        }
        QSqlDatabase::removeDatabase("itemsdb_conn");
    }
#endif

    if (!loaded) {
        // TSV fallback: best-effort parse from generated items.tsv
        QStringList fallback = {QStringLiteral("47+"), QStringLiteral("01+"), QStringLiteral("02+"), QStringLiteral("mo01")};
        QString candidate = QDir::cleanPath(QCoreApplication::applicationDirPath() + QStringLiteral("/../utils/txt_parser/generated/en/items.tsv"));
        QFile f(candidate);
        if (f.exists() && f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&f);
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.contains(QLatin1String("Mystic Orb"), Qt::CaseInsensitive) || line.contains(QLatin1String("\tmyst"))) {
                    QStringList parts = line.split('\t');
                    if (parts.size() >= 2) {
                        QString code = parts[0].trimmed();
                        QString name = parts[1].trimmed();
                        if (!code.isEmpty()) _orbCombo->addItem(code + " - " + name, code);
                    }
                }
            }
            f.close();
        }
        if (_orbCombo->count() == 0) {
            for (const QString &c : fallback) _orbCombo->addItem(c, c);
        }
    }
}

void MysticOrbCreationWidget::setItemPosition(int row, int column)
{
    _targetRow = row; _targetCol = column;
    _rowSpin->setValue(row); _colSpin->setValue(column);
}

void MysticOrbCreationWidget::onCreateClicked()
{
    _copies = _copiesSpin->value();

    ItemInfo *orb = createMysticOrbItem();
    if (!orb) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create Mystic Orb from template."));
        return;
    }
    _createdOrb = orb;
    accept();
}

void MysticOrbCreationWidget::onCancelClicked()
{
    reject();
}

ItemInfo* MysticOrbCreationWidget::createMysticOrbItem()
{
    // Determine selected orb code (prefer explicit selection)
    QString selectedCode;
    if (_orbCombo && _orbCombo->currentIndex() >= 0)
        selectedCode = _orbCombo->currentData().toString();
    if (selectedCode.isEmpty()) selectedCode = QStringLiteral("47+");

    // Try to load canonical template by the selected code
    ItemInfo *tpl = ItemDataBase::loadItemFromFile(selectedCode);
    if (!tpl) {
        // Try file path fallback (items/<code>.d2i)
        QString explicitPath = ResourcePathManager::dataPathForFileName(QStringLiteral("items/%1.d2i").arg(selectedCode));
        if (!explicitPath.isEmpty() && QFile::exists(explicitPath))
            tpl = ItemDataBase::loadItemFromFile(explicitPath);
        else {
            // fallback to known 47+
            tpl = ItemDataBase::loadItemFromFile(QStringLiteral("47+"));
            if (!tpl) {
                QString explicit47 = ResourcePathManager::dataPathForFileName("items/47+.d2i");
                if (!explicit47.isEmpty() && QFile::exists(explicit47))
                    tpl = ItemDataBase::loadItemFromFile(explicit47);
            }
        }
    }

    if (!tpl) {
        // qWarning() << "Mystic Orb template (47+) not found";
        return nullptr;
    }

    ItemInfo *item = new ItemInfo(*tpl);

    // Replace template itemType with selected code (space-padded to 4 bytes)
    QByteArray newType = selectedCode.toUtf8();
    // Ensure exactly 4 bytes (pad with spaces or truncate)
    while (newType.length() < 4) newType.append(' ');
    if (newType.length() > 4) newType = newType.left(4);
    for (int i = 0; i < 4; ++i)
        ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Type + i * 8, newType.at(i));

    item->move(_rowSpin->value(), _colSpin->value(), 0, true);
    item->storage = -1;
    item->hasChanged = true;
    item->itemType = selectedCode.toUtf8();
    ReverseBitWriter::byteAlignBits(item->bitString);
    return item;
}
