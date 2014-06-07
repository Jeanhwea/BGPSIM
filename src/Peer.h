#ifndef PEER_V3V7QZZV

#define PEER_V3V7QZZV

#include "global.h"
#include "Buffer.h"
#include "Message.h"

using namespace std;

typedef enum {
    IDLE,
    CONNECT,
    ACTIVE,
    OPEN_SENT,
    OPEN_CONFIRM,
    ESTABLISHED
} state_t;

typedef struct _peer_config {
    u_int32_t        id;
    bool             passive;
    struct in_addr   remote_addr;
    struct in_addr   local_addr;
    u_int16_t        remote_as;
    u_int16_t        holdtime;
    u_int16_t        min_holdtime;
} peer_config;

class Peer {
    private:
        u_int16_t       holdtime;

        state_t         mState;

        static map<state_t, string> mapStateName;
        
    public:
        sockfd              sfd;
        u_int32_t           remote_bgpid;
        peer_config         conf;

        time_t              ConnetRetryTimer;
        time_t              KeepaliveTimer;
        time_t              HoldTimer;
        time_t              IdleHoldTimer;
        time_t              IdleHoldResetTimer;
        time_t              IdleHoldTime;
        
        struct sockaddr_in  sa_local;
        struct sockaddr_in  sa_remote;
    
        Buffer            * rbuf;
        Message           * wbuf;

        Peer ();
        virtual ~Peer ();
    
        string PeerStateStr() {
            return mapStateName[mState];
        }

        state_t GetPeerState() {
            return mState; 
        }
        void SetPeerState(state_t state) {
            mState = state; 
        }
        u_int16_t GetHoldtime() { 
            return holdtime;
        }
        void SetHoldtime(u_int16_t ht) {
            holdtime = ht;
        }

        void TimerBeZero() {
            HoldTimer = 0;
            KeepaliveTimer = 0;
            IdleHoldTimer = 0;
        }

        void StartTimerHoldtime();
        void StartTimerKeepalive();
        void InitWbuf();
};

#endif /* end of include guard: PEER_V3V7QZZV */
