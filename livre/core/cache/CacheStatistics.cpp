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

#include <livre/core/cache/CacheStatistics.h>
#include <livre/core/cache/CacheObject.h>
#include <livre/core/util/ThreadClock.h>

namespace livre
{

struct CacheStatistics::LoadInfo
{
    enum Operation
    {
        OP_LOAD,
        OP_UNLOAD
    };

    LoadInfo( )
        : op( OP_LOAD ),
          cacheSize( 0 ),
          cumulativeCacheSize( 0 ),
          cumulativeNbNodes( 0 )
    {
        time = ThreadClock::getClock().getTimed();
    }

    LoadInfo(  const LoadInfo& previous,
               Operation operation,
               const size_t cSize,
               const float lTime = 0.0f )
        : op( operation ), cacheSize( cSize ), loadTime( lTime )
    {
        time = ThreadClock::getClock().getTimed();
        switch( op )
        {
        case OP_LOAD:
            cumulativeCacheSize = previous.cumulativeCacheSize + cacheSize;
            cumulativeNbNodes = previous.cumulativeNbNodes + 1;
            break;
        case OP_UNLOAD:
            cumulativeCacheSize = previous.cumulativeCacheSize - cacheSize;
            cumulativeNbNodes = previous.cumulativeNbNodes - 1;
            break;
        }
    }

    friend std::ostream& operator<<( std::ostream& stream, const LoadInfo& loadInfo )
    {
        stream << "Time: " << loadInfo.time
               << " Cumulative Nodes: " << loadInfo.cumulativeNbNodes
               << " Cumulative Size: " << loadInfo.cumulativeCacheSize;
        return stream;
    }

    Operation op;
    double time;
    size_t cacheSize;
    size_t cumulativeCacheSize;
    size_t cumulativeNbNodes;
    double loadTime;
};

CacheStatistics::CacheStatistics( const std::string& statisticsName,
                                  const size_t queueSize )
    : statisticsName_( statisticsName )
    , usedMemoryInBytes_( 0 )
    , maxMemoryInBytes_( 0 )
    , blockCount_( 0 )
    , cacheHit_( 0 )
    , cacheMiss_( 0 )
    , queueSize_( queueSize )
{
}

void CacheStatistics::onLoaded_( const CacheObject& cacheObject )
{
   ++blockCount_;
   usedMemoryInBytes_ += cacheObject.getCacheSize();

   if( ioQueue_.empty() )
       ioQueue_.push( LoadInfoPtr( new LoadInfo()) );

   if( ioQueue_.getSize() == queueSize_ )
       ioQueue_.pop( );

   LoadInfoPtr previous;
   ioQueue_.getBack( previous );
   ioQueue_.push( LoadInfoPtr( new LoadInfo( *previous, LoadInfo::OP_LOAD,
                                             cacheObject.getCacheSize(),
                                             cacheObject.getLoadTime( ))));
}

void CacheStatistics::onUnload_( const CacheObject& cacheObject )
{
    --blockCount_;
    usedMemoryInBytes_ -= cacheObject.getCacheSize();

    if( ioQueue_.getSize() == queueSize_ )
        ioQueue_.pop( );

    LoadInfoPtr previous;
    ioQueue_.getBack( previous );
    ioQueue_.push( LoadInfoPtr( new LoadInfo( *previous, LoadInfo::OP_UNLOAD,
                                              cacheObject.getCacheSize( ))));
}

std::ostream& operator<<( std::ostream& stream, const CacheStatistics& cacheStatistics )
{
    const int hits = int(
        100.f * float( cacheStatistics.cacheHit_ ) /
        float( cacheStatistics.cacheHit_ + cacheStatistics.cacheMiss_ ));
    stream << cacheStatistics.statisticsName_ << std::endl;
    stream << "  Used Memory: "
           << (cacheStatistics.usedMemoryInBytes_ + LB_1MB - 1) / LB_1MB << "/"
           << (cacheStatistics.maxMemoryInBytes_ + LB_1MB - 1) / LB_1MB << "MB"
           << std::endl;
    stream << "  Block Count: "
           << cacheStatistics.blockCount_ << std::endl;
    stream << "  Cache hits: "
           << cacheStatistics.cacheHit_ << " (" << hits << "%)" << std::endl;
    stream << "  Cache misses: "
           << cacheStatistics.cacheMiss_ << std::endl;

    return stream;
}

CacheStatistics::~CacheStatistics()
{

}

}
