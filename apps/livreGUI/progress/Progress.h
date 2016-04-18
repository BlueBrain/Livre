/* Copyright (c) 2016, EPFL/Blue Brain Project
 *                     Stefan.Eilemann@epfl.ch
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


#ifndef _Progress_h_
#define _Progress_h_

#include <livreGUI/types.h>
#include <QWidget>

namespace livre
{

/**
 * Progress is the Widget for playing/pausing animations as well as
 * positionning animation to a specific frame
 **/
class Progress : public QWidget
{
    Q_OBJECT

public:
    /**
     * @param controller The GUI connection to zeq world.
     * @param parent Parent widget.
     */
    Progress( Controller& controller, QWidget *parent = 0 );
    ~Progress( );

signals:
    void updated();

private Q_SLOTS:
    void onUpdated();
    void onExpired();

private:
    struct Impl;
    std::unique_ptr< Impl > _impl;
};

}
#endif
