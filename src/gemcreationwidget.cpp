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
        // Convert gem code to file path using the same approach as runes
        QString gemFile = getGemFileForCode(gemCode);
        if (gemFile.isEmpty()) {
            qWarning() << "No gem file mapping found for:" << gemCode;
            return nullptr;
        }
        
        // Load the actual gem template directly from resources using ItemDataBase::loadItemFromFile
        QString gemFilePath = QString("gems/%1").arg(gemFile);
        qDebug() << "Attempting to load gem from:" << gemFilePath;
        qDebug() << "Full resource path will be:" << QString(":/Items/items/%1.d2i").arg(gemFilePath);
        
        ItemInfo *gem = ItemDataBase::loadItemFromFile(gemFilePath);
        
        if (!gem) {
            qWarning() << "Failed to load gem template:" << gemFilePath;
            qWarning() << "Gem code was:" << gemCode << "mapped to file:" << gemFile;
            return nullptr;
        }
        
        qDebug() << "Successfully loaded gem template from:" << gemFilePath;
        qDebug() << "Template itemType:" << gem->itemType << "bitString.length:" << gem->bitString.length();
        
        // Update itemType to match selected gem if needed
        if (gem->itemType != gemCode.toUtf8()) {
            qDebug() << "Updating itemType from" << gem->itemType << "to" << gemCode;
            gem->itemType = gemCode.toUtf8();
            
            // Update itemType in bitString using the same method as runes
            for (int i = 0; i < gem->itemType.length() && i < 4; i++) {
                ReverseBitWriter::replaceValueInBitString(gem->bitString, 
                    Enums::ItemOffsets::Type + i * 8, gem->itemType.at(i));
            }
            
            gem->hasChanged = true;
        }
        
        // Set position using move method
        gem->move(_targetRow, _targetColumn, 0, true);
        
        qDebug() << "Successfully created gem:" << gemCode << "from" << gemFilePath;
        qDebug() << "Gem details: storage=" << gem->storage << "location=" << gem->location 
                 << "row=" << gem->row << "column=" << gem->column
                 << "bitString.length=" << gem->bitString.length() << "hasChanged=" << gem->hasChanged;
        
        return gem;
        
    } catch (const std::exception& e) {
        qWarning() << "Exception occurred while creating gem:" << gemCode << "Error:" << e.what();
        return nullptr;
    } catch (...) {
        qWarning() << "Unknown exception occurred while creating gem:" << gemCode;
        return nullptr;
    }
}

QString GemCreationWidget::getGemFileForCode(const QString &gemCode)
{
    // Map gem codes to .d2i file names based on the resources/items/gems/ structure
    // Pattern: gemXY.d2i where X is gem type letter, Y is quality number
    
    qDebug() << "getGemFileForCode called with:" << gemCode;
    
    static QHash<QString, QString> gemFileMapping;
    if (gemFileMapping.isEmpty()) {
        // Sapphire (Cold Resistance) - 's' files
        gemFileMapping["gcb"] = "gems1";  // Chipped Sapphire
        gemFileMapping["gfb"] = "gems2";  // Flawed Sapphire
        gemFileMapping["gsb"] = "gems3";  // Sapphire
        gemFileMapping["glb"] = "gems4";  // Flawless Sapphire
        gemFileMapping["gpb"] = "gems4";  // Perfect Sapphire (use flawless as fallback)
        
        // Emerald (Dexterity) - 'e' files
        gemFileMapping["gcg"] = "geme1";  // Chipped Emerald
        gemFileMapping["gfg"] = "geme2";  // Flawed Emerald
        gemFileMapping["gsg"] = "geme3";  // Emerald
        gemFileMapping["glg"] = "geme4";  // Flawless Emerald
        gemFileMapping["gpg"] = "geme4";  // Perfect Emerald
        
        // Ruby (Fire Resistance) - 'r' files
        gemFileMapping["gcr"] = "gemr1";  // Chipped Ruby
        gemFileMapping["gfr"] = "gemr2";  // Flawed Ruby
        gemFileMapping["gsr"] = "gemr3";  // Ruby
        gemFileMapping["glr"] = "gemr4";  // Flawless Ruby
        gemFileMapping["gpr"] = "gemr4";  // Perfect Ruby
        
        // Amethyst (Strength) - 'a' files
        gemFileMapping["gcv"] = "gema1";  // Chipped Amethyst
        gemFileMapping["gfv"] = "gema2";  // Flawed Amethyst
        gemFileMapping["gsv"] = "gema3";  // Amethyst
        gemFileMapping["glv"] = "gema4";  // Flawless Amethyst
        gemFileMapping["gpv"] = "gema4";  // Perfect Amethyst
        
        // Diamond (Attack Rating) - 'd' files
        gemFileMapping["gcw"] = "gemd1";  // Chipped Diamond
        gemFileMapping["gfw"] = "gemd2";  // Flawed Diamond
        gemFileMapping["gsw"] = "gemd3";  // Diamond
        gemFileMapping["glw"] = "gemd4";  // Flawless Diamond
        gemFileMapping["gpw"] = "gemd4";  // Perfect Diamond
        
        // Topaz (Lightning Resistance) - 't' files
        gemFileMapping["gcy"] = "gemt1";  // Chipped Topaz
        gemFileMapping["gfy"] = "gemt2";  // Flawed Topaz
        gemFileMapping["gsy"] = "gemt3";  // Topaz
        gemFileMapping["gly"] = "gemt4";  // Flawless Topaz
        gemFileMapping["gpy"] = "gemt4";  // Perfect Topaz
        
        // Special MedianXL Gems (use available gem files as fallbacks)
        // Amber - use 'm' files if available, fallback to gems1
        gemFileMapping["5$a"] = "gema1";  // Chipped Amber -> Amethyst 1
        gemFileMapping["5$b"] = "gema2";  // Flawed Amber -> Amethyst 2
        gemFileMapping["5$c"] = "gema3";  // Amber -> Amethyst 3
        gemFileMapping["5$d"] = "gema4";  // Flawless Amber -> Amethyst 4
        gemFileMapping["5$e"] = "gema4";  // Perfect Amber -> Amethyst 4
        
        // Turquoise - use 'q' files if available, fallback to emerald
        gemFileMapping["7$a"] = "geme1";  // Chipped Turquoise -> Emerald 1
        gemFileMapping["7$b"] = "geme2";  // Flawed Turquoise -> Emerald 2
        gemFileMapping["7$c"] = "geme3";  // Turquoise -> Emerald 3
        gemFileMapping["7$d"] = "geme4";  // Flawless Turquoise -> Emerald 4
        gemFileMapping["7$e"] = "geme4";  // Perfect Turquoise -> Emerald 4
        
        // Bloodstone - use 'l' files if available, fallback to ruby
        gemFileMapping["9$a"] = "gemr1";  // Chipped Bloodstone -> Ruby 1
        gemFileMapping["9$b"] = "gemr2";  // Flawed Bloodstone -> Ruby 2
        gemFileMapping["9$c"] = "gemr3";  // Bloodstone -> Ruby 3
        gemFileMapping["9$d"] = "gemr4";  // Flawless Bloodstone -> Ruby 4
        gemFileMapping["9$e"] = "gemr4";  // Perfect Bloodstone -> Ruby 4
        
        // Onyx - use 'o' files if available, fallback to diamond  
        gemFileMapping["g$a"] = "gemd1";  // Chipped Onyx -> Diamond 1
        gemFileMapping["g$b"] = "gemd2";  // Flawed Onyx -> Diamond 2
        gemFileMapping["g$c"] = "gemd3";  // Onyx -> Diamond 3
        gemFileMapping["g$d"] = "gemd4";  // Flawless Onyx -> Diamond 4
        gemFileMapping["g$e"] = "gemd4";  // Perfect Onyx -> Diamond 4
    }
    
    QString result = gemFileMapping.value(gemCode, QString());
    qDebug() << "Gem code" << gemCode << "mapped to file:" << result;
    return result;
}

ItemInfo* GemCreationWidget::createBasicGem(const QString &gemCode)
{
    try {
        // Create a basic gem with minimal valid structure
        ItemInfo *gem = new ItemInfo();
        
        // Initialize basic fields manually (init() is private)
        gem->plugyPage = 0;
        gem->hasChanged = true;
        gem->ilvl = 1;
        gem->variableGraphicIndex = 0;
        gem->location = 0; // Character inventory
        gem->row = -1;
        gem->column = -1;
        gem->storage = 0; // Inventory
        gem->whereEquipped = 0;
        gem->shouldDeleteEverything = true;
        
        gem->itemType = gemCode.toUtf8();
        gem->quality = 2; // Normal quality
        gem->socketsNumber = 0;
        gem->socketablesNumber = 0;
        gem->isSocketed = false;
        gem->isRW = false;
        gem->isEthereal = false;
        gem->isPersonalized = false;
        
        // Create a minimal valid bitString for a basic gem
        // This is a simplified approach - in reality, bitStrings are complex
        // but we create a minimal structure that the parser can handle
        
        // Basic JM item structure:
        // 2 bytes: "JM" signature
        // 4 bytes: item type (gem code)
        // Additional minimal required fields to make it valid
        QByteArray bitStringBytes;
        
        // JM signature (16 bits = 2 bytes)
        bitStringBytes.append("JM");
        
        // Item type (32 bits = 4 bytes)
        QByteArray typeBytes = gemCode.toUtf8();
        while (typeBytes.length() < 4) typeBytes.append('\0');
        bitStringBytes.append(typeBytes);
        
        // Simple flags and basic structure (simplified)
        // Quality: Normal (2) - 4 bits
        // Other basic fields to make a minimal valid item
        QByteArray additionalData(10, 0); // 10 bytes of minimal data
        additionalData[0] = 0x20; // Quality = 2 (normal) in lower 4 bits
        additionalData[1] = 0x01; // Level = 1
        bitStringBytes.append(additionalData);
        
        // Convert bytes to bit string (each byte becomes 8 bits) using helper
        QString bitString;
        for (int i = 0; i < bitStringBytes.length(); ++i) {
            quint8 byte = static_cast<quint8>(bitStringBytes.at(i));
            bitString.append(binaryStringFromNumber(byte, false, 8));
        }

        gem->bitString = bitString;
        
        qDebug() << "Created basic gem from scratch:" << gemCode;
        qDebug() << "BitString length:" << gem->bitString.length();
        
        return gem;
        
    } catch (const std::exception& e) {
        qWarning() << "Exception creating basic gem:" << e.what();
        return nullptr;
    } catch (...) {
        qWarning() << "Unknown exception creating basic gem";
        return nullptr;
    }
}