
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */

//TODO: clean up

#ifndef MASS_VOL__TENSOR_PARAMETERS_H
#define MASS_VOL__TENSOR_PARAMETERS_H

#include <co/dataIStream.h>
#include <co/dataOStream.h>
#include <msv/types/types.h> // byte

namespace massVolVis
{

/**
 * Parameters for tensor data decompression
 */
class TensorParameters
{
    static const size_t N_RANKS = 10;
    
public:
    std::vector<byte>   ranks;
    
    TensorParameters():
        ranks( N_RANKS, 32 )
    {}

    size_t size() const { return ranks.size(); }

    byte getRank( uint32_t level ) const
    {
        return level < size() ? ranks[ level ] : ranks[ size() - 1 ];
    }
};

} //namespace massVolVis

namespace lunchbox
{
    template<> inline void byteswap( massVolVis::TensorParameters& value )
    {
        for( size_t i = 0; i < value.size(); ++i ) byteswap(value.ranks[i]);
    }
}

#endif //MASS_VOL__TENSOR_PARAMETERS_H

