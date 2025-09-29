/********************************************************************************
** Form generated from reading UI file 'propertiesviewerwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.17
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROPERTIESVIEWERWIDGET_H
#define UI_PROPERTIESVIEWERWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PropertiesViewerWidget
{
public:
    QHBoxLayout *horizontalLayout;
    QTabWidget *tabWidget;
    QWidget *tabWidgetPage1;
    QHBoxLayout *horizontalLayout_6;
    QTextEdit *allTextEdit;
    QWidget *tabWidgetPage2;
    QVBoxLayout *verticalLayout;
    QTextEdit *itemAndMysticOrbsTextEdit;
    QWidget *tabWidgetPage3;
    QHBoxLayout *horizontalLayout_4;
    QTextEdit *rwAndMysticOrbsTextEdit;
    QWidget *tabWidgetPage4;
    QHBoxLayout *horizontalLayout_3;
    QTextEdit *socketablesTextEdit;

    void setupUi(QWidget *PropertiesViewerWidget)
    {
        if (PropertiesViewerWidget->objectName().isEmpty())
            PropertiesViewerWidget->setObjectName(QString::fromUtf8("PropertiesViewerWidget"));
        PropertiesViewerWidget->resize(279, 406);
        horizontalLayout = new QHBoxLayout(PropertiesViewerWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        tabWidget = new QTabWidget(PropertiesViewerWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setTabPosition(QTabWidget::North);
        tabWidget->setTabShape(QTabWidget::Rounded);
        tabWidget->setUsesScrollButtons(false);
        tabWidgetPage1 = new QWidget();
        tabWidgetPage1->setObjectName(QString::fromUtf8("tabWidgetPage1"));
        horizontalLayout_6 = new QHBoxLayout(tabWidgetPage1);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        allTextEdit = new QTextEdit(tabWidgetPage1);
        allTextEdit->setObjectName(QString::fromUtf8("allTextEdit"));
        allTextEdit->setLineWrapMode(QTextEdit::NoWrap);
        allTextEdit->setReadOnly(true);

        horizontalLayout_6->addWidget(allTextEdit);

        tabWidget->addTab(tabWidgetPage1, QString());
        tabWidgetPage2 = new QWidget();
        tabWidgetPage2->setObjectName(QString::fromUtf8("tabWidgetPage2"));
        verticalLayout = new QVBoxLayout(tabWidgetPage2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        itemAndMysticOrbsTextEdit = new QTextEdit(tabWidgetPage2);
        itemAndMysticOrbsTextEdit->setObjectName(QString::fromUtf8("itemAndMysticOrbsTextEdit"));
        itemAndMysticOrbsTextEdit->setLineWrapMode(QTextEdit::NoWrap);
        itemAndMysticOrbsTextEdit->setReadOnly(true);

        verticalLayout->addWidget(itemAndMysticOrbsTextEdit);

        tabWidget->addTab(tabWidgetPage2, QString());
        tabWidgetPage3 = new QWidget();
        tabWidgetPage3->setObjectName(QString::fromUtf8("tabWidgetPage3"));
        horizontalLayout_4 = new QHBoxLayout(tabWidgetPage3);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        rwAndMysticOrbsTextEdit = new QTextEdit(tabWidgetPage3);
        rwAndMysticOrbsTextEdit->setObjectName(QString::fromUtf8("rwAndMysticOrbsTextEdit"));
        rwAndMysticOrbsTextEdit->setLineWrapMode(QTextEdit::NoWrap);
        rwAndMysticOrbsTextEdit->setReadOnly(true);

        horizontalLayout_4->addWidget(rwAndMysticOrbsTextEdit);

        tabWidget->addTab(tabWidgetPage3, QString());
        tabWidgetPage4 = new QWidget();
        tabWidgetPage4->setObjectName(QString::fromUtf8("tabWidgetPage4"));
        horizontalLayout_3 = new QHBoxLayout(tabWidgetPage4);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        socketablesTextEdit = new QTextEdit(tabWidgetPage4);
        socketablesTextEdit->setObjectName(QString::fromUtf8("socketablesTextEdit"));
        socketablesTextEdit->setLineWrapMode(QTextEdit::NoWrap);
        socketablesTextEdit->setReadOnly(true);

        horizontalLayout_3->addWidget(socketablesTextEdit);

        tabWidget->addTab(tabWidgetPage4, QString());

        horizontalLayout->addWidget(tabWidget);


        retranslateUi(PropertiesViewerWidget);

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(PropertiesViewerWidget);
    } // setupUi

    void retranslateUi(QWidget *PropertiesViewerWidget)
    {
        tabWidget->setTabText(tabWidget->indexOf(tabWidgetPage1), QCoreApplication::translate("PropertiesViewerWidget", "All", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabWidgetPage2), QCoreApplication::translate("PropertiesViewerWidget", "Item / MO", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabWidgetPage3), QCoreApplication::translate("PropertiesViewerWidget", "RW / MO", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabWidgetPage4), QCoreApplication::translate("PropertiesViewerWidget", "Socketables", nullptr));
        (void)PropertiesViewerWidget;
    } // retranslateUi

};

namespace Ui {
    class PropertiesViewerWidget: public Ui_PropertiesViewerWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROPERTIESVIEWERWIDGET_H
