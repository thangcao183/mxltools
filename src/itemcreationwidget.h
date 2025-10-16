#ifndef ItemCREATIONWIDGET_H
#define ItemCREATIONWIDGET_H

#include <QDialog>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>

class ItemInfo;

class ItemCreationWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ItemCreationWidget(QWidget *parent = nullptr);
    ~ItemCreationWidget();

    ItemInfo* getCreatedOrb() const { return _createdOrb; }
    int copies() const { return _copies; }
    int requestedRow() const { return _targetRow; }
    int requestedColumn() const { return _targetCol; }

    void setItemPosition(int row, int column);

private slots:
    void onCreateClicked();
    void onCancelClicked();
    void _onFilterTextChanged(const QString &text);

private:
    void setupUI();
    ItemInfo* createItemItem();

    QSpinBox *_rowSpin;
    QSpinBox *_colSpin;
    QSpinBox *_copiesSpin;
    QComboBox *_itemCombo;
    QLineEdit *_filterEdit;
    QStringList _allItems; // stored as "display\tdata" or "code\tcode"
    QLabel *_previewLabel;
    QPushButton *_createButton;
    QPushButton *_cancelButton;

    ItemInfo *_createdOrb;
    int _targetRow;
    int _targetCol;
    int _copies;
};

#endif // ItemCREATIONWIDGET_H
