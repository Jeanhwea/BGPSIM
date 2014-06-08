#ifndef EVENT_7QDU12AL

#define EVENT_7QDU12AL

#include "global.h"

using namespace std;


class Event {
    private:
        event_t mType;

        static std::map<event_t, string> mapEventName;

    public:
        Event ();
        virtual ~Event ();

        event_t GetEventType() {
            return mType;
        }
        void SetEventType(event_t type) {
            mType = type;
        }

};

#endif /* end of include guard: EVENT_7QDU12AL */
