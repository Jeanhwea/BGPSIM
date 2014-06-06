#ifndef BUFFER_3NKWNENG

#define BUFFER_3NKWNENG

#include "global.h"

using namespace std;

class Buffer {
    private:
        static deque<Buffer *> mqBuffer;

    public:
        u_char    * buf;
        ssize_t     size;
        ssize_t     wpos;
        ssize_t     rpos;
        sockfd      sfd;

        Buffer();
        Buffer(ssize_t len);
        virtual ~Buffer();
        bool Add(void * data, ssize_t len);
        u_char * Reserve(ssize_t len);
        bool Close();
        bool Write();
        bool Write(sockfd sfd, Buffer * buf);

        void BufDeque();
        void BufEnque(Buffer *);
};

#endif /* end of include guard: BUFFER_3NKWNENG */
