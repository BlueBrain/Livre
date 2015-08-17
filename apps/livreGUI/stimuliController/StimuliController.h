/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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


#ifndef _StimuliController_h_
#define _StimuliController_h_

#include <livreGUI/types.h>

#ifndef Q_MOC_RUN
#  include <livreGUI/ui_StimuliController.h>
#endif // Q_MOC_RUN

#include <QWidget>

struct StimuliControllerImpl;

namespace livre
{

/**
 * Simulation controller widget that receives the cell ids through zeq
 * and can apply nest stimulation generators on selected cells using zeq.
 **/
class StimuliController : public QWidget
{
    Q_OBJECT

public:

    /**
     * @param controller The GUI connection to zeq world.
     * @param parentWgt Parent widget.
     */
    explicit StimuliController( Controller& controller,
                                QWidget *parentWgt = 0 );
     ~StimuliController( );

signals:

    void updateCellIdsTextBox( std::vector<uint32_t> cellIds );

private Q_SLOTS:

    void _injectStimuli();
    void _generatorSelected( const QItemSelection& selected,
                             const QItemSelection& deselected );

    void _updateCellIdsTextBox( std::vector<uint32_t> cellIds );

    void _connectISC();
    void _connectHBP();
    void _disconnectISC();
    void _disconnectHBP();
    void _propertiesChanged();

private:

     struct Impl;
     Impl *_impl;

     Ui_stimuliController _ui;
};

}
#endif
