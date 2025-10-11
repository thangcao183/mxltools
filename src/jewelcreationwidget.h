#ifndef JEWELCREATIONWIDGET_H
#define JEWELCREATIONWIDGET_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTextEdit>

// Forward declarations
class ItemInfo;

/**
 * @brief Simple widget for creating jewels (amulets, rings, charms)
 */
class JewelCreationWidget : public QDialog
{
    Q_OBJECT

public:
    explicit JewelCreationWidget(QWidget *parent = nullptr);
    ~JewelCreationWidget();
    
    /**
     * @brief Get the created jewel item
     * @return Created ItemInfo or nullptr if cancelled
     */
    ItemInfo* getCreatedJewel() const { return _createdJewel; }
    
    /**
     * @brief Set the position where the jewel should be created
     */
    void setItemPosition(int row, int column);

private slots:
    void onCreateClicked();
    void onCancelClicked();
    void onJewelSelectionChanged();
    void onJewelTypeChanged();

private:
    void setupUI();
    void populateJewelTypes();
    void populateJewelList();
    ItemInfo* createJewelItem(const QString &jewelCode);
    ItemInfo* createBasicJewel(const QString &jewelCode);
    QString getJewelFileForCode(const QString &jewelCode);
    QString getJewelProperties(const QString &jewelCode);
    
    // UI components
    QComboBox *_jewelTypeCombo;
    QComboBox *_jewelCombo;
    QSpinBox *_rowSpin;
    QSpinBox *_columnSpin;
    QPushButton *_createButton;
    QPushButton *_cancelButton;
    QLabel *_previewLabel;
    QTextEdit *_propertiesText;
    
    // Data
    ItemInfo *_createdJewel;
    int _targetRow;
    int _targetColumn;
};

#endif // JEWELCREATIONWIDGET_H
