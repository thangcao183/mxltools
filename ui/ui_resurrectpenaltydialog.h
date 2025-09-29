/********************************************************************************
** Form generated from reading UI file 'resurrectpenaltydialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RESURRECTPENALTYDIALOG_H
#define UI_RESURRECTPENALTYDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ResurrectPenaltyDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QRadioButton *levelsRadioButton;
    QRadioButton *skillsRadioButton;
    QRadioButton *statsRadioButton;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ResurrectPenaltyDialog)
    {
        if (ResurrectPenaltyDialog->objectName().isEmpty())
            ResurrectPenaltyDialog->setObjectName("ResurrectPenaltyDialog");
        ResurrectPenaltyDialog->resize(254, 146);
        verticalLayout_2 = new QVBoxLayout(ResurrectPenaltyDialog);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setSizeConstraint(QLayout::SetFixedSize);
        groupBox = new QGroupBox(ResurrectPenaltyDialog);
        groupBox->setObjectName("groupBox");
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName("verticalLayout");
        levelsRadioButton = new QRadioButton(groupBox);
        levelsRadioButton->setObjectName("levelsRadioButton");
        levelsRadioButton->setChecked(true);

        verticalLayout->addWidget(levelsRadioButton);

        skillsRadioButton = new QRadioButton(groupBox);
        skillsRadioButton->setObjectName("skillsRadioButton");

        verticalLayout->addWidget(skillsRadioButton);

        statsRadioButton = new QRadioButton(groupBox);
        statsRadioButton->setObjectName("statsRadioButton");

        verticalLayout->addWidget(statsRadioButton);


        verticalLayout_2->addWidget(groupBox);

        buttonBox = new QDialogButtonBox(ResurrectPenaltyDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_2->addWidget(buttonBox);


        retranslateUi(ResurrectPenaltyDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, ResurrectPenaltyDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, ResurrectPenaltyDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(ResurrectPenaltyDialog);
    } // setupUi

    void retranslateUi(QDialog *ResurrectPenaltyDialog)
    {
        ResurrectPenaltyDialog->setWindowTitle(QCoreApplication::translate("ResurrectPenaltyDialog", "Resurrect", nullptr));
        groupBox->setTitle(QCoreApplication::translate("ResurrectPenaltyDialog", "Choose your resurrection penalty", nullptr));
        levelsRadioButton->setText(QCoreApplication::translate("ResurrectPenaltyDialog", "Pay with 10 levels", nullptr));
        skillsRadioButton->setText(QCoreApplication::translate("ResurrectPenaltyDialog", "Pay with 2.5% of skill points", nullptr));
        statsRadioButton->setText(QCoreApplication::translate("ResurrectPenaltyDialog", "Pay with 4% of stat points", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ResurrectPenaltyDialog: public Ui_ResurrectPenaltyDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RESURRECTPENALTYDIALOG_H
