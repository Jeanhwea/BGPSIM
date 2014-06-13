#ifndef MESSAGE_DKT2MXIT

#define MESSAGE_DKT2MXIT

#include "global.h"
#include "Buffer.h"


#define MSGSIZE_HEADER              19
#define MSGSIZE_HEADER_MARKER       16
#define MSGSIZE_NOTIFICATION_MIN    21  /* 19 hdr + 1 code + 1 sub */
#define MSGSIZE_OPEN_MIN            29
#define MSGSIZE_UPDATE_MIN          23
#define MSGSIZE_KEEPALIVE           19  /* 19 hdr */
#define MSGSIZE_MAX                 4096

typedef enum {
    OPEN = 1,
    UPDATE,
    NOTIFICATION,
    KEEPALIVE
} message_t;

typedef enum {
    ERR_HEADER = 1,
    ERR_OPEN,
    ERR_UPDATE,
    ERR_HOLDTIMEREXPIRED,
    ERR_FSM,
    ERR_CEASE
} err_codes;

typedef enum _suberr_header {
    ERR_HDR_SYNC = 1,
    ERR_HDR_LEN,
    ERR_HDR_TYPE
} suberr_header;

typedef enum _suberr_open {
    ERR_OPEN_VERSION = 1,
    ERR_OPEN_AS,
    ERR_OPEN_BGPID,
    ERR_OPEN_OPT,
    ERR_OPEN_AUTH,
    ERR_OPEN_HOLDTIME,
    ERR_OPEN_CAPA
} suberr_open;

#pragma pack(push)  /* push current align to stack */
#pragma pack(1)     /* set alignment to 1 byte */

using namespace std;

struct bgphdr {
    u_char          marker[16];           // Marker
    u_int16_t       length;               // Length
    u_int8_t        type;                 // Type
};

struct openmsg {
    struct bgphdr   msghdr;               // Message header
    u_int8_t        version;              // Version
    u_int16_t       myas;                 // My Autonomous System
    u_int16_t       holdtime;             // Hold Time
    u_int32_t       bgpid;                // BGP Identifier
    u_int8_t        optparamlen;          // Optional Parameters Length
};


#pragma pack(pop)   /* restore original alignment */

using namespace std;


class Message : public Buffer {
    private:
        static deque<Message *> mqMessage;

    public:
        sockfd      sfd;

        Message(int len);
        virtual ~Message();

};

extern std::map<message_t, string> mapMsgName;
#endif /* end of include guard: MESSAGE_DKT2MXIT */
