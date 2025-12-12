#include "runecreationwidget.h"
#include "structs.h"
#include "itemdatabase.h"
#include "enums.h"
#include "reversebitwriter.h"
#include "characterinfo.hpp"
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <cstdlib>

RuneCreationWidget::RuneCreationWidget(QWidget *parent)
    : QDialog(parent), _createdRune(nullptr), _targetRow(0), _targetColumn(0)
{
    setupUI();
    populateRuneList();
    
    setWindowTitle(tr("Create Rune"));
    setModal(true);
    resize(400, 300);
}

RuneCreationWidget::~RuneCreationWidget()
{
    // _createdRune is managed by the caller
}

void RuneCreationWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Title
    QLabel *titleLabel = new QLabel(tr("Select Rune to Create"));
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(titleLabel);
    
    // Rune selection
    QGridLayout *gridLayout = new QGridLayout();
    
    gridLayout->addWidget(new QLabel(tr("Rune:")), 0, 0);
    _runeCombo = new QComboBox();
    _runeCombo->setMinimumWidth(200);
    gridLayout->addWidget(_runeCombo, 0, 1);
    
    // Position
    gridLayout->addWidget(new QLabel(tr("Row:")), 1, 0);
    _rowSpin = new QSpinBox();
    _rowSpin->setRange(0, 9);
    gridLayout->addWidget(_rowSpin, 1, 1);
    
    gridLayout->addWidget(new QLabel(tr("Column:")), 2, 0);
    _columnSpin = new QSpinBox();
    _columnSpin->setRange(0, 9);
    gridLayout->addWidget(_columnSpin, 2, 1);
    gridLayout->addWidget(new QLabel(tr("Copies: ")), 3, 0);
    _copiesSpin = new QSpinBox(); _copiesSpin->setRange(1, 999); _copiesSpin->setValue(1); gridLayout->addWidget(_copiesSpin, 3, 1);
    
    // Copies control already present; enchanted option removed
    
    mainLayout->addLayout(gridLayout);
    
    // Preview area
    _previewLabel = new QLabel();
    _previewLabel->setFrameStyle(QFrame::Box);
    _previewLabel->setMinimumHeight(80);
    _previewLabel->setAlignment(Qt::AlignCenter);
    _previewLabel->setStyleSheet("background-color: #f0f0f0; margin: 10px; padding: 10px;");
    mainLayout->addWidget(_previewLabel);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    _createButton = new QPushButton(tr("Create Rune"));
    _createButton->setDefault(true);
    _createButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; padding: 8px 16px; }");
    buttonLayout->addWidget(_createButton);
    
    _cancelButton = new QPushButton(tr("Cancel"));
    _cancelButton->setStyleSheet("QPushButton { padding: 8px 16px; }");
    buttonLayout->addWidget(_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(_runeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onRuneSelectionChanged()));
    connect(_createButton, SIGNAL(clicked()), this, SLOT(onCreateClicked()));
    connect(_cancelButton, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
    
    // Initial preview update
    onRuneSelectionChanged();
}

void RuneCreationWidget::populateRuneList()
{
    _runeCombo->clear();
    
    // Add all runes from MedianXL
    struct RuneInfo {
        QString code;
        QString name;
        QString description;
    };
    
    QList<RuneInfo> runes;

    // Standard Diablo 2 Runes (r01-r33)
    runes << RuneInfo{"r01", "El Rune", "El Rune"};
    runes << RuneInfo{"r02", "Eld Rune", "Eld Rune"};
    runes << RuneInfo{"r03", "Tir Rune", "Tir Rune"};
    runes << RuneInfo{"r04", "Nef Rune", "Nef Rune"};
    runes << RuneInfo{"r05", "Eth Rune", "Eth Rune"};
    runes << RuneInfo{"r06", "Ith Rune", "Ith Rune"};
    runes << RuneInfo{"r07", "Tal Rune", "Tal Rune"};
    runes << RuneInfo{"r08", "Ral Rune", "Ral Rune"};
    runes << RuneInfo{"r09", "Ort Rune", "Ort Rune"};
    runes << RuneInfo{"r10", "Thul Rune", "Thul Rune"};
    runes << RuneInfo{"r11", "Amn Rune", "Amn Rune"};
    runes << RuneInfo{"r12", "Sol Rune", "Sol Rune"};
    runes << RuneInfo{"r13", "Shael Rune", "Shael Rune"};
    runes << RuneInfo{"r14", "Dol Rune", "Dol Rune"};
    runes << RuneInfo{"r15", "Hel Rune", "Hel Rune"};
    runes << RuneInfo{"r16", "Io Rune", "Io Rune"};
    runes << RuneInfo{"r17", "Lum Rune", "Lum Rune"};
    runes << RuneInfo{"r18", "Ko Rune", "Ko Rune"};
    runes << RuneInfo{"r19", "Fal Rune", "Fal Rune"};
    runes << RuneInfo{"r20", "Lem Rune", "Lem Rune"};
    runes << RuneInfo{"r21", "Pul Rune", "Pul Rune"};
    runes << RuneInfo{"r22", "Um Rune", "Um Rune"};
    runes << RuneInfo{"r23", "Mal Rune", "Mal Rune"};
    runes << RuneInfo{"r24", "Ist Rune", "Ist Rune"};
    runes << RuneInfo{"r25", "Gul Rune", "Gul Rune"};
    runes << RuneInfo{"r26", "Vex Rune", "Vex Rune"};
    runes << RuneInfo{"r27", "Ohm Rune", "Ohm Rune"};
    runes << RuneInfo{"r28", "Lo Rune", "Lo Rune"};
    runes << RuneInfo{"r29", "Sur Rune", "Sur Rune"};
    runes << RuneInfo{"r30", "Ber Rune", "Ber Rune"};
    runes << RuneInfo{"r31", "Jah Rune", "Jah Rune"};
    runes << RuneInfo{"r32", "Cham Rune", "Cham Rune"};
    runes << RuneInfo{"r33", "Zod Rune", "Zod Rune"};

    // MedianXL Extended Runes (r34-r53)
    runes << RuneInfo{"r51", "Taha Rune", "Taha Rune"};
    runes << RuneInfo{"r52", "Ghal Rune", "Ghal Rune"};
    runes << RuneInfo{"r53", "Qor Rune", "Qor Rune"};

    // MedianXL Enchanted Runes (r54-r62, r98-r99)
    runes << RuneInfo{"r54", "Krys Rune", "Krys Rune"};
    runes << RuneInfo{"r55", "Auhe Rune", "Auhe Rune"};
    runes << RuneInfo{"r56", "Sha'ad Rune", "Sha'ad Rune"};
    runes << RuneInfo{"r57", "Fire Rune", "Fire Rune"};
    runes << RuneInfo{"r58", "Stone Rune", "Stone Rune"};
    runes << RuneInfo{"r59", "Arcane Rune", "Arcane Rune"};
    runes << RuneInfo{"r60", "Poison Rune", "Poison Rune"};
    runes << RuneInfo{"r61", "Light Rune", "Light Rune"};
    runes << RuneInfo{"r62", "Ice Rune", "Ice Rune"};
    runes << RuneInfo{"r98", "Xis Rune", "Can be Cubed with Low Rune"};
    runes << RuneInfo{"r99", "Runestone", "Runestone"};

    // Super Runes (rx01-rx33)
    runes << RuneInfo{"rx01", "Ol Rune", "Ol Rune"};
    runes << RuneInfo{"rx02", "Elq Rune", "Elq Rune"};
    runes << RuneInfo{"rx03", "Tyr Rune", "Tyr Rune"};
    runes << RuneInfo{"rx04", "Nif Rune", "Nif Rune"};
    runes << RuneInfo{"rx05", "Xeth Rune", "Xeth Rune"};
    runes << RuneInfo{"rx06", "Xith Rune", "Xith Rune"};
    runes << RuneInfo{"rx07", "Thal Rune", "Thal Rune"};
    runes << RuneInfo{"rx08", "Rhal Rune", "Rhal Rune"};
    runes << RuneInfo{"rx09", "Urt Rune", "Urt Rune"};
    runes << RuneInfo{"rx10", "Tuul Rune", "Tuul Rune"};
    runes << RuneInfo{"rx11", "Ahmn Rune", "Ahmn Rune"};
    runes << RuneInfo{"rx12", "Zol Rune", "Zol Rune"};
    runes << RuneInfo{"rx13", "Shaen Rune", "Shaen Rune"};
    runes << RuneInfo{"rx14", "Doj Rune", "Doj Rune"};
    runes << RuneInfo{"rx15", "Hem Rune", "Hem Rune"};
    runes << RuneInfo{"rx16", "Iu Rune", "Iu Rune"};
    runes << RuneInfo{"rx17", "Lux Rune", "Lux Rune"};
    runes << RuneInfo{"rx18", "Ka Rune", "Ka Rune"};
    runes << RuneInfo{"rx19", "Fel Rune", "Fel Rune"};
    runes << RuneInfo{"rx20", "Lew Rune", "Lew Rune"};
    runes << RuneInfo{"rx21", "Phul Rune", "Phul Rune"};
    runes << RuneInfo{"rx22", "Un Rune", "Un Rune"};
    runes << RuneInfo{"rx23", "Mhal Rune", "Mhal Rune"};
    runes << RuneInfo{"rx24", "Yst Rune", "Yst Rune"};
    runes << RuneInfo{"rx25", "Gur Rune", "Gur Rune"};
    runes << RuneInfo{"rx26", "Vez Rune", "Vez Rune"};
    runes << RuneInfo{"rx27", "Ohn Rune", "Ohn Rune"};
    runes << RuneInfo{"rx28", "Loz Rune", "Loz Rune"};
    runes << RuneInfo{"rx29", "Zur Rune", "Zur Rune"};
    runes << RuneInfo{"rx30", "Bur Rune", "Bur Rune"};
    runes << RuneInfo{"rx31", "Iah Rune", "Iah Rune"};
    runes << RuneInfo{"rx32", "Yham Rune", "Yham Rune"};
    runes << RuneInfo{"rx33", "Xod Rune", "Xod Rune"};
    
    foreach (const RuneInfo &rune, runes) {
        _runeCombo->addItem(QString("%1 - %2").arg(rune.name).arg(rune.description), rune.code);
    }
}

void RuneCreationWidget::onRuneSelectionChanged()
{
    QString runeCode = _runeCombo->currentData().toString();
    QString runeName = _runeCombo->currentText().split(" - ").first();
    
    if (!runeCode.isEmpty()) {
        QString preview = QString("<b>%1</b><br>Code: %2<br>Position: (%3, %4)")
                         .arg(runeName)
                         .arg(runeCode)
                         .arg(_rowSpin->value())
                         .arg(_columnSpin->value());
        _previewLabel->setText(preview);
    }
}

void RuneCreationWidget::onCreateClicked()
{
    QString runeCode = _runeCombo->currentData().toString();
    
    if (runeCode.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a rune to create."));
        return;
    }
    
    _copies = _copiesSpin->value();
    _createdRune = createRuneItem(runeCode);
    
    if (_createdRune) {
        qDebug() << "Created rune:" << runeCode << "at position" << _targetRow << "," << _targetColumn;
        accept();
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create rune. Please try again."));
    }
}

void RuneCreationWidget::onCancelClicked()
{
    reject();
}

void RuneCreationWidget::setItemPosition(int row, int column)
{
    // Ensure safe positions for Diablo 2 character inventory (4x10 visible area)
    if (row >= 10) row = 9;
    if (column >= 4) column = 3;
    if (row < 0) row = 0;
    if (column < 0) column = 0;
    
    _targetRow = row;
    _targetColumn = column;
    _rowSpin->setValue(row);
    _columnSpin->setValue(column);
    onRuneSelectionChanged(); // Update preview
}

ItemInfo* RuneCreationWidget::createRuneItem(const QString &runeCode)
{
    try {
        // Convert runeCode to file format
        QString runeFile = runeCode;
        if (runeCode.startsWith("rx0") && runeCode.length() == 4) {
            // Super runes: rx01 -> rx1, rx02 -> rx2, etc.
            runeFile = "rx" + runeCode.mid(3);
        } else if (runeCode.startsWith("r0") && runeCode.length() == 3) {
            // Regular runes: r01 -> r1, r02 -> r2, etc.
            runeFile = "r" + runeCode.mid(2);
        }
        // For other codes (r34-r99, rx34-rx99), keep as is
        
        // Load the actual rune template directly from resources
        QString runeFilePath = QString("runes/%1").arg(runeFile);
        ItemInfo *rune = ItemDataBase::loadItemFromFile(runeFilePath);
        
        // If template not found, try fallback strategies
        if (!rune) {
            qWarning() << "Failed to load rune template:" << runeFilePath;
            
            // Strategy 1: For Super runes (rx series), try using regular rune equivalent
            if (runeCode.startsWith("rx")) {
                QString fallbackCode = runeCode;
                fallbackCode.replace("rx", "r");
                QString fallbackFile = fallbackCode;
                if (fallbackCode.startsWith("r0") && fallbackCode.length() == 3) {
                    fallbackFile = "r" + fallbackCode.mid(2);
                }
                runeFilePath = QString("runes/%1").arg(fallbackFile);
                rune = ItemDataBase::loadItemFromFile(runeFilePath);
                qDebug() << "Trying Super rune fallback:" << runeFilePath;
            }
            
            // Strategy 2: For high-numbered runes, use a basic rune template
            if (!rune) {
                // Use r1 (El) as basic template for any missing rune
                runeFilePath = "runes/r1";
                rune = ItemDataBase::loadItemFromFile(runeFilePath);
                qDebug() << "Using r1 fallback template for:" << runeCode;
            }
            
            if (!rune) {
                qWarning() << "All fallback strategies failed for:" << runeCode;
                return nullptr;
            }
        }
        
        // Find an existing rune to copy format from
        ItemInfo *existingRune = nullptr;
        for (ItemInfo* item : CharacterInfo::instance().items.character) {
            if (item->storage == 0 && item->location == 0 && item->itemType.contains("r")) {
                existingRune = item;
                qDebug() << "EXISTING RUNE DETAILS:";
                qDebug() << "  itemType:" << item->itemType;
                qDebug() << "  bitString.length:" << item->bitString.length();
                qDebug() << "  guid:" << item->guid;
                qDebug() << "  hasChanged:" << item->hasChanged;
                qDebug() << "  First 20 chars of bitString:" << item->bitString.left(20);
                break;
            }
        }
        
        // If we have an existing rune, delete the template and copy the existing one instead
        if (existingRune) {
            delete rune;
            rune = new ItemInfo(*existingRune);
            qDebug() << "Copied existing rune format - storage:" << rune->storage << "location:" << rune->location;
        } else {
            qDebug() << "Using template rune - storage:" << rune->storage << "location:" << rune->location;
        }
        qDebug() << "Template bitString length:" << rune->bitString.length() << "itemType:" << rune->itemType;
        
        // Set position
        rune->move(_rowSpin->value(), _columnSpin->value(), 0, true);
        
        // Simply return the loaded rune - let the caller handle storage using proven methods
        qDebug() << "Loaded template - itemType:" << rune->itemType << "bitString.length:" << rune->bitString.length();
        qDebug() << "Requested runeCode:" << runeCode << "vs loaded itemType:" << rune->itemType;
        
        // Update itemType to match selected rune if different
        if (rune->itemType != runeCode.toUtf8()) {
            qDebug() << "Updating itemType from" << rune->itemType << "to" << runeCode;
            rune->itemType = runeCode.toUtf8();
            
            // Update itemType in bitString
            for (int i = 0; i < rune->itemType.length() && i < 4; i++) {
                ReverseBitWriter::replaceValueInBitString(rune->bitString, 
                    Enums::ItemOffsets::Type + i * 8, rune->itemType.at(i));
            }
            
            rune->hasChanged = true;
            qDebug() << "Updated itemType in bitString, hasChanged=" << rune->hasChanged;
        }
        
        qDebug() << "Successfully created rune item:" << runeCode << "from" << runeFilePath;
        qDebug() << "Rune details: storage=" << rune->storage << "location=" << rune->location 
                 << "row=" << rune->row << "column=" << rune->column
                 << "bitString.length=" << rune->bitString.length() << "hasChanged=" << rune->hasChanged;
        
        return rune;
        
    } catch (...) {
        qWarning() << "Exception occurred while creating rune:" << runeCode;
        return nullptr;
    }
}


