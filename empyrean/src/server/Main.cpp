#include <stdexcept>
#include "Configuration.h"
#include "Error.h"
#include "ListenerThread.h"
#include "NSPRUtility.h"
#include "Server.h"
#include "Utility.h"
#include "World.h"


namespace pyr {

    void runServer() {
        World world;

        int port = Configuration::instance().getServerPort();
        ListenerThread* listener = new ListenerThread(port);
        ScopedPtr<Thread> thread(new Thread(listener, PR_PRIORITY_HIGH));
        
        float last = getNow();
        for (;;) {
            float now = getNow();
            float dt = now - last;
            last = now;
            
            std::vector<Connection*> connections;
            listener->getConnections(connections);
            for (unsigned i = 0; i < connections.size(); ++i) {
                world.addConnection(connections[i]);
            }
            
            world.update(dt);
            PR_Sleep(secondsToInterval(0.050f));
        }
    }
}


IMPLEMENT_APP(pyr::Server)
