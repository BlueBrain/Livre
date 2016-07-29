/* Copyright (c) 2015-2016, EPFL/Blue Brain Project
 *                          ahmet.bilgili@epfl.ch
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


#ifndef _ClipPlanesController_h_
#define _ClipPlanesControllerr_h_

#include <livreGUI/types.h>
#include <QWidget>

namespace livre
{

/** A widget which shows and modifies the clip planes of Livre. */
class ClipPlanesController : public QWidget
{
    Q_OBJECT

public:
    /**
     * @param controller The GUI connection to ZeroEQ world.
     * @param parentWgt Parent widget.
     */
    ClipPlanesController( Controller& controller,
                          QWidget* parentWgt = nullptr );
    virtual ~ClipPlanesController();

signals:

    void clipPlanesReceived();

private:
    struct Impl;
    std::unique_ptr< Impl > _impl;
};

}

#endif
