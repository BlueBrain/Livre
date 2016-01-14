/* Copyright (c) 2015-2016, EPFL/Blue Brain Project
 *                          Daniel.Nachbaur@epfl.ch
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


#ifndef _RenderParametersController_h_
#define _RenderParametersController_h_

#include <livreGUI/types.h>
#include <livre/lib/zerobuf/volumeRendererParameters.h>
#include <QWidget>

namespace livre
{

/** A widget with shows the renderer parameters that can be changed in Livre. */
class RenderParametersController : public QWidget
{
    Q_OBJECT

public:
    /**
     * @param controller The GUI connection to zeq world.
     * @param parentWgt Parent widget.
     */
    RenderParametersController( Controller& controller,
                                QWidget *parentWgt = nullptr );
    ~RenderParametersController( );

signals:
    void paramsUpdated();

private Q_SLOTS:
    void onParamsUpdated();

private:
    struct Impl;
    std::unique_ptr< Impl > _impl;

};

}

#endif
