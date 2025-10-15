#include "propertyeditor.h"
#include "addpropertydialog.h"
#include "propertymodificationengine.h"
#include "enhancedpropertyadditionengine.h"
#include "itemparser.h"
#include "itemdatabase.h"
#include "reversebitwriter.h"
#include "enums.h"
#include "helpers.h"
#include "characterinfo.hpp"

#include <algorithm>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QGroupBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QDebug>

PropertyEditor::PropertyEditor(QWidget *parent)
    : QWidget(parent)
    , _item(nullptr)
    , _hasChanges(false)
    , _updatingUI(false)
    , _enhancedEngine(nullptr)
{
    // Initialize Enhanced Property Addition Engine
    _enhancedEngine = new EnhancedPropertyAdditionEngine(this);
    connect(_enhancedEngine, &EnhancedPropertyAdditionEngine::statusChanged,
            this, [this](const QString &status) {
                if (_statusLabel) {
                    _statusLabel->setText(status);
                }
            });
    connect(_enhancedEngine, &EnhancedPropertyAdditionEngine::errorOccurred,
            this, [this](const QString &error) {
                QMessageBox::warning(this, tr("Property Addition Error"), error);
            });
    connect(_enhancedEngine, &EnhancedPropertyAdditionEngine::propertyAdded,
            this, [this](int propertyId, const QString &propertyName, int value) {
                QMessageBox::information(this, tr("Property Added"), 
                    tr("Successfully added %1: %2").arg(propertyName).arg(value));
            });
    
    setupUI();
}

PropertyEditor::~PropertyEditor()
{
    clear();
}

void PropertyEditor::setupUI()
{
    _mainLayout = new QVBoxLayout(this);
    
    // Header controls
    QHBoxLayout *headerLayout = new QHBoxLayout;
    
    _showAllPropertiesCheck = new QCheckBox(tr("Show all properties"), this);
    _showAllPropertiesCheck->setToolTip(tr("Show properties that are normally hidden in game"));
    connect(_showAllPropertiesCheck, &QCheckBox::toggled, [this](bool) { 
        if (!_updatingUI) populatePropertyCombo(nullptr);
    });
    
    _statusLabel = new QLabel(this);
    _statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
    
    headerLayout->addWidget(_showAllPropertiesCheck);
    headerLayout->addStretch();
    headerLayout->addWidget(_statusLabel);
    
    _mainLayout->addLayout(headerLayout);
    
    // Scroll area for properties
    _scrollArea = new QScrollArea(this);
    _scrollArea->setWidgetResizable(true);
    _scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    _scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    _scrollWidget = new QWidget;
    _scrollLayout = new QVBoxLayout(_scrollWidget);
    _scrollLayout->setSpacing(5);
    
    _scrollArea->setWidget(_scrollWidget);
    _mainLayout->addWidget(_scrollArea);
    
    // Control buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    
    _addButton = new QPushButton(tr("Add Property"), this);
    _addButton->setIcon(qApp->style()->standardIcon(QStyle::SP_FileIcon));
    connect(_addButton, &QPushButton::clicked, this, &PropertyEditor::addProperty);
    
    _applyButton = new QPushButton(tr("Apply Changes"), this);
    _applyButton->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogApplyButton));
    _applyButton->setEnabled(false);
    connect(_applyButton, &QPushButton::clicked, this, &PropertyEditor::applyChanges);
    
    _revertButton = new QPushButton(tr("Revert Changes"), this);
    _revertButton->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogCancelButton));
    _revertButton->setEnabled(false);
    connect(_revertButton, &QPushButton::clicked, this, &PropertyEditor::revertChanges);
    
    buttonLayout->addWidget(_addButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(_applyButton);
    buttonLayout->addWidget(_revertButton);
    
    _mainLayout->addLayout(buttonLayout);

    // Basic stats group (Required Level, Max Durability)
    _basicStatsGroup = new QGroupBox(tr("Basic stats"));
    QHBoxLayout *basicLayout = new QHBoxLayout;
    _requiredLevelSpin = new QSpinBox;
    _requiredLevelSpin->setRange(0, 65535);
    _requiredLevelSpin->setToolTip(tr("Required Level (item base RLvl + property)"));
    _maxDurabilitySpin = new QSpinBox;
    _maxDurabilitySpin->setRange(0, 65535);
    _maxDurabilitySpin->setToolTip(tr("Maximum Durability (0 = no durability)"));
    basicLayout->addWidget(new QLabel(tr("Required Level:")));
    basicLayout->addWidget(_requiredLevelSpin);
    basicLayout->addSpacing(20);
    basicLayout->addWidget(new QLabel(tr("Max Durability:")));
    basicLayout->addWidget(_maxDurabilitySpin);
    _basicStatsGroup->setLayout(basicLayout);
    _mainLayout->addWidget(_basicStatsGroup);
}

void PropertyEditor::setItem(ItemInfo *item)
{
    if (_item == item) return;
    
    if (_hasChanges) {
        int ret = QMessageBox::question(this, tr("Unsaved Changes"),
                                       tr("You have unsaved changes. Do you want to apply them?"),
                                       QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) return;
        if (ret == QMessageBox::Yes) applyChanges();
    }
    
    clear();
    _item = item;
    
    if (!_item) {
        _addButton->setEnabled(false);
        return;
    }
    
    // Enable add button for adding new properties
    _addButton->setEnabled(true);
    backupOriginalProperties();
    
    // Populate existing properties
    _updatingUI = true;
    
    // Add item properties (existing only)
#ifndef QT_NO_DEBUG
    qDebug() << "PropertyEditor: Loading" << _item->props.size() << "item properties";
#endif
    for (auto it = _item->props.constBegin(); it != _item->props.constEnd(); ++it) {
        // Skip properties with zero value or missing descFunc (same as PropertiesDisplayManager::propertyDisplay)
        ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(it.key());
        if (it.value()->value == 0 || !propTxt || !propTxt->descFunc) {
#ifndef QT_NO_DEBUG
            QString reason = it.value()->value == 0 ? "zero value" : 
                           (!propTxt ? "missing propTxt" : "missing descFunc");
            qDebug() << QString("PropertyEditor: Skipping item prop ID=%1 (%2)")
                       .arg(it.key()).arg(reason);
#endif
            continue;
        }
        
        // Debug: Log all properties being loaded, especially ID=0 (Strength)
#ifndef QT_NO_DEBUG  
        if (it.key() == 0) {
            qDebug() << QString("PropertyEditor: !!! Found Strength prop ID=0, value=%1, param=%2")
                       .arg(it.value()->value).arg(it.value()->param);
        }
#endif
        
        int displayValue = getDisplayValueForProperty(it.key(), it.value());
#ifndef QT_NO_DEBUG
        qDebug() << QString("PropertyEditor: Item prop ID=%1, value=%2, param=%3, displayValue=%4")
                   .arg(it.key()).arg(it.value()->value).arg(it.value()->param).arg(displayValue);
#endif
        addPropertyRow(it.key(), displayValue, it.value()->param, false, false);
    }
    
    // Add runeword properties (existing only)
#ifndef QT_NO_DEBUG
    qDebug() << "PropertyEditor: Loading" << _item->rwProps.size() << "runeword properties";
#endif
    for (auto it = _item->rwProps.constBegin(); it != _item->rwProps.constEnd(); ++it) {
        // Skip runeword properties with zero value or missing descFunc (same as PropertiesDisplayManager::propertyDisplay)
        ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(it.key());
        if (it.value()->value == 0 || !propTxt || !propTxt->descFunc) {
#ifndef QT_NO_DEBUG
            QString reason = it.value()->value == 0 ? "zero value" : 
                           (!propTxt ? "missing propTxt" : "missing descFunc");
            qDebug() << QString("PropertyEditor: Skipping RW prop ID=%1 (%2)")
                       .arg(it.key()).arg(reason);
#endif
            continue;
        }
        
        int displayValue = getDisplayValueForProperty(it.key(), it.value());
#ifndef QT_NO_DEBUG
        qDebug() << QString("PropertyEditor: RW prop ID=%1, value=%2, param=%3, displayValue=%4")
                   .arg(it.key()).arg(it.value()->value).arg(it.value()->param).arg(displayValue);
#endif
        addPropertyRow(it.key(), displayValue, it.value()->param, false, true);
    }
    
    _updatingUI = false;
    
    // Count actual loaded properties (excluding zero values and missing descFunc)
    int loadedItemProps = 0;
    for (auto it = _item->props.constBegin(); it != _item->props.constEnd(); ++it) {
        ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(it.key());
        if (it.value()->value != 0 && propTxt && propTxt->descFunc) loadedItemProps++;
    }
    
    int loadedRwProps = 0;
    for (auto it = _item->rwProps.constBegin(); it != _item->rwProps.constEnd(); ++it) {
        ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(it.key());
        if (it.value()->value != 0 && propTxt && propTxt->descFunc) loadedRwProps++;
    }
    
    int totalLoadedProps = loadedItemProps + loadedRwProps;
    _statusLabel->setText(tr("Loaded %1 existing properties (%2 item + %3 runeword) - modify values only")
                         .arg(totalLoadedProps).arg(loadedItemProps).arg(loadedRwProps));

    // Populate basic stats fields
    if (_item) {
        // Required Level: default to base rlvl from ItemDataBase if available plus RequiredLevel property
        int baseRlvl = 0;
        ItemBase *base = ItemDataBase::Items()->value(_item->itemType);
        if (base) baseRlvl = base->rlvl;
        int reqPropVal = 0;
        ItemProperty *reqProp = _item->props.value(Enums::ItemProperties::RequiredLevel);
        if (!reqProp) reqProp = _item->rwProps.value(Enums::ItemProperties::RequiredLevel);
        if (reqProp) reqPropVal = reqProp->value;
        _requiredLevelSpin->setValue(baseRlvl + reqPropVal);

        // Max Durability: item->maxDurability if present, else try property DurabilityMax
        int maxDur = _item->maxDurability;
        if (maxDur == 0) {
            ItemProperty *maxDurProp = _item->props.value(Enums::ItemProperties::DurabilityMax);
            if (!maxDurProp) maxDurProp = _item->rwProps.value(Enums::ItemProperties::DurabilityMax);
            if (maxDurProp) maxDur = maxDurProp->value;
        }
        _maxDurabilitySpin->setValue(maxDur);
    } else {
        _requiredLevelSpin->setValue(0);
        _maxDurabilitySpin->setValue(0);
    }
}

void PropertyEditor::clear()
{
    _updatingUI = true;
    
    // Remove all property rows
    for (PropertyEditorRow *row : _propertyRows) {
        delete row->propertyCombo;
        delete row->valueSpinBox;
        delete row->parameterSpinBox;
        delete row->parameterLabel;
        delete row->removeButton;
        delete row->warningLabel;
        delete row->typeLabel;
        delete row;
    }
    _propertyRows.clear();
    
    // Clear layouts
    while (_scrollLayout->count() > 0) {
        QLayoutItem *item = _scrollLayout->takeAt(0);
        if (item->layout()) {
            while (item->layout()->count() > 0) {
                QLayoutItem *child = item->layout()->takeAt(0);
                delete child->widget();
                delete child;
            }
            delete item->layout();
        }
        delete item->widget();
        delete item;
    }
    
    // Clean up original properties backup
    qDeleteAll(_originalProperties);
    _originalProperties.clear();
    qDeleteAll(_originalRwProperties);
    _originalRwProperties.clear();
    
    _hasChanges = false;
    _updatingUI = false;
    
    updateButtonStates();
}

void PropertyEditor::populatePropertyCombo(QComboBox *combo)
{
    if (!combo) {
        // Update all combos
        for (PropertyEditorRow *row : _propertyRows) {
            populatePropertyCombo(row->propertyCombo);
        }
        return;
    }
    
    int currentId = combo->currentData().toInt();
    combo->clear();
    
    QHash<uint, ItemPropertyTxt *> *allProperties = ItemDataBase::Properties();
    
    for (auto it = allProperties->constBegin(); it != allProperties->constEnd(); ++it) {
        ItemPropertyTxt *propTxt = it.value();
        if (!propTxt) continue;
        
        // Skip hidden properties unless show all is checked
        if (!_showAllPropertiesCheck->isChecked()) {
            if (propTxt->descPositive.isEmpty() && propTxt->descNegative.isEmpty()) {
                continue;
            }
        }
        
        QString displayName = getPropertyDisplayName(it.key());
        combo->addItem(displayName, it.key());
    }
    
    // Restore selection
    if (currentId >= 0) {
        int index = combo->findData(currentId);
        if (index >= 0) {
            combo->setCurrentIndex(index);
        }
    }
}

void PropertyEditor::addProperty()
{
    if (!_item) {
        QMessageBox::warning(this, tr("Add Property"), 
                           tr("No item selected."));
        return;
    }
    
    // Show enhanced dialog to select property to add
    AddPropertyDialog dialog(this);
    
    // Connect Enhanced Property Engine for better validation and info
    if (_enhancedEngine) {
        dialog.setPropertyEngine(_enhancedEngine);
    }
    
    if (dialog.exec() == QDialog::Accepted) {
        int propertyId = dialog.getSelectedPropertyId();
        int value = dialog.getDefaultValue();
        quint32 parameter = dialog.getDefaultParameter();
        
        if (propertyId >= 0) {
            // Check if property already exists
            if (_item->props.contains(propertyId)) {
                QMessageBox::information(this, tr("Add Property"),
                                        tr("Property already exists. You can modify its value instead."));
                return;
            }
            
            // Add property using PropertyModificationEngine
            addPropertyToItem(propertyId, value, parameter);
        }
    }
}

void PropertyEditor::addPropertyRow(int propertyId, int value, quint32 parameter, bool isNew, bool isRunewordProperty)
{
    PropertyEditorRow *row = new PropertyEditorRow;
    row->isNew = isNew;
    row->originalPropertyId = propertyId;
    row->originalValue = value;
    row->originalParameter = parameter;
    row->isRunewordProperty = isRunewordProperty;
    
    // Create layout for this row
    QHBoxLayout *rowLayout = new QHBoxLayout;
    QWidget *rowWidget = new QWidget;
    rowWidget->setLayout(rowLayout);
    
    // Property selection combo
    row->propertyCombo = new QComboBox;
    row->propertyCombo->setMinimumWidth(200);
    populatePropertyCombo(row->propertyCombo);
    
    if (propertyId >= 0) {
        int index = row->propertyCombo->findData(propertyId);
        if (index >= 0) row->propertyCombo->setCurrentIndex(index);
        
        // Disable combo for existing properties - only allow value changes
        if (!isNew) {
            row->propertyCombo->setEnabled(false);
        }
    }
    
    connect(row->propertyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, row]() { 
                if (!_updatingUI) {
                    updatePropertyRow(row);
                    onPropertyChanged();
                }
            });
    
    // Value spin box
    row->valueSpinBox = new QSpinBox;
    row->valueSpinBox->setRange(-2147483648, 2147483647);
    row->valueSpinBox->setValue(value);
    row->valueSpinBox->setMinimumWidth(100);
    row->valueSpinBox->setToolTip(tr("Property value. Range will be automatically limited based on property type to prevent overflow."));
    
    connect(row->valueSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this]() { 
                if (!_updatingUI) onValueChanged();
            });
    
    // Parameter label and spin box
    row->parameterLabel = new QLabel(tr("Param:"));
    row->parameterSpinBox = new QSpinBox;
    row->parameterSpinBox->setRange(0, 2147483647); // Maximum signed 32-bit int
    row->parameterSpinBox->setValue(parameter);
    row->parameterSpinBox->setMinimumWidth(80);
    row->parameterSpinBox->setToolTip(tr("Property parameter. Range unrestricted - overflow protection bypassed."));
    
    connect(row->parameterSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this]() { 
                if (!_updatingUI) onParameterChanged();
            });
    
    // Remove button
    row->removeButton = new QPushButton;
    row->removeButton->setIcon(qApp->style()->standardIcon(QStyle::SP_TrashIcon));
    row->removeButton->setToolTip(tr("Remove Property"));
    row->removeButton->setMaximumWidth(30);
    
    connect(row->removeButton, &QPushButton::clicked, 
            [this, row]() { removePropertyRow(row); });
    
    // Warning label
    row->warningLabel = new QLabel;
    row->warningLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    row->warningLabel->hide();
    
    // Type label (Item/RW indicator)
    row->typeLabel = new QLabel(isRunewordProperty ? tr("RW") : tr("Item"));
    row->typeLabel->setStyleSheet(isRunewordProperty ? 
                                 "QLabel { color: green; font-weight: bold; }" :
                                 "QLabel { color: blue; font-weight: bold; }");
    row->typeLabel->setMinimumWidth(35);
    row->typeLabel->setToolTip(isRunewordProperty ? 
                              tr("Runeword Property") : 
                              tr("Item Property"));
    
    // Add to layout
    rowLayout->addWidget(row->typeLabel);
    rowLayout->addWidget(new QLabel(tr("Property:")));
    rowLayout->addWidget(row->propertyCombo, 1);
    rowLayout->addWidget(new QLabel(tr("Value:")));
    rowLayout->addWidget(row->valueSpinBox);
    rowLayout->addWidget(row->parameterLabel);
    rowLayout->addWidget(row->parameterSpinBox);
    rowLayout->addWidget(row->removeButton);
    rowLayout->addWidget(row->warningLabel);
    
    _scrollLayout->addWidget(rowWidget);
    
    _propertyRows.append(row);
    
    // Update the row based on selected property
    updatePropertyRow(row);
    
    if (isNew && !_updatingUI) {
        onPropertyChanged();
    }
}

void PropertyEditor::removePropertyRow(PropertyEditorRow *row)
{
    int index = _propertyRows.indexOf(row);
    if (index < 0) return;
    
    // Find and remove the widget from scroll layout
    QWidget *rowWidget = nullptr;
    for (int i = 0; i < _scrollLayout->count(); ++i) {
        QLayoutItem *item = _scrollLayout->itemAt(i);
        if (item && item->widget()) {
            QHBoxLayout *rowLayout = qobject_cast<QHBoxLayout*>(item->widget()->layout());
            if (rowLayout) {
                // Check if this layout contains our combo box
                for (int j = 0; j < rowLayout->count(); ++j) {
                    if (rowLayout->itemAt(j)->widget() == row->propertyCombo) {
                        rowWidget = item->widget();
                        break;
                    }
                }
            }
        }
        if (rowWidget) break;
    }
    
    _propertyRows.removeAt(index);
    
    // Clean up
    delete row->propertyCombo;
    delete row->valueSpinBox;
    delete row->parameterSpinBox;
    delete row->parameterLabel;
    delete row->removeButton;
    delete row->warningLabel;
    delete row;
    
    if (rowWidget) {
        delete rowWidget;
    }
    
    onPropertyChanged();
}

void PropertyEditor::updatePropertyRow(PropertyEditorRow *row)
{
    if (_updatingUI) return;
    
    _updatingUI = true;
    
    int propertyId = row->propertyCombo->currentData().toInt();
    if (propertyId < 0) {
        row->parameterLabel->setVisible(false);
        row->parameterSpinBox->setVisible(false);
        _updatingUI = false;
        return;
    }
    
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) {
        _updatingUI = false;
        return;
    }
    
    // Update parameter visibility
    bool hasParam = propTxt->paramBits > 0;
    row->parameterLabel->setVisible(hasParam);
    row->parameterSpinBox->setVisible(hasParam);
    
    // Update value range - use safe range to prevent overflow
    QPair<int, int> valueRange = getSafeValueRange(propertyId);
    row->valueSpinBox->setRange(valueRange.first, valueRange.second);
    
    // Update tooltip with specific range information
    QString valueTooltip = tr("Property value range: %1 to %2\nSafe range applied to prevent overflow and game corruption.")
                          .arg(valueRange.first).arg(valueRange.second);
    row->valueSpinBox->setToolTip(valueTooltip);
    
    // Update parameter range - range limits bypassed per user request
    if (hasParam) {
        // Allow full parameter range (0 to maximum signed 32-bit)
        row->parameterSpinBox->setRange(0, 2147483647); // Maximum signed 32-bit int
        
        QString paramTooltip = tr("Parameter range: unrestricted (0 to 2147483647)\nOverflow protection bypassed - use with caution!");
        row->parameterSpinBox->setToolTip(paramTooltip);
    }
    
    // Validate current values
    validatePropertyValue(row);
    
    _updatingUI = false;
}

void PropertyEditor::validatePropertyValue(PropertyEditorRow *row)
{
    row->warningLabel->hide();
    row->warningLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }"); // Reset to default warning style
    
    int propertyId = row->propertyCombo->currentData().toInt();
    if (propertyId < 0) return;
    
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) return;
    
    // Check for duplicate properties within the same type (item or runeword)
    // Properties are considered duplicates only if they have same ID, same parameter, and same type
    // Note: Properties with same ID but different parameters are legitimate (e.g., different skills)
    // Note: Same property in item and runeword can be merged, so they're not duplicates
    bool allowMultiple = isPropertyAllowedMultipleTimes(propertyId);
    
    if (!allowMultiple) {
        quint32 currentParam = row->parameterSpinBox->value();
        int count = 0;
        
        for (const PropertyEditorRow *otherRow : _propertyRows) {
            if (otherRow->propertyCombo->currentData().toInt() == propertyId &&
                otherRow->isRunewordProperty == row->isRunewordProperty &&
                otherRow->parameterSpinBox->value() == currentParam) {
                count++;
            }
        }
        
        if (count > 1) {
            QString duplicateType = row->isRunewordProperty ? tr("runeword") : tr("item");
            row->warningLabel->setText(tr("Duplicate %1 property (same param)!").arg(duplicateType));
            row->warningLabel->show();
            return;
        }
    }
    
    // Check if this property will be merged with another one during display
    // (same property ID and parameter but different type - item vs runeword)
    bool willBeMerged = false;
    quint32 currentParam = row->parameterSpinBox->value();
    
    for (const PropertyEditorRow *otherRow : _propertyRows) {
        if (otherRow != row &&
            otherRow->propertyCombo->currentData().toInt() == propertyId &&
            otherRow->isRunewordProperty != row->isRunewordProperty &&
            otherRow->parameterSpinBox->value() == currentParam) {
            willBeMerged = true;
            break;
        }
    }
    
    if (willBeMerged) {
        QString mergeInfo = tr("Will merge with %1 property in display")
                           .arg(row->isRunewordProperty ? tr("item") : tr("runeword"));
        row->warningLabel->setText(mergeInfo);
        row->warningLabel->setStyleSheet("QLabel { color: blue; font-weight: bold; }");
        row->warningLabel->show();
        // Don't return - this is just informational
    }
    
    // Get current values
    int value = row->valueSpinBox->value();
    quint32 parameter = row->parameterSpinBox->value();
    
    // Validate overflow protection first (most critical)
    if (!validateValueOverflow(propertyId, value)) {
        row->warningLabel->setText(tr("Value overflow risk! Use safer range"));
        row->warningLabel->show();
        return;
    }
    
    // Parameter overflow check bypassed per user request
    /*
    if (!validateParameterOverflow(propertyId, parameter)) {
        row->warningLabel->setText(tr("Parameter overflow risk!"));
        row->warningLabel->show();
        return;
    }
    */
    
    // Get correct value range using the same logic as PropertyModificationEngine
    QPair<int, int> range = getValueRange(propertyId);
    int minValue = range.first;
    int maxValue = range.second;
    
    // Validate value is within allowed range
    if (value < minValue || value > maxValue) {
        QString warningText;
        if (minValue >= 0) {
            warningText = tr("Range: %1-%2").arg(minValue).arg(maxValue);
        } else {
            warningText = tr("Range: %1 to +%2").arg(minValue).arg(maxValue);
        }
        row->warningLabel->setText(warningText);
        row->warningLabel->show();
        return;
    }
    
    // Parameter validation if applicable
    if (propTxt->paramBits > 0) {
        QPair<quint32, quint32> paramRange = getParameterRange(propertyId);
        if (parameter > paramRange.second) {
            row->warningLabel->setText(tr("Parameter: 0-%1").arg(paramRange.second));
            row->warningLabel->show();
            return;
        }
    }
    
    // Special validation for specific properties
    switch (propertyId) {
        case Enums::ItemProperties::Defence:
            if (value < 0) {
                row->warningLabel->setText(tr("Defense cannot be negative"));
                row->warningLabel->show();
            }
            break;
        case Enums::ItemProperties::MaximumDamage:
        case Enums::ItemProperties::MaximumDamageSecondary:
        case Enums::ItemProperties::MaximumDamageFire:
        case Enums::ItemProperties::MaximumDamageLightning:
        case Enums::ItemProperties::MaximumDamageMagic:
        case Enums::ItemProperties::MaximumDamageCold:
        case Enums::ItemProperties::MaximumDamagePoison:
            // Check if max damage is less than min damage for same type
            if (value <= 0) {
                row->warningLabel->setText(tr("Maximum damage must be positive"));
                row->warningLabel->show();
            }
            break;
        case Enums::ItemProperties::Strength:
        case Enums::ItemProperties::Dexterity:
        case Enums::ItemProperties::Vitality:
        case Enums::ItemProperties::Energy:
            if (qAbs(value) > 1000) {
                row->warningLabel->setText(tr("Extreme stat values may cause issues"));
                row->warningLabel->show();
            }
            break;
        default:
            break;
    }
}

QString PropertyEditor::getPropertyDisplayName(int propertyId) const
{
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) return tr("Unknown Property (%1)").arg(propertyId);
    
    QString name;
    if (!propTxt->descPositive.isEmpty()) {
        name = propTxt->descPositive;
    } else if (!propTxt->descNegative.isEmpty()) {
        name = propTxt->descNegative;
    } else if (!propTxt->stat.isEmpty()) {
        name = QString::fromUtf8(propTxt->stat);
    } else {
        name = tr("Property %1").arg(propertyId);
    }
    
    return QString("%1 (%2)").arg(name).arg(propertyId);
}

QPair<int, int> PropertyEditor::getValueRange(int propertyId) const
{
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) return QPair<int, int>(-32768, 32767);
    
    // Calculate correct range based on bit storage format
    // This matches the logic in PropertyModificationEngine
    // Add safety check to prevent bit overflow
    int bits = qMin(31, (int)propTxt->bits); // Limit to 31 bits to prevent signed overflow
    int maxValue = (1 << bits) - 1 - propTxt->add;
    int minValue = -propTxt->add;
    
    // Special cases that override the general calculation
    switch (propertyId) {
        case Enums::ItemProperties::EnhancedDamage:
            return QPair<int, int>(0, 32767);
        case Enums::ItemProperties::Defence:
            return QPair<int, int>(0, 65535);
            
        // Damage properties - cannot be negative
        case Enums::ItemProperties::MinimumDamage:
        case Enums::ItemProperties::MaximumDamage:
        case Enums::ItemProperties::MinimumDamageSecondary:
        case Enums::ItemProperties::MaximumDamageSecondary:
        case Enums::ItemProperties::MinimumDamageFire:
        case Enums::ItemProperties::MaximumDamageFire:
        case Enums::ItemProperties::MinimumDamageLightning:
        case Enums::ItemProperties::MaximumDamageLightning:
        case Enums::ItemProperties::MinimumDamageMagic:
        case Enums::ItemProperties::MaximumDamageMagic:
        case Enums::ItemProperties::MinimumDamageCold:
        case Enums::ItemProperties::MaximumDamageCold:
        case Enums::ItemProperties::MinimumDamagePoison:
        case Enums::ItemProperties::MaximumDamagePoison:
            return QPair<int, int>(qMax(0, minValue), maxValue);
            
        // Life, Mana, Stamina - can be negative (penalties)
        case Enums::ItemProperties::Life:
        case Enums::ItemProperties::Mana:
        case Enums::ItemProperties::Stamina:
        // Stats - can be negative (curses, debuffs)
        case Enums::ItemProperties::Strength:
        case Enums::ItemProperties::Dexterity:
        case Enums::ItemProperties::Vitality:
        case Enums::ItemProperties::Energy:
            // Use full calculated range - allows negative values
            return QPair<int, int>(minValue, maxValue);
            
        default:
            // For most properties, use the calculated range
            // This includes resistances which CAN be negative (penalties)
            return QPair<int, int>(minValue, maxValue);
    }
}

QPair<quint32, quint32> PropertyEditor::getParameterRange(int propertyId) const
{
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt || propTxt->paramBits == 0) {
        return QPair<quint32, quint32>(0, 0);
    }
    
    // Safety: Limit parameter bits to prevent overflow
    int bits = qMin(16, (int)propTxt->paramBits);
    quint32 maxParam = (bits >= 32) ? 0xFFFFFFFF : ((1U << bits) - 1);
    return QPair<quint32, quint32>(0, maxParam);
}

QPair<int, int> PropertyEditor::getSafeValueRange(int propertyId) const
{
    QPair<int, int> range = getValueRange(propertyId);
    
    // Apply additional safety limits to prevent game crashes
    const int ABSOLUTE_MIN = -2147483647;  // Safe minimum for signed 32-bit
    const int ABSOLUTE_MAX = 2147483647;   // Safe maximum for signed 32-bit
    
    // Clamp to safe bounds
    range.first = qMax(ABSOLUTE_MIN, range.first);
    range.second = qMin(ABSOLUTE_MAX, range.second);
    
    // Additional specific property limits based on game mechanics
    switch (propertyId) {
        case Enums::ItemProperties::Defence:
            return QPair<int, int>(0, 65535);  // Defense max in D2
            
        case Enums::ItemProperties::Life:
        case Enums::ItemProperties::Mana:
        case Enums::ItemProperties::Stamina:
            return QPair<int, int>(-32767, 32767);  // Reasonable stat range
            
        case Enums::ItemProperties::Strength:
        case Enums::ItemProperties::Dexterity:
        case Enums::ItemProperties::Vitality:
        case Enums::ItemProperties::Energy:
            return QPair<int, int>(-1000, 10000);  // Reasonable attribute range
            
        case Enums::ItemProperties::EnhancedDamage:
            return QPair<int, int>(0, 65535);  // ED% cannot be negative
            
        // Damage properties
        case Enums::ItemProperties::MinimumDamage:
        case Enums::ItemProperties::MaximumDamage:
        case Enums::ItemProperties::MinimumDamageSecondary:
        case Enums::ItemProperties::MaximumDamageSecondary:
        case Enums::ItemProperties::MinimumDamageFire:
        case Enums::ItemProperties::MaximumDamageFire:
        case Enums::ItemProperties::MinimumDamageLightning:
        case Enums::ItemProperties::MaximumDamageLightning:
        case Enums::ItemProperties::MinimumDamageMagic:
        case Enums::ItemProperties::MaximumDamageMagic:
        case Enums::ItemProperties::MinimumDamageCold:
        case Enums::ItemProperties::MaximumDamageCold:
        case Enums::ItemProperties::MinimumDamagePoison:
        case Enums::ItemProperties::MaximumDamagePoison:
            return QPair<int, int>(0, 32767);  // Damage cannot be negative
            
        default:
            return range;
    }
}

bool PropertyEditor::validateValueOverflow(int propertyId, int value) const
{
    QPair<int, int> safeRange = getSafeValueRange(propertyId);
    return value >= safeRange.first && value <= safeRange.second;
}

bool PropertyEditor::validateParameterOverflow(int propertyId, quint32 parameter) const
{
    QPair<quint32, quint32> range = getParameterRange(propertyId);
    return parameter <= range.second;
}

int PropertyEditor::getDisplayValueForProperty(int propertyId, const ItemProperty *property) const
{
    if (!property) return 0;
    
    // Most properties display the stored value directly
    // But some need special handling
    
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) {
#ifndef QT_NO_DEBUG
        qDebug() << "PropertyEditor: No property definition found for ID" << propertyId;
#endif
        return property->value;
    }
    
    int result = property->value;
    
    switch (propertyId) {
        case Enums::ItemProperties::EnhancedDamage:
            // Enhanced damage is stored with special handling
            // The displayed value should match what user sees in game
            break;
            
        case Enums::ItemProperties::Defence:
            // Defense is stored as (value + add) in bitstring
            // but property->value already has 'add' subtracted
            break;
            
        // For most properties, the stored value is what should be displayed
        default:
            break;
    }
    
#ifndef QT_NO_DEBUG
    qDebug() << QString("PropertyEditor: Property %1 - stored: %2, display: %3, add: %4, bits: %5")
                .arg(propertyId).arg(property->value).arg(result).arg(propTxt->add).arg(propTxt->bits);
#endif
    
    return result;
}

int PropertyEditor::getStorageValueFromDisplay(int propertyId, int displayValue) const
{
    // Convert from display value back to storage value
    // This reverses getDisplayValueForProperty
    
    ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) return displayValue;
    
    switch (propertyId) {
        case Enums::ItemProperties::EnhancedDamage:
            return displayValue;
            
        case Enums::ItemProperties::Defence:
            return displayValue;
            
        default:
            return displayValue;
    }
}

bool PropertyEditor::isPropertyAllowedMultipleTimes(int propertyId) const
{
    // Some properties can legitimately appear multiple times on the same item
    switch (propertyId) {
        case Enums::ItemProperties::ChargedSkill:
            // Charged skills can have multiple instances (different skills)
            return true;
            
        case Enums::ItemProperties::Oskill:
        case Enums::ItemProperties::ClassOnlySkill:
            // Oskills can have multiple instances (different skills)
            return true;
            
        // Add other properties that can appear multiple times as needed
        // Examples might include: specific aura grants, specific proc chances, etc.
        
        default:
            return false;
    }
}

void PropertyEditor::backupOriginalProperties()
{
    if (!_item) return;
    
    qDebug() << "PropertyEditor: Backing up" << _item->props.size() << "properties and" << _item->rwProps.size() << "RW properties";
    
    // Clean up existing backups
    qDeleteAll(_originalProperties);
    _originalProperties.clear();
    qDeleteAll(_originalRwProperties);
    _originalRwProperties.clear();
    
    // Backup item properties
    for (auto it = _item->props.constBegin(); it != _item->props.constEnd(); ++it) {
        if (it.value()) {
            _originalProperties.insert(it.key(), new ItemProperty(*it.value()));
        }
    }
    
    // Backup runeword properties
    for (auto it = _item->rwProps.constBegin(); it != _item->rwProps.constEnd(); ++it) {
        if (it.value()) {
            _originalRwProperties.insert(it.key(), new ItemProperty(*it.value()));
        }
    }
    
    qDebug() << "PropertyEditor: Backup completed";
}

bool PropertyEditor::hasChanges() const
{
    return _hasChanges;
}

void PropertyEditor::onPropertyChanged()
{
    if (_updatingUI) return;
    
    _hasChanges = true;
    updateButtonStates();
    
    // Validate all rows
    for (PropertyEditorRow *row : _propertyRows) {
        validatePropertyValue(row);
    }
    
    emit propertyModified();
}

void PropertyEditor::onValueChanged()
{
    onPropertyChanged();
}

void PropertyEditor::onParameterChanged()
{
    onPropertyChanged();
}

void PropertyEditor::validateProperty()
{
    // Validate all properties
    for (PropertyEditorRow *row : _propertyRows) {
        validatePropertyValue(row);
    }
}

void PropertyEditor::updateButtonStates()
{
    _applyButton->setEnabled(_hasChanges);
    _revertButton->setEnabled(_hasChanges);
    
    if (_hasChanges) {
        _statusLabel->setText(tr("Modified - %1 properties").arg(_propertyRows.size()));
        _statusLabel->setStyleSheet("QLabel { color: orange; font-weight: bold; }");
    } else {
        _statusLabel->setText(tr("No changes - %1 properties").arg(_propertyRows.size()));
        _statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
    }
}

void PropertyEditor::applyChanges()
{
    if (!_item || !_hasChanges) return;
    
    // Strong warning about potential corruption
    QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Property Editor Warning"),
        tr("⚠️ IMPORTANT WARNING ⚠️\n\n"
           "Property Editor is EXPERIMENTAL and may cause file corruption!\n\n"
           "• Backup your save files before proceeding\n"
           "• Only modify EXISTING property values, avoid adding/removing properties\n"
           "• Test changes on non-important characters first\n"
           "• The save file may become corrupted and unloadable\n\n"
           "Do you want to proceed at your own risk?"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, 
        QMessageBox::No);
    
    if (ret != QMessageBox::Yes) return;
    
    try {
        applyPropertyChanges();
        
        _hasChanges = false;
        updateButtonStates();
        
        _statusLabel->setText(tr("Changes applied successfully!"));
        _statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
        
    qDebug() << "PropertyEditor: emitting itemChanged for item" << (_item ? _item->itemType : QString("<null>"));
    qDebug() << "PropertyEditor: emitting itemChanged (revert) for item" << (_item ? _item->itemType : QString("<null>"));
    emit itemChanged();
        
    } catch (const std::exception &e) {
        QMessageBox::critical(this, tr("Error"), 
                             tr("Failed to apply changes: %1").arg(e.what()));
    } catch (...) {
        QMessageBox::critical(this, tr("Error"), 
                             tr("Failed to apply changes: Unknown error"));
    }
}

void PropertyEditor::applyPropertyChanges()
{
    if (!_item) return;
    
    // Temporarily disable PropertyModificationEngine due to bit loss issues
    // Use original ReverseBitWriter approach with fixed offset calculation
    // PropertyModificationEngine engine;
    // PropertiesMultiMap newProperties;
    
    // Pre-validate all values for overflow before applying any changes
    QStringList validationErrors;
    for (const PropertyEditorRow *row : _propertyRows) {
        int propertyId = row->propertyCombo->currentData().toInt();
        if (propertyId < 0) continue;
        
        int value = row->valueSpinBox->value();
        quint32 parameter = row->parameterSpinBox->value();
        
        if (!validateValueOverflow(propertyId, value)) {
            validationErrors << tr("Property %1: Value %2 exceeds safe range")
                               .arg(propertyId).arg(value);
        }
        
        // Parameter overflow check bypassed per user request
        /*
        if (!validateParameterOverflow(propertyId, parameter)) {
            validationErrors << tr("Property %1: Parameter %2 exceeds safe range")
                               .arg(propertyId).arg(parameter);
        }
        */
    }
    
    if (!validationErrors.isEmpty()) {
        QMessageBox::warning(this, tr("Overflow Protection"), 
                           tr("Cannot apply changes due to overflow risks:\n\n%1")
                           .arg(validationErrors.join("\n")));
        return;
    }
    
    // Use original ReverseBitWriter approach with corrected offset calculation
    for (const PropertyEditorRow *row : _propertyRows) {
        int propertyId = row->propertyCombo->currentData().toInt();
        if (propertyId < 0) continue;
        
        int displayValue = row->valueSpinBox->value();
        quint32 newParameter = row->parameterSpinBox->value();
        
        // Find existing property in item (check both props and rwProps)
        ItemProperty *existingProp = _item->props.value(propertyId);
        bool isRunewordProperty = false;
        
        if (!existingProp) {
            existingProp = _item->rwProps.value(propertyId);
            isRunewordProperty = true;
        }
        
        if (!existingProp) {
            // Property doesn't exist in either props or rwProps - skip for safety
            qDebug() << "PropertyEditor: Property" << propertyId << "not found, skipping";
            continue;
        }
        
        // Convert display value back to storage value
        int storageValue = getStorageValueFromDisplay(propertyId, displayValue);
        
        qDebug() << "PropertyEditor: Updating property" << propertyId << "from" << existingProp->value 
                 << "to" << storageValue << "param from" << existingProp->param << "to" << newParameter;
        
        // Update the property value in memory
        existingProp->value = storageValue;
        existingProp->param = newParameter;
        
        // Update the property in the bit string using ReverseBitWriter with CORRECTED offsets
        ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(propertyId);
        if (propTxt && existingProp->bitStringOffset > 16) { // Must be after JM header
            
            qDebug() << "PropertyEditor: Property" << propertyId << "bitStringOffset=" << existingProp->bitStringOffset 
                     << "paramBits=" << propTxt->paramBits << "valueBits=" << propTxt->bits;
            
            // CORRECTED: bitStringOffset points to start of param+value section (after property ID)
            // Parameter comes first, then value
            if (propTxt->paramBits > 0) {
                int paramOffset = existingProp->bitStringOffset;
                qDebug() << "PropertyEditor: Updating parameter at offset" << paramOffset;
                ReverseBitWriter::replaceValueInBitString(_item->bitString, 
                                                        paramOffset, 
                                                        newParameter, 
                                                        propTxt->paramBits);
            }
            
            // Value comes after parameter
            int valueOffset = existingProp->bitStringOffset + propTxt->paramBits;
            int bitStringValue = storageValue + propTxt->add; // Add back the 'add' value for storage
            qDebug() << "PropertyEditor: Updating value at offset" << valueOffset << "value=" << bitStringValue;
            ReverseBitWriter::replaceValueInBitString(_item->bitString, 
                                                    valueOffset, 
                                                    bitStringValue, 
                                                    propTxt->bits);
        }
        
        // Update display string
        ItemParser::createDisplayStringForPropertyWithId(propertyId, existingProp);
    }
    
    // Ensure bit string is properly aligned
    // Apply basic stats edits from UI
    // 1) Required Level (property 92) - stored as a property that is added to base rlvl
    int desiredReqLevel = _requiredLevelSpin ? _requiredLevelSpin->value() : 0;
    int baseRlvl = 0;
    ItemBase *base = ItemDataBase::Items()->value(_item->itemType);
    if (base) baseRlvl = base->rlvl;
    int reqPropValue = desiredReqLevel - baseRlvl;

    if (desiredReqLevel >= 0) {
        ItemProperty *existingReq = _item->props.value(Enums::ItemProperties::RequiredLevel);
        bool isRw = false;
        if (!existingReq) { existingReq = _item->rwProps.value(Enums::ItemProperties::RequiredLevel); isRw = true; }

        if (existingReq) {
            // update in-memory and bitstring
            existingReq->value = reqPropValue;
            ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(Enums::ItemProperties::RequiredLevel);
            if (propTxt && existingReq->bitStringOffset > 16) {
                int valueOffset = existingReq->bitStringOffset + propTxt->paramBits;
                int bitValue = existingReq->value + propTxt->add;
                ReverseBitWriter::replaceValueInBitString(_item->bitString, valueOffset, bitValue, propTxt->bits);
            }
        } else {
            // add as a property (engine will refresh UI)
            addPropertyToItem(Enums::ItemProperties::RequiredLevel, reqPropValue, 0);
        }
    }

    // 2) Max Durability (property DurabilityMax = 73)
    int desiredMaxDur = _maxDurabilitySpin ? _maxDurabilitySpin->value() : 0;
    if (desiredMaxDur >= 0) {
        // Update ItemInfo field
        _item->maxDurability = desiredMaxDur;

        ItemProperty *existingMaxDur = _item->props.value(Enums::ItemProperties::DurabilityMax);
        bool isRw2 = false;
        if (!existingMaxDur) { existingMaxDur = _item->rwProps.value(Enums::ItemProperties::DurabilityMax); isRw2 = true; }

        if (existingMaxDur) {
            existingMaxDur->value = desiredMaxDur;
            ItemPropertyTxt *propTxt = ItemDataBase::Properties()->value(Enums::ItemProperties::DurabilityMax);
            if (propTxt && existingMaxDur->bitStringOffset > 16) {
                int valueOffset = existingMaxDur->bitStringOffset + propTxt->paramBits;
                int bitValue = existingMaxDur->value + propTxt->add;
                ReverseBitWriter::replaceValueInBitString(_item->bitString, valueOffset, bitValue, propTxt->bits);
            }
        } else {
            // Add property so it will be saved properly
            addPropertyToItem(Enums::ItemProperties::DurabilityMax, desiredMaxDur, 0);
        }
    }

    // Now ensure bit string is properly aligned
    ReverseBitWriter::byteAlignBits(_item->bitString);
    
    // Mark item as changed for save mechanism
    _item->hasChanged = true;
    
    qDebug() << "PropertyEditor: Updating backup...";
    
    // Update backup
    backupOriginalProperties();
    
    qDebug() << "PropertyEditor: Marking changes and updating UI...";
    
    // Note: Save file offsets will be recalculated during save process
    
    // Mark item as changed and reset UI state
    _hasChanges = false;  // Reset since we just applied changes
    updateButtonStates();
    
    qDebug() << "PropertyEditor: All done!";
}

void PropertyEditor::revertChanges()
{
    if (!_item || !_hasChanges) return;
    
    int ret = QMessageBox::question(this, tr("Revert Changes"),
                                   tr("Revert all changes and restore original properties?"),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret != QMessageBox::Yes) return;
    
    // Restore original properties
    qDeleteAll(_item->props);
    _item->props.clear();
    
    for (auto it = _originalProperties.constBegin(); it != _originalProperties.constEnd(); ++it) {
        _item->props.insert(it.key(), new ItemProperty(*it.value()));
    }
    
    // Restore original runeword properties
    qDeleteAll(_item->rwProps);
    _item->rwProps.clear();
    
    for (auto it = _originalRwProperties.constBegin(); it != _originalRwProperties.constEnd(); ++it) {
        _item->rwProps.insert(it.key(), new ItemProperty(*it.value()));
    }
    
    // Reload the editor
    setItem(_item);
    
    _hasChanges = false;
    updateButtonStates();
    
    _statusLabel->setText(tr("Changes reverted"));
    _statusLabel->setStyleSheet("QLabel { color: blue; font-weight: bold; }");
    
    emit itemChanged();
}

void PropertyEditor::addPropertyToItem(int propertyId, int value, quint32 parameter)
{
    if (!_item || !_enhancedEngine) return;
    
    // Set status
    if (_statusLabel) {
        _statusLabel->setText(tr("Adding property..."));
    }
    
    // Use Enhanced Property Addition Engine with LSB-first algorithm
    if (_enhancedEngine->addPropertyToItem(_item, propertyId, value, parameter)) {
        // Success! Update UI
        _hasChanges = true;
        backupOriginalProperties();
    qDebug() << "PropertyEditor: emitting itemChanged (addProperty) for item" << (_item ? _item->itemType : QString("<null>"));
    emit itemChanged();
        
        // Refresh property display by re-setting the item
        if (!_updatingUI) {
            ItemInfo* currentItem = _item;
            _item = nullptr; // Reset to force refresh
            setItem(currentItem);
        }
        
        if (_statusLabel) {
            _statusLabel->setText(tr("Property added successfully"));
        }
    } else {
        // Error handling is done via Enhanced Engine's error signals
        if (_statusLabel) {
            _statusLabel->setText(tr("Failed to add property"));
        }
    }
}

void PropertyEditor::removeProperty()
{
    QPushButton *removeButton = qobject_cast<QPushButton*>(sender());
    if (!removeButton) return;
    
    // Find the property row that contains this remove button
    for (int i = 0; i < _propertyRows.size(); ++i) {
        PropertyEditorRow *row = _propertyRows[i];
        if (row->removeButton == removeButton) {
            removePropertyRow(row);
            break;
        }
    }
}

