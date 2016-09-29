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

#ifndef _HoverPoints_h_
#define _HoverPoints_h_

#include <QtWidgets>

#include <lexis/render/ColorMap.h>

namespace livre
{

QT_FORWARD_DECLARE_CLASS( QBypassWidget )

/**
 * This class is used to create control points for color curves.
 */
class HoverPoints : public QObject
{
    Q_OBJECT
public:

    /**
     * The constructor of HoverPoints.
     * @param colorMap the colormap carries the control points
     * @param channel  Type of shade used to choose the color of the curve (RGBA).
     */
    HoverPoints( QWidget *widget,
                 lexis::render::ColorMap& colorMap,
                 lexis::render::ColorMap::Channel channel );

    /** Draw the control point. */
    void paintPoints();

    /**
     * Set "editable" boolean for the control points. If true, new control
     * points can be added.
     * @param editable The desired "editable" boolean.
     */
    void setEditable( const bool editable );

    /**
     * @brief Return the "editable" boolean for the control points. If true, new control
     * points can be added.
     */
    bool isEditable() const;

public:

    /**
     * @brief eventFilter
     * @param object
     * @param event
     * @return
     */
    bool eventFilter( QObject* object, QEvent* hoverEvent ) Q_DECL_OVERRIDE;

public slots:

    /**
     * setEnabled
     * @param enabled "Enabled" boolean.
     */
    void setEnabled( bool enabled );

    /**
     * setDisabled
     * @param disabled "Disabled" boolean.
     */
    void setDisabled( bool disabled ) { setEnabled(!disabled); }

signals:

    /** pointsChanged signal. */
    void pointsChanged();

private:

    QWidget *_colorMapWidget;
    bool _editable;
    bool _enabled;

    QPointF _selectedPoint;
    lexis::render::ColorMap& _colorMap;
    lexis::render::ColorMap::Channel _channel;
};

}
#endif // _HoverPoints_h_
