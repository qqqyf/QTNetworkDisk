#include "protocol.h"

PDU *mkPDU(uint MsgLen)
{
    uint PDULen = sizeof(PDU)+MsgLen;
    PDU *pdu = (PDU*)malloc(PDULen);
    if(pdu == NULL){
        exit(EXIT_FAILURE);
    }
    memset(pdu,0,PDULen);//清空分配的内存
    pdu->PDULen = PDULen;
    pdu->MsgLen = MsgLen;
    return pdu;
}
