#include <vector>
#include "Configuration.h"
#include "Connection.h"
#include "ListenerThread.h"
#include "Log.h"
#include "NSPRUtility.h"
#include "ServerThread.h"
#include "World.h"


namespace pyr {

    void ServerThread::run() {
        World world;

        int port = Configuration::instance().getServerPort();
        
        /// @todo IS THIS A LEAK?
        ListenerThread* listener = new ListenerThread(port);
        ScopedPtr<Thread> thread(new Thread(listener, PR_PRIORITY_HIGH));
        
        char str[80];
        sprintf(str, "%d", port);
        logMessage("Listening on port " + std::string(str));
        
        float last = getNow();
        while (!shouldQuit()) {
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
