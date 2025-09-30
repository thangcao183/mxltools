#include "gemcreationwidget.h"
#include "structs.h"
#include "itemdatabase.h"
#include "enums.h"
#include "reversebitwriter.h"
#include "characterinfo.hpp"
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <cstdlib>

GemCreationWidget::GemCreationWidget(QWidget *parent)
    : QDialog(parent), _createdGem(nullptr), _targetRow(0), _targetColumn(0)
{
    setupUI();
    populateGemList();
    
    setWindowTitle(tr("Create Gem"));
    setModal(true);
    resize(500, 400);
}

GemCreationWidget::~GemCreationWidget()
{
    // _createdGem is managed by the caller
}

void GemCreationWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Title
    QLabel *titleLabel = new QLabel(tr("Select Gem to Create"));
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(titleLabel);
    
    // Gem selection
    QGridLayout *gridLayout = new QGridLayout();
    
    gridLayout->addWidget(new QLabel(tr("Gem:")), 0, 0);
    _gemCombo = new QComboBox();
    _gemCombo->setMinimumWidth(250);
    gridLayout->addWidget(_gemCombo, 0, 1);
    
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
    _previewLabel->setMinimumHeight(60);
    _previewLabel->setAlignment(Qt::AlignCenter);
    _previewLabel->setStyleSheet("background-color: #f0f0f0; margin: 10px; padding: 10px;");
    mainLayout->addWidget(_previewLabel);
    
    // Properties text area
    QLabel *propertiesLabel = new QLabel(tr("Gem Properties:"));
    propertiesLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(propertiesLabel);
    
    _propertiesText = new QTextEdit();
    _propertiesText->setReadOnly(true);
    _propertiesText->setMaximumHeight(120);
    _propertiesText->setStyleSheet("background-color: #f8f8f8; border: 1px solid #ccc;");
    mainLayout->addWidget(_propertiesText);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    _createButton = new QPushButton(tr("Create Gem"));
    _createButton->setDefault(true);
    _createButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 8px 16px; }");
    buttonLayout->addWidget(_createButton);
    
    _cancelButton = new QPushButton(tr("Cancel"));
    _cancelButton->setStyleSheet("QPushButton { padding: 8px 16px; }");
    buttonLayout->addWidget(_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(_gemCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onGemSelectionChanged()));
    connect(_createButton, SIGNAL(clicked()), this, SLOT(onCreateClicked()));
    connect(_cancelButton, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
    
    // Initial preview update
    onGemSelectionChanged();
}

void GemCreationWidget::populateGemList()
{
    _gemCombo->clear();
    
    // Add all gems from MedianXL
    struct GemInfo {
        const char* code;
        const char* name;
        const char* quality;
    };
    
    static const GemInfo gems[] = {
        // Sapphire (Cold Resistance)
        {"gcb", "Chipped Sapphire", "Chipped"},
        {"gfb", "Flawed Sapphire", "Flawed"},
        {"gsb", "Sapphire", "Normal"},
        {"glb", "Flawless Sapphire", "Flawless"},
        {"gpb", "Perfect Sapphire", "Perfect"},
        
        // Emerald (Dexterity)
        {"gcg", "Chipped Emerald", "Chipped"},
        {"gfg", "Flawed Emerald", "Flawed"},
        {"gsg", "Emerald", "Normal"},
        {"glg", "Flawless Emerald", "Flawless"},
        {"gpg", "Perfect Emerald", "Perfect"},
        
        // Ruby (Fire Resistance)
        {"gcr", "Chipped Ruby", "Chipped"},
        {"gfr", "Flawed Ruby", "Flawed"},
        {"gsr", "Ruby", "Normal"},
        {"glr", "Flawless Ruby", "Flawless"},
        {"gpr", "Perfect Ruby", "Perfect"},
        
        // Amethyst (Attack Rating)
        {"gcv", "Chipped Amethyst", "Chipped"},
        {"gfv", "Flawed Amethyst", "Flawed"},
        {"gsv", "Amethyst", "Normal"},
        
        // Diamond (All Resistances)
        {"gcw", "Chipped Diamond", "Chipped"},
        {"gfw", "Flawed Diamond", "Flawed"},
        {"gsw", "Diamond", "Normal"},
        {"glw", "Flawless Diamond", "Flawless"},
        {"gpw", "Perfect Diamond", "Perfect"},
        
        // Topaz (Lightning Resistance)
        {"gcy", "Chipped Topaz", "Chipped"},
        {"gfy", "Flawed Topaz", "Flawed"},
        {"gsy", "Topaz", "Normal"},
        {"gly", "Flawless Topaz", "Flawless"},
        {"gpy", "Perfect Topaz", "Perfect"},
        
        // Special Gems
        {"5$a", "Chipped Amber", "Chipped"},
        {"5$b", "Flawed Amber", "Flawed"}, 
        {"5$c", "Amber", "Normal"},
        {"5$d", "Flawless Amber", "Flawless"},
        {"5$e", "Perfect Amber", "Perfect"},
        
        {"7$a", "Chipped Turquoise", "Chipped"},
        {"7$b", "Flawed Turquoise", "Flawed"},
        {"7$c", "Turquoise", "Normal"},
        {"7$d", "Flawless Turquoise", "Flawless"},
        {"7$e", "Perfect Turquoise", "Perfect"},
        
        {"9$a", "Chipped Bloodstone", "Chipped"},
        {"9$b", "Flawed Bloodstone", "Flawed"},
        {"9$c", "Bloodstone", "Normal"},
        {"9$d", "Flawless Bloodstone", "Flawless"},
        {"9$e", "Perfect Bloodstone", "Perfect"},
        
        {"g$a", "Chipped Onyx", "Chipped"},
        {"g$b", "Flawed Onyx", "Flawed"},
        {"g$c", "Onyx", "Normal"},
        {"g$d", "Flawless Onyx", "Flawless"},
        {"g$e", "Perfect Onyx", "Perfect"}
    };
    
    for (const auto& gem : gems) {
        QString displayName = QString("%1 (%2)").arg(gem.name).arg(gem.quality);
        _gemCombo->addItem(displayName, QString(gem.code));
    }
}

QString GemCreationWidget::getGemProperties(const QString &gemCode)
{
    // Load gem properties from socketables data
    const auto& socketables = *ItemDataBase::Socketables();
    QByteArray codeBytes = gemCode.toUtf8();
    
    if (socketables.contains(codeBytes)) {
        const SocketableItemInfo* gemInfo = socketables[codeBytes];
        
        QString properties;
        properties += QString("Gem: %1\n").arg(gemInfo->name);
        
        // Show armor properties
        if (gemInfo->properties.contains(SocketableItemInfo::Armor)) {
            const auto& armorProps = gemInfo->properties[SocketableItemInfo::Armor];
            if (!armorProps.isEmpty()) {
                properties += "\nWhen socketed in Armor:\n";
                for (const auto& prop : armorProps) {
                    properties += QString("  • Property %1: +%2\n").arg(prop.code).arg(prop.value);
                }
            }
        }
        
        // Show shield properties
        if (gemInfo->properties.contains(SocketableItemInfo::Shield)) {
            const auto& shieldProps = gemInfo->properties[SocketableItemInfo::Shield];
            if (!shieldProps.isEmpty()) {
                properties += "\nWhen socketed in Shield:\n";
                for (const auto& prop : shieldProps) {
                    properties += QString("  • Property %1: +%2\n").arg(prop.code).arg(prop.value);
                }
            }
        }
        
        // Show weapon properties
        if (gemInfo->properties.contains(SocketableItemInfo::Weapon)) {
            const auto& weaponProps = gemInfo->properties[SocketableItemInfo::Weapon];
            if (!weaponProps.isEmpty()) {
                properties += "\nWhen socketed in Weapon:\n";
                for (const auto& prop : weaponProps) {
                    properties += QString("  • Property %1: +%2\n").arg(prop.code).arg(prop.value);
                }
            }
        }
        
        return properties;
    }
    
    return QString("No properties found for gem code: %1").arg(gemCode);
}

void GemCreationWidget::onGemSelectionChanged()
{
    if (_gemCombo->currentIndex() < 0) {
        _previewLabel->setText(tr("Select a gem..."));
        _propertiesText->clear();
        return;
    }
    
    QString gemCode = _gemCombo->currentData().toString();
    QString gemName = _gemCombo->currentText();
    
    _previewLabel->setText(QString(tr("Creating: %1\nCode: %2")).arg(gemName.split(" (").first()).arg(gemCode));
    _propertiesText->setPlainText(getGemProperties(gemCode));
}

void GemCreationWidget::onCreateClicked()
{
    QString gemCode = _gemCombo->currentData().toString();
    
    if (gemCode.isEmpty()) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select a gem to create."));
        return;
    }
    
    // Validate position
    if (_rowSpin->value() < 0 || _columnSpin->value() < 0) {
        QMessageBox::warning(this, tr("Invalid Position"), tr("Please specify a valid inventory position."));
        return;
    }
    
    try {
        _createdGem = createGemItem(gemCode);
        if (!_createdGem) {
            QMessageBox::critical(this, tr("Creation Failed"), tr("Failed to create gem. Unknown gem type."));
            return;
        }
        
        // Set position
        _createdGem->row = _targetRow;
        _createdGem->column = _targetColumn;
        
        qDebug() << "Created gem:" << gemCode << "at position" << _targetRow << _targetColumn;
        
        accept();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create gem: %1").arg(e.what()));
    }
}

void GemCreationWidget::onCancelClicked()
{
    _createdGem = nullptr;
    reject();
}

void GemCreationWidget::setItemPosition(int row, int column)
{
    _targetRow = row;
    _targetColumn = column;
    _rowSpin->setValue(row);
    _columnSpin->setValue(column);
}

ItemInfo* GemCreationWidget::createGemItem(const QString &gemCode)
{
    try {
        // Try to load template gem from save folder first
        QString gemFilePath = QString("gems/%1").arg(gemCode);
        ItemInfo* gem = ItemDataBase::loadItemFromFile(gemFilePath);
        
        if (!gem) {
            // Try loading a basic gem as template (chipped ruby as fallback)
            gemFilePath = "gems/gcr";
            gem = ItemDataBase::loadItemFromFile(gemFilePath);
            qDebug() << "Using gcr fallback template for:" << gemCode;
        }
        
        if (!gem) {
            qWarning() << "Could not load gem template for:" << gemCode;
            return nullptr;
        }
        
        // Update itemType to match selected gem if different
        if (gem->itemType != gemCode.toUtf8()) {
            qDebug() << "Updating itemType from" << gem->itemType << "to" << gemCode;
            gem->itemType = gemCode.toUtf8();
            gem->hasChanged = true;
        }
        
        // Set position
        gem->move(_targetRow, _targetColumn, 0, true);
        
        qDebug() << "Successfully created gem:" << gemCode;
        qDebug() << "Gem details: storage=" << gem->storage << "location=" << gem->location 
                 << "row=" << gem->row << "column=" << gem->column
                 << "bitString.length=" << gem->bitString.length();
        
        return gem;
        
    } catch (...) {
        qWarning() << "Exception occurred while creating gem:" << gemCode;
        return nullptr;
    }
}