#include <prnetdb.h>
#include "ByteBuffer.h"
#include "CondVar.h"
#include "Packet.h"
#include "Socket.h"
#include "WriterThread.h"


namespace pyr {

    WriterThread::WriterThread(Socket* socket) {
        _socket = socket;
        _outgoingLock = new Mutex();
        _packetsAvailable = new CondVar(_outgoingLock);
    }
    
    WriterThread::~WriterThread() {
        stop();
        delete _packetsAvailable;
        delete _outgoingLock;
    }
    
    void WriterThread::run() {
        while (!shouldQuit()) {
            _outgoingLock->lock();
            while (_outgoing.empty()) {
                _packetsAvailable->wait(1);
                if (shouldQuit()) {
                    break;
                }
            }
            if (shouldQuit()) {
                break;
            }
            
            // write a single packet
            Packet* p = _outgoing.front();
            _outgoing.pop();
            
            ByteBuffer out;
            p->serialize(out);
            
            u16 id = PR_htons(p->getID());
            u16 size = PR_htons(out.getSize());
            
            if (_socket->write(&id, sizeof(id)) != sizeof(id) ||
                _socket->write(&size, sizeof(size)) != sizeof(size) ||
                _socket->write(out.getBuffer(), out.getSize()) != out.getSize())
            {
                _outgoingLock->unlock();
                break;
            }
            
            _outgoingLock->unlock();
        }
    }
    
    void WriterThread::addPacket(Packet* packet) {
        std::vector<Packet*> packets(1);
        packets[0] = packet;
        addPackets(packets);
    }
    
    void WriterThread::addPackets(const std::vector<Packet*>& packets) {
        _outgoingLock->lock();
        
        for (unsigned i = 0; i < packets.size(); ++i) {
            _outgoing.push(packets[i]);
        }
        
        _packetsAvailable->notify();
        _outgoingLock->unlock();
    }

}