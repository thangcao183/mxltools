/********************************************************************************
** Form generated from reading UI file 'allstatsdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ALLSTATSDIALOG_H
#define UI_ALLSTATSDIALOG_H

#include <QtCore/QVariant>
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
            AllStatsDialog->setObjectName(QString::fromUtf8("AllStatsDialog"));
        AllStatsDialog->resize(361, 340);
        verticalLayout = new QVBoxLayout(AllStatsDialog);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        textEdit = new QTextEdit(AllStatsDialog);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));
        textEdit->setReadOnly(true);

        verticalLayout->addWidget(textEdit);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(AllStatsDialog);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        lineEdit = new QLineEdit(AllStatsDialog);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

        horizontalLayout->addWidget(lineEdit);


        verticalLayout->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(AllStatsDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Close);

        verticalLayout->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        label->setBuddy(lineEdit);
#endif // QT_CONFIG(shortcut)

        retranslateUi(AllStatsDialog);
        QObject::connect(buttonBox, SIGNAL(rejected()), AllStatsDialog, SLOT(reject()));

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
