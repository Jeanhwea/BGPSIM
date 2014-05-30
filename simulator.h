#ifndef SIMULATOR_SQ47GXYM

#define SIMULATOR_SQ47GXYM

#include "global.h"
#include "event.h"
#include "message.h"
#include "thread.h"

using namespace std;

typedef enum {
    IDLE,
    CONNECT,
    ACTIVE,
    OPEN_SENT,
    OPEN_CONFIRM,
    ESTABLISHED
} State_t;

class Event;
class Message;

class Simulator : public Thread {
    public:
        Simulator ();
        virtual ~Simulator ();
        
        void * Run();

        void FSM(Event * pEve, Simulator * sim);
        void FSM(Event * pEve);
        void IdleStateHandler(Event * event);
        void ConnectStateHandler(Event * event);
        void ActiveStateHandler(Event * event);
        void OpenSendStateHandler(Event * event);
        void OpenConfirmStateHandler(Event * event);
        void EstablishedStateHandler(Event * event);

        string CurStateToString() {
            return mapStateName[curState];
        }
        State_t GetCurState() { 
            return curState; 
        }

        bool InitConn();
        void SendOpenMsg();

    private:
        State_t curState;
        Event * pEve;
        Message * pMsg;
        vector<int> vSockFd;
        // time_t hold_timer, connect_timer, keepalive_timer;

        static map<State_t, string> mapStateName;
};


#endif /* end of include guard: SIMULATOR_SQ47GXYM */
