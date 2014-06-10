#include "Message.h"

using namespace std;

map<message_t, string> mapMsgName = {
    { OPEN, "open" },
    { UPDATE, "update" },
    { NOTIFICATION, "notification" },
    { KEEPALIVE, "keepalive" }
};

deque<Message *> Message::mqMessage;

Message::Message(ssize_t len)
{
    buf = (u_char *) malloc(len);
    rpos = 0;
    wpos = 0;
    sfd = -1;
    if (buf == NULL) {
        size = 0;
        free(buf);
        g_log->Error("cannot malloc Message");
    } else {
        size = len;
    }
}

Message::~Message()
{
    if (buf != NULL)
        free(buf);
}

bool
Message::Add(void * data, ssize_t len)
{
    if (wpos + len > size)
        return false;
    memcpy(buf + wpos, data, len);
    wpos += len;
    return true;
}

u_char *
Message::Reserve(ssize_t len)
{
    u_char * ret;
    if (wpos + len > size)
        return NULL;
    ret = buf + wpos;
    wpos += len;
    return ret;
}

bool
Message::Write()
{
    return Write(sfd, this);
}

bool
Message::Write(sockfd sfd, Message * buf)
{
    ssize_t nleft, nwrite;
    u_char * ptr;

    nleft = buf->size - buf->rpos;
    ptr = buf->buf + buf->wpos;
    while (nleft > 0) {
        nwrite = write(sfd, ptr, nleft);
        if (nwrite < 0 && errno == EAGAIN) {
            g_log->Error("failed to write buf\n");
            return false;
        }
        nleft -= nwrite;
        ptr += nwrite;
    }
    return true;
}

void
Message::BufDeque()
{
   mqMessage.pop_back();
}

void
Message::BufEnque(Message * buf)
{
    mqMessage.push_front(buf);
}

