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

//#define T_CONNRETRY                 120
#define T_CONNRETRY                 10
#define T_HOLD_INITIAL              240
#define T_IDLE_INITIAL              60

using namespace std;

struct sim_config {
    u_int16_t           as;     // as number
    struct in_addr      raddr;  // remote ip address
};

class Simulator : public Thread {
    private:
        // flags for Simulator
        bool                        mQuit;
        vector<struct sim_config>   vPeerConf;
        vector<struct in_addr>      vLisAddr;

        u_int16_t                   conf_as;
        u_int16_t                   conf_holdtime;
        u_int32_t                   conf_bgpid;
        struct in_addr              lisaddr;

        Timer                       tim;

    public:
        Simulator();
        virtual ~Simulator();
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
        void SimUpdate(u_int32_t, void *, size_t);
        void SimNotification(Peer *, u_int8_t, u_int8_t, void *, ssize_t);
        // message parser
        bool ParseHeader(Peer *, u_char *, u_int16_t &, u_int8_t &);
        bool ParseOpen(Peer *);
        bool ParseNotification(Peer *);
        bool ParseUpdate(Peer *);
        bool ParseKeepalive(Peer *);
        // socket helper
        bool SimSetupSocket(Peer *);
        // peer blabla
        Peer * GetPeerByAddr(struct sockaddr_in * pSad);
        Peer * GetPeerByAddr(struct in_addr * pAd);
        Peer * GetPeerByAddr(u_int32_t bgpid);
        Peer * GetPeerBySockfd(sockfd fd);
        // read config file
        bool LoadSimConf(const char * filename);
        bool LoadListConf(const char * filename);

};


#endif /* end of include guard: SIMULATOR_SQ47GXYM */
