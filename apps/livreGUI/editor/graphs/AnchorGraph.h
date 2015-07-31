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

#ifndef _AnchorGraph_h_
#define _AnchorGraph_h_

#include <QPoint>
#include <QColor>
#include <QWidget>

#ifndef Q_MOC_RUN
#include <livreGUI/qtTypes.h>
#include <livreGUI/editor/graphcore/AnchorGraphCore.h>
#include <livreGUI/editor/graphs/TransferFunctionGraph.h>
#endif // Q_MOC_RUN

namespace livre
{

/**
 * Anchored graph widget, where the transfer function is defined by a modifiable anchors.
 **/
class AnchorGraph: public TransferFunctionGraph
{
    Q_OBJECT

public:

    typedef AnchorGraphCore::AnchorVector AnchorVector; //!< Anchor array definition.

    /**
     * @param anchorGraphCorePtr Operational part of the widget.
     * @param parentWgt Parent widget.
     */
    AnchorGraph( AnchorGraphCorePtr anchorGraphCorePtr, QWidget *parentWgt = 0 );

    virtual ~AnchorGraph( );

    /**
     * Initializes the graph.
     */
    virtual void initialize( );

    /**
     * @return The operation part of the widget.
     */
    virtual GraphCorePtr getGraphCore( ) const;

    /**
     * Sets the graph core.
     * @param graphCore Operational part of the graph widget.
     */
    virtual void setGraphCore( GraphCorePtr graphCorePtr );

    /**
     * @param channel Color channel.
     * @return The anchors for the color channel.
     */
    const AnchorVector& getAnchorVector( const ColorChannel channel ) const;

    /**
     * @param channel Color channel.
     * @param anchorIndex Anchor index.
     * @return The anchox x,y position.
     */
    const Vector2f& getAnchor( const ColorChannel channel, uint32_t anchorIndex ) const;

    /**
     * Sets the anchor for a color channel and for an index.
     * @param channel Color channel.
     * @param anchorIndex Index of the anchor.
     * @param anchor The source anchor.
     */
    void setAnchor( const ColorChannel channel, uint32_t anchorIndex, const Vector2f &anchor );

    /**
     * Sets the anchors for a color channel.
     * @param channel Color channel.
     * @param anchors Anchor array.
     */
    void setAnchorVector( const ColorChannel channel, const AnchorVector& anchors );

    /**
     * @param channel Color channel.
     * @return The transformed anchors.
     */
    AnchorVector getCurve( const ColorChannel channel ) const;

    /**
     * Sets the anchor array from transformed anchors.
     * @param channel Color channel.
     * @param anchors Transformed anchors.
     */
    void setCurve( const ColorChannel channel, const AnchorVector& anchors );

private:

    typedef std::pair< uint32_t, ColorChannel > AnchorIndex;
    typedef std::vector< AnchorIndex > AnchorIndexPairVector;

    virtual void mouseMoveEvent( QMouseEvent *mouseEvt );
    virtual void mousePressEvent( QMouseEvent *mouseEvt );
    virtual void mouseReleaseEvent( QMouseEvent *mouseEvt);
    virtual void paintEvent( QPaintEvent *pntEvt );

    QPoint getTransformedPoint_( const Vector2f &point,
                                 const float xOffset = 0.0f,
                                 const float yOffset = 0.0f ) const;

    void paintAnchors_( QPainter &painter, const ColorChannel channel );
    void paint_( QPainter &painter );

    AnchorIndexPairVector currentAnchors_;
    AnchorGraphCorePtr graphCorePtr_;
    QPoint lastPoint_;
    bool lastPointOk_;
};

}

#endif //_AnchorGraph_h_
