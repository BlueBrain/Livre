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
               const size_t cSize )
        : op( operation ), cacheSize( cSize )
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

CacheStatistics::CacheStatistics( const std::string& name,
                                  const size_t queueSize /* = CACHE_LOG_SIZE */ )
    : _name( name )
    , _usedMemBytes( 0 )
    , _maxMemBytes( 0 )
    , _objCount( 0 )
    , _cacheHit( 0 )
    , _cacheMiss( 0 )
    , _queueSize( queueSize )
{
}

void CacheStatistics::notifyLoaded( const CacheObject& cacheObject )
{
   ++_objCount;
   _usedMemBytes += cacheObject.getSize();

   if( _ioQueue.empty() )
       _ioQueue.push( LoadInfoPtr( new LoadInfo()) );

   if( _ioQueue.getSize() == _queueSize )
       _ioQueue.pop( );

   LoadInfoPtr previous;
   _ioQueue.getBack( previous );
   _ioQueue.push( LoadInfoPtr( new LoadInfo( *previous, LoadInfo::OP_LOAD,
                                             cacheObject.getSize( ))));
}

void CacheStatistics::notifyUnloaded( const CacheObject& cacheObject )
{
    --_objCount;
    _usedMemBytes -= cacheObject.getSize();

    if( _ioQueue.getSize() == _queueSize )
        _ioQueue.pop( );

    LoadInfoPtr previous;
    _ioQueue.getBack( previous );
    _ioQueue.push( LoadInfoPtr( new LoadInfo( *previous, LoadInfo::OP_UNLOAD,
                                              cacheObject.getSize( ))));
}

std::ostream& operator<<( std::ostream& stream,
                          const CacheStatistics& statistics )
{
    const int hits = int(
        100.f * float( statistics._cacheHit ) /
        float( statistics._cacheHit + statistics._cacheMiss ));
    stream << statistics._name << std::endl;
    stream << "  Used Memory: "
           << (statistics._usedMemBytes + LB_1MB - 1) / LB_1MB << "/"
           << (statistics._maxMemBytes + LB_1MB - 1) / LB_1MB << "MB"
           << std::endl;
    stream << "  Block Count: "
           << statistics._objCount << std::endl;
    stream << "  Cache hits: "
           << statistics._cacheHit << " (" << hits << "%)" << std::endl;
    stream << "  Cache misses: "
           << statistics._cacheMiss << std::endl;

    return stream;
}

CacheStatistics::~CacheStatistics()
{}

}
