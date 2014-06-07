#ifndef BUFFER_3NKWNENG

#define BUFFER_3NKWNENG

#include "global.h"
#include "Logger.h"

using namespace std;

class Buffer {
    private:
        static deque<Buffer *> mqBuffer;
        Logger log;

    public:
        u_char    * buf;
        u_char    * rptr;
        ssize_t     wpos;

        Buffer();
        virtual ~Buffer();

};

#endif /* end of include guard: BUFFER_3NKWNENG */
