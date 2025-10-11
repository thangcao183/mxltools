#include "jewelcreationwidget.h"
#include "structs.h"
#include "itemdatabase.h"
#include "enums.h"
#include "reversebitwriter.h"
#include "characterinfo.hpp"
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QBitArray>
#include <cstdlib>

JewelCreationWidget::JewelCreationWidget(QWidget *parent)
    : QDialog(parent), _createdJewel(nullptr), _targetRow(0), _targetColumn(0)
{
    setupUI();
    populateJewelTypes();
    
    setWindowTitle(tr("Create Jewel"));
    setModal(true);
    resize(500, 450);
}

JewelCreationWidget::~JewelCreationWidget()
{
    // _createdJewel is managed by the caller
}

void JewelCreationWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Title
    QLabel *titleLabel = new QLabel(tr("Select Jewel to Create"));
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    mainLayout->addWidget(titleLabel);
    
    // Jewel selection
    QGridLayout *gridLayout = new QGridLayout();
    
    // Jewel type selection
    gridLayout->addWidget(new QLabel(tr("Type:")), 0, 0);
    _jewelTypeCombo = new QComboBox();
    _jewelTypeCombo->setMinimumWidth(250);
    gridLayout->addWidget(_jewelTypeCombo, 0, 1);
    
    // Specific jewel selection
    gridLayout->addWidget(new QLabel(tr("Jewel:")), 1, 0);
    _jewelCombo = new QComboBox();
    _jewelCombo->setMinimumWidth(250);
    gridLayout->addWidget(_jewelCombo, 1, 1);
    
    // Position
    gridLayout->addWidget(new QLabel(tr("Row:")), 2, 0);
    _rowSpin = new QSpinBox();
    _rowSpin->setRange(0, 9);
    gridLayout->addWidget(_rowSpin, 2, 1);
    
    gridLayout->addWidget(new QLabel(tr("Column:")), 3, 0);
    _columnSpin = new QSpinBox();
    _columnSpin->setRange(0, 9);
    gridLayout->addWidget(_columnSpin, 3, 1);
    
    mainLayout->addLayout(gridLayout);
    
    // Preview area
    _previewLabel = new QLabel();
    _previewLabel->setFrameStyle(QFrame::Box);
    _previewLabel->setMinimumHeight(60);
    _previewLabel->setAlignment(Qt::AlignCenter);
    _previewLabel->setStyleSheet("background-color: #f0f0f0; margin: 10px; padding: 10px;");
    mainLayout->addWidget(_previewLabel);
    
    // Properties text area
    QLabel *propertiesLabel = new QLabel(tr("Jewel Properties:"));
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
    
    _createButton = new QPushButton(tr("Create Jewel"));
    _createButton->setDefault(true);
    _createButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 8px 16px; }");
    buttonLayout->addWidget(_createButton);
    
    _cancelButton = new QPushButton(tr("Cancel"));
    _cancelButton->setStyleSheet("QPushButton { padding: 8px 16px; }");
    buttonLayout->addWidget(_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(_jewelTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onJewelTypeChanged()));
    connect(_jewelCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onJewelSelectionChanged()));
    connect(_createButton, SIGNAL(clicked()), this, SLOT(onCreateClicked()));
    connect(_cancelButton, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
}

void JewelCreationWidget::populateJewelTypes()
{
    _jewelTypeCombo->clear();
    _jewelTypeCombo->addItem(tr("Amulets"), "amulets");
    _jewelTypeCombo->addItem(tr("Rings"), "rings");
    _jewelTypeCombo->addItem(tr("Small Charms"), "small_charms");
    _jewelTypeCombo->addItem(tr("Large Charms"), "large_charms");
    _jewelTypeCombo->addItem(tr("Grand Charms"), "grand_charms");
    
    // Trigger initial population
    onJewelTypeChanged();
}

void JewelCreationWidget::populateJewelList()
{
    _jewelCombo->clear();
    
    QString selectedType = _jewelTypeCombo->currentData().toString();
    
    if (selectedType == "amulets") {
        // Amulets
        _jewelCombo->addItem(tr("Amulet"), "amu");
        _jewelCombo->addItem(tr("Amulet (Magic)"), "amu2");
        _jewelCombo->addItem(tr("Amulet (Rare)"), "amu3");
    } else if (selectedType == "rings") {
        // Rings
        _jewelCombo->addItem(tr("Ring"), "rin");
        _jewelCombo->addItem(tr("Ring (Magic)"), "rin2");
        _jewelCombo->addItem(tr("Ring (Rare)"), "rin3");
    } else if (selectedType == "small_charms") {
        // Small Charms
        _jewelCombo->addItem(tr("Small Charm"), "cm1");
        _jewelCombo->addItem(tr("Small Charm (Magic)"), "cm1");  // Use same code for now
        _jewelCombo->addItem(tr("Small Charm (Rare)"), "cm1");
    } else if (selectedType == "large_charms") {
        // Large Charms
        _jewelCombo->addItem(tr("Large Charm"), "cm2");
        _jewelCombo->addItem(tr("Large Charm (Magic)"), "cm2");
        _jewelCombo->addItem(tr("Large Charm (Rare)"), "cm2");
    } else if (selectedType == "grand_charms") {
        // Grand Charms
        _jewelCombo->addItem(tr("Grand Charm"), "cm3");
        _jewelCombo->addItem(tr("Grand Charm (Magic)"), "cm3");
        _jewelCombo->addItem(tr("Grand Charm (Rare)"), "cm3");
    }
    
    // Update preview after populating
    onJewelSelectionChanged();
}

void JewelCreationWidget::onJewelTypeChanged()
{
    populateJewelList();
}

void JewelCreationWidget::onJewelSelectionChanged()
{
    QString jewelCode = _jewelCombo->currentData().toString();
    QString jewelName = _jewelCombo->currentText();
    
    if (jewelCode.isEmpty()) {
        _previewLabel->setText(tr("No jewel selected"));
        _propertiesText->clear();
        return;
    }
    
    // Update preview
    _previewLabel->setText(tr("Selected: %1 (Code: %2)").arg(jewelName).arg(jewelCode));
    
    // Update properties
    QString properties = getJewelProperties(jewelCode);
    _propertiesText->setPlainText(properties);
}

void JewelCreationWidget::onCreateClicked()
{
    QString jewelCode = _jewelCombo->currentData().toString();
    
    if (jewelCode.isEmpty()) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select a jewel to create."));
        return;
    }
    
    // Validate position
    if (_rowSpin->value() < 0 || _columnSpin->value() < 0) {
        QMessageBox::warning(this, tr("Invalid Position"), tr("Please specify a valid inventory position."));
        return;
    }
    
    try {
        _createdJewel = createJewelItem(jewelCode);
        if (!_createdJewel) {
            QMessageBox::critical(this, tr("Creation Failed"), tr("Failed to create jewel. Unknown jewel type."));
            return;
        }
        
        // Set position
        _createdJewel->row = _targetRow;
        _createdJewel->column = _targetColumn;
        
        qDebug() << "Created jewel:" << jewelCode << "at position" << _targetRow << _targetColumn;
        
        accept();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create jewel: %1").arg(e.what()));
    }
}

void JewelCreationWidget::onCancelClicked()
{
    _createdJewel = nullptr;
    reject();
}

void JewelCreationWidget::setItemPosition(int row, int column)
{
    _targetRow = row;
    _targetColumn = column;
    _rowSpin->setValue(row);
    _columnSpin->setValue(column);
}

ItemInfo* JewelCreationWidget::createJewelItem(const QString &jewelCode)
{
    try {
        // Convert jewel code to file path using the same approach as gems
        QString jewelFile = getJewelFileForCode(jewelCode);
        if (jewelFile.isEmpty()) {
            qWarning() << "No jewel file mapping found for:" << jewelCode;
            return createBasicJewel(jewelCode);
        }
        
        // Load the actual jewel template directly from resources
        QString jewelFilePath = QString("jewels/%1").arg(jewelFile);
        qDebug() << "Attempting to load jewel from:" << jewelFilePath;
        qDebug() << "Full resource path will be:" << QString(":/Items/items/%1.d2i").arg(jewelFilePath);
        
        ItemInfo *jewel = ItemDataBase::loadItemFromFile(jewelFilePath);
        
        if (!jewel) {
            qWarning() << "Failed to load jewel template:" << jewelFilePath;
            qWarning() << "Jewel code was:" << jewelCode << "mapped to file:" << jewelFile;
            qWarning() << "Falling back to basic jewel creation";
            return createBasicJewel(jewelCode);
        }
        
        qDebug() << "Successfully loaded jewel template from:" << jewelFilePath;
        qDebug() << "Template itemType:" << jewel->itemType << "bitString.length:" << jewel->bitString.length();
        
        // Update itemType to match selected jewel if needed
        if (jewel->itemType != jewelCode.toUtf8()) {
            qDebug() << "Updating itemType from" << jewel->itemType << "to" << jewelCode;
            jewel->itemType = jewelCode.toUtf8();
            
            // Update itemType in bitString
            for (int i = 0; i < jewel->itemType.length() && i < 4; i++) {
                ReverseBitWriter::replaceValueInBitString(jewel->bitString, 
                    Enums::ItemOffsets::Type + i * 8, jewel->itemType.at(i));
            }
            
            jewel->hasChanged = true;
        }
        
        // Set position using move method
        jewel->move(_targetRow, _targetColumn, 0, true);
        
        qDebug() << "Successfully created jewel:" << jewelCode << "from" << jewelFilePath;
        qDebug() << "Jewel details: storage=" << jewel->storage << "location=" << jewel->location 
                 << "row=" << jewel->row << "column=" << jewel->column
                 << "bitString.length=" << jewel->bitString.length() << "hasChanged=" << jewel->hasChanged;
        
        return jewel;
        
    } catch (const std::exception& e) {
        qWarning() << "Exception occurred while creating jewel:" << jewelCode << "Error:" << e.what();
        return createBasicJewel(jewelCode);
    } catch (...) {
        qWarning() << "Unknown exception occurred while creating jewel:" << jewelCode;
        return createBasicJewel(jewelCode);
    }
}

ItemInfo* JewelCreationWidget::createBasicJewel(const QString &jewelCode)
{
    try {
        qDebug() << "Creating basic jewel with code:" << jewelCode;
        
        // Create a new ItemInfo structure
        ItemInfo *jewel = new ItemInfo();
        
        // Set basic properties
        jewel->itemType = jewelCode.toUtf8();
        jewel->storage = Enums::ItemStorage::Inventory;
        jewel->location = Enums::ItemLocation::Stored;
        jewel->row = _targetRow;
        jewel->column = _targetColumn;
        jewel->hasChanged = true;
        
        // Create a basic bit string for the jewel using same approach as gems
        QByteArray bitStringBytes;
        
        // JM signature (16 bits = 2 bytes)
        bitStringBytes.append("JM");
        
        // Item type (32 bits = 4 bytes)
        QByteArray typeBytes = jewelCode.toUtf8();
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

        jewel->bitString = bitString;
        
        qDebug() << "Successfully created basic jewel:" << jewelCode;
        qDebug() << "Basic jewel details: storage=" << jewel->storage << "location=" << jewel->location 
                 << "row=" << jewel->row << "column=" << jewel->column
                 << "bitString.length=" << jewel->bitString.length() << "hasChanged=" << jewel->hasChanged;
        
        return jewel;
        
    } catch (const std::exception& e) {
        qWarning() << "Exception occurred while creating basic jewel:" << jewelCode << "Error:" << e.what();
        return nullptr;
    } catch (...) {
        qWarning() << "Unknown exception occurred while creating basic jewel:" << jewelCode;
        return nullptr;
    }
}

QString JewelCreationWidget::getJewelFileForCode(const QString &jewelCode)
{
    qDebug() << "getJewelFileForCode called with:" << jewelCode;
    
    static QHash<QString, QString> jewelFileMapping;
    if (jewelFileMapping.isEmpty()) {
        // Amulets
        jewelFileMapping["amu"] = "amulet1";
        jewelFileMapping["amu2"] = "amulet2";
        jewelFileMapping["amu3"] = "amulet3";
        
        // Rings
        jewelFileMapping["rin"] = "ring1";
        jewelFileMapping["rin2"] = "ring2";
        jewelFileMapping["rin3"] = "ring3";
        
        // Small Charms
        jewelFileMapping["cm1"] = "smallcharm1";
        
        // Large Charms
        jewelFileMapping["cm2"] = "largecharm1";
        
        // Grand Charms
        jewelFileMapping["cm3"] = "grandcharm1";
    }
    
    QString fileName = jewelFileMapping.value(jewelCode);
    qDebug() << "Mapped" << jewelCode << "to file:" << fileName;
    return fileName;
}

QString JewelCreationWidget::getJewelProperties(const QString &jewelCode)
{
    QString properties;
    
    if (jewelCode.startsWith("amu")) {
        properties = tr("Amulet Properties:\n");
        properties += tr("- Can be worn around the neck\n");
        properties += tr("- Provides various magical properties\n");
        properties += tr("- Takes 1x1 inventory space\n");
    } else if (jewelCode.startsWith("rin")) {
        properties = tr("Ring Properties:\n");
        properties += tr("- Can be worn on fingers\n");
        properties += tr("- Provides various magical properties\n");
        properties += tr("- Takes 1x1 inventory space\n");
    } else if (jewelCode == "cm1") {
        properties = tr("Small Charm Properties:\n");
        properties += tr("- Provides passive bonuses while in inventory\n");
        properties += tr("- Takes 1x1 inventory space\n");
        properties += tr("- Can have various magical properties\n");
    } else if (jewelCode == "cm2") {
        properties = tr("Large Charm Properties:\n");
        properties += tr("- Provides passive bonuses while in inventory\n");
        properties += tr("- Takes 1x2 inventory space\n");
        properties += tr("- Can have more powerful magical properties\n");
    } else if (jewelCode == "cm3") {
        properties = tr("Grand Charm Properties:\n");
        properties += tr("- Provides passive bonuses while in inventory\n");
        properties += tr("- Takes 1x3 inventory space\n");
        properties += tr("- Can have the most powerful magical properties\n");
    } else {
        properties = tr("Unknown jewel type: %1").arg(jewelCode);
    }
    
    return properties;
}
