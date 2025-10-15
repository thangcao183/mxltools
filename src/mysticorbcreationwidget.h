#ifndef MYSTICORBCREATIONWIDGET_H
#define MYSTICORBCREATIONWIDGET_H

#include <QDialog>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

class ItemInfo;

class MysticOrbCreationWidget : public QDialog
{
    Q_OBJECT

public:
    explicit MysticOrbCreationWidget(QWidget *parent = nullptr);
    ~MysticOrbCreationWidget();

    ItemInfo* getCreatedOrb() const { return _createdOrb; }
    int copies() const { return _copies; }
    int requestedRow() const { return _targetRow; }
    int requestedColumn() const { return _targetCol; }

    void setItemPosition(int row, int column);

private slots:
    void onCreateClicked();
    void onCancelClicked();

private:
    void setupUI();
    ItemInfo* createMysticOrbItem();

    QSpinBox *_rowSpin;
    QSpinBox *_colSpin;
    QSpinBox *_copiesSpin;
    QComboBox *_orbCombo;
    QLabel *_previewLabel;
    QPushButton *_createButton;
    QPushButton *_cancelButton;

    ItemInfo *_createdOrb;
    int _targetRow;
    int _targetCol;
    int _copies;
};

#endif // MYSTICORBCREATIONWIDGET_H
