#ifndef SIMULATOR_SQ47GXYM

#define SIMULATOR_SQ47GXYM

#include "global.h"
#include "Event.h"
#include "Listener.h"
#include "Logger.h"
#include "Message.h"
#include "Peer.h"
#include "Thread.h"

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

typedef struct sim_config {
    int                 opts;
    u_int16_t           as;
    u_int32_t           bgpid;
    u_int16_t           holdtime;
    u_int16_t           min_holdtime;
    struct in_addrs *   listen_addrs;
} sim_config;

typedef enum {
    ERR_HEADER = 1,
    ERR_OPEN,
    ERR_UPDATE,
    ERR_HOLDTIMEREXPIRED,
    ERR_FSM,
    ERR_CEASE
} err_codes;

class Simulator : public Thread {

    private:
        // flags for Simulator
        bool            mQuit;

        vector<Peer *>  mvPeers;
        sim_config      conf;

        Listener        lis;
        Logger          log;

    public:
        Simulator ();
        virtual ~Simulator ();
        
        void * Run();

        void FSM(Peer *, event_t);
        void FSM(Peer *, Event *);
        void ChangeState(Peer *, state_t, event_t);

        void SimMain();
        // connection operation
        void SimTCPEstablished(Peer *);
        bool SimConnect(Peer *);
        void SimColseConnect(Peer *);
        // message handler
        void SimOpen(Peer *);
        void SimKeepalive(Peer *);
        void SimUpdate(Peer *, void *, ssize_t);
        void SimNotification(Peer *, u_int8_t, u_int8_t, void *, ssize_t);
        // message parser
        bool ParseHeader(Peer *, u_char &, u_int16_t &, u_int8_t &);
        bool ParseOpen(Peer *);
        bool ParseNotification(Peer *);
        bool ParseUpdate(Peer *);
        bool ParseKeepalive(Peer *);
        // socket helper
        bool SimSetupSocket(Peer *);
        bool SetBlock(sockfd sfd);
        bool UnsetBlock(sockfd sfd);

        Peer * GetPeerByAddr(struct in_addr *);

};


#endif /* end of include guard: SIMULATOR_SQ47GXYM */
