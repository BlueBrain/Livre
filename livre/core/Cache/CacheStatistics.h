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

#include <livre/core/Cache/CacheObjectObserver.h>
#include <lunchbox/atomic.h>
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
     * @return Total number of objects in the corresponding \see Cache.
     */
    uint32_t getBlockCount( ) const { return totalBlockCount_; }

    /**
     * @return Total memory in MB used by the \see Cache.
     */
    uint32_t getUsedMemory( ) const { return totalMemoryUsed_; }

    /**
     * @param statisticsName The name of the statistics.
     */
    void setStatisticsName( const std::string& statisticsName )
        { statisticsName_ = statisticsName; }

    /** @param Maximum memory in MB used by the associated cache. */
    void setMaximumMemory( const uint32_t maxMemory )
        { maxMemory_ = maxMemory; }

    /**
     * @param stream Output stream.
     * @param cacheStatistics Input \see CacheStatistics
     * @return The output stream.
     */
    friend std::ostream& operator<<( std::ostream& stream,
                                     const CacheStatistics& cacheStatistics );

    ~CacheStatistics();

private:

    friend class Cache;

    CacheStatistics( const std::string& statisticsName,
                     const uint32_t queueSize );

    void onLoaded_( const CacheObject& cacheObject ) final;
    void onPreUnload_( const CacheObject& cacheObject ) final;

    void onCacheMiss_( const CacheObject& ) final { ++cacheMiss_; }
    void onCacheHit_( const CacheObject& ) final { ++cacheHit_; }

    lunchbox::Atomic< uint32_t > totalBlockCount_;
    lunchbox::Atomic< uint32_t > totalMemoryUsed_;

    std::string statisticsName_;
    uint32_t maxMemory_;

    lunchbox::Atomic< uint32_t > cacheHit_;
    lunchbox::Atomic< uint32_t > cacheMiss_;

    struct LoadInfo;
    typedef boost::shared_ptr< LoadInfo > LoadInfoPtr;
    typedef lunchbox::MTQueue< LoadInfoPtr > LoadInfoPtrQueue;

    LoadInfoPtrQueue ioQueue_;
    const uint32_t queueSize_;
};

}

#endif // _CacheStatistics_h_
