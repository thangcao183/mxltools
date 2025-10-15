#ifndef RUNECREATIONWIDGET_H
#define RUNECREATIONWIDGET_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

// Forward declarations
class ItemInfo;

/**
 * @brief Simple widget for creating runes
 */
class RuneCreationWidget : public QDialog
{
    Q_OBJECT

public:
    explicit RuneCreationWidget(QWidget *parent = nullptr);
    ~RuneCreationWidget();
    
    /**
     * @brief Get the created rune item
     * @return Created ItemInfo or nullptr if cancelled
     */
    ItemInfo* getCreatedRune() const { return _createdRune; }
    
    /**
     * @brief Set the position where the rune should be created
     */
    void setItemPosition(int row, int column);

private slots:
    void onCreateClicked();
    void onCancelClicked();
    void onRuneSelectionChanged();

private:
    void setupUI();
    void populateRuneList();
    ItemInfo* createRuneItem(const QString &runeCode);
    
    // UI components
    QComboBox *_runeCombo;
    QSpinBox *_rowSpin;
    QSpinBox *_columnSpin;
    QSpinBox *_copiesSpin;
    QLabel *_previewLabel;
    QPushButton *_createButton;
    QPushButton *_cancelButton;
    
    // Result
    ItemInfo *_createdRune;
    
    // Position
    int _targetRow;
    int _targetColumn;
    int _copies;

public:
    int copies() const { return _copies; }
    int requestedRow() const { return _targetRow; }
    int requestedColumn() const { return _targetColumn; }
};

#endif // RUNECREATIONWIDGET_H
