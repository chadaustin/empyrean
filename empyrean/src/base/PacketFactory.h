#ifndef PYR_PACKET_FACTORY_H
#define PYR_PACKET_FACTORY_H


#include <map>
#include "LokiTypeInfo.h"
#include "Utility.h"


namespace pyr {

    class Packet;

    class PacketFactory {
    public:
        PacketFactory();
        
        Packet* create(int id, int size, void* buffer);
        
    private:
        typedef Packet* (*Factory)(int size, const void* buffer);
    
        template<typename T>
        void registerType(Type2Type<T> = Type2Type<T>()) {
            _factoryMap[T::ID] = T::create;
        }
        
        std::map<int, Factory> _factoryMap;
    };

}


#endif