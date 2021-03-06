/**
 * These classes were originally written for the Midworld project
 * in the ISU Game Developers Club.  Licensed under the LGPL.
 *
 * This file (c) Ben Scott
 */

#ifndef PYR_RESOURCE_MANAGER_H
#define PYR_RESOURCE_MANAGER_H


#include <map>
#include <string>
#include "LokiTypeInfo.h"
#include "Singleton.h"


namespace pyr {


    /**
     * Specializations must define the following static methods:
     * T create(const std::string& id);
     * void destroy(T value);
     */
    template<typename T>
    class CachePolicy;


    /**
     * This class is the base for all ResourceCache implementations.
     * It has a virtual destructor so implementations are destroyed
     * properly.
     */
    class ResourceCache {
    public:
        virtual ~ResourceCache() { }
        virtual void* get(const std::string& id) = 0;
    };

    
    template<typename T>
    class ResourceCacheImpl : public ResourceCache {
    public:
        ~ResourceCacheImpl() {
            typename ResourceMap::iterator itr = _cache.begin();
            for (; itr != _cache.end(); ++itr) {
                CachePolicy<T>::destroy(itr->second);
            }
        }
    
        void* get(const std::string& id) {
            if (_cache.count(id)) {
                return _cache[id];
            } else {
                T value = CachePolicy<T>::create(id);
                _cache[id] = value;
                return value;
            }
        }
        
    private:
        typedef std::map<std::string, T> ResourceMap;
        ResourceMap _cache;
    };


    class ResourceManager {
        PYR_DECLARE_SINGLETON(ResourceManager)
        
        ResourceManager() { }
        ~ResourceManager();

    public:
        /**
         * Returns the cached version of a resource, or creates a new
         * one if it isn't cached.  Throws std::runtime_error if it fails.
         */
        template<typename T>
        T get(const std::string& id) {
            return static_cast<T>(getCache<T>().get(id));
        }
        
    private:
        /**
         * Return the cache for a given resource type.  If it doesn't exist,
         * this function creates one.
         */
        template<typename T>
        ResourceCache& getCache() {
            TypeInfo typeT(typeid(T));
        
            ResourceCache* cache = _cacheMap[typeT];
            if (!cache) {
                cache = new ResourceCacheImpl<T>();
                _cacheMap[typeT] = cache;
            }
            return *cache;
        }
        
        typedef std::map<TypeInfo, ResourceCache*> CacheMap;
        CacheMap _cacheMap;
    };

}


#endif
