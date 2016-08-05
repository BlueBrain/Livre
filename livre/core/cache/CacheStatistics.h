/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
 *
 * This file is part of Livre <https://github.com/BlueBrain/Livre>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _CacheStatistics_h_
#define _CacheStatistics_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <lunchbox/mtQueue.h>

#define CACHE_LOG_SIZE 1000000

namespace livre
{
/**
 * The CacheStatistics struct keeps the statistics of the \see Cache.
 */
class CacheStatistics
{
public:

    /**
     * Constructor
     * @param name of the cache statistics
     * @param maxMemBytes maximum memory.
     */
    LIVRECORE_API CacheStatistics( const std::string& name, size_t maxMemBytes );

    LIVRECORE_API ~CacheStatistics();

    /**
     * @return Number of objects in the corresponding \see Cache.
     */
    LIVRECORE_API size_t getBlockCount() const { return _objCount; }

    /**
     * @return Used memory in bytes used by the \see Cache.
     */
    LIVRECORE_API size_t getUsedMemory() const { return _usedMemBytes; }

    /**
     * @return Max memory in bytes used by the \see Cache.
     */
    LIVRECORE_API size_t getMaximumMemory() const { return _maxMemBytes; }

    /**
     * @return the name of the statistics
     */
    LIVRECORE_API std::string getName() const { return _name; }

    /**
     * Notifies the statistics for cache misses
     */
    void notifyMiss() { ++_cacheMiss; }

    /**
     * Notifies the statistics for cache hits
     */
    void notifyHit() { ++_cacheHit; }

    /**
     * Notifies statistics when an object is loaded.
     * @param cacheObject is the cache object.
     */
    LIVRECORE_API void notifyLoaded( const CacheObject& cacheObject );

    /**
     * Notifies statistics when an object is unloaded.
     * @param cacheObject is the cache object.
     */
    LIVRECORE_API void notifyUnloaded( const CacheObject& cacheObject );

    /**
      * Clears the statistics
      */
    LIVRECORE_API void clear();

    /**
     * @param stream Output stream.
     * @param cacheStatistics Input \see CacheStatistics
     * @return The output stream.
     */
    LIVRECORE_API friend std::ostream& operator<<( std::ostream& stream,
                                                   const CacheStatistics& statistics );

private:

    std::string _name;
    size_t _usedMemBytes;
    const size_t _maxMemBytes;
    size_t _objCount;
    size_t _cacheHit;
    size_t _cacheMiss;  
};

}

#endif // _CacheStatistics_h_
