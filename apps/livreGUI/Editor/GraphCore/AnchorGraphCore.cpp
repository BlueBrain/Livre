/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                     Philipp Schlegel <schlegel@ifi.uzh.ch>
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

#include <livreGUI/Editor/GraphCore/AnchorGraphCore.h>

namespace livre
{


void AnchorGraphCore::initialize( TransferFunction1Df& transferFunction )
{
    for ( uint32_t i = 0; i < transferFunction.getNumChannels( ); ++i)
    {
        updateCurve_( transferFunction, static_cast< ColorChannel >( i ) );
    }
}

void AnchorGraphCore::setAnchorVector( TransferFunction1Df& transferFunction,
                                       const ColorChannel channel,
                                       const AnchorVector& anchors )
{
    AnchorVector& channelAnchors = anchorLists_[ channel ];
    AnchorVector::iterator it = channelAnchors.begin( );

    for( AnchorVector::const_iterator posIt = anchors.begin( ); posIt != anchors.end( ); ++posIt, ++it )
    {
        Vector2f& dest = *it;
        const Vector2f& src = *posIt;
        dest = Vector2f( src.x( ), 1.0f - src.y( ) );
    }
    updateCurve_( transferFunction, channel );
}


void AnchorGraphCore::updateCurve_( TransferFunction1Df& transferFunction, const ColorChannel channel )
{
     writeCurve_( transferFunction, channel, anchorLists_[ channel ] );
}

const AnchorGraphCore::AnchorVector &AnchorGraphCore::getAnchorVector( const ColorChannel channel ) const
{
    return anchorLists_[ channel ];
}

const Vector2f& AnchorGraphCore::getAnchor( const ColorChannel channel, const uint32_t index ) const
{
    return anchorLists_[ channel ][ index ];
}


}
