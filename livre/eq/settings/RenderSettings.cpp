
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

namespace lunchbox
{
template<> inline void byteswap( livre::RendererType& value )
    { byteswap( reinterpret_cast< uint32_t& >( value )); }
}

namespace livre
{

RenderSettings::RenderSettings( )
    : rendererType_( RT_RAYCAST ),
      depth_( 0 )
{
    setDirty( DIRTY_RENDERER_TYPE );
}

const TransferFunction1D& RenderSettings::getTransferFunction( ) const
{
    return transferFunction_;
}

RendererType RenderSettings::getRendererType( ) const
{
    return rendererType_;
}

void RenderSettings::resetTransferFunction( )
{
    transferFunction_.reset( );
    setDirty( DIRTY_TF );
}

void RenderSettings::setTransferFunction( const TransferFunction1D& tf )
{
    transferFunction_ = tf;
    setDirty( DIRTY_TF );
}

void RenderSettings::setRendererType( const RendererType rendererType )
{
    rendererType_ = rendererType;
    setDirty( DIRTY_RENDERER_TYPE );
}

void RenderSettings::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_TF )
        os << transferFunction_;

    if( dirtyBits & DIRTY_RENDERER_TYPE )
        os << rendererType_;

    if( dirtyBits & DIRTY_DEPTH )
        os << depth_;

    co::Serializable::serialize( os, dirtyBits );
}

void RenderSettings::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    if( dirtyBits & DIRTY_TF )
        is >> transferFunction_;

    if( dirtyBits & DIRTY_RENDERER_TYPE )
        is >> rendererType_;

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
