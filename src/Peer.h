#ifndef PEER_V3V7QZZV

#define PEER_V3V7QZZV

#include "global.h"
#include "Buffer.h"
#include "Message.h"
#include "Thread.h"
#include "Logger.h"

using namespace std;

struct peer_config {
    bool             passive;
    struct in_addr   remote_addr;
    // struct in_addr   local_addr;
    u_int16_t        remote_as;
    u_int32_t        remote_bgpid;
    u_int16_t        holdtime;
    u_int16_t        min_holdtime;
};

class Peer : public Thread {
    private:
        state_t         mState;

    public:
        sockfd              sfd;
        u_int16_t           holdtime;
        struct peer_config  conf;

        time_t              ConnetRetryTimer;
        time_t              KeepaliveTimer;
        time_t              HoldTimer;
        time_t              IdleHoldTimer;

        struct sockaddr_in  sa_local;
        struct sockaddr_in  sa_remote;

        Buffer            * rbuf;
        Message           * wbuf;

        Peer ();
        virtual ~Peer ();
        void * Run();
        void * Run(event_t eve);

        state_t GetPeerState() {
            return mState;
        }
        void SetPeerState(state_t state) {
            mState = state;
        }

        void InitTimer();

        void StartTimerHoldtime();
        void StartTimerKeepalive();
};

extern map<state_t, string> mapStateName;
extern vector<Peer *>       mvPeers;
#endif /* end of include guard: PEER_V3V7QZZV */