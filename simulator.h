#ifndef SIMULATOR_SQ47GXYM

#define SIMULATOR_SQ47GXYM

#include "global.h"
#include "thread.h"
#include "event.h"
#include "peer.h"
#include "message.h"

using namespace std;

class Peer;
class Event;
class Message;

class Simulator : public Thread {
    public:
        Simulator ();
        virtual ~Simulator ();
        
        void * Run();

        void FSM(Peer * pPeer, Event * pEve);
        void ChangeState(Peer * pPeer, Event * pEve, state_t state);

        // to remove
        bool InitConn();
        void SendOpenMsg();

    private:
        Peer * mpPeer;
        Event * mpEve;

        // to remove
        Message * mpMsg;
        sockfd fd;

};


#endif /* end of include guard: SIMULATOR_SQ47GXYM */
