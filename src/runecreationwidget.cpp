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
    
    // Add common runes
    struct RuneInfo {
        QString code;
        QString name;
        QString description;
    };
    
    QList<RuneInfo> runes;
    runes << RuneInfo{"r01", "El", "El Rune (+1 Light Radius, +15 Attack Rating)"};
    runes << RuneInfo{"r02", "Eld", "Eld Rune (+75% Damage to Undead, +50 Attack Rating vs Undead)"};
    runes << RuneInfo{"r03", "Tir", "Tir Rune (+2 Mana per Kill)"};
    runes << RuneInfo{"r04", "Nef", "Nef Rune (Knockback)"};
    runes << RuneInfo{"r05", "Eth", "Eth Rune (-25% Target Defense)"};
    runes << RuneInfo{"r06", "Ith", "Ith Rune (+9 Max Damage)"};
    runes << RuneInfo{"r07", "Tal", "Tal Rune (+75 Poison Damage over 5 seconds)"};
    runes << RuneInfo{"r08", "Ral", "Ral Rune (Adds 5-30 Fire Damage)"};
    runes << RuneInfo{"r09", "Ort", "Ort Rune (Adds 1-50 Lightning Damage)"};
    runes << RuneInfo{"r10", "Thul", "Thul Rune (Adds 3-14 Cold Damage)"};
    runes << RuneInfo{"r11", "Amn", "Amn Rune (+7% Life Stolen per Hit)"};
    runes << RuneInfo{"r12", "Sol", "Sol Rune (+3 Min Damage)"};
    runes << RuneInfo{"r13", "Shael", "Shael Rune (+20% Faster Hit Recovery, +20% Faster Block Rate)"};
    runes << RuneInfo{"r14", "Dol", "Dol Rune (Hit Causes Monster to Flee 25%)"};
    runes << RuneInfo{"r15", "Hel", "Hel Rune (Requirements -20%)"};
    runes << RuneInfo{"r16", "Io", "Io Rune (+10 Vitality)"};
    runes << RuneInfo{"r17", "Lum", "Lum Rune (+10 Energy)"};
    runes << RuneInfo{"r18", "Ko", "Ko Rune (+10 Dexterity)"};
    runes << RuneInfo{"r19", "Fal", "Fal Rune (+10 Strength)"};
    runes << RuneInfo{"r20", "Lem", "Lem Rune (+75% Extra Gold from Monsters)"};
    runes << RuneInfo{"r21", "Pul", "Pul Rune (+75% Damage to Demons, +100 Attack Rating vs Demons)"};
    runes << RuneInfo{"r22", "Um", "Um Rune (+15% Chance of Open Wounds, +25% All Resistances)"};
    runes << RuneInfo{"r23", "Mal", "Mal Rune (Prevent Monster Heal)"};
    runes << RuneInfo{"r24", "Ist", "Ist Rune (+30% Better Chance of Getting Magic Items)"};
    runes << RuneInfo{"r25", "Gul", "Gul Rune (+20% Bonus to Attack Rating)"};
    runes << RuneInfo{"r26", "Vex", "Vex Rune (+7% Mana Stolen per Hit)"};
    runes << RuneInfo{"r27", "Ohm", "Ohm Rune (+50% Enhanced Damage)"};
    runes << RuneInfo{"r28", "Lo", "Lo Rune (+20% Deadly Strike)"};
    runes << RuneInfo{"r29", "Sur", "Sur Rune (Hit Blinds Target)"};
    runes << RuneInfo{"r30", "Ber", "Ber Rune (+20% Chance of Crushing Blow)"};
    runes << RuneInfo{"r31", "Jah", "Jah Rune (Ignore Target's Defense)"};
    runes << RuneInfo{"r32", "Cham", "Cham Rune (Freezes Target +3)"};
    runes << RuneInfo{"r33", "Zod", "Zod Rune (Indestructible)"};
    
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
        // Convert runeCode (r01, r02) to file format (r1, r2)
        QString runeFile = runeCode;
        if (runeCode.startsWith("r0")) {
            runeFile = "r" + runeCode.mid(2); // r01 -> r1
        }
        
        // Load the actual rune template directly from resources
        QString runeFilePath = QString("runes/%1").arg(runeFile);
        ItemInfo *rune = ItemDataBase::loadItemFromFile(runeFilePath);
        
        if (!rune) {
            qWarning() << "Failed to load rune template:" << runeFilePath;
            return nullptr;
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


