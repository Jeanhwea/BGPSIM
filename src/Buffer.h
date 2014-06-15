#ifndef BUFFER_3NKWNENG

#define BUFFER_3NKWNENG

#include "global.h"

using namespace std;

class Buffer {
    private:

    public:
        u_char    * data;
        ssize_t     size;
        ssize_t     wpos;
        ssize_t     rpos;

        Buffer(int len);
        virtual ~Buffer();

        bool Add(void *, ssize_t);
        u_char * Reserve(ssize_t);
        ssize_t Length();
        u_char * ReadPos();
};

#endif /* end of include guard: BUFFER_3NKWNENG */
