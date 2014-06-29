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

// 1 - malformed attribute list.
// 2 - unrecognized well-known attribute.
// 3 - missing well-known attribute.
// 4 - attribute flags error.
// 5 - attribute length error.
// 6 - invalid origin attribute
// 7 - as routing loop.
// 8 - invalid next_hop attribute.
// 9 - optional attribute error.
// 10 - invalid network field.
// 11 - malformed as_path.
typedef enum _suberr_update {
    ERR_UPDATE_MALATTRLIST = 1,
    ERR_UPDATE_URECOWELLKNOWNATTR,
    ERR_UPDATE_MISSWELLKONWATTR,
    ERR_UPDATE_ATTRFLAGERROR,
    ERR_UPDATE_ATTRLENERROR,
    ERR_UPDATE_INVALIDORIGIN,
    ERR_UPDATE_ASROUTINGLOOP,
    ERR_UPDATE_INVALIDNEXTLOOP,
    ERR_UPDATE_OPTIONALATTRERROR,
    ERR_UPDATE_INVALIDNETWORKFIELD,
    ERR_UPDATE_MALASPATH
} suberr_update;

#define FLAG_OPTIONAL   0x80
#define FLAG_TRANSITIVE 0x40
#define FLAG_PARTIAL    0x20
#define FLAG_EXTENDED   0x10

typedef enum _attr_code {
    PATHATTR_ORIGIN     = 1,
    PATHATTR_ASPATH     = 2,
    PATHATTR_NEXTHOP    = 3,
    PATHATTR_MED        = 4,
    PATHATTR_LOCALPREF  = 5
} attr_code;

typedef enum _origin_type {
    ORIGIN_IGP = 0,
    ORIGIN_BGP ,
    ORIGIN_INCOMPLETE 
} origin_t;

typedef enum _as_type {
    AS_SET  = 0,
    AS_SEQUENCE 
} as_t;

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

struct _prefix {
    u_int8_t        length;
    struct in_addr  prefix;
};

struct _path_attr_type {
    u_char          flag;
    u_char          typecode;
};

struct _as_path_segment {
    u_int8_t    type;
    u_int8_t    length;     // number of AS
    Buffer    * value;      // list of 2-octet value, in network order
};

struct _bgp_path_attr {
    u_int8_t                    origin;
    struct _as_path_segment     as_path;
    struct in_addr              nhop;
};

struct _bgp_update_info {
    vector<struct _prefix *>            withdraw;   // Withdrawn Routes
    struct _bgp_path_attr             * pathattr;   // Path Attributes
    vector<struct _prefix *>            nlri;       // Network Layer Reachability Information
};

#pragma pack(pop)   /* restore original alignment */

using namespace std;


class Message : public Buffer {
    private:

    public:
        sockfd      sfd;

        Message(int len);
        virtual ~Message();

        bool Write(sockfd, Message *);
};

extern std::map<message_t, string> mapMsgName;
#endif /* end of include guard: MESSAGE_DKT2MXIT */
