/* Copyright (c) 2015, EPFL/Blue Brain Project
 *                     Daniel.Nachbaur@epfl.ch
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


#ifndef _RenderParameters_h_
#define _RenderParameters_h_

#include <livre/lib/zerobuf/volumeRendererParameters.h>
#include <QObject>

namespace livre
{

/**
 * Enhance the VolumeRendererParameters ZeroBuf object with a signal when
 * updates were received to update the GUI accordingly.
 */
class RenderParameters : public QObject,
                         public zerobuf::VolumeRendererParameters
{
    Q_OBJECT

signals:
    void received();

public:
    RenderParameters() {}

private:
    void notifyReceived() final { emit received(); }
};

}

#endif
