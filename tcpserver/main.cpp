#include "tcpserver.h"
#include "dboperator.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DBOperator::getInstance().init();

    TcpServer w;
    w.show();
    return a.exec();
}
