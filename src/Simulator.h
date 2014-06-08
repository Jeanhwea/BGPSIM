#ifndef SIMULATOR_SQ47GXYM

#define SIMULATOR_SQ47GXYM

#include "global.h"
#include "Event.h"
#include "Listener.h"
#include "Logger.h"
#include "Message.h"
#include "Peer.h"
#include "Thread.h"
#include "Timer.h"

#define T_CONNRETRY                 120
#define T_HOLD_INITIAL              240

using namespace std;

struct sim_config {
    u_int16_t           as;
    u_int32_t           bgpid;
    struct in_addr      ipaddr;
};

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
        bool                        mQuit;
        vector<struct sim_config>   vPeerConf;
        u_int16_t                   conf_as;
        u_int16_t                   conf_holdtime;
        u_int16_t                   conf_bgpid;
        struct in_addrs *           listen_addrs;

        Listener                    lis;
        Timer                       tim;

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
        bool LoadSimConf(const char * filename);

};


#endif /* end of include guard: SIMULATOR_SQ47GXYM */
