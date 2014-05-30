#ifndef MESSAGE_DKT2MXIT

#define MESSAGE_DKT2MXIT

#include "global.h"
#include "simulator.h"

typedef unsigned char octet;
typedef unsigned short hexet;
typedef unsigned int dhexet;
#define VARMSGLEN 16
#define MAX_MSG_LEN 2048

typedef enum {
    OPEN = 1,
    UPDATE,
    NOTIFICATION,
    KEEPALIVE
} Message_t;

#pragma pack(push)  /* push current align to stack */
#pragma pack(1)     /* set alignment to 1 byte */ 

typedef struct _bgphdr {
    octet marker[16];           // Marker
    hexet length;               // Length
    octet type;                 // Type
} bgphdr;

typedef struct _openmsg {
    octet version;              // Version
    hexet asno;                 // My Autonomous System
    hexet hold_time;            // Hold Time
    dhexet bgp_ident;           // BGP Identifier
    octet op_para_len;          // Optional Parameters Length
    octet op_para[VARMSGLEN];   // Optional Parameters
} openmsg;

typedef struct _updatemsg {
    hexet ur_len;               // Unfeasible Routes Length
    octet wr[VARMSGLEN];        // Withdrawn Routes (variable)
    hexet tpatt_len;            // Total Path Attribute Length 
    octet patt[VARMSGLEN];      // Path Attribute (variable)
    octet info[VARMSGLEN];      // Network layer reachability information (variable)
} updatemsg;

typedef struct _keepalivemsg {  // keepalivemsg is zero bit defined by rfc1771
} keepalivemsg;

typedef struct _notificationmsg {
    octet err;                  // Error code
    octet err_sub;              // Error subcode
    octet data[VARMSGLEN];      // Data (variable)
} notificationmsg;

#pragma pack(pop)   /* restore original alignment */

using namespace std;

class Simulator;

class Message {
    public:
        Message ();
        virtual ~Message ();

        void SetMsgType(Message_t ty) { type = ty;}
        string MsgToString() {
            return mapMsgName[type];
        }
        
        void InitHeader();
        void InitOpenMsg(hexet no, hexet ht, dhexet id);
        void InitUpdateMsg(hexet urlen, hexet alen);
        void InitKeepaliveMsg();
        void InitNotificationMsg(octet er, octet erb);

        bool SendMsg(int sockfd);
        void DumpRawMsg(octet * buf, ssize_t size);
        void DumpSelf();

    private:
        Message_t type;
        bgphdr hdr;
        octet bufMSG[MAX_MSG_LEN];
        Simulator * sim;

        static std::map<Message_t, string> mapMsgName;
};

#endif /* end of include guard: MESSAGE_DKT2MXIT */
