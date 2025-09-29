/********************************************************************************
** Form generated from reading UI file 'finditemsdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FINDITEMSDIALOG_H
#define UI_FINDITEMSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_FindItemsDialog
{
public:
    QComboBox *searchComboBox;
    QCheckBox *regexCheckBox;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *minimalMatchCheckBox;
    QCheckBox *wrapAroundCheckBox;
    QPushButton *previousButton;
    QPushButton *searchResultsButton;
    QPushButton *nextButton;
    QCheckBox *searchPropsCheckBox;
    QCheckBox *multilineMatchCheckBox;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *FindItemsDialog)
    {
        if (FindItemsDialog->objectName().isEmpty())
            FindItemsDialog->setObjectName("FindItemsDialog");
        FindItemsDialog->resize(347, 176);
        searchComboBox = new QComboBox(FindItemsDialog);
        searchComboBox->setObjectName("searchComboBox");
        searchComboBox->setGeometry(QRect(20, 20, 221, 22));
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(searchComboBox->sizePolicy().hasHeightForWidth());
        searchComboBox->setSizePolicy(sizePolicy);
        searchComboBox->setEditable(true);
        searchComboBox->setInsertPolicy(QComboBox::InsertAtTop);
        regexCheckBox = new QCheckBox(FindItemsDialog);
        regexCheckBox->setObjectName("regexCheckBox");
        regexCheckBox->setGeometry(QRect(140, 100, 115, 17));
        caseSensitiveCheckBox = new QCheckBox(FindItemsDialog);
        caseSensitiveCheckBox->setObjectName("caseSensitiveCheckBox");
        caseSensitiveCheckBox->setGeometry(QRect(21, 100, 92, 17));
        minimalMatchCheckBox = new QCheckBox(FindItemsDialog);
        minimalMatchCheckBox->setObjectName("minimalMatchCheckBox");
        minimalMatchCheckBox->setEnabled(false);
        minimalMatchCheckBox->setGeometry(QRect(140, 120, 103, 17));
        wrapAroundCheckBox = new QCheckBox(FindItemsDialog);
        wrapAroundCheckBox->setObjectName("wrapAroundCheckBox");
        wrapAroundCheckBox->setGeometry(QRect(20, 140, 86, 17));
        wrapAroundCheckBox->setChecked(true);
        previousButton = new QPushButton(FindItemsDialog);
        previousButton->setObjectName("previousButton");
        previousButton->setEnabled(false);
        previousButton->setGeometry(QRect(261, 40, 75, 23));
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(previousButton->sizePolicy().hasHeightForWidth());
        previousButton->setSizePolicy(sizePolicy1);
        searchResultsButton = new QPushButton(FindItemsDialog);
        searchResultsButton->setObjectName("searchResultsButton");
        searchResultsButton->setEnabled(false);
        searchResultsButton->setGeometry(QRect(261, 69, 76, 23));
        sizePolicy1.setHeightForWidth(searchResultsButton->sizePolicy().hasHeightForWidth());
        searchResultsButton->setSizePolicy(sizePolicy1);
        nextButton = new QPushButton(FindItemsDialog);
        nextButton->setObjectName("nextButton");
        nextButton->setEnabled(false);
        nextButton->setGeometry(QRect(261, 11, 75, 23));
        sizePolicy1.setHeightForWidth(nextButton->sizePolicy().hasHeightForWidth());
        nextButton->setSizePolicy(sizePolicy1);
        nextButton->setFlat(false);
        searchPropsCheckBox = new QCheckBox(FindItemsDialog);
        searchPropsCheckBox->setObjectName("searchPropsCheckBox");
        searchPropsCheckBox->setGeometry(QRect(20, 120, 119, 17));
        searchPropsCheckBox->setChecked(false);
        multilineMatchCheckBox = new QCheckBox(FindItemsDialog);
        multilineMatchCheckBox->setObjectName("multilineMatchCheckBox");
        multilineMatchCheckBox->setEnabled(false);
        multilineMatchCheckBox->setGeometry(QRect(140, 140, 107, 17));
        buttonBox = new QDialogButtonBox(FindItemsDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setGeometry(QRect(260, 110, 75, 52));
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::Help);

        retranslateUi(FindItemsDialog);
        QObject::connect(regexCheckBox, &QCheckBox::toggled, multilineMatchCheckBox, &QCheckBox::setEnabled);
        QObject::connect(regexCheckBox, &QCheckBox::toggled, minimalMatchCheckBox, &QCheckBox::setEnabled);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, FindItemsDialog, qOverload<>(&QDialog::reject));

        nextButton->setDefault(true);


        QMetaObject::connectSlotsByName(FindItemsDialog);
    } // setupUi

    void retranslateUi(QDialog *FindItemsDialog)
    {
        regexCheckBox->setText(QCoreApplication::translate("FindItemsDialog", "Regular expression", nullptr));
        caseSensitiveCheckBox->setText(QCoreApplication::translate("FindItemsDialog", "Case sensitive", nullptr));
#if QT_CONFIG(tooltip)
        minimalMatchCheckBox->setToolTip(QCoreApplication::translate("FindItemsDialog", "All greedy quantifiers become non-greedy, for example .+ and .* transform in .+? and .*? respectively", nullptr));
#endif // QT_CONFIG(tooltip)
        minimalMatchCheckBox->setText(QCoreApplication::translate("FindItemsDialog", "Minimal matching", nullptr));
        wrapAroundCheckBox->setText(QCoreApplication::translate("FindItemsDialog", "Wrap around", nullptr));
        previousButton->setText(QCoreApplication::translate("FindItemsDialog", "Find previous", nullptr));
        nextButton->setText(QCoreApplication::translate("FindItemsDialog", "Find next", nullptr));
        searchPropsCheckBox->setText(QCoreApplication::translate("FindItemsDialog", "Search in properties", nullptr));
#if QT_CONFIG(tooltip)
        multilineMatchCheckBox->setToolTip(QCoreApplication::translate("FindItemsDialog", "Treat string as single line (emulation of Perl's /s option)", nullptr));
#endif // QT_CONFIG(tooltip)
        multilineMatchCheckBox->setText(QCoreApplication::translate("FindItemsDialog", "Multiline matching", nullptr));
        (void)FindItemsDialog;
    } // retranslateUi

};

namespace Ui {
    class FindItemsDialog: public Ui_FindItemsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FINDITEMSDIALOG_H
