#include "Message.h"

using namespace std;

map<message_t, string> Message::mapMsgName = {
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
        log.Error("cannot malloc Message");
    } else {
        size = len;
    }
}

Message::~Message() 
{
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
            log.Error("failed to write buf\n");
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

// void 
// Message::InitHeader(bgphdr & hdr, u_int16_t len, message_t type)
// {
//     memset(hdr.marker, 0xff, 16);
//     hdr.length = htons(len);
//     hdr.type = type;
// }

// void 
// Message::InitOpenMsg(u_int16_t as, u_int16_t ht, u_int32_t ip)
// {
//     openmsg * msg = (openmsg *) bufMSG;
//     ssize_t msgLen = MSGSIZE_HEADER;

//     msg->version = 4;
//     msg->myas = htons(as);
//     msg->holdtime = htons(ht);
//     msg->bgpid = htonl(ip);
//     msgLen += 1 + 2 + 2 + 4;
    
//     msg->optparamlen = 0;
//     msgLen += 1;

//     InitHeader(msg->msghdr, msgLen, OPEN);
// }

// bool 
// Message::SendMsg(sockfd sfd)
// {
//     ssize_t msgLen;
//     ssize_t res;
//     bgphdr * hdr = NULL;

//     hdr = &((openmsg *) bufMSG )->msghdr;

//     msgLen = ntohs(hdr->length);
//     res = send(sfd, bufMSG, msgLen, 0);

//     return (res == msgLen);
// }

// #define PRINT_ALIGN 16
// void 
// Message::DumpRawMsg(u_char * buf, ssize_t size)
// {
//     for (int i = 0; i < size; ++i) {
//         if (!((i)%PRINT_ALIGN)) fprintf(logfd, "0x%04x : ", i);
//         fprintf(logfd, "%02x%c", *(buf+i), (i+1)%PRINT_ALIGN ? ' ' : '\n');
//     }
//     if (size%PRINT_ALIGN) fprintf(logfd, "\n");
// }

// void 
// Message::DumpSelf()
// {
//     bgphdr * hdr = (bgphdr *) bufMSG;
//     ssize_t size = ntohs(hdr->length);
//     DumpRawMsg(bufMSG, size);
// }
