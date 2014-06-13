#include "Buffer.h"
#include "Logger.h"

Buffer::Buffer(int len)
{
    data = (u_char *) malloc(len * sizeof(u_char));
    rpos = 0;
    wpos = 0;
    if (data == NULL) {
        size = 0;
        g_log->Error("cannot malloc Message");
    } else {
        size = len;
    }
}

Buffer::~Buffer()
{
    if (data != NULL)
        free(data);
}

bool
Buffer::Add(void * data, ssize_t len)
{
    assert(wpos + len <= size);
    memcpy(data + wpos, data, len);
    wpos += len;
    return true;
}

u_char *
Buffer::Reserve(ssize_t len)
{
    u_char * ret;
    if (wpos + len > size)
        return NULL;
    ret = data + wpos;
    wpos += len;
    return ret;
}

bool
Buffer::Write(sockfd sfd, Buffer * pBuf)
{
    ssize_t nleft, nwrite;
    u_char * ptr;

    nleft = pBuf->wpos - pBuf->rpos;
    ptr = pBuf->data + pBuf->rpos;
    while (nleft > 0) {
        nwrite = write(sfd, ptr, nleft);
        if (nwrite < 0) {
            g_log->Error("failed to write data\n");
            return false;
        }
        nleft -= nwrite;
        ptr += nwrite;
    }
    pBuf->rpos = pBuf->wpos;
    return true;
}
