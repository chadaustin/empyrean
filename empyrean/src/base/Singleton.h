#ifndef PYR_SINGLETON_H
#define PYR_SINGLETON_H


#include "Debug.h"
#include "Utility.h"


namespace pyr {

    /**
     * Declares a class 'name' to be a singleton.  Should be placed
     * in the header file.  Note that the default constructor
     * and destructor still need to be defined as private.  (This
     * leaves the responsibility of defining them up to the client
     * code in case custom behavior is desired.)
     */
    #define PYR_DECLARE_SINGLETON(name)     \
        public:                             \
            static name& instance();        \
        private:                            \
            name(const name& n);            \
            void operator=(const name& n);  \
            static void delete_instance();  \
            static name* _instance;         \
            static bool _deleted;
            
    /**
     * Defines implementation of singleton class.  Should be placed
     * in the source file.
     */
    #define PYR_DEFINE_SINGLETON(name)                                      \
        name* name::_instance = 0;                                          \
        bool name::_deleted = false;                                        \
        name& name::instance() {                                            \
            PYR_ASSERT(!_deleted, "Used singleton after it was deleted!");  \
            if (!_instance) {                                               \
                _instance = new name;                                       \
                atexit(delete_instance);                                    \
            }                                                               \
            return *_instance;                                              \
        }                                                                   \
        void name::delete_instance() {                                      \
            delete _instance;                                               \
            _instance = 0;                                                  \
            _deleted = true;                                                \
        }

    /**
     * Nifty function that lets you access a singleton with the following syntax:
     * the<Singleton>().method();
     *
     * delmoi: 'the' is the worst name for a function in the history of the world
     */
    template<typename T>
    T& the(Type2Type<T> = Type2Type<T>()) {
        return T::instance();
    }

}

#endif
