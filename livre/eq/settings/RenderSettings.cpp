
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
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

#include <livre/eq/settings/RenderSettings.h>

namespace livre
{

RenderSettings::RenderSettings( )
    :  depth_( 0 )
{
    transferFunction_.registerDeserializedCallback( [this]
        { setDirty( DIRTY_TF ); });
}

void RenderSettings::resetTransferFunction( )
{
    setTransferFunction( TransferFunction1D( ));
}

void RenderSettings::setTransferFunction( const TransferFunction1D& tf )
{
    transferFunction_ = tf;
    setDirty( DIRTY_TF );
}

void RenderSettings::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_TF )
        os << transferFunction_;

    if( dirtyBits & DIRTY_DEPTH )
        os << depth_;

    co::Serializable::serialize( os, dirtyBits );
}

void RenderSettings::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_TF )
        is >> transferFunction_;

    if( dirtyBits & DIRTY_DEPTH )
        is >> depth_;

    co::Serializable::deserialize( is, dirtyBits );
}

void RenderSettings::setMaxTreeDepth( const uint8_t depth )
{
    depth_ = depth;
    setDirty( DIRTY_DEPTH );
}

uint8_t RenderSettings::getMaxTreeDepth( ) const
{
    return depth_;
}

}
