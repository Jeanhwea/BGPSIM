#include "main.h"

using namespace std;

int 
main(int argc, char const *argv[])
{
    if (isDebug) printf("Debug is on ... ...\n");
    Timer tim;
    Simulator sim;
    Listener lis;
    Event eve;
    Message msg;

    tim.Start();
    tim.Join();
    if (!lis.SetPromisc("eth0")) 
        cout << "can not set promisc" << endl;
            

    sim.InitConn();
    sim.SendOpenMsg();
    lis.Start();
    lis.Join();
    // cout << eve.EventToString() << endl;

    return 0;
}
