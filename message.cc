#include "message.h"

using namespace std;

#define HEAD_LEN (sizeof(bgphdr))

map<Message_t, string> Message::mapMsgName = {
    { OPEN, "open" },
    { UPDATE, "update" },
    { NOTIFICATION, "notification" },
    { KEEPALIVE, "keepalive" }
};

Message::Message()
: type(OPEN)
{
    InitHeader();
}

Message::~Message()
{

}

void Message::InitHeader()
{
    memset(&hdr.marker, 0xff, 16);
    hdr.length = htons(0);
    hdr.type = 0;
}

void Message::InitOpenMsg(hexet no, hexet ht, dhexet id)
{
    openmsg * msg = (openmsg *) (bufMSG + HEAD_LEN);
    ssize_t msgLen = HEAD_LEN;

    msg->version = 4;
    msg->asno = htons(no);
    msg->hold_time = htons(ht);
    msg->bgp_ident = htonl(id);
    msgLen += 1 + 2 + 2 + 4;
    
    msg->op_para_len = 0;
    msgLen += 1;

    hdr.length = htons(msgLen);
    hdr.type = OPEN;
}

void Message::InitUpdateMsg(hexet urlen, hexet alen)
{
    updatemsg * msg = (updatemsg *) (bufMSG + HEAD_LEN);
    ssize_t msgLen = HEAD_LEN;

    msg->ur_len = htons(urlen);
    msg->tpatt_len = htons(alen);
    msgLen += 2 + 2;

    hdr.length = htons(msgLen);
    hdr.type = UPDATE;
}

void Message::InitKeepaliveMsg()
{
    // do nothing but set header length = 0
    ssize_t msgLen = HEAD_LEN;

    hdr.length = htons(msgLen);
    hdr.type = KEEPALIVE;
}

void Message::InitNotificationMsg(octet er, octet erb)
{
    notificationmsg * msg = (notificationmsg *) (bufMSG + HEAD_LEN);
    msg += sizeof(bgphdr);
    ssize_t msgLen = HEAD_LEN;

    msg->err = er;
    msg->err_sub = erb;

    msgLen += 1 + 1;

    hdr.length = htons(msgLen);
    hdr.type = NOTIFICATION;
}

bool Message::SendMsg(int sockfd)
{
    ssize_t msgLen;
    ssize_t res;

    msgLen = ntohs(hdr.length);
    memcpy(bufMSG, &hdr, HEAD_LEN);
    res = send(sockfd, bufMSG, msgLen, 0);

    return (res == msgLen);
}

#define PRINT_ALIGN 16
void Message::DumpRawMsg(octet * buf, ssize_t size)
{
    for (int i = 0; i < size; ++i) {
        if (!((i)%PRINT_ALIGN)) printf("0x%04x : ", i);
        printf("%02x%c", *(buf+i), (i+1)%PRINT_ALIGN ? ' ' : '\n');
    }
    if (size%PRINT_ALIGN) printf("\n");
}

void Message::DumpSelf()
{
    octet * buf = bufMSG;
    ssize_t size = ntohs(hdr.length);
    DumpRawMsg(buf, size);
}
