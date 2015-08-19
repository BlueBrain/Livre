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

#ifndef _GradientRenderer_h_
#define _GradientRenderer_h_

#include <livreGUI/editor/TFWidget.h>

namespace livre
{

class HoverPoints;

/**
 * This class is used to render a pattern of gradients. Can be used to visualize
 * colormaps.
 */
class GradientRenderer : public TFWidget
{
    Q_OBJECT

public:

    /**
     * The constructor of GradientRenderer.
     * @param parent Parent widget.
     */
    GradientRenderer( QWidget* parent );

    /**
     * Set the gradient stops.
     * @param stops The stop points for gradient calculation.
     */
    void setGradientStops( const QGradientStops& stops );

    /**
     * paintEvent
     */
    void paintEvent( QPaintEvent* ) final;

    /**
     * sizeHint
     * @return Return the size.
     */
    QSize sizeHint() const final;

signals:

    /**
     * ColorsChanged signal.
     */
    void colorsChanged();

private:

    void _generateShade();
};

}

#endif // _GradientRenderer_h_
