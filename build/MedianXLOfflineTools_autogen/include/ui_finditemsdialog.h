/********************************************************************************
** Form generated from reading UI file 'finditemsdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.17
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FINDITEMSDIALOG_H
#define UI_FINDITEMSDIALOG_H

#include <QtCore/QVariant>
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
            FindItemsDialog->setObjectName(QString::fromUtf8("FindItemsDialog"));
        FindItemsDialog->resize(347, 176);
        searchComboBox = new QComboBox(FindItemsDialog);
        searchComboBox->setObjectName(QString::fromUtf8("searchComboBox"));
        searchComboBox->setGeometry(QRect(20, 20, 221, 22));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(searchComboBox->sizePolicy().hasHeightForWidth());
        searchComboBox->setSizePolicy(sizePolicy);
        searchComboBox->setEditable(true);
        searchComboBox->setInsertPolicy(QComboBox::InsertAtTop);
        regexCheckBox = new QCheckBox(FindItemsDialog);
        regexCheckBox->setObjectName(QString::fromUtf8("regexCheckBox"));
        regexCheckBox->setGeometry(QRect(140, 100, 115, 17));
        caseSensitiveCheckBox = new QCheckBox(FindItemsDialog);
        caseSensitiveCheckBox->setObjectName(QString::fromUtf8("caseSensitiveCheckBox"));
        caseSensitiveCheckBox->setGeometry(QRect(21, 100, 92, 17));
        minimalMatchCheckBox = new QCheckBox(FindItemsDialog);
        minimalMatchCheckBox->setObjectName(QString::fromUtf8("minimalMatchCheckBox"));
        minimalMatchCheckBox->setEnabled(false);
        minimalMatchCheckBox->setGeometry(QRect(140, 120, 103, 17));
        wrapAroundCheckBox = new QCheckBox(FindItemsDialog);
        wrapAroundCheckBox->setObjectName(QString::fromUtf8("wrapAroundCheckBox"));
        wrapAroundCheckBox->setGeometry(QRect(20, 140, 86, 17));
        wrapAroundCheckBox->setChecked(true);
        previousButton = new QPushButton(FindItemsDialog);
        previousButton->setObjectName(QString::fromUtf8("previousButton"));
        previousButton->setEnabled(false);
        previousButton->setGeometry(QRect(261, 40, 75, 23));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(previousButton->sizePolicy().hasHeightForWidth());
        previousButton->setSizePolicy(sizePolicy1);
        searchResultsButton = new QPushButton(FindItemsDialog);
        searchResultsButton->setObjectName(QString::fromUtf8("searchResultsButton"));
        searchResultsButton->setEnabled(false);
        searchResultsButton->setGeometry(QRect(261, 69, 76, 23));
        sizePolicy1.setHeightForWidth(searchResultsButton->sizePolicy().hasHeightForWidth());
        searchResultsButton->setSizePolicy(sizePolicy1);
        nextButton = new QPushButton(FindItemsDialog);
        nextButton->setObjectName(QString::fromUtf8("nextButton"));
        nextButton->setEnabled(false);
        nextButton->setGeometry(QRect(261, 11, 75, 23));
        sizePolicy1.setHeightForWidth(nextButton->sizePolicy().hasHeightForWidth());
        nextButton->setSizePolicy(sizePolicy1);
        nextButton->setFlat(false);
        searchPropsCheckBox = new QCheckBox(FindItemsDialog);
        searchPropsCheckBox->setObjectName(QString::fromUtf8("searchPropsCheckBox"));
        searchPropsCheckBox->setGeometry(QRect(20, 120, 119, 17));
        searchPropsCheckBox->setChecked(false);
        multilineMatchCheckBox = new QCheckBox(FindItemsDialog);
        multilineMatchCheckBox->setObjectName(QString::fromUtf8("multilineMatchCheckBox"));
        multilineMatchCheckBox->setEnabled(false);
        multilineMatchCheckBox->setGeometry(QRect(140, 140, 107, 17));
        buttonBox = new QDialogButtonBox(FindItemsDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(260, 110, 75, 52));
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Close|QDialogButtonBox::Help);

        retranslateUi(FindItemsDialog);
        QObject::connect(regexCheckBox, SIGNAL(toggled(bool)), multilineMatchCheckBox, SLOT(setEnabled(bool)));
        QObject::connect(regexCheckBox, SIGNAL(toggled(bool)), minimalMatchCheckBox, SLOT(setEnabled(bool)));
        QObject::connect(buttonBox, SIGNAL(rejected()), FindItemsDialog, SLOT(reject()));

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
