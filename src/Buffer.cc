#include "Buffer.h"
#include "Logger.h"

Buffer::Buffer(int len)
: wpos(0), rpos(0), isDispathed(false)
{
    data = (u_char *) malloc(len * sizeof(u_char));
    wpos = 0;
    rpos = 0;
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
Buffer::Add(void * pData, ssize_t len)
{
    assert(wpos + len <= size);
    memcpy(data + wpos, pData, len);
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
Buffer::Skip(ssize_t len)
{
    assert(rpos + len <= wpos);
    rpos += len;
    return true;
}

ssize_t
Buffer::Length()
{
    return (wpos - rpos);
}

u_char *
Buffer::ReadPos()
{
    return (data + rpos);
}

