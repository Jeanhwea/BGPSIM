#ifndef SIMULATOR_SQ47GXYM

#define SIMULATOR_SQ47GXYM

#include "global.h"
#include "thread.h"
#include "event.h"
#include "peer.h"
#include "message.h"

#define MAX_BACKLOG                 5
#define MAX_IDLE_HOLD               3600
#define T_CONNRETRY                 120
#define T_HOLD_INITIAL              240
#define T_HOLD                      90
#define T_IDLE_HOLD_INITIAL         30

using namespace std;

class Peer;
class Event;
class Message;

typedef enum {
    ERR_HEADER = 1,
    ERR_OPEN,
    ERR_UPDATE,
    ERR_HOLDTIMEREXPIRED,
    ERR_FSM,
    ERR_CEASE
} err_codes;

class Simulator : public Thread {
    public:
        Simulator ();
        virtual ~Simulator ();
        
        void * Run();

        void FSM(Peer *, Event *);
        void ChangeState(Peer *, state_t, Event *);

        // to remove
        bool InitConn();
        void SendOpenMsg();

        void SimTCPEstablished(Peer *);
        void SimConnect(Peer *);
        void SimColseConnect(Peer *);
        
        void SimOpen(Peer *);
        void SimKeepalive(Peer *);
        void SimUpdate(Peer *, void *, ssize_t);
        void SimNotification(Peer *, u_int8_t, u_int8_t, void *, ssize_t);

    private:
        Peer * mpPeer;
        Event * mpEve;

        // to remove
        Message * mpMsg;
        sockfd fd;

};


#endif /* end of include guard: SIMULATOR_SQ47GXYM */
