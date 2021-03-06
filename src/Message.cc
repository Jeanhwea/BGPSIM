#include "Message.h"
#include "Logger.h"

using namespace std;

map<message_t, string> mapMsgName = {
    { OPEN, "open" },
    { UPDATE, "update" },
    { NOTIFICATION, "notification" },
    { KEEPALIVE, "keepalive" }
};

Message::Message(int len)
: Buffer(len)
{

}

Message::~Message()
{

}

bool Message::Write(sockfd sfd, Message * pMsg)
{
    ssize_t nleft, nwrite;
    u_char * ptr;

    nleft = pMsg->wpos - pMsg->rpos;
    ptr = pMsg->data + pMsg->rpos;
    while (nleft > 0) {
        nwrite = write(sfd, ptr, nleft);
        if (nwrite < 0) {
            g_log->Error("Buffer :: failed to write data\n");
            return false;
        }
        nleft -= nwrite;
        ptr += nwrite;
    }
    pMsg->rpos = pMsg->wpos;
    return true;
}