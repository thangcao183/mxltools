#ifndef OILCREATIONWIDGET_H
#define OILCREATIONWIDGET_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>

class ItemInfo;

class OilCreationWidget : public QDialog
{
    Q_OBJECT

public:
    explicit OilCreationWidget(QWidget *parent = nullptr);
    ~OilCreationWidget();

    ItemInfo* getCreatedOil() const { return _createdOil; }
    void setItemPosition(int row, int column);
    QString selectedCode() const;

private slots:
    void onCreateClicked();
    void onCancelClicked();
    void onSelectionChanged();

private:
    void setupUI();
    void populateOilList();
    ItemInfo* createOilItem(const QString &code);

    QComboBox *_oilCombo;
    QSpinBox *_rowSpin;
    QSpinBox *_colSpin;
    QSpinBox *_copiesSpin;
    QLabel *_previewLabel;
    QPushButton *_createButton;
    QPushButton *_cancelButton;

    ItemInfo *_createdOil;
    int _targetRow;
    int _targetCol;
    int _copies;

public:
    int copies() const { return _copies; }
    int requestedRow() const { return _targetRow; }
    int requestedColumn() const { return _targetCol; }
};

#endif // OILCREATIONWIDGET_H
