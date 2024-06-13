/********************************************************************************
** Form generated from reading UI file 'tcpclient.ui'
**
** Created by: Qt User Interface Compiler version 6.2.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TCPCLIENT_H
#define UI_TCPCLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TcpClient
{
public:
    QPushButton *login_pb;
    QPushButton *regist_pb;
    QPushButton *delete_pb;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QLineEdit *name_lineEdit;
    QLineEdit *pwd_lineEdit;

    void setupUi(QWidget *TcpClient)
    {
        if (TcpClient->objectName().isEmpty())
            TcpClient->setObjectName(QString::fromUtf8("TcpClient"));
        TcpClient->resize(410, 348);
        login_pb = new QPushButton(TcpClient);
        login_pb->setObjectName(QString::fromUtf8("login_pb"));
        login_pb->setGeometry(QRect(140, 210, 121, 31));
        regist_pb = new QPushButton(TcpClient);
        regist_pb->setObjectName(QString::fromUtf8("regist_pb"));
        regist_pb->setGeometry(QRect(10, 320, 80, 18));
        delete_pb = new QPushButton(TcpClient);
        delete_pb->setObjectName(QString::fromUtf8("delete_pb"));
        delete_pb->setGeometry(QRect(320, 320, 80, 18));
        widget = new QWidget(TcpClient);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(110, 110, 181, 71));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        name_lineEdit = new QLineEdit(widget);
        name_lineEdit->setObjectName(QString::fromUtf8("name_lineEdit"));

        verticalLayout->addWidget(name_lineEdit);

        pwd_lineEdit = new QLineEdit(widget);
        pwd_lineEdit->setObjectName(QString::fromUtf8("pwd_lineEdit"));
        pwd_lineEdit->setEchoMode(QLineEdit::Password);

        verticalLayout->addWidget(pwd_lineEdit);


        retranslateUi(TcpClient);

        QMetaObject::connectSlotsByName(TcpClient);
    } // setupUi

    void retranslateUi(QWidget *TcpClient)
    {
        TcpClient->setWindowTitle(QCoreApplication::translate("TcpClient", "TcpClient", nullptr));
        login_pb->setText(QCoreApplication::translate("TcpClient", "\347\231\273\345\275\225", nullptr));
        regist_pb->setText(QCoreApplication::translate("TcpClient", "\346\263\250\345\206\214\350\264\246\345\217\267", nullptr));
        delete_pb->setText(QCoreApplication::translate("TcpClient", "\346\263\250\351\224\200\350\264\246\345\217\267", nullptr));
        name_lineEdit->setPlaceholderText(QCoreApplication::translate("TcpClient", "\350\257\267\350\276\223\345\205\245\347\224\250\346\210\267\345\220\215", nullptr));
        pwd_lineEdit->setPlaceholderText(QCoreApplication::translate("TcpClient", "\350\257\267\350\276\223\345\205\245\345\257\206\347\240\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TcpClient: public Ui_TcpClient {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TCPCLIENT_H
