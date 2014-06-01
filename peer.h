#ifndef PEER_V3V7QZZV

#define PEER_V3V7QZZV

#include "global.h"

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

        void InitTimer() {
            hold_timer = 0;
            connect_timer = 0;
            keepalive_timer = 0;
        }
    
    private:
        state_t mState;
        sockfd sfd;
        time_t hold_timer, connect_timer, keepalive_timer;

        static map<state_t, string> mapStateName;
};

#endif /* end of include guard: PEER_V3V7QZZV */
