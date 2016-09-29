/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Marwan Abdellah <marwan.abdellah@epfl.ch>
 *                     Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#include <livre/core/data/Histogram.h>
#include <livre/core/types.h>

#include <lexis/render/ColorMap.h>

#include <QWidget>
#include <QtGui/QLinearGradient>
#include <QtGui/QImage>

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

    /**
     * Constructor.
     * @param parent Parent widget.
     * @param colorMap the colormap carries the control points
     * @param channel  Type of shade used to choose the color of the curve (RGBA).
     */
    ColorMapWidget( QWidget* parent,
                    lexis::render::ColorMap& colorMap,
                    lexis::render::ColorMap::Channel channel );

    /**
     * @return the shade type
     */
    lexis::render::ColorMap::Channel getChannel() const { return _channel; }

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
    void setHistogram( const Histogram& histogram, bool isLogScale );

    /**
     * paintEvent
     */
    void paintEvent( QPaintEvent* ) final;

    /**
     * sizeHint
     * @return Return the size.
     */
    QSize sizeHint() const final;

    /**
     * @return the gradient
     */
    const QLinearGradient& getGradient() const { return _gradient; }

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

    void mouseMoveEvent( QMouseEvent* event );
    void leaveEvent( QEvent* event );

    void _generateBackground();
    void _drawHistogram();
    void _createCheckersBackground();

    lexis::render::ColorMap& _colorMap;
    lexis::render::ColorMap::Channel _channel;
    HoverPoints* _hoverPoints;
    Histogram _histogram;
    bool _isLogScale;
    QImage _background;
    QLinearGradient _gradient;
};

}

#endif // _ColorMapWidget_h_
