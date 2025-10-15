#ifndef SPECIALITEMCREATIONWIDGET_H
#define SPECIALITEMCREATIONWIDGET_H

#include <QDialog>
class QComboBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QLabel;
class QListWidget;

class ItemInfo;

class SpecialItemCreationWidget : public QDialog
{
    Q_OBJECT
public:
    explicit SpecialItemCreationWidget(QWidget *parent = nullptr);
    ~SpecialItemCreationWidget();

    void setItemPosition(int row, int column);
    ItemInfo *getCreatedItem() const { return _createdItem; }

private slots:
    void onSelectionChanged();
    void onCreateClicked();
    void onCancelClicked();

private:
    void setupUI();
    void populateItemList();

    QListWidget *_list;
    QSpinBox *_rowSpin, *_colSpin;
    QSpinBox *_propIdSpin, *_propValueSpin;
    QPushButton *_createButton, *_cancelButton;
    QLabel *_previewLabel;

    ItemInfo *_createdItem;
    int _targetRow, _targetCol;
};

#endif // SPECIALITEMCREATIONWIDGET_H
