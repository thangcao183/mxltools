/********************************************************************************
** Form generated from reading UI file 'stashsortingoptionsdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STASHSORTINGOPTIONSDIALOG_H
#define UI_STASHSORTINGOPTIONSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_StashSortingOptionsDialog
{
public:
    QDialogButtonBox *buttonBox;
    QGroupBox *itemQualityOrderingGroupBox;
    QRadioButton *ascQualityRadioButton;
    QRadioButton *descQualityRadioButton;
    QGroupBox *pageRangeGroupBox;
    QDoubleSpinBox *firstPageSpinBox;
    QLabel *firstPageLabel;
    QDoubleSpinBox *lastPageSpinBox;
    QLabel *lastPageLabel;
    QGroupBox *blankPagesGroupBox;
    QLabel *diffQualitiesLabel;
    QLabel *diffTypesLabel;
    QSpinBox *diffQualitiesSpinBox;
    QSpinBox *diffTypesSpinBox;
    QGroupBox *newRowGroupBox;
    QCheckBox *newRowTierCheckBox;
    QCheckBox *newRowVisuallyDifferentMiscCheckBox;
    QCheckBox *newRowCotwCheckBox;
    QGroupBox *separationBox;
    QCheckBox *separateEthCheckBox;
    QCheckBox *similarMiscItemsOnOnePageCheckBox;
    QCheckBox *eachTypeFromNewPageCheckBox;
    QCheckBox *separateSacredCheckBox;
    QButtonGroup *buttonGroup;

    void setupUi(QDialog *StashSortingOptionsDialog)
    {
        if (StashSortingOptionsDialog->objectName().isEmpty())
            StashSortingOptionsDialog->setObjectName("StashSortingOptionsDialog");
        StashSortingOptionsDialog->resize(615, 241);
        buttonBox = new QDialogButtonBox(StashSortingOptionsDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setGeometry(QRect(450, 210, 156, 23));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Help);
        itemQualityOrderingGroupBox = new QGroupBox(StashSortingOptionsDialog);
        itemQualityOrderingGroupBox->setObjectName("itemQualityOrderingGroupBox");
        itemQualityOrderingGroupBox->setGeometry(QRect(10, 10, 121, 91));
        ascQualityRadioButton = new QRadioButton(itemQualityOrderingGroupBox);
        buttonGroup = new QButtonGroup(StashSortingOptionsDialog);
        buttonGroup->setObjectName("buttonGroup");
        buttonGroup->addButton(ascQualityRadioButton);
        ascQualityRadioButton->setObjectName("ascQualityRadioButton");
        ascQualityRadioButton->setGeometry(QRect(10, 60, 91, 20));
        descQualityRadioButton = new QRadioButton(itemQualityOrderingGroupBox);
        buttonGroup->addButton(descQualityRadioButton);
        descQualityRadioButton->setObjectName("descQualityRadioButton");
        descQualityRadioButton->setGeometry(QRect(10, 30, 99, 20));
        descQualityRadioButton->setChecked(true);
        pageRangeGroupBox = new QGroupBox(StashSortingOptionsDialog);
        pageRangeGroupBox->setObjectName("pageRangeGroupBox");
        pageRangeGroupBox->setGeometry(QRect(220, 10, 101, 91));
        firstPageSpinBox = new QDoubleSpinBox(pageRangeGroupBox);
        firstPageSpinBox->setObjectName("firstPageSpinBox");
        firstPageSpinBox->setGeometry(QRect(50, 30, 42, 22));
        firstPageSpinBox->setDecimals(0);
        firstPageSpinBox->setMinimum(1.000000000000000);
        firstPageLabel = new QLabel(pageRangeGroupBox);
        firstPageLabel->setObjectName("firstPageLabel");
        firstPageLabel->setGeometry(QRect(10, 30, 32, 16));
        lastPageSpinBox = new QDoubleSpinBox(pageRangeGroupBox);
        lastPageSpinBox->setObjectName("lastPageSpinBox");
        lastPageSpinBox->setGeometry(QRect(50, 60, 42, 22));
        lastPageSpinBox->setDecimals(0);
        lastPageSpinBox->setMinimum(1.000000000000000);
        lastPageLabel = new QLabel(pageRangeGroupBox);
        lastPageLabel->setObjectName("lastPageLabel");
        lastPageLabel->setGeometry(QRect(10, 60, 30, 16));
        blankPagesGroupBox = new QGroupBox(StashSortingOptionsDialog);
        blankPagesGroupBox->setObjectName("blankPagesGroupBox");
        blankPagesGroupBox->setGeometry(QRect(410, 10, 191, 91));
        diffQualitiesLabel = new QLabel(blankPagesGroupBox);
        diffQualitiesLabel->setObjectName("diffQualitiesLabel");
        diffQualitiesLabel->setGeometry(QRect(10, 30, 118, 16));
        diffTypesLabel = new QLabel(blankPagesGroupBox);
        diffTypesLabel->setObjectName("diffTypesLabel");
        diffTypesLabel->setGeometry(QRect(10, 60, 98, 16));
        diffQualitiesSpinBox = new QSpinBox(blankPagesGroupBox);
        diffQualitiesSpinBox->setObjectName("diffQualitiesSpinBox");
        diffQualitiesSpinBox->setGeometry(QRect(140, 30, 42, 22));
        diffQualitiesSpinBox->setMaximum(100);
        diffTypesSpinBox = new QSpinBox(blankPagesGroupBox);
        diffTypesSpinBox->setObjectName("diffTypesSpinBox");
        diffTypesSpinBox->setGeometry(QRect(140, 60, 42, 22));
        diffTypesSpinBox->setMaximum(100);
        newRowGroupBox = new QGroupBox(StashSortingOptionsDialog);
        newRowGroupBox->setObjectName("newRowGroupBox");
        newRowGroupBox->setGeometry(QRect(310, 110, 291, 91));
        newRowTierCheckBox = new QCheckBox(newRowGroupBox);
        newRowTierCheckBox->setObjectName("newRowTierCheckBox");
        newRowTierCheckBox->setGeometry(QRect(10, 20, 80, 20));
        newRowTierCheckBox->setChecked(true);
        newRowVisuallyDifferentMiscCheckBox = new QCheckBox(newRowGroupBox);
        newRowVisuallyDifferentMiscCheckBox->setObjectName("newRowVisuallyDifferentMiscCheckBox");
        newRowVisuallyDifferentMiscCheckBox->setGeometry(QRect(10, 60, 280, 20));
        newRowCotwCheckBox = new QCheckBox(newRowGroupBox);
        newRowCotwCheckBox->setObjectName("newRowCotwCheckBox");
        newRowCotwCheckBox->setGeometry(QRect(10, 40, 247, 20));
        separationBox = new QGroupBox(StashSortingOptionsDialog);
        separationBox->setObjectName("separationBox");
        separationBox->setGeometry(QRect(10, 110, 291, 111));
        separateEthCheckBox = new QCheckBox(separationBox);
        separateEthCheckBox->setObjectName("separateEthCheckBox");
        separateEthCheckBox->setGeometry(QRect(10, 80, 173, 20));
        separateEthCheckBox->setChecked(true);
        similarMiscItemsOnOnePageCheckBox = new QCheckBox(separationBox);
        similarMiscItemsOnOnePageCheckBox->setObjectName("similarMiscItemsOnOnePageCheckBox");
        similarMiscItemsOnOnePageCheckBox->setGeometry(QRect(30, 40, 263, 20));
        similarMiscItemsOnOnePageCheckBox->setChecked(true);
        eachTypeFromNewPageCheckBox = new QCheckBox(separationBox);
        eachTypeFromNewPageCheckBox->setObjectName("eachTypeFromNewPageCheckBox");
        eachTypeFromNewPageCheckBox->setGeometry(QRect(10, 20, 250, 20));
        eachTypeFromNewPageCheckBox->setChecked(true);
        separateSacredCheckBox = new QCheckBox(separationBox);
        separateSacredCheckBox->setObjectName("separateSacredCheckBox");
        separateSacredCheckBox->setGeometry(QRect(10, 60, 171, 17));
        separateSacredCheckBox->setChecked(true);
#if QT_CONFIG(shortcut)
        firstPageLabel->setBuddy(firstPageSpinBox);
        lastPageLabel->setBuddy(lastPageSpinBox);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(descQualityRadioButton, ascQualityRadioButton);
        QWidget::setTabOrder(ascQualityRadioButton, firstPageSpinBox);
        QWidget::setTabOrder(firstPageSpinBox, lastPageSpinBox);
        QWidget::setTabOrder(lastPageSpinBox, diffQualitiesSpinBox);
        QWidget::setTabOrder(diffQualitiesSpinBox, diffTypesSpinBox);
        QWidget::setTabOrder(diffTypesSpinBox, similarMiscItemsOnOnePageCheckBox);
        QWidget::setTabOrder(similarMiscItemsOnOnePageCheckBox, newRowTierCheckBox);
        QWidget::setTabOrder(newRowTierCheckBox, newRowCotwCheckBox);
        QWidget::setTabOrder(newRowCotwCheckBox, newRowVisuallyDifferentMiscCheckBox);
        QWidget::setTabOrder(newRowVisuallyDifferentMiscCheckBox, buttonBox);

        retranslateUi(StashSortingOptionsDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, StashSortingOptionsDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, StashSortingOptionsDialog, qOverload<>(&QDialog::reject));
        QObject::connect(eachTypeFromNewPageCheckBox, &QCheckBox::toggled, diffTypesSpinBox, &QSpinBox::setEnabled);
        QObject::connect(eachTypeFromNewPageCheckBox, &QCheckBox::toggled, diffTypesLabel, &QLabel::setEnabled);

        QMetaObject::connectSlotsByName(StashSortingOptionsDialog);
    } // setupUi

    void retranslateUi(QDialog *StashSortingOptionsDialog)
    {
        StashSortingOptionsDialog->setWindowTitle(QCoreApplication::translate("StashSortingOptionsDialog", "Sort options", nullptr));
        itemQualityOrderingGroupBox->setTitle(QCoreApplication::translate("StashSortingOptionsDialog", "Item quality ordering", nullptr));
        ascQualityRadioButton->setText(QCoreApplication::translate("StashSortingOptionsDialog", "Ascending", nullptr));
        descQualityRadioButton->setText(QCoreApplication::translate("StashSortingOptionsDialog", "Descending", nullptr));
        pageRangeGroupBox->setTitle(QCoreApplication::translate("StashSortingOptionsDialog", "Page range", nullptr));
        firstPageLabel->setText(QCoreApplication::translate("StashSortingOptionsDialog", "First:", nullptr));
        lastPageLabel->setText(QCoreApplication::translate("StashSortingOptionsDialog", "Last:", nullptr));
        blankPagesGroupBox->setTitle(QCoreApplication::translate("StashSortingOptionsDialog", "Blank pages between:", nullptr));
        diffQualitiesLabel->setText(QCoreApplication::translate("StashSortingOptionsDialog", "Different qualities:", nullptr));
        diffTypesLabel->setText(QCoreApplication::translate("StashSortingOptionsDialog", "Different types:", nullptr));
        newRowGroupBox->setTitle(QCoreApplication::translate("StashSortingOptionsDialog", "New row for:", nullptr));
        newRowTierCheckBox->setText(QCoreApplication::translate("StashSortingOptionsDialog", "Each tier", nullptr));
        newRowVisuallyDifferentMiscCheckBox->setText(QCoreApplication::translate("StashSortingOptionsDialog", "Similar but visually different 'misc' items", nullptr));
        newRowCotwCheckBox->setText(QCoreApplication::translate("StashSortingOptionsDialog", "Each Cornerstone of the World skill", nullptr));
        separationBox->setTitle(QCoreApplication::translate("StashSortingOptionsDialog", "Separation", nullptr));
        separateEthCheckBox->setText(QCoreApplication::translate("StashSortingOptionsDialog", "Separate ethereal items", nullptr));
        similarMiscItemsOnOnePageCheckBox->setText(QCoreApplication::translate("StashSortingOptionsDialog", "Place similar 'misc' items on one page", nullptr));
        eachTypeFromNewPageCheckBox->setText(QCoreApplication::translate("StashSortingOptionsDialog", "Start each item type from new page", nullptr));
        separateSacredCheckBox->setText(QCoreApplication::translate("StashSortingOptionsDialog", "Separate sacred items", nullptr));
    } // retranslateUi

};

namespace Ui {
    class StashSortingOptionsDialog: public Ui_StashSortingOptionsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STASHSORTINGOPTIONSDIALOG_H
