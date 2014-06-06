#include "Buffer.h"

deque<Buffer *> Buffer::mqBuffer;

Buffer::Buffer() 
{
    Buffer(MSG_HEADER_MAX);
}

Buffer::Buffer(ssize_t len) 
{
    buf = (u_char *) malloc(len);
    if (buf == NULL) {
        size = 0;
        sfd = -1;
        free(buf);
        log.Error("cannot malloc buffer");
    } else {
        size = len;
        sfd = -1;
    }
}

Buffer::~Buffer() 
{
    free(buf);
}

bool
Buffer::Add(void * data, ssize_t len) 
{
    if (wpos + len > size) 
        return false;
    memcpy(buf + wpos, data, len);
    wpos += len;
    return true;
}

u_char *
Buffer::Reserve(ssize_t len) 
{
    u_char * ret;
    if (wpos + len > size) 
        return NULL;
    ret = buf + wpos;
    wpos += len;
    return ret;
}

bool
Buffer::Write() 
{
    return Write(sfd, this);
}

bool
Buffer::Write(sockfd sfd, Buffer * buf) 
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
Buffer::BufDeque() 
{
   mqBuffer.pop_back();
}

void
Buffer::BufEnque(Buffer * buf) 
{
    mqBuffer.push_front(buf);
}
