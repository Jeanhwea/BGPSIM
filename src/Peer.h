#ifndef PEER_V3V7QZZV

#define PEER_V3V7QZZV

#include "global.h"
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
    u_int32_t        groupid;
    char             group[PEER_DESCR_LEN];
    char             descr[PEER_DESCR_LEN];
    struct in_addr   remote_addr;
    struct in_addr   local_addr;
    u_int8_t         remote_masklen;
    u_int8_t         cloned;
    u_int32_t        max_prefix;
    u_int16_t        remote_as;
    u_int8_t         ebgp;      /* 1 = ebgp, 0 = ibgp */
    u_int8_t         distance;  /* 1 = direct, >1 = multihop */
    u_int8_t         passive;
    u_int16_t        holdtime;
    u_int16_t        min_holdtime;
} peer_config;

class Peer {
    private:
        u_int16_t       holdtime;

        state_t         mState;

        static map<state_t, string> mapStateName;
        
    public:
        sockfd          sfd;
        u_int32_t       remote_bgpid;
        peer_config     conf;
        time_t          ConnetRetryTimer;
        time_t          KeepaliveTimer;
        time_t          HoldTimer;
        time_t          IdleHoldTimer;
        time_t          IdleHoldResetTimer;
        time_t          IdleHoldTime;
        
        struct sockaddr_storage sa_local;
        struct sockaddr_storage sa_remote;
    
        Message       * rbuf;
        Message       * wbuf;

        Peer ();
        virtual ~Peer ();
    
        string PeerStateStr() {
            return mapStateName[mState];
        }

        state_t GetPeerState() {return mState; }
        void SetPeerState(state_t state) {mState = state; }
        u_int16_t GetHoldtime() { return holdtime;}
        void SetHoldtime(u_int16_t ht) {holdtime = ht;}

        void TimerBeZero() {
            HoldTimer = 0;
            KeepaliveTimer = 0;
            IdleHoldTimer = 0;
        }

        void StartTimerHoldtime();
        void StartTimerKeepalive();
};

#endif /* end of include guard: PEER_V3V7QZZV */
