#ifndef BUFFER_3NKWNENG

#define BUFFER_3NKWNENG

#include "global.h"
#include "Logger.h"

using namespace std;

#define MAX_BUF_SIZE 65535

class Buffer {
    private:
        Logger log;

    public:
        u_char      buf[MAX_BUF_SIZE];
        u_char    * rptr;
        ssize_t     wpos;

        Buffer();
        virtual ~Buffer();

};

#endif /* end of include guard: BUFFER_3NKWNENG */
