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
    
    // MedianXL Extended Runes (r34-r53)
    runes << RuneInfo{"r34", "Sha", "Sha Rune (MedianXL Extended)"};
    runes << RuneInfo{"r35", "Lah", "Lah Rune (MedianXL Extended)"};
    runes << RuneInfo{"r36", "Kur", "Kur Rune (MedianXL Extended)"};
    runes << RuneInfo{"r37", "Ix", "Ix Rune (MedianXL Extended)"};
    runes << RuneInfo{"r38", "Thur", "Thur Rune (MedianXL Extended)"};
    runes << RuneInfo{"r39", "Nas", "Nas Rune (MedianXL Extended)"};
    runes << RuneInfo{"r40", "Ath", "Ath Rune (MedianXL Extended)"};
    runes << RuneInfo{"r41", "Kra", "Kra Rune (MedianXL Extended)"};
    runes << RuneInfo{"r42", "Vith", "Vith Rune (MedianXL Extended)"};
    runes << RuneInfo{"r43", "No", "No Rune (MedianXL Extended)"};
    runes << RuneInfo{"r44", "Yul", "Yul Rune (MedianXL Extended)"};
    runes << RuneInfo{"r45", "Thai", "Thai Rune (MedianXL Extended)"};
    runes << RuneInfo{"r46", "Rha", "Rha Rune (MedianXL Extended)"};
    runes << RuneInfo{"r47", "Xar", "Xar Rune (MedianXL Extended)"};
    runes << RuneInfo{"r48", "Nih", "Nih Rune (MedianXL Extended)"};
    runes << RuneInfo{"r49", "Lai", "Lai Rune (MedianXL Extended)"};
    runes << RuneInfo{"r50", "On", "On Rune (MedianXL Extended)"};
    runes << RuneInfo{"r51", "Taha", "Taha Rune (MedianXL High Level)"};
    runes << RuneInfo{"r52", "Ghal", "Ghal Rune (MedianXL High Level)"};
    runes << RuneInfo{"r53", "Qor", "Qor Rune (MedianXL High Level)"};
    
    // MedianXL Enchanted Runes (r54-r62, r98-r99)
    runes << RuneInfo{"r54", "Krys", "Krys Rune (MedianXL Enchanted)"};
    runes << RuneInfo{"r55", "Auhe", "Auhe Rune (MedianXL Enchanted)"};
    runes << RuneInfo{"r56", "Sha'ad", "Sha'ad Rune (MedianXL Enchanted)"};
    runes << RuneInfo{"r57", "Fire", "Fire Rune (Elemental)"};
    runes << RuneInfo{"r58", "Earth", "Earth Rune (Elemental)"};
    runes << RuneInfo{"r59", "Magic", "Magic Rune (Elemental)"};
    runes << RuneInfo{"r60", "Poison", "Poison Rune (Elemental)"};
    runes << RuneInfo{"r61", "Lightning", "Lightning Rune (Elemental)"};
    runes << RuneInfo{"r62", "Cold", "Cold Rune (Elemental)"};
    runes << RuneInfo{"r98", "Xis", "Xis Rune (Special)"};
    runes << RuneInfo{"r99", "Runestone", "Runestone (Special)"};
    
    // Super Runes (rx01-rx33) - Enhanced versions of regular runes
    runes << RuneInfo{"rx01", "El Super", "El Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx02", "Eld Super", "Eld Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx03", "Tir Super", "Tir Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx04", "Nef Super", "Nef Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx05", "Eth Super", "Eth Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx06", "Ith Super", "Ith Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx07", "Tal Super", "Tal Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx08", "Ral Super", "Ral Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx09", "Ort Super", "Ort Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx10", "Thul Super", "Thul Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx11", "Amn Super", "Amn Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx12", "Sol Super", "Sol Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx13", "Shael Super", "Shael Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx14", "Dol Super", "Dol Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx15", "Hel Super", "Hel Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx16", "Io Super", "Io Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx17", "Lum Super", "Lum Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx18", "Ko Super", "Ko Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx19", "Fal Super", "Fal Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx20", "Lem Super", "Lem Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx21", "Pul Super", "Pul Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx22", "Um Super", "Um Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx23", "Mal Super", "Mal Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx24", "Ist Super", "Ist Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx25", "Gul Super", "Gul Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx26", "Vex Super", "Vex Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx27", "Ohm Super", "Ohm Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx28", "Lo Super", "Lo Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx29", "Sur Super", "Sur Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx30", "Ber Super", "Ber Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx31", "Jah Super", "Jah Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx32", "Cham Super", "Cham Rune Super (Enhanced Version)"};
    runes << RuneInfo{"rx33", "Zod Super", "Zod Rune Super (Enhanced Version)"};
    
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


