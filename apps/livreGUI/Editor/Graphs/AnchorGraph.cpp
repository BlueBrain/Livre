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

#include <QMouseEvent>
#include <QPainter>

#include <livre/core/Render/TransferFunction1D.h>

#include <livreGUI/Editor/Graphs/AnchorGraph.h>

#define INNER_RADIUS 3
#define OUTER_RADIUS 6

namespace livre
{

AnchorGraph::AnchorGraph( AnchorGraphCorePtr anchorGraphCorePtr, QWidget *parentWgt )
    : TransferFunctionGraph( parentWgt ),
      graphCorePtr_( anchorGraphCorePtr ),
      lastPointOk_( false )
{}

AnchorGraph::~AnchorGraph( )
{}

void AnchorGraph::initialize( )
{
    graphCorePtr_->initialize( *transferFunctionPtr_ );
    curveUpdate( true );
}

void AnchorGraph::paint_( QPainter &painter )
{
    QBrush brush( Qt::lightGray );
    painter.setRenderHints( QPainter::Antialiasing | QPainter::HighQualityAntialiasing );
    painter.setPen( Qt::lightGray );
    painter.setBrush( brush );

    const FloatVector& tfData = transferFunctionPtr_->getData( );
    for ( uint32_t i = 0; i < transferFunctionPtr_->getNumChannels( ); ++i )
    {
        painter.setPen( colors_[ i ] );
        paintCurve_( painter, static_cast< ColorChannel >( i ), uint32_t( tfData.size( )) / transferFunctionPtr_->getNumChannels( ) );
        paintAnchors_( painter, static_cast< ColorChannel >( i ) );
    }
}

GraphCorePtr AnchorGraph::getGraphCore( ) const
{
    return graphCorePtr_;
}

void AnchorGraph::setGraphCore( GraphCorePtr graphCorePtr )
{
    lastPointOk_ = false;
    currentAnchors_.clear( );
    graphCorePtr_ = boost::dynamic_pointer_cast< AnchorGraphCore >( graphCorePtr );
}

const AnchorGraph::AnchorVector& AnchorGraph::getAnchorVector( const ColorChannel channel ) const
{
    return graphCorePtr_->getAnchorVector( channel);
}

const Vector2f &AnchorGraph::getAnchor( const ColorChannel channel, const uint32_t anchorIndex ) const
{
    return graphCorePtr_->getAnchor( channel, anchorIndex );
}

void AnchorGraph::setAnchor(const ColorChannel channel, const uint32_t anchorIndex, const Vector2f &anchor )
{
    graphCorePtr_->setAnchor( *transferFunctionPtr_, channel, anchorIndex, anchor ) ;
    curveUpdate( true );
}

void AnchorGraph::setAnchorVector( const ColorChannel channel, const AnchorVector &anchors )
{
      graphCorePtr_->setAnchorVector( *transferFunctionPtr_, channel, anchors );
      curveUpdate( true );
}

AnchorGraph::AnchorVector AnchorGraph::getCurve( const ColorChannel channel ) const
{
     return graphCorePtr_->getCurve( channel );
}

void AnchorGraph::setCurve(const ColorChannel channel, const AnchorVector &anchors )
{
      graphCorePtr_->setCurve( *transferFunctionPtr_, channel, anchors);
      curveUpdate(true);
}

void AnchorGraph::mouseReleaseEvent( QMouseEvent *mouseEvt )
{
    if ( mouseEvt->button( ) & Qt::LeftButton )
    {
        lastPointOk_ = false;
        currentAnchors_.clear( );
    }
}

void AnchorGraph::paintEvent( QPaintEvent *pntEvt LB_UNUSED )
{
    QPainter painter( this );
    paint_( painter );
}

void AnchorGraph::mouseMoveEvent( QMouseEvent *mouseEvt )
{
    if( !( mouseEvt->buttons( ) & Qt::LeftButton ) || !transferFunctionPtr_ )
        return;

    const int posX = mouseEvt->pos( ).x( );
    const int posZ = mouseEvt->pos( ).y( );

    Vector2f delta( static_cast<float>( posX - lastPoint_.x( ) ) / ( width( ) - 1 ),
                    static_cast<float>( posZ - lastPoint_.y( ) ) / ( height( ) - 1 ) );

    if( lastPointOk_ )
    {
        for( AnchorIndexPairVector::const_iterator it = currentAnchors_.begin( ); it != currentAnchors_.end( ); ++it )
        {
            Vector2f anchor = graphCorePtr_->getAnchor( it->second, it->first ) + delta;
            anchor.y( ) = 1.0f - anchor.y( );
            graphCorePtr_->setAnchor( *transferFunctionPtr_, it->second, it->first, anchor );
        }
    }

    lastPoint_ = mouseEvt->pos( );
    lastPointOk_ = true;

    if ( !currentAnchors_.empty( ) )
    {
        curveUpdate( true );
    }
    update( );
}

void AnchorGraph::mousePressEvent( QMouseEvent *mouseEvt )
{
    if ( !( mouseEvt->button( ) & Qt::LeftButton ) )
         return;

    lastPoint_ = mouseEvt->pos( );
    for ( uint32_t i = 0; i < transferFunctionPtr_->getNumChannels( ) ; ++i )
    {
        if( colorChannelEnabled_[ i ] )
        {
            const AnchorVector& anchorVector = graphCorePtr_->getAnchorVector( static_cast< ColorChannel >( i ) );
            uint32_t index = 0;
            for ( AnchorVector::const_iterator it = anchorVector.begin( ); it != anchorVector.end( ); ++it, ++index )
            {
                const QPoint& point = getTransformedPoint_( *it );
                if (lastPoint_.x( ) <= point.x( ) + OUTER_RADIUS &&
                    lastPoint_.x( ) >= point.x( ) - OUTER_RADIUS &&
                    lastPoint_.y( ) <= point.y( ) + OUTER_RADIUS &&
                    lastPoint_.y( ) >= point.y( ) - OUTER_RADIUS)
                {
                    currentAnchors_.push_back( AnchorIndex( index, static_cast< ColorChannel >( i ) ) );
                }
            }
       }
    }
    lastPointOk_ = true;
}

void AnchorGraph::paintAnchors_( QPainter &painter, const ColorChannel channel )
{
    QBrush brush( colors_[ channel ] );
    const AnchorVector& anchorVector = graphCorePtr_->getAnchorVector( channel );
    for ( AnchorVector::const_iterator it = anchorVector.begin( ); it != anchorVector.end( ); ++it )
    {
        painter.setBrush( brush);
        painter.drawEllipse( getTransformedPoint_( *it ), INNER_RADIUS, INNER_RADIUS );
        painter.setBrush( Qt::NoBrush );
        painter.drawEllipse(getTransformedPoint_( *it ), OUTER_RADIUS, OUTER_RADIUS);
    }
}

QPoint AnchorGraph::getTransformedPoint_( const Vector2f& point, const float xOffset, const float yOffset) const
{
    return QPoint( point.x( ) * ( width( ) - 1 ) + xOffset + 0.5f,
                   point.y( ) * ( height( ) - 1 ) + yOffset + 0.5f );
}

}
