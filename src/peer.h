#ifndef PEER_V3V7QZZV

#define PEER_V3V7QZZV

#include "global.h"
#include "message.h"

using namespace std;

typedef enum {
    IDLE,
    CONNECT,
    ACTIVE,
    OPEN_SENT,
    OPEN_CONFIRM,
    ESTABLISHED
} state_t;

class Peer {
    public:
        sockfd          sfd;
        time_t          ConnetRetryTimer;
        time_t          KeepaliveTimer;
        time_t          HoldTimer;
        time_t          IdleHoldTimer;
        time_t          IdleHoldResetTimer;
        time_t          IdleHoldTime;
        
        struct sockaddr_storage sa_local;
        struct sockaddr_storage sa_remote;
    
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

        bool ParseHeader();
        bool ParseOpen();
        bool ParseNotification();
        bool ParseUpdate();
        bool ParseKeepalive();

    private:
        u_int32_t       remote_bgpid;
        u_int16_t       holdtime;
        Message       * rbuf;
        Message       * wbuf;

        state_t         mState;

        static map<state_t, string> mapStateName;
};

#endif /* end of include guard: PEER_V3V7QZZV */
