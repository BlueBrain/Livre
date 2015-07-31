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

#include <QPainter>

#include <livreGUI/editor/TransferFunctionEditor.h>

#include <livreGUI/editor/graphs/TransferFunctionGraph.h>

namespace livre
{

TransferFunctionGraph::TransferFunctionGraph( QWidget *parentWgt ):
    QWidget( parentWgt )
{
    QPalette pal;
    pal.setColor( QPalette::Background, Qt::white );
    setPalette( pal );
    setBackgroundRole( QPalette::Window );
    setAutoFillBackground( true );
}

TransferFunctionGraph::~TransferFunctionGraph( )
{}

void TransferFunctionGraph::paintCurve_( QPainter& painter,
                                         const ColorChannel channel,
                                         const uint32_t count )
{
    QPolygon line;
    line.resize( count );

    const float xFactor = static_cast<float>( width( ) - 1 ) / ( count - 1 );
    const float yFactor = height( ) - 1;

    const FloatVector& tfData = transferFunctionPtr_->getData( );
    for ( uint32_t i = 0; i < count; ++i )
    {
        const size_t dataPos = i * transferFunctionPtr_->getNumChannels() + channel;
        line.setPoint( i,
                       static_cast< int >( xFactor * i ),
                       height( ) - 1 - ( static_cast< int >( yFactor * tfData[ dataPos ] ) ) );
    }
    painter.drawPolyline( line );
}

TransferFunction1DfPtr TransferFunctionGraph::getTransferFunction( ) const
{
    return transferFunctionPtr_;
}

void TransferFunctionGraph::setTransferFunction( TransferFunction1DfPtr transferFunctionPtr)
{
    transferFunctionPtr_ = transferFunctionPtr;

    colors_.resize( transferFunctionPtr_->getNumChannels( ) );
    colorChannelEnabled_.resize( transferFunctionPtr_->getNumChannels( ) );

    colors_[ 0 ] = Qt::red;
    colors_[ 1 ] = Qt::green;
    colors_[ 2 ] = Qt::blue;
    colors_[ 3 ] = Qt::black;

    initialize( );
}

void TransferFunctionGraph::setHistogram( const UInt8Vector& histogram )
{
    histogram_ = histogram;
}

void TransferFunctionGraph::setStateR( const bool checked )
{
    if( !transferFunctionPtr_ )
        return;

    colorChannelEnabled_[ CC_RED ] = checked;
}

void TransferFunctionGraph::setStateG( const bool checked )
{
    if( !transferFunctionPtr_ )
        return;

    colorChannelEnabled_[ CC_GREEN ] = checked;
}

void TransferFunctionGraph::setStateB( const bool checked )
{
    if( !transferFunctionPtr_ )
        return;

    colorChannelEnabled_[ CC_BLUE ] = checked;
}

void TransferFunctionGraph::setStateA( const bool checked )
{
    if( !transferFunctionPtr_ )
        return;

    colorChannelEnabled_[ CC_ALPHA ] = checked;
}

QPoint TransferFunctionGraph::getClippedPoint_( const QPoint& point ) const
{
  return QPoint(std::max( 0, std::min( width( ) - 1, point.x( ) ) ),
                std::max( 0, std::min( height( ) - 1, point.y( ) ) ) );
}


}
