#ifndef DISPATCHER_Q0GBWRKL

#define DISPATCHER_Q0GBWRKL

#include "global.h"
#include "Thread.h"

class Peer;

class Dispatcher : public Thread {
    private:
        sockfd          sfd;
        bool            isReading;

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

        bool isRead();
        bool ReadMsg();
        bool ReadMsg(Peer * pPeer);
        bool DispatchMsg();
        bool DispatchMsg(Peer * pPeer);
};

#endif /* end of include guard: DISPATCHER_Q0GBWRKL */
