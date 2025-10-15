#include "oilcreationwidget.h"
#include "structs.h"
#include "itemdatabase.h"
#include "reversebitwriter.h"
#include "resourcepathmanager.hpp"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QDebug>

OilCreationWidget::OilCreationWidget(QWidget *parent)
    : QDialog(parent), _createdOil(nullptr), _targetRow(0), _targetCol(0)
{
    setupUI();
    populateOilList();

    setWindowTitle(tr("Create Oil"));
    setModal(true);
    resize(420, 260);
}

OilCreationWidget::~OilCreationWidget()
{
}

void OilCreationWidget::setupUI()
{
    QVBoxLayout *main = new QVBoxLayout(this);

    QLabel *title = new QLabel(tr("Select oil to create"));
    title->setStyleSheet("font-weight:bold; font-size:13px;");
    main->addWidget(title);

    QGridLayout *grid = new QGridLayout();
    grid->addWidget(new QLabel(tr("Oil: ")), 0, 0);
    _oilCombo = new QComboBox();
    _oilCombo->setMinimumWidth(220);
    grid->addWidget(_oilCombo, 0, 1);

    grid->addWidget(new QLabel(tr("Row: ")), 1, 0);
    _rowSpin = new QSpinBox(); _rowSpin->setRange(0, 9); grid->addWidget(_rowSpin, 1, 1);
    grid->addWidget(new QLabel(tr("Column: ")), 2, 0);
    _colSpin = new QSpinBox(); _colSpin->setRange(0, 9); grid->addWidget(_colSpin, 2, 1);
    grid->addWidget(new QLabel(tr("Copies: ")), 3, 0);
    _copiesSpin = new QSpinBox(); _copiesSpin->setRange(1, 999); _copiesSpin->setValue(1); grid->addWidget(_copiesSpin, 3, 1);

    main->addLayout(grid);

    _previewLabel = new QLabel(); _previewLabel->setFrameStyle(QFrame::Box);
    _previewLabel->setMinimumHeight(60); _previewLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    _previewLabel->setStyleSheet("background:#f7f7f7; padding:8px;");
    main->addWidget(_previewLabel);

    QHBoxLayout *buttons = new QHBoxLayout(); buttons->addStretch();
    _createButton = new QPushButton(tr("Create Oil")); _createButton->setDefault(true);
    _cancelButton = new QPushButton(tr("Cancel"));
    buttons->addWidget(_createButton); buttons->addWidget(_cancelButton);
    main->addLayout(buttons);

    connect(_oilCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectionChanged()));
    connect(_createButton, SIGNAL(clicked()), this, SLOT(onCreateClicked()));
    connect(_cancelButton, SIGNAL(clicked()), this, SLOT(onCancelClicked()));

    onSelectionChanged();
}

void OilCreationWidget::populateOilList()
{
    _oilCombo->clear();
    const QStringList oilCodes = {
        "1p[","2o[","2p[","3p[","4o[","4p[","5o[","6o[","7o[","9o[",
        "ebs1","ebs2","ebs3","ebt1","ebt2","ebt3","ooc"
    };

    for (const QString &code : oilCodes) {
        ItemBase *b = ItemDataBase::Items()->value(code.toUtf8());
        QString label = b ? QString("%1 - %2").arg(b->name).arg(code) : code;
        _oilCombo->addItem(label, code);
    }
}

void OilCreationWidget::onSelectionChanged()
{
    QString code = _oilCombo->currentData().toString();
    ItemBase *b = ItemDataBase::Items()->value(code.toUtf8());
    QString txt = QString("<b>%1</b><br>Code: %2<br>Size: %3x%4<br>Req Lvl: %5")
            .arg(b ? b->name : QString("<unknown>"))
            .arg(code)
            .arg(b ? b->width : 0)
            .arg(b ? b->height : 0)
            .arg(b ? b->rlvl : 0);
    _previewLabel->setText(txt);
}

void OilCreationWidget::setItemPosition(int row, int column)
{
    _targetRow = row; _targetCol = column;
    _rowSpin->setValue(row); _colSpin->setValue(column);
}

void OilCreationWidget::onCreateClicked()
{
    QString code = _oilCombo->currentData().toString();
    if (code.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an oil to create."));
        return;
    }

    _copies = _copiesSpin->value();

    // create first oil to return via getCreatedOil (others will be added by caller via ItemsPropertiesSplitter)
    ItemInfo *oil = createOilItem(code);
    if (!oil) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create oil."));
        return;
    }
    _createdOil = oil;
    accept();
}

void OilCreationWidget::onCancelClicked()
{
    reject();
}

QString OilCreationWidget::selectedCode() const
{
    return _oilCombo->currentData().toString();
}

ItemInfo* OilCreationWidget::createOilItem(const QString &code)
{
    // Try to load canonical oil template
    const QStringList tplCandidates = { QStringLiteral("4o["), QStringLiteral("4o[.d2i"), QStringLiteral("oil"), QStringLiteral("oil.d2i") };
    ItemInfo *tpl = nullptr;
    for (const QString &cand : tplCandidates) {
        tpl = ItemDataBase::loadItemFromFile(cand);
        if (tpl) break;
    }
    if (!tpl) {
        QString explicitPath = ResourcePathManager::dataPathForFileName("items/4o[.d2i");
        if (!explicitPath.isEmpty() && QFile::exists(explicitPath))
            tpl = ItemDataBase::loadItemFromFile(explicitPath);
    }

    if (!tpl) {
        qWarning() << "Oil template not found";
        return nullptr;
    }

    ItemInfo *item = new ItemInfo(*tpl);

    // Update 4-bytes itemType
    QByteArray newType = code.toUtf8();
    while (newType.length() < 4) newType.append(' ');
    for (int i = 0; i < 4; ++i)
        ReverseBitWriter::replaceValueInBitString(item->bitString, Enums::ItemOffsets::Type + i * 8, newType.at(i));

    item->move(_rowSpin->value(), _colSpin->value(), 0, true);
    item->storage = -1;
    item->hasChanged = true;
    item->itemType = code.toUtf8();
    ReverseBitWriter::byteAlignBits(item->bitString);
    return item;
}
