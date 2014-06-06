#include "Event.h"

using namespace std;

map<event_t, string> Event::mapEventName = {
    { BGP_START, "bgp_start" },
    { BGP_STOP, "bgp_stop" },
    { BGP_TRANS_CONN_OPEN, "bgp_trans_conn_open" },
    { BGP_TRANS_CONN_CLOSED, "bgp_trans_conn_closed" },
    { BGP_TRANS_CONN_OPEN_FAILED, "bgp_trans_conn_open_failed" },
    { BGP_TRANS_FATAL_ERROR, "bgp_trans_fatal_error" },
    { CONN_RETRY_TIMER_EXPIRED, "conn_retry_timer_expired" },
    { HOLD_TIMER_EXPIRED, "hold_timer_expired" },
    { KEEPALIVE_TIMER_EXPIRED, "keepalive_timer_expired" },
    { RECV_OPEN_MSG, "recv_open_msg" },
    { RECV_KEEPALIVE_MSG, "recv_keepalive_msg" },
    { RECV_UPDATE_MSG, "recv_update_msg" },
    { RECV_NOTIFICATION_MSG, "recv_notification_msg" }
};

Event::Event()
{
}

Event::~Event() 
{

}
