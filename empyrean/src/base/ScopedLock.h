#ifndef PYR_SCOPED_LOCK_H
#define PYR_SCOPED_LOCK_H


#include "Mutex.h"
#include "Utility.h"


namespace pyr {

    class ScopedLock {
    public:
        ScopedLock(Mutex* m) {
            _mutex = m;
            _mutex->lock();
        }
    
        ScopedLock(Mutex& m) {
            _mutex = &m;
            _mutex->lock();
        }
        
        ~ScopedLock() {
            _mutex->unlock();
        }

    private:    
        Mutex* _mutex;
    };
        
    #define PYR_SYNCHRONIZE_SCOPE(mutex)        \
        ScopedLock PYR_UNIQUE_NAME()(mutex)
        
    #define PYR_SYNCHRONIZED(mutex, block) {    \
        PYR_SYNCHRONIZE_SCOPE(mutex);           \
        block                                   \
    }

}


#endif
