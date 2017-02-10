/* Copyright (c) 2015-2017, EPFL/Blue Brain Project
 *                          Marwan Abdellah <marwan.abdellah@epfl.ch>
 *                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#ifndef _ColorMapWidget_h_
#define _ColorMapWidget_h_

#include <livreGUI/types.h>
#include <lexis/render/Histogram.h>

#include <QImage>
#include <QLinearGradient>
#include <QWidget>

namespace livre
{

class HoverPoints;

/**
 * This class is used to create color curves with controls points. Each instance of
 * this class should be responsible for one of the RGBA component.
 */
class ColorMapWidget : public QWidget
{
    Q_OBJECT

public:

    enum class Channel
    {
        red,
        green,
        blue,
        alpha
    };

    ColorMapWidget( QWidget* parent, Channel channel );

    Channel getChannel() const { return _channel; }

    /**
     * Set the gradient stops.
     * @param stops The stop points for gradient calculation.
     */
    void setGradientStops( const QGradientStops& stops );

    /**
     * Sets the histogram and scale to visualise
     * @param histogram the histogram.
     * @param isLogScale if true the histogram is visualised as log scale.
     */
    void setHistogram( const lexis::render::Histogram& histogram, bool isLogScale );

    /**
     * Set the 2d positions of control points.
     * @param controlPoints QPolygonF with the control points.
     */
    void setControlPoints( const QPolygonF& controlPoints );

    /**
     * Get the 2d positions of control points.
     * @return QPolygonF with the control points.
     */
    QPolygonF getControlPoints() const;

    /**
     * Get the color at given control point.
     * @param xPosition Position of the mouse cursor on the x-axis.
     * @return An uint containing RGBA 8-bits values.
     */
    uint32_t getColorAtPoint( float xPosition ) const;

    /**
     * Return curve's values.
     * @return A vector of 8-bits uints with the curve values.
     */
    UInt8s getCurve() const;

signals:

    /**
     * ColorsChanged signal.
     */
    void colorsChanged();

    /**
     * Hist index changed signal
     */
    void histIndexChanged( size_t index, double ratio );

private:
    void paintEvent( QPaintEvent* ) final;
    QSize sizeHint() const final { return { 255, 255 }; }
    void resizeEvent( QResizeEvent* ) final;
    void mouseMoveEvent( QMouseEvent* event ) final;
    void leaveEvent( QEvent* event ) final;

    void _generateBackground();
    void _drawHistogram();
    void _createCheckersBackground();

    Channel _channel;
    HoverPoints* _hoverPoints { nullptr };
    lexis::render::Histogram _histogram;
    bool _isLogScale { false };
    QImage _background;
    QLinearGradient _gradient { 0, 0, 0, 0 };
};

}

#endif // _ColorMapWidget_h_
