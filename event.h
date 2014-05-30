#ifndef EVENT_7QDU12AL

#define EVENT_7QDU12AL

#include "global.h"
#include "simulator.h"
#include "message.h"

using namespace std;

typedef enum {
    BGP_START,
    BGP_STOP,
    BGP_TRANS_CONN_OPEN,
    BGP_TRANS_CONN_CLOSED,
    BGP_TRANS_CONN_OPEN_FAILED,
    BGP_TRANS_FATAL_ERROR,
    CONN_RETRY_TIMER_EXPIRED,
    HOLD_TIMER_EXPIRED,
    KEEPALIVE_TIMER_EXPIRED,
    RECV_OPEN_MSG,
    RECV_KEEPALIVE_MSG,
    RECV_UPDATE_MSG,
    RECV_NOTIFICATION_MSG
} Event_t;

class Simulator;
class Message;

class Event {
    public:
        Event ();
        virtual ~Event ();

        // void BgpStart();
        // void BgpStop();
        // void BgpTransConnOpen();
        // void BgpTransConnClosed();
        // void BgpTransConnOpenFailed();
        // void BgpTransFatalError();
        // void ConnRetryTimerExpired();
        // void HoldTimerExpired();
        // void KeepaliveTimerExpired();
        // void RecvOpenMsg();
        // void RecvKeepaliveMsg();
        // void RecvUpdateMsg();
        // void RecvNotificationMsg();

        string EventToString() {
            return mapEventName[type];
        }
        Event_t GetEventType() { 
            return type; 
        }
        Simulator * GetSimulator() {
            return sim;
        }

    private:
        Event_t type;
        Simulator * sim;
        Message * msg;

        static std::map<Event_t, string> mapEventName;
};

#endif /* end of include guard: EVENT_7QDU12AL */
