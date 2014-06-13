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
    public:
        struct in_addr * pLAdd;     // listener address
        struct in_addr * pRAdd;     // remote address
        sockfd           lfd;      // listen socket fd
        sockfd           afd;      // accepet socket fd

    public:
        Listener(struct in_addr * pL, struct in_addr * pR);
        Listener(struct in_addr * pL);
        virtual ~Listener();
        void * Run();

        // peer listen helper or connect helper
        bool InitConn(struct in_addr * pAd);
        bool TryAccept(struct sockaddr_in * pSad);

        // ...
        struct in_addr * GetLisAddr();

        // socket helper
        static bool SetNonBlock(sockfd sfd);
        static bool UnsetNonBlock(sockfd sfd);
        static bool SetTTL(sockfd sfd, int ttl);

};

extern vector<Listener *> vListeners;
#endif /* end of include guard: LISTENER_FCV7JYL9 */
