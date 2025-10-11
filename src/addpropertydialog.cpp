#include "addpropertydialog.h"
#include "enhancedpropertyadditionengine.h"
#include "itemdatabase.h"
#include "enums.h"
#include "helpers.h"

#include <QMessageBox>
#include <QApplication>
#include <QStyle>

AddPropertyDialog::AddPropertyDialog(QWidget *parent)
    : QDialog(parent)
    , _propertyEngine(nullptr)
{
    setupUI();
    populatePropertyCombo();
    updatePropertyInfo();
}

void AddPropertyDialog::setPropertyEngine(EnhancedPropertyAdditionEngine *engine)
{
    _propertyEngine = engine;
    // Refresh the property list with enhanced engine data
    if (_propertyEngine) {
        populatePropertyCombo();
        updatePropertyInfo();
    }
}

void AddPropertyDialog::setupUI()
{
    setWindowTitle(tr("Add New Property"));
    setModal(true);
    resize(450, 300);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Property selection group
    QGroupBox *selectionGroup = new QGroupBox(tr("Property Selection"), this);
    QFormLayout *formLayout = new QFormLayout(selectionGroup);
    
    _propertyCombo = new QComboBox(this);
    _propertyCombo->setMinimumWidth(250);
    formLayout->addRow(tr("Property:"), _propertyCombo);
    
    _propertyInfoLabel = new QLabel(this);
    _propertyInfoLabel->setWordWrap(true);
    _propertyInfoLabel->setStyleSheet("QLabel { color: #555; font-size: 10px; padding: 5px; }");
    formLayout->addRow(_propertyInfoLabel);
    
    mainLayout->addWidget(selectionGroup);
    
    // Value configuration group
    QGroupBox *valueGroup = new QGroupBox(tr("Property Values"), this);
    QFormLayout *valueLayout = new QFormLayout(valueGroup);
    
    _valueSpinBox = new QSpinBox(this);
    _valueSpinBox->setRange(-2147483648, 2147483647);
    _valueSpinBox->setValue(1);
    _valueSpinBox->setMinimumWidth(100);
    valueLayout->addRow(tr("Value:"), _valueSpinBox);
    
    _valueRangeLabel = new QLabel(this);
    _valueRangeLabel->setStyleSheet("QLabel { color: #666; font-size: 9px; }");
    valueLayout->addRow(_valueRangeLabel);
    
    _parameterLabel = new QLabel(tr("Parameter:"));
    _parameterSpinBox = new QSpinBox(this);
    _parameterSpinBox->setRange(0, 2147483647);
    _parameterSpinBox->setValue(0);
    _parameterSpinBox->setMinimumWidth(100);
    _parameterSpinBox->setToolTip(tr("Used for skill IDs, class restrictions, etc."));
    valueLayout->addRow(_parameterLabel, _parameterSpinBox);
    
    mainLayout->addWidget(valueGroup);
    
    // Button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    
    _okButton = new QPushButton(tr("Add Property"), this);
    _okButton->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogApplyButton));
    _okButton->setDefault(true);
    
    _cancelButton = new QPushButton(tr("Cancel"), this);
    _cancelButton->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogCancelButton));
    
    buttonLayout->addWidget(_okButton);
    buttonLayout->addWidget(_cancelButton);
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(_propertyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AddPropertyDialog::onPropertySelectionChanged);
    connect(_okButton, &QPushButton::clicked, this, &AddPropertyDialog::onAccept);
    connect(_cancelButton, &QPushButton::clicked, this, &AddPropertyDialog::onCancel);
}

void AddPropertyDialog::populatePropertyCombo()
{
    _propertyCombo->clear();
    _propertyCombo->addItem(tr("-- Select Property --"), -1);
    
    // Use Enhanced Property Engine if available
    if (_propertyEngine) {
        QVector<EnhancedPropertyAdditionEngine::PropertySpec> supportedProps = _propertyEngine->getSupportedProperties();
        
        // Sort by property name for better user experience
        std::sort(supportedProps.begin(), supportedProps.end(), 
                  [](const EnhancedPropertyAdditionEngine::PropertySpec &a, 
                     const EnhancedPropertyAdditionEngine::PropertySpec &b) {
                      return a.name < b.name;
                  });
        
        for (const auto &spec : supportedProps) {
            QString displayText = QString("%1 (ID: %2)").arg(spec.name).arg(spec.id);
            _propertyCombo->addItem(displayText, spec.id);
        }
        
        return;
    }
    
    // Fallback: Add common properties manually  
    struct CommonProperty {
        int id;
        const char* name;
    };
    
    CommonProperty commonProps[] = {
        {0, "Strength"}, 
        {1, "Energy"},
        {2, "Dexterity"},
        {3, "Vitality"},
        {7, "Life"},
        {9, "Mana"},
        {16, "Enhanced Defense"},
        {17, "Enhanced Damage"},
        {39, "Fire Resistance"},
        {41, "Cold Resistance"},
        {43, "Lightning Resistance"},
        {45, "Poison Resistance"},
        {79, "Gold Find"},      // TESTED SUCCESSFULLY!
        {80, "Magic Find"},
        {93, "Attack Speed"},
        {127, "All Skills"}
    };
    
    for (const auto& prop : commonProps) {
        ItemPropertyTxt* propTxt = ItemDataBase::Properties()->value(prop.id);
        if (propTxt) {
            QString displayName = QString("%1 (%2)").arg(prop.name).arg(prop.id);
            _propertyCombo->addItem(displayName, prop.id);
        }
    }
    
    // Add separator
    _propertyCombo->insertSeparator(_propertyCombo->count());
    
    // Add all other properties
    auto* properties = ItemDataBase::Properties();
    QList<int> propertyIds;
    
    // Convert keys from the hash to QList<int>
    for (auto it = properties->begin(); it != properties->end(); ++it) {
        propertyIds.append(static_cast<int>(it.key()));
    }
    std::sort(propertyIds.begin(), propertyIds.end());
    
    for (int propertyId : propertyIds) {
        // Skip if already added in common properties
        bool isCommon = false;
        for (const auto& prop : commonProps) {
            if (prop.id == propertyId) {
                isCommon = true;
                break;
            }
        }
        if (isCommon) continue;
        
        ItemPropertyTxt* propTxt = properties->value(propertyId);
        if (!propTxt || !propTxt->descFunc) continue;
        
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
        
        QString displayName = QString("%1 (%2)").arg(name).arg(propertyId);
        _propertyCombo->addItem(displayName, propertyId);
    }
}

void AddPropertyDialog::onPropertySelectionChanged()
{
    updatePropertyInfo();
    updateValueRange();
    updateParameterVisibility();
}

void AddPropertyDialog::updatePropertyInfo()
{
    int propertyId = getSelectedPropertyId();
    if (propertyId < 0) {
        _propertyInfoLabel->setText(tr("Please select a property to add."));
        _okButton->setEnabled(false);
        return;
    }
    
    // Use Enhanced Property Engine if available
    if (_propertyEngine && _propertyEngine->isPropertySupported(propertyId)) {
        auto spec = _propertyEngine->getPropertySpec(propertyId);
        
        QString info = QString(tr("Property: %1\nID: %2\nBits: %3"))
                          .arg(spec.name)
                          .arg(spec.id)
                          .arg(spec.bits);
        
        if (spec.paramBits > 0) {
            info += tr("\nParameter Bits: %1").arg(spec.paramBits);
        }
        
        // Calculate and show value range
        int maxRawValue = (1 << spec.bits) - 1;
        int maxValue = maxRawValue - spec.add;
        int minValue = 0 - spec.add;
        info += tr("\nValid Range: %1 to %2").arg(minValue).arg(maxValue);
        
        // Show special notes for tested properties
        if (propertyId == 79) {
            info += tr("\n✅ TESTED: Gold Find property addition works perfectly!");
        } else if (propertyId == 80) {
            info += tr("\n✨ Magic Find property using same algorithm as Gold Find");
        }
        
        _propertyInfoLabel->setText(info);
        _okButton->setEnabled(true);
        return;
    }
    
    // Fallback to ItemDatabase
    ItemPropertyTxt* propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt) {
        _propertyInfoLabel->setText(tr("Invalid property selected."));
        _okButton->setEnabled(false);
        return;
    }
    
    QString info;
    if (!propTxt->descPositive.isEmpty()) {
        info += tr("Description: %1").arg(propTxt->descPositive);
    }
    if (!propTxt->stat.isEmpty()) {
        info += tr("\nStat: %1").arg(QString::fromUtf8(propTxt->stat));
    }
    info += tr("\nBits: %1").arg(propTxt->bits);
    if (propTxt->paramBits > 0) {
        info += tr("\nParameter Bits: %1").arg(propTxt->paramBits);
    }
    
    _propertyInfoLabel->setText(info);
    _okButton->setEnabled(true);
}

void AddPropertyDialog::updateValueRange()
{
    int propertyId = getSelectedPropertyId();
    if (propertyId < 0) {
        _valueRangeLabel->clear();
        return;
    }
    
    int minValue = 0, maxValue = 100;
    
    // Use Enhanced Property Engine if available
    if (_propertyEngine && _propertyEngine->isPropertySupported(propertyId)) {
        auto spec = _propertyEngine->getPropertySpec(propertyId);
        
        // Calculate value range using Enhanced Engine specs
        int maxRawValue = (1 << spec.bits) - 1;
        maxValue = maxRawValue - spec.add;
        minValue = 0 - spec.add;
        
        _valueRangeLabel->setText(tr("Valid range: %1 to %2").arg(minValue).arg(maxValue));
    } else {
        // Fallback to ItemDatabase
        ItemPropertyTxt* propTxt = ItemDataBase::Properties()->value(propertyId);
        if (!propTxt) {
            _valueRangeLabel->clear();
            return;
        }
        
        int maxRawValue = (1 << propTxt->bits) - 1;
        maxValue = maxRawValue - propTxt->add;
        minValue = 0 - propTxt->add;
        
        _valueRangeLabel->setText(tr("Valid range: %1 to %2").arg(minValue).arg(maxValue));
    }
    
    // Set spinbox range
    _valueSpinBox->setRange(minValue, maxValue);
    
    // Set smart default values based on property type
    int defaultValue = 1;
    if (propertyId == 127) {           // All Skills
        defaultValue = 1;
    } else if (propertyId == 79) {     // Gold Find - TESTED!
        defaultValue = 50;
    } else if (propertyId == 80) {     // Magic Find
        defaultValue = 30;
    } else if (propertyId == 16 || propertyId == 17) { // Enhanced Defense/Damage
        defaultValue = 100;
    } else if (propertyId >= 39 && propertyId <= 45) { // Resistances
        defaultValue = 30;
    } else if (propertyId >= 0 && propertyId <= 3) {   // Core stats
        defaultValue = 20;
    } else if (propertyId == 7 || propertyId == 9) {   // Life/Mana
        defaultValue = 50;
    } else if (propertyId == 93) {     // Attack Speed
        defaultValue = 20;
    } else {
        defaultValue = qMax(1, qMin(10, maxValue));
    }
    
    _valueSpinBox->setValue(qBound(minValue, defaultValue, maxValue));
}

void AddPropertyDialog::updateParameterVisibility()
{
    int propertyId = getSelectedPropertyId();
    if (propertyId < 0) {
        _parameterLabel->hide();
        _parameterSpinBox->hide();
        return;
    }
    
    ItemPropertyTxt* propTxt = ItemDataBase::Properties()->value(propertyId);
    if (!propTxt || propTxt->paramBits == 0) {
        _parameterLabel->hide();
        _parameterSpinBox->hide();
        _parameterSpinBox->setValue(0);
        return;
    }
    
    _parameterLabel->show();
    _parameterSpinBox->show();
    
    // Set parameter range
    quint32 maxParam = (1u << propTxt->paramBits) - 1;
    _parameterSpinBox->setRange(0, qMin(maxParam, 2147483647u));
    
    // Set default parameter for skill properties
    if (propertyId == 97) { // Individual skill
        _parameterSpinBox->setValue(36); // Fire Bolt as example
        _parameterSpinBox->setToolTip(tr("Skill ID (e.g., 36 = Fire Bolt, 54 = Meteor)"));
    } else if (propertyId == 83) { // Skill tab
        _parameterSpinBox->setValue(0); // Fire Skills tab for Sorceress
        _parameterSpinBox->setToolTip(tr("Skill Tab ID + Class (e.g., 0 = Sorc Fire, 8 = Sorc Light)"));
    } else {
        _parameterSpinBox->setValue(0);
        _parameterSpinBox->setToolTip(tr("Parameter value (see game documentation)"));
    }
}

int AddPropertyDialog::getSelectedPropertyId() const
{
    return _propertyCombo->currentData().toInt();
}

int AddPropertyDialog::getDefaultValue() const
{
    return _valueSpinBox->value();
}

quint32 AddPropertyDialog::getDefaultParameter() const
{
    return _parameterSpinBox->value();
}

void AddPropertyDialog::onAccept()
{
    int propertyId = getSelectedPropertyId();
    if (propertyId < 0) {
        QMessageBox::warning(this, tr("Add Property"), 
                           tr("Please select a property to add."));
        return;
    }
    
    accept();
}

void AddPropertyDialog::onCancel()
{
    reject();
}