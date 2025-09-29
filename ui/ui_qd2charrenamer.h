/********************************************************************************
** Form generated from reading UI file 'qd2charrenamer.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QD2CHARRENAMER_H
#define UI_QD2CHARRENAMER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_QD2CharRenamerClass
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLineEdit *charNameLineEdit;
    QPushButton *colorButton;
    QLabel *charNamePreviewLabel;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *QD2CharRenamerClass)
    {
        if (QD2CharRenamerClass->objectName().isEmpty())
            QD2CharRenamerClass->setObjectName("QD2CharRenamerClass");
        QD2CharRenamerClass->resize(253, 91);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(QD2CharRenamerClass->sizePolicy().hasHeightForWidth());
        QD2CharRenamerClass->setSizePolicy(sizePolicy);
        verticalLayout = new QVBoxLayout(QD2CharRenamerClass);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName("horizontalLayout");
        charNameLineEdit = new QLineEdit(QD2CharRenamerClass);
        charNameLineEdit->setObjectName("charNameLineEdit");
        sizePolicy.setHeightForWidth(charNameLineEdit->sizePolicy().hasHeightForWidth());
        charNameLineEdit->setSizePolicy(sizePolicy);
        charNameLineEdit->setMinimumSize(QSize(0, 23));
        charNameLineEdit->setMaximumSize(QSize(16777215, 23));

        horizontalLayout->addWidget(charNameLineEdit);

        colorButton = new QPushButton(QD2CharRenamerClass);
        colorButton->setObjectName("colorButton");
        colorButton->setEnabled(true);

        horizontalLayout->addWidget(colorButton);


        verticalLayout->addLayout(horizontalLayout);

        charNamePreviewLabel = new QLabel(QD2CharRenamerClass);
        charNamePreviewLabel->setObjectName("charNamePreviewLabel");
        charNamePreviewLabel->setText(QString::fromUtf8("name preview goes here"));

        verticalLayout->addWidget(charNamePreviewLabel);

        buttonBox = new QDialogButtonBox(QD2CharRenamerClass);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(QD2CharRenamerClass);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, QD2CharRenamerClass, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(QD2CharRenamerClass);
    } // setupUi

    void retranslateUi(QDialog *QD2CharRenamerClass)
    {
#if QT_CONFIG(tooltip)
        charNameLineEdit->setToolTip(QCoreApplication::translate("QD2CharRenamerClass", "<html><body>You can use the following ANSI characters (maximum 15 including colors):<ul><li>codes 32-127: all except ?*<>.|:\"/\\</li><li>codes 146, 147 and 160-191</li></ul>Also name can't start with _ (underscore) or end with - (hyphen).</body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        colorButton->setText(QCoreApplication::translate("QD2CharRenamerClass", "Color", nullptr));
        (void)QD2CharRenamerClass;
    } // retranslateUi

};

namespace Ui {
    class QD2CharRenamerClass: public Ui_QD2CharRenamerClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QD2CHARRENAMER_H
