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

#ifndef _TFWidget_h_
#define _TFWidget_h_

#include <QWidget>

namespace livre
{

/**
 * This class contains the logic to draw checkers patterns. It's a QWidget.
 * ColorMapWidget and GradientRenderer are inheriting from thid class.
 */
class TFWidget : public QWidget
{
    Q_OBJECT

public:

    /**
     * The constructor of TFWidget
     * @param parent The parent widget.
     */
    TFWidget( QWidget* parent );

    const QLinearGradient& getGradient() const { return _gradient; }

protected:

    void _createCheckersBackground();

    QImage _shade;
    QLinearGradient _gradient;
};

}

#endif // _TFWidget_h_
