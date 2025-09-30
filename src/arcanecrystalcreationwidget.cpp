#include "arcanecrystalcreationwidget.h"
#include "structs.h"
#include "itemdatabase.h"
#include "enums.h"
#include "reversebitwriter.h"
#include "characterinfo.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QTextEdit>
#include <QMessageBox>
#include <QDebug>

ArcaneCrystalCreationWidget::ArcaneCrystalCreationWidget(QWidget *parent)
    : QDialog(parent), _startRow(0), _startColumn(0)
{
    setupUI();
    
    setWindowTitle(tr("Create Arcane Crystals"));
    setModal(true);
    resize(450, 350);
}

ArcaneCrystalCreationWidget::~ArcaneCrystalCreationWidget()
{
    // The parent class will handle cleanup, we don't need to delete crystals here
    // as they either get added to the character or are cleaned up elsewhere
}

void ArcaneCrystalCreationWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Title
    QLabel *titleLabel = new QLabel(tr("Create Arcane Crystals"));
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #2196F3;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Quantity selection
    QHBoxLayout *quantityLayout = new QHBoxLayout();
    quantityLayout->addWidget(new QLabel(tr("Quantity:")));
    
    _quantitySpinBox = new QSpinBox();
    _quantitySpinBox->setRange(1, 50);  // Allow up to 50 crystals
    _quantitySpinBox->setValue(1);
    _quantitySpinBox->setStyleSheet("QSpinBox { padding: 4px; }");
    quantityLayout->addWidget(_quantitySpinBox);
    
    quantityLayout->addStretch();
    mainLayout->addLayout(quantityLayout);
    
    // Position controls
    QGroupBox *positionGroup = new QGroupBox(tr("Starting Position"));
    QGridLayout *positionLayout = new QGridLayout(positionGroup);
    
    positionLayout->addWidget(new QLabel(tr("Row:")), 0, 0);
    _rowSpin = new QSpinBox();
    _rowSpin->setRange(0, 9);
    _rowSpin->setStyleSheet("QSpinBox { padding: 4px; }");
    positionLayout->addWidget(_rowSpin, 0, 1);
    
    positionLayout->addWidget(new QLabel(tr("Column:")), 0, 2);
    _columnSpin = new QSpinBox();
    _columnSpin->setRange(0, 9);
    _columnSpin->setStyleSheet("QSpinBox { padding: 4px; }");
    positionLayout->addWidget(_columnSpin, 0, 3);
    
    positionLayout->setColumnStretch(4, 1);
    mainLayout->addWidget(positionGroup);
    
    // Description area
    _previewLabel = new QLabel(tr("Arcane Crystal Information"));
    _previewLabel->setStyleSheet("font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(_previewLabel);
    
    _descriptionText = new QTextEdit();
    _descriptionText->setReadOnly(true);
    _descriptionText->setMaximumHeight(120);
    _descriptionText->setStyleSheet("QTextEdit { background-color: #f5f5f5; border: 1px solid #ddd; }");
    mainLayout->addWidget(_descriptionText);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    _createButton = new QPushButton(tr("Create Crystals"));
    _createButton->setDefault(true);
    _createButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; font-weight: bold; padding: 8px 16px; }");
    buttonLayout->addWidget(_createButton);
    
    _cancelButton = new QPushButton(tr("Cancel"));
    _cancelButton->setStyleSheet("QPushButton { padding: 8px 16px; }");
    buttonLayout->addWidget(_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(_quantitySpinBox, SIGNAL(valueChanged(int)), this, SLOT(onQuantityChanged()));
    connect(_createButton, SIGNAL(clicked()), this, SLOT(onCreateClicked()));
    connect(_cancelButton, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
    
    // Initial preview update
    updatePreview();
}

void ArcaneCrystalCreationWidget::onQuantityChanged()
{
    updatePreview();
}

void ArcaneCrystalCreationWidget::updatePreview()
{
    int quantity = _quantitySpinBox->value();
    
    QString description;
    description += tr("Arcane Crystal") + "\n";
    description += tr("Type: Miscellaneous Item") + "\n";
    description += tr("Size: 1x1") + "\n\n";
    description += tr("A mystical crystal containing arcane energy.") + "\n";
    description += tr("Used in various magical processes and enchantments.") + "\n\n";
    description += tr("Quantity to create: %1").arg(quantity) + "\n";
    description += tr("Starting position: Row %1, Column %2").arg(_rowSpin->value() + 1).arg(_columnSpin->value() + 1);
    
    _descriptionText->setText(description);
}

void ArcaneCrystalCreationWidget::onCreateClicked()
{
    int quantity = _quantitySpinBox->value();
    
    if (quantity <= 0) {
        QMessageBox::warning(this, tr("Invalid Quantity"), tr("Please specify a valid quantity (1-50)."));
        return;
    }
    
    // Validate starting position
    if (_rowSpin->value() < 0 || _columnSpin->value() < 0) {
        QMessageBox::warning(this, tr("Invalid Position"), tr("Please specify a valid starting position."));
        return;
    }
    
    try {
        _createdCrystals.clear();
        
        int currentRow = _startRow;
        int currentCol = _startColumn;
        int created = 0;
        
        for (int i = 0; i < quantity; i++) {
            ItemInfo* crystal = createArcaneCrystalItem();
            if (!crystal) {
                QMessageBox::critical(this, tr("Creation Failed"), 
                    tr("Failed to create Arcane Crystal %1 of %2.").arg(i + 1).arg(quantity));
                break;
            }
            
            // Set position for this crystal
            crystal->row = currentRow;
            crystal->column = currentCol;
            
            _createdCrystals.append(crystal);
            created++;
            
            qDebug() << "Created Arcane Crystal" << (i + 1) << "at position" << currentRow << currentCol;
            
            // Calculate next position (move right, wrap to next row if needed)
            currentCol++;
            if (currentCol >= 10) {  // Inventory is 10 columns wide
                currentCol = 0;
                currentRow++;
                if (currentRow >= 4) {  // Inventory is 4 rows for character items
                    currentRow = 0;  // Wrap around (though this may cause overlaps)
                }
            }
        }
        
        if (created > 0) {
            qDebug() << "Successfully created" << created << "Arcane Crystals";
            accept();
        } else {
            reject();
        }
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create crystals: %1").arg(e.what()));
    }
}

void ArcaneCrystalCreationWidget::onCancelClicked()
{
    _createdCrystals.clear();
    reject();
}

void ArcaneCrystalCreationWidget::setStartPosition(int row, int column)
{
    _startRow = row;
    _startColumn = column;
    _rowSpin->setValue(row);
    _columnSpin->setValue(column);
    updatePreview();
}

ItemInfo* ArcaneCrystalCreationWidget::createArcaneCrystalItem()
{
    try {
        // Load the Arcane Crystal template from resources using ItemDataBase::loadItemFromFile
        QString crystalFilePath = "arcane_crystal";
        qDebug() << "Attempting to load Arcane Crystal from:" << crystalFilePath;
        
        ItemInfo *crystal = ItemDataBase::loadItemFromFile(crystalFilePath);
        
        if (!crystal) {
            qWarning() << "Failed to load Arcane Crystal template:" << crystalFilePath;
            return nullptr;
        }
        
        qDebug() << "Successfully loaded Arcane Crystal template from:" << crystalFilePath;
        qDebug() << "Template itemType:" << crystal->itemType << "bitString.length:" << crystal->bitString.length();
        
        // Ensure the crystal has the correct itemType
        // Don't override the loaded itemType unless it's empty or wrong
        qDebug() << "Loaded crystal itemType:" << crystal->itemType;
        
        // The itemType should come from the loaded file, don't force it
        // unless there's a problem
        
        // Set basic properties
        crystal->hasChanged = true;
        
        qDebug() << "Successfully created Arcane Crystal";
        qDebug() << "Crystal details: itemType=" << crystal->itemType 
                 << "bitString.length=" << crystal->bitString.length() 
                 << "hasChanged=" << crystal->hasChanged;
        
        return crystal;
        
    } catch (const std::exception& e) {
        qWarning() << "Exception occurred while creating Arcane Crystal:" << e.what();
        return nullptr;
    } catch (...) {
        qWarning() << "Unknown exception occurred while creating Arcane Crystal";
        return nullptr;
    }
}