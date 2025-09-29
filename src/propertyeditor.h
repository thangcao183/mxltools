#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include "structs.h"
#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QCheckBox>

class ItemInfo;
class QTabWidget;

class PropertyEditor : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyEditor(QWidget *parent = nullptr);
    virtual ~PropertyEditor();

    void setItem(ItemInfo *item);
    void clear();
    
    bool hasChanges() const;
    void applyChanges();
    void revertChanges();

signals:
    void itemChanged();
    void propertyModified();

private slots:
    void addProperty();
    void removeProperty();
    void onPropertyChanged();
    void onValueChanged();
    void onParameterChanged();
    void validateProperty();

private:
    struct PropertyEditorRow {
        QComboBox *propertyCombo;
        QSpinBox *valueSpinBox;
        QSpinBox *parameterSpinBox;
        QLabel *parameterLabel;
        QPushButton *removeButton;
        QLabel *warningLabel;
        QLabel *typeLabel; // New: shows "Item" or "RW" to indicate property type
        int originalPropertyId;
        int originalValue;
        quint32 originalParameter;
        bool isNew;
        bool isRunewordProperty; // New: track if this is a runeword property
    };

    void setupUI();
    void populatePropertyCombo(QComboBox *combo);
    void addPropertyRow(int propertyId = -1, int value = 0, quint32 parameter = 0, bool isNew = true, bool isRunewordProperty = false);
    void removePropertyRow(PropertyEditorRow *row);
    void updatePropertyRow(PropertyEditorRow *row);
    void validatePropertyValue(PropertyEditorRow *row);
    void updateParameterVisibility(PropertyEditorRow *row);
    void updateButtonStates();
    
    QString getPropertyDisplayName(int propertyId) const;
    QPair<int, int> getValueRange(int propertyId) const;
    QPair<quint32, quint32> getParameterRange(int propertyId) const;
    QPair<int, int> getSafeValueRange(int propertyId) const;
    bool validateValueOverflow(int propertyId, int value) const;
    bool validateParameterOverflow(int propertyId, quint32 parameter) const;
    int getDisplayValueForProperty(int propertyId, const ItemProperty *property) const;
    int getStorageValueFromDisplay(int propertyId, int displayValue) const;
    bool isPropertyAllowedMultipleTimes(int propertyId) const;
    
    void backupOriginalProperties();
    void applyPropertyChanges();
    
    ItemInfo *_item;
    QList<PropertyEditorRow*> _propertyRows;
    PropertiesMultiMap _originalProperties;
    PropertiesMultiMap _originalRwProperties;
    
    // UI Components
    QVBoxLayout *_mainLayout;
    QScrollArea *_scrollArea;
    QWidget *_scrollWidget;
    QVBoxLayout *_scrollLayout;
    QPushButton *_addButton;
    QPushButton *_applyButton;
    QPushButton *_revertButton;
    QLabel *_statusLabel;
    QCheckBox *_showAllPropertiesCheck;
    
    bool _hasChanges;
    bool _updatingUI;
};

#endif // PROPERTYEDITOR_H