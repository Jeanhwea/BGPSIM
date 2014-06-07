#include "Buffer.h"

deque<Buffer *> Buffer::mqBuffer;

Buffer::Buffer() 
{
    rptr = buf;
    wpos = 0;
}


Buffer::~Buffer() 
{

}
