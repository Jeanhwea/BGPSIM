#ifndef DISPATCHER_Q0GBWRKL

#define DISPATCHER_Q0GBWRKL

#include "global.h"
#include "Thread.h"

class Buffer;
class Peer;

class Dispatcher : public Thread {
    private:
        sockfd              sfd;
        pthread_mutex_t     mutex; // reading flag mutex
        bool                isReading;
        Buffer            * preBuf;

    public:
        Dispatcher();
        virtual ~Dispatcher();
        void * Run();

        void SetReadFd(sockfd fd) {
            sfd = fd;
        }
        sockfd GetReadFd() {
            return sfd;
        }

        bool ReadMsg();
        bool ReadMsg(Peer * pPeer);
        bool DispatchMsg();
        bool DispatchMsg(Peer * pPeer);
        bool GetMsgLen(u_char * data, u_int16_t & len);
        
        void Lock();
        void Unlock();
};

#endif /* end of include guard: DISPATCHER_Q0GBWRKL */
