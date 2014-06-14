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


ssize_t
Buffer::Length()
{
    return (wpos - rpos);
}

