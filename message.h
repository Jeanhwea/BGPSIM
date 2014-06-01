#ifndef MESSAGE_DKT2MXIT

#define MESSAGE_DKT2MXIT

#include "global.h"

#define MAX_BACKLOG                 5
#define INTERVAL_CONNECTRETRY       120
#define INTERVAL_HOLD_INITIAL       240
#define INTERVAL_HOLD               90
#define INTERVAL_IDLE_HOLD_INITIAL  30
#define MAX_IDLE_HOLD               3600
#define MSGSIZE_HEADER              19
#define MSGSIZE_HEADER_MARKER       16
#define MSGSIZE_NOTIFICATION_MIN    21  /* 19 hdr + 1 code + 1 sub */
#define MSGSIZE_OPEN_MIN            29
#define MSGSIZE_UPDATE_MIN          23
#define MSGSIZE_KEEPALIVE           19  /* 19 hdr */

typedef enum {
    OPEN = 1,
    UPDATE,
    NOTIFICATION,
    KEEPALIVE
} message_t;

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

typedef enum _opt_params {
    OPT_PARAM_NONE,
    OPT_PARAM_AUTH,
    OPT_PARAM_CAPABILITIES
} opt_params;

typedef enum _capa_codes {
    CAPA_NONE,
    CAPA_MP,
    CAPA_REFRESH
} capa_codes;

#pragma pack(push)  /* push current align to stack */
#pragma pack(1)     /* set alignment to 1 byte */ 

using namespace std;

typedef struct _bgphdr {
    u_char      marker[16];           // Marker
    u_int16_t   length;               // Length
    u_int8_t    type;                 // Type
} bgphdr;

typedef struct _openmsg {
    bgphdr      msghdr;               // Message header
    u_int8_t    version;              // Version
    u_int16_t   myas;                 // My Autonomous System
    u_int16_t   holdtime;             // Hold Time
    u_int32_t   bgpid;                // BGP Identifier
    u_int8_t    optparamlen;          // Optional Parameters Length
} openmsg;


#pragma pack(pop)   /* restore original alignment */

using namespace std;

class Simulator;

class Message {
    public:
        Message ();
        virtual ~Message ();

        message_t GetMsgType() {
            return mType;
        }
        void SetMsgType(message_t ty) {
            mType = ty;
        }
        string MsgToString() {
            return mapMsgName[mType];
        }
        
        void InitHeader();
        void InitHeader(bgphdr & hdr);
        void InitOpenMsg(u_int16_t no, u_int16_t ht, u_int32_t ip);

        bool SendMsg(sockfd sfd);
        void DumpRawMsg(u_int8_t * buf, ssize_t size);
        void DumpSelf();

    private:
        message_t mType;
        bgphdr mHdr;
        u_int8_t bufMSG[4096];

        static std::map<message_t, string> mapMsgName;
};

#endif /* end of include guard: MESSAGE_DKT2MXIT */
