#ifndef LISTENER_FCV7JYL9

#define LISTENER_FCV7JYL9

#include "global.h"
#include "Logger.h"
#include "Thread.h"
#include "Peer.h"

#define MAX_BACKLOG 10

using namespace std;


class Listener : public Thread {
    private:
        struct in_addr  la;     // local address
        struct in_addr  ra;     // remote address
        sockfd          lfd;    // listen socket fd
        sockfd          afd;    // accepet socket fd

    public:
        Listener(struct in_addr & l, struct in_addr & r);
        Listener(struct in_addr & l);
        virtual ~Listener();
        void * Run();

        // peer listen helper or connect helper
        bool InitConn(struct in_addr & addr);
        bool TryAccept(struct sockaddr_in & addr);

        // socket helper
        static bool SetNonBlock(sockfd sfd);
        static bool UnsetNonBlock(sockfd sfd);

};


#endif /* end of include guard: LISTENER_FCV7JYL9 */
