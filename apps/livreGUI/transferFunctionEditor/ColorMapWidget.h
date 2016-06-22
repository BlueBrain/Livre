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

#include <livreGUI/transferFunctionEditor/TFWidget.h> // base class
#include <livre/core/data/Histogram.h>
#include <livre/core/types.h>

namespace livre
{

class HoverPoints;

/**
 * This class is used to create color curves with controls points. Each instance of
 * this class should be responsible for one of the RGBA component.
 */
class ColorMapWidget : public TFWidget
{
    Q_OBJECT

public:

    /**
     * The ShadeType enum
     */
    enum ShadeType
    {
        RED_SHADE   = 0,
        GREEN_SHADE = 1,
        BLUE_SHADE  = 2,
        ALPHA_SHADE = 3,
        SHADE_COUNT = 4
    };

    /**
     * Constructor of ColorMapWidget.
     * @param type  Type of shade used to choose the color of the curve (RGBA).
     * @param parent Parent widget.
     */
    ColorMapWidget( const ShadeType type, QWidget* parent );

    /**
     * @return the shade type
     */
    ShadeType getShadeType() const { return _shadeType; }

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
     * Get the 2d positions of control points.
     * @return QPolygonF with the control points.
     */
    QPolygonF getPoints() const;

    /**
     * Set the 2d positions of control points.
     * @param points QPolygonF with the control points.
     */
    void setPoints( const QPolygonF& getPoints );

    /**
     * getHoverPoints Get the hoverpoints.
     * @return The hoverpoints.
     */
    HoverPoints* getHoverPoints() const;

    /**
     * Get the color at given control point.
     * @param xPosition Position of the mouse cursor on the x-axis.
     * @return An uint containing RGBA 8-bits values.
     */
    uint32_t getColorAtPoint( int32_t xPosition );

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
    void histIndexChanged( size_t index, uint64_t value );

private:

    void mouseMoveEvent( QMouseEvent* event );
    void leaveEvent( QEvent* event );

    void _generateBackground();
    void _drawHistogram();

    ShadeType _shadeType;
    HoverPoints* _hoverPoints;
    Histogram _histogram;
    bool _isLogScale;
};

}

#endif // _ColorMapWidget_h_
