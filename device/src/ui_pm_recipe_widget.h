/********************************************************************************
** Form generated from reading UI file 'pm_recipe_widget.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PM_RECIPE_WIDGET_H
#define UI_PM_RECIPE_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QPmRecipeWidgetClass
{
public:
    QTableWidget *pm_cavity_param_edit_tbw;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *add_an_item_pbt;
    QPushButton *delete_the_selected_item_pbt;
    QPushButton *clear_sequence_pbt;
    QPushButton *pushButton;
    QTabWidget *tabWidget;
    QWidget *tab;
    QWidget *tab_2;
    QWidget *tab_3;
    QWidget *tab_4;

    void setupUi(QWidget *QPmRecipeWidgetClass)
    {
        if (QPmRecipeWidgetClass->objectName().isEmpty())
            QPmRecipeWidgetClass->setObjectName(QStringLiteral("QPmRecipeWidgetClass"));
        QPmRecipeWidgetClass->resize(1162, 787);
        pm_cavity_param_edit_tbw = new QTableWidget(QPmRecipeWidgetClass);
        if (pm_cavity_param_edit_tbw->columnCount() < 9)
            pm_cavity_param_edit_tbw->setColumnCount(9);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        pm_cavity_param_edit_tbw->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        pm_cavity_param_edit_tbw->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        pm_cavity_param_edit_tbw->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        pm_cavity_param_edit_tbw->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        pm_cavity_param_edit_tbw->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        pm_cavity_param_edit_tbw->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        pm_cavity_param_edit_tbw->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        pm_cavity_param_edit_tbw->setHorizontalHeaderItem(7, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        pm_cavity_param_edit_tbw->setHorizontalHeaderItem(8, __qtablewidgetitem8);
        pm_cavity_param_edit_tbw->setObjectName(QStringLiteral("pm_cavity_param_edit_tbw"));
        pm_cavity_param_edit_tbw->setGeometry(QRect(40, 110, 981, 300));
        pm_cavity_param_edit_tbw->setMinimumSize(QSize(600, 0));
        pm_cavity_param_edit_tbw->setMaximumSize(QSize(16777215, 300));
        pm_cavity_param_edit_tbw->setAlternatingRowColors(true);
        pm_cavity_param_edit_tbw->horizontalHeader()->setVisible(true);
        pm_cavity_param_edit_tbw->verticalHeader()->setVisible(false);
        layoutWidget = new QWidget(QPmRecipeWidgetClass);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(40, 50, 991, 41));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        add_an_item_pbt = new QPushButton(layoutWidget);
        add_an_item_pbt->setObjectName(QStringLiteral("add_an_item_pbt"));

        horizontalLayout->addWidget(add_an_item_pbt);

        delete_the_selected_item_pbt = new QPushButton(layoutWidget);
        delete_the_selected_item_pbt->setObjectName(QStringLiteral("delete_the_selected_item_pbt"));

        horizontalLayout->addWidget(delete_the_selected_item_pbt);

        clear_sequence_pbt = new QPushButton(layoutWidget);
        clear_sequence_pbt->setObjectName(QStringLiteral("clear_sequence_pbt"));

        horizontalLayout->addWidget(clear_sequence_pbt);

        pushButton = new QPushButton(layoutWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        horizontalLayout->addWidget(pushButton);

        tabWidget = new QTabWidget(QPmRecipeWidgetClass);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(50, 440, 981, 261));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        tabWidget->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QStringLiteral("tab_4"));
        tabWidget->addTab(tab_4, QString());

        retranslateUi(QPmRecipeWidgetClass);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(QPmRecipeWidgetClass);
    } // setupUi

    void retranslateUi(QWidget *QPmRecipeWidgetClass)
    {
        QPmRecipeWidgetClass->setWindowTitle(QApplication::translate("QPmRecipeWidgetClass", "QPmRecipeWidget", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem = pm_cavity_param_edit_tbw->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("QPmRecipeWidgetClass", "\345\267\245\350\211\272\346\254\241\346\225\260", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem1 = pm_cavity_param_edit_tbw->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("QPmRecipeWidgetClass", "\345\215\207\351\231\215\350\275\264\345\212\240\351\200\237\345\272\2461", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem2 = pm_cavity_param_edit_tbw->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("QPmRecipeWidgetClass", "\345\215\207\351\231\215\350\275\264\345\212\240\351\200\237\345\272\2462", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem3 = pm_cavity_param_edit_tbw->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("QPmRecipeWidgetClass", "\345\215\207\351\231\215\350\275\264\345\212\240\351\200\237\345\272\2463", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem4 = pm_cavity_param_edit_tbw->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("QPmRecipeWidgetClass", "\345\215\207\351\231\215\350\275\264\345\212\240\351\200\237\345\272\2464", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem5 = pm_cavity_param_edit_tbw->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QApplication::translate("QPmRecipeWidgetClass", "\346\227\213\350\275\254\345\212\240\351\200\237\345\272\2461", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem6 = pm_cavity_param_edit_tbw->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QApplication::translate("QPmRecipeWidgetClass", "\346\227\213\350\275\254\345\212\240\351\200\237\345\272\2462", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem7 = pm_cavity_param_edit_tbw->horizontalHeaderItem(7);
        ___qtablewidgetitem7->setText(QApplication::translate("QPmRecipeWidgetClass", "\346\227\213\350\275\254\345\212\240\351\200\237\345\272\2463", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem8 = pm_cavity_param_edit_tbw->horizontalHeaderItem(8);
        ___qtablewidgetitem8->setText(QApplication::translate("QPmRecipeWidgetClass", "\346\227\213\350\275\254\345\212\240\351\200\237\345\272\2464", Q_NULLPTR));
        add_an_item_pbt->setText(QApplication::translate("QPmRecipeWidgetClass", "\345\242\236\345\212\240\344\270\200\351\241\271", Q_NULLPTR));
        delete_the_selected_item_pbt->setText(QApplication::translate("QPmRecipeWidgetClass", "\345\210\240\351\231\244\351\200\211\344\270\255\351\241\271", Q_NULLPTR));
        clear_sequence_pbt->setText(QApplication::translate("QPmRecipeWidgetClass", "\346\270\205\347\251\272\346\225\260\346\215\256", Q_NULLPTR));
        pushButton->setText(QApplication::translate("QPmRecipeWidgetClass", "\350\256\276\347\275\256", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("QPmRecipeWidgetClass", "PM1", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("QPmRecipeWidgetClass", "PM2", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QApplication::translate("QPmRecipeWidgetClass", "PM3", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QApplication::translate("QPmRecipeWidgetClass", "PM4", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class QPmRecipeWidgetClass: public Ui_QPmRecipeWidgetClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PM_RECIPE_WIDGET_H
