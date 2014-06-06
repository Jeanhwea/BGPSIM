#ifndef EVENT_7QDU12AL

#define EVENT_7QDU12AL

#include "global.h"

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
} event_t;


class Event {
    private:
        event_t mType;

        static std::map<event_t, string> mapEventName;
        
    public:
        Event ();
        virtual ~Event ();


        string EventTypeStr() {
            return mapEventName[mType];
        }
        event_t GetEventType() { 
            return mType; 
        }
        void SetEventType(event_t type) {
            mType = type;
        }

};

#endif /* end of include guard: EVENT_7QDU12AL */
