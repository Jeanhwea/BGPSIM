#include "message.h"

using namespace std;

#define HEAD_LEN (sizeof(bgphdr))

map<message_t, string> Message::mapMsgName = {
    { OPEN, "open" },
    { UPDATE, "update" },
    { NOTIFICATION, "notification" },
    { KEEPALIVE, "keepalive" }
};

Message::Message()
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

void Message::InitOpenMsg(u_int16_t no, u_int16_t ht, u_int32_t ip)
{
    openmsg * msg = (openmsg *) (bufMSG + HEAD_LEN);
    ssize_t msgLen = HEAD_LEN;

    msg->version = 4;
    msg->myas = htons(no);
    msg->holdtime = htons(ht);
    msg->bgpid = htonl(ip);
    msgLen += 1 + 2 + 2 + 4;
    
    msg->optparamlen = 0;
    msgLen += 1;

    hdr.length = htons(msgLen);
    hdr.type = OPEN;
}

bool Message::SendMsg(sockfd sfd)
{
    ssize_t msgLen;
    ssize_t res;

    msgLen = ntohs(hdr.length);
    memcpy(bufMSG, &hdr, HEAD_LEN);
    res = send(sfd, bufMSG, msgLen, 0);

    return (res == msgLen);
}

#define PRINT_ALIGN 16
void Message::DumpRawMsg(u_int8_t * buf, ssize_t size)
{
    for (int i = 0; i < size; ++i) {
        if (!((i)%PRINT_ALIGN)) printf("0x%04x : ", i);
        printf("%02x%c", *(buf+i), (i+1)%PRINT_ALIGN ? ' ' : '\n');
    }
    if (size%PRINT_ALIGN) printf("\n");
}

void Message::DumpSelf()
{
    u_int8_t * buf = bufMSG;
    ssize_t size = ntohs(hdr.length);
    DumpRawMsg(buf, size);
}
