/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <livre/core/Cache/CacheStatistics.h>

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
               const uint32_t cSize,
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
    uint32_t cacheSize;
    uint32_t cumulativeCacheSize;
    uint32_t cumulativeNbNodes;
    double loadTime;
};

CacheStatistics::CacheStatistics( const std::string& statisticsName,
                                  const uint32_t queueSize )
    : totalBlockCount_( 0 ),
      totalMemoryUsed_( 0 ),
      statisticsName_( statisticsName ),
      cacheHit_( 0 ),
      cacheMiss_( 0 ),
      queueSize_( queueSize )
{
}

void CacheStatistics::onLoaded_( const CacheObject& cacheObject )
{
   ++totalBlockCount_;
   totalMemoryUsed_ += cacheObject.getCacheSize();

   if( ioQueue_.empty() )
       ioQueue_.push( LoadInfoPtr( new LoadInfo()) );

   if( ioQueue_.getSize() == queueSize_ )
       ioQueue_.pop( );

   LoadInfoPtr previous;
   ioQueue_.getBack( previous );
   ioQueue_.push( LoadInfoPtr( new LoadInfo( *previous, LoadInfo::OP_LOAD,
                                             cacheObject.getCacheSize( ),
                                             cacheObject.getLoadTime() ) ) );
}

void CacheStatistics::onPreUnload_( const CacheObject& cacheObject )
{
    --totalBlockCount_;
    totalMemoryUsed_ -= cacheObject.getCacheSize();

    if( ioQueue_.getSize() == queueSize_ )
        ioQueue_.pop( );

    LoadInfoPtr previous;
    ioQueue_.getBack( previous );
    ioQueue_.push( LoadInfoPtr( new LoadInfo( *previous, LoadInfo::OP_UNLOAD, cacheObject.getCacheSize() ) ) );
}

std::ostream& operator<<( std::ostream& stream, const CacheStatistics& cacheStatistics )
{
    stream << std::setiosflags( std::ios::fixed )
           << std::setprecision( 3 ) << "Total Block Count (" << cacheStatistics.statisticsName_ << "): "
           << cacheStatistics.totalBlockCount_ << std::endl;
    stream << std::setprecision( 3 ) << "Cache hit (" << cacheStatistics.statisticsName_ << "): "
           << cacheStatistics.cacheHit_ << std::endl;
    stream << std::setprecision( 3 ) << "Cache miss (" << cacheStatistics.statisticsName_ << "): "
           << cacheStatistics.cacheMiss_ << std::endl;
    stream <<  "Total Used Memory (" << cacheStatistics.statisticsName_ << "): "
           << cacheStatistics.totalMemoryUsed_ / ( 1024.0 * 1024.0 );

    return stream;
}

CacheStatistics::~CacheStatistics()
{

}

}
