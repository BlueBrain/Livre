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
    size_t getBlockCount() const { return blockCount_; }

    /**
     * @return Used memory in bytes used by the \see Cache.
     */
    size_t getUsedMemory() const { return usedMemoryInBytes_; }

    /**
     * @param statisticsName The name of the statistics.
     */
    void setStatisticsName( const std::string& statisticsName )
        { statisticsName_ = statisticsName; }

    /** @param Maximum memory in bytes used by the associated cache. */
    void setMaximumMemory( const size_t maxMemoryInBytes )
        { maxMemoryInBytes_ = maxMemoryInBytes; }

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

    void onLoaded_( const CacheObject& cacheObject ) final;
    void onUnload_( const CacheObject& cacheObject ) final;

    void onCacheMiss_( const CacheObject& ) final { ++cacheMiss_; }
    void onCacheHit_( const CacheObject& ) final { ++cacheHit_; }

    std::string statisticsName_;
    size_t usedMemoryInBytes_;
    size_t maxMemoryInBytes_;
    size_t blockCount_;
    size_t cacheHit_;
    size_t cacheMiss_;

    struct LoadInfo;
    typedef boost::shared_ptr< LoadInfo > LoadInfoPtr;
    typedef lunchbox::MTQueue< LoadInfoPtr > LoadInfoPtrQueue;

    LoadInfoPtrQueue ioQueue_;
    const size_t queueSize_;
};

}

#endif // _CacheStatistics_h_
