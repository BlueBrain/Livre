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

#pragma once

#include <livreGUI/transferFunctionEditor/ControlPointsWidget.h>

namespace livre
{
/**
 * A widget showing the channel's color gradient to black and providing a curve
 * to change the channel's color curve.
 */
class ColorWidget : public ControlPointsWidget
{
public:
    ColorWidget(QWidget* parent, Channel channel);

private:
    QSize sizeHint() const final { return {255, 255}; }
    void resizeEvent(QResizeEvent*) final;
};
}
