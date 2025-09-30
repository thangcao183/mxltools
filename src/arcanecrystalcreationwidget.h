#ifndef ARCANECRYSTALCREATIONWIDGET_H
#define ARCANECRYSTALCREATIONWIDGET_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QComboBox;
class QSpinBox;
class QPushButton;
class QLabel;
class QTextEdit;
QT_END_NAMESPACE

class ItemInfo;

/**
 * @brief Dialog for creating Arcane Crystals with quantity selection
 * 
 * This widget allows users to create Arcane Crystals at specified inventory positions
 * with the ability to choose the quantity to create.
 */
class ArcaneCrystalCreationWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ArcaneCrystalCreationWidget(QWidget *parent = nullptr);
    ~ArcaneCrystalCreationWidget();
    
    /**
     * @brief Get the list of created crystal items
     * @return List of created ItemInfo objects or empty list if cancelled
     */
    QList<ItemInfo*> getCreatedCrystals() const { return _createdCrystals; }
    
    /**
     * @brief Set the starting position where crystals should be created
     */
    void setStartPosition(int row, int column);

private slots:
    void onCreateClicked();
    void onCancelClicked();
    void onQuantityChanged();

private:
    void setupUI();
    ItemInfo* createArcaneCrystalItem();
    void updatePreview();
    
    // UI components
    QSpinBox *_quantitySpinBox;
    QSpinBox *_rowSpin;
    QSpinBox *_columnSpin;
    QPushButton *_createButton;
    QPushButton *_cancelButton;
    QLabel *_previewLabel;
    QTextEdit *_descriptionText;
    
    // Data
    QList<ItemInfo*> _createdCrystals;
    int _startRow;
    int _startColumn;
};

#endif // ARCANECRYSTALCREATIONWIDGET_H