#ifndef ADDPROPERTYDIALOG_H
#define ADDPROPERTYDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

class EnhancedPropertyAdditionEngine;

class AddPropertyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddPropertyDialog(QWidget *parent = nullptr);
    
    int getSelectedPropertyId() const;
    int getDefaultValue() const;
    quint32 getDefaultParameter() const;
    
    // Enhanced functionality
    void setPropertyEngine(EnhancedPropertyAdditionEngine *engine);

private slots:
    void onPropertySelectionChanged();
    void onAccept();
    void onCancel();

private:
    void setupUI();
    void populatePropertyCombo();
    void updatePropertyInfo();
    void updateValueRange();
    void updateParameterVisibility();
    
    QComboBox *_propertyCombo;
    QSpinBox *_valueSpinBox;
    QSpinBox *_parameterSpinBox;
    QLabel *_parameterLabel;
    QLabel *_propertyInfoLabel;
    QLabel *_valueRangeLabel;
    QPushButton *_okButton;
    QPushButton *_cancelButton;
    
    // Enhanced property engine for validation and info
    EnhancedPropertyAdditionEngine *_propertyEngine;
};

#endif // ADDPROPERTYDIALOG_H