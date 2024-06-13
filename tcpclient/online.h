#ifndef ONLINE_H
#define ONLINE_H

#include "protocol.h"

#include <QWidget>

namespace Ui {
class Online;
}

class Online : public QWidget
{
    Q_OBJECT

public:
    explicit Online(QWidget *parent = nullptr);
    ~Online();
    void showUsr(PDU* pdu);

private slots:
    void on_adFriend_pb_clicked();

private:
    Ui::Online *ui;
};

#endif // ONLINE_H
