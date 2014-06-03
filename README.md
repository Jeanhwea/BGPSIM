BGP Simulator
=============

Introduction
-------------
> I am writing this project for an simple BGP simulator, which is well-definded in [rfc1771](http://www.rfc-editor.org/rfc/rfc1771.txt "rfc1771")

Messge Format
--------------

### Messge Header Format


> Each message has a fixed-size header.  There may or may not be a data
> portion following the header, depending on the message type.  The
> layout of these fields is shown below:
>
>        0                   1                   2                   3
>        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
>        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
>        |                                                               |
>        +                                                               +
>        |                                                               |
>        +                                                               +
>        |                           Marker                              |
>        +                                                               +
>        |                                                               |
>        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
>        |          Length               |      Type     |
>        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

### OPEN Message Format


> After a transport protocol connection is established, the first
> message sent by each side is an OPEN message.  If the OPEN message is
> acceptable, a KEEPALIVE message confirming the OPEN is sent back.
> Once the OPEN is confirmed, UPDATE, KEEPALIVE, and NOTIFICATION
> messages may be exchanged.
>
> In addition to the fixed-size BGP header, the OPEN message contains
> the following fields:
>
>        0                   1                   2                   3
>        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
>        +-+-+-+-+-+-+-+-+
>        |    Version    |
>        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
>        |     My Autonomous System      |
>        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
>        |           Hold Time           |
>        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
>        |                         BGP Identifier                        |
>        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
>        | Opt Parm Len  |
>        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
>        |                                                               |
>        |                       Optional Parameters                     |
>        |                                                               |
>        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

### UPDATE Message Format


> UPDATE messages are used to transfer routing information between BGP
> peers.  The information in the UPDATE packet can be used to construct
> a graph describing the relationships of the various Autonomous
> Systems.  By applying rules to be discussed, routing information
> loops and some other anomalies may be detected and removed from
> inter-AS routing.

> An UPDATE message is used to advertise a single feasible route to a
> peer, or to withdraw multiple unfeasible routes from service (see
> 3.1). An UPDATE message may simultaneously advertise a feasible route
> and withdraw multiple unfeasible routes from service.  The UPDATE
> message always includes the fixed-size BGP header, and can optionally
> include the other fields as shown below:
>
>        +-----------------------------------------------------+
>        |   Unfeasible Routes Length (2 octets)               |
>        +-----------------------------------------------------+
>        |  Withdrawn Routes (variable)                        |
>        +-----------------------------------------------------+
>        |   Total Path Attribute Length (2 octets)            |
>        +-----------------------------------------------------+
>        |    Path Attributes (variable)                       |
>        +-----------------------------------------------------+
>        |   Network Layer Reachability Information (variable) |
>        +-----------------------------------------------------+

### KEEPALIVE Message Format


> BGP does not use any transport protocol-based keep-alive mechanism to
> determine if peers are reachable.  Instead, KEEPALIVE messages are
> exchanged between peers often enough as not to cause the Hold Timer
> to expire.  A reasonable maximum time between KEEPALIVE messages
> would be one third of the Hold Time interval.  KEEPALIVE messages
> MUST NOT be sent more frequently than one per second.  An
> implementation MAY adjust the rate at which it sends KEEPALIVE
> messages as a function of the Hold Time interval.
> 
> If the negotiated Hold Time interval is zero, then periodic KEEPALIVE
> messages MUST NOT be sent.
> 
> KEEPALIVE message consists of only message header and has a length of
> 19 octets.


### NOTIFICATION Message Format


> A NOTIFICATION message is sent when an error condition is detected.
> The BGP connection is closed immediately after sending it.
> 
> In addition to the fixed-size BGP header, the NOTIFICATION message
> contains the following fields:
>
>        0                   1                   2                   3
>        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
>        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
>        | Error code    | Error subcode |           Data                |
>        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               +
>        |                                                               |
>        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

Reference
------------------------
[1] : http://www.rfc-editor.org/rfc/rfc1771.txt "rfc1771"