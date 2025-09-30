#ifndef GEMCREATIONWIDGET_H
#define GEMCREATIONWIDGET_H

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
 * @brief Simple widget for creating gems
 */
class GemCreationWidget : public QDialog
{
    Q_OBJECT

public:
    explicit GemCreationWidget(QWidget *parent = nullptr);
    ~GemCreationWidget();
    
    /**
     * @brief Get the created gem item
     * @return Created ItemInfo or nullptr if cancelled
     */
    ItemInfo* getCreatedGem() const { return _createdGem; }
    
    /**
     * @brief Set the position where the gem should be created
     */
    void setItemPosition(int row, int column);

private slots:
    void onCreateClicked();
    void onCancelClicked();
    void onGemSelectionChanged();

private:
    void setupUI();
    void populateGemList();
    ItemInfo* createGemItem(const QString &gemCode);
    ItemInfo* createBasicGem(const QString &gemCode);
    QString getGemFileForCode(const QString &gemCode);
    QString getGemProperties(const QString &gemCode);
    
    // UI components
    QComboBox *_gemCombo;
    QSpinBox *_rowSpin;
    QSpinBox *_columnSpin;
    QPushButton *_createButton;
    QPushButton *_cancelButton;
    QLabel *_previewLabel;
    QTextEdit *_propertiesText;
    
    // Data
    ItemInfo *_createdGem;
    int _targetRow;
    int _targetColumn;
};

#endif // GEMCREATIONWIDGET_H