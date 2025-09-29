/********************************************************************************
** Form generated from reading UI file 'allstatsdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ALLSTATSDIALOG_H
#define UI_ALLSTATSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AllStatsDialog
{
public:
    QVBoxLayout *verticalLayout;
    QTextEdit *textEdit;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *AllStatsDialog)
    {
        if (AllStatsDialog->objectName().isEmpty())
            AllStatsDialog->setObjectName("AllStatsDialog");
        AllStatsDialog->resize(361, 340);
        verticalLayout = new QVBoxLayout(AllStatsDialog);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName("verticalLayout");
        textEdit = new QTextEdit(AllStatsDialog);
        textEdit->setObjectName("textEdit");
        textEdit->setReadOnly(true);

        verticalLayout->addWidget(textEdit);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName("horizontalLayout");
        label = new QLabel(AllStatsDialog);
        label->setObjectName("label");

        horizontalLayout->addWidget(label);

        lineEdit = new QLineEdit(AllStatsDialog);
        lineEdit->setObjectName("lineEdit");

        horizontalLayout->addWidget(lineEdit);


        verticalLayout->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(AllStatsDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setStandardButtons(QDialogButtonBox::Close);

        verticalLayout->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        label->setBuddy(lineEdit);
#endif // QT_CONFIG(shortcut)

        retranslateUi(AllStatsDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, AllStatsDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(AllStatsDialog);
    } // setupUi

    void retranslateUi(QDialog *AllStatsDialog)
    {
        AllStatsDialog->setWindowTitle(QCoreApplication::translate("AllStatsDialog", "All character stats", nullptr));
        label->setText(QCoreApplication::translate("AllStatsDialog", "Find", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AllStatsDialog: public Ui_AllStatsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ALLSTATSDIALOG_H
