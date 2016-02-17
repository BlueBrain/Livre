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
#include <livre/core/cache/CacheObjectObserver.h>
#include <lunchbox/mtQueue.h>

namespace livre
{
/**
 * The CacheStatistics struct keeps the statistics of the \see Cache.
 */
class CacheStatistics : public CacheObjectObserver
{
public:
    /**
     * @return Number of objects in the corresponding \see Cache.
     */
    size_t getBlockCount() const { return _objCount; }

    /**
     * @return Used memory in bytes used by the \see Cache.
     */
    size_t getUsedMemory() const { return _usedMemBytes; }

    /**
     * @param statisticsName The name of the statistics.
     */
    void setName( const std::string& statisticsName )
        { _name = statisticsName; }

    /** @param Maximum memory in bytes used by the associated cache. */
    void setMaximumMemory( const size_t maxMemBytes )
        { _maxMemBytes = maxMemBytes; }

    /**
     * @param stream Output stream.
     * @param cacheStatistics Input \see CacheStatistics
     * @return The output stream.
     */
    LIVRECORE_API friend std::ostream& operator<<( std::ostream& stream,
                                     const CacheStatistics& cacheStatistics );

    ~CacheStatistics();

private:

    friend class Cache;

    CacheStatistics( const std::string& statisticsName,
                     const size_t queueSize );

    void _onLoaded( const CacheObject& cacheObject ) final;
    void _onUnload( const CacheObject& cacheObject ) final;

    void _onCacheMiss( const CacheObject& ) final { ++_cacheMiss; }
    void _onCacheHit( const CacheObject& ) final { ++_cacheHit; }

    std::string _name;
    size_t _usedMemBytes;
    size_t _maxMemBytes;
    size_t _objCount;
    size_t _cacheHit;
    size_t _cacheMiss;

    struct LoadInfo;
    typedef boost::shared_ptr< LoadInfo > LoadInfoPtr;
    typedef lunchbox::MTQueue< LoadInfoPtr > LoadInfoPtrQueue;

    LoadInfoPtrQueue _ioQueue;
    const size_t _queueSize;
};

}

#endif // _CacheStatistics_h_
