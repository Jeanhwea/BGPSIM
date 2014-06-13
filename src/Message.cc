#include "Message.h"
#include "Logger.h"

using namespace std;

map<message_t, string> mapMsgName = {
    { OPEN, "open" },
    { UPDATE, "update" },
    { NOTIFICATION, "notification" },
    { KEEPALIVE, "keepalive" }
};

Message::Message(int len)
: Buffer(len)
{

}

Message::~Message()
{

}

