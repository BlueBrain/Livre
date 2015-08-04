/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Maxim Makhinya <maxmah@gmail.com>
 *                          Philipp Schlegel <schlegel@ifi.uzh.ch>
 *                          David Steiner <steiner@ifi.uzh.ch>
 *                          Stefan.Eilemann@epfl.ch
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

#ifndef _TransferFunctionEditor_h_
#define _TransferFunctionEditor_h_

#ifndef Q_MOC_RUN
#  include <livre/core/render/TransferFunction1D.h>
#  include <livreGUI/ui_TransferFunctionEditor.h>
#  include <livreGUI/editor/graphcore/TransferFunctionGraphCore.h>
#  include <livreGUI/editor/graphs/TransferFunctionGraph.h>
#  include <livreGUI/editor/TransferFunctionEditor.h>
#endif // Q_MOC_RUN

#include <QWidget>

namespace livre
{

/**
 * TransferFunctionEditor is the dialog for editing the transfer function.
 **/
class TransferFunctionEditor : public QWidget
{
    Q_OBJECT

public:

    enum GraphType
    {
        GT_GAUSS,
        GT_DOUBLEGAUSS
    };

    /**
     * @param parentWgt Parent widget.
     */
    explicit TransferFunctionEditor( QWidget *parentWgt = 0 );
     ~TransferFunctionEditor( );

    /**
     * Sets the transfer function.
     * @param Transfer function.
     */
    void setTransferFunction( TransferFunction1DfPtr transferFunctionPtr );

    /**
     * Sets the histogram.
     * @param histogram Histogram vector.
     */
    void setHistogram( const UInt8Vector& histogram );

    /**
     * @return The graph type.
     */
    GraphType getGraphType( ) const;

    /**
     * Sets the graph type.
     * @param graph The graph type.
     */
    void setGraphType( const GraphType graphType );

    /**
     * @return The transfer function graph.
     */
    TransferFunctionGraphPtr getGraph( ) const;

    /**
     * Enables/disables a channel.
     * @param channel Channel to enable.
     * @param enabled If true channel is enabled.
     */
    void toggleChannel( const ColorChannel channel, bool enabled );

public Q_SLOTS:

    /**
     * Updates the curve.
     * @param immediate If immediate is true, curve is updated immediately.
     */
    void updateCurve( const bool immediate = true );


protected Q_SLOTS:

    /**
     * Changes graph to the given type.
     * @param graphTypei The graphtype.
     */
    void changeGraph( int graphTypei );

private:

    void initializeGraph_( );
    void createAndConnectGraph_(const GraphType graphType);
    void connectGraph_();

    void closeEvent( QCloseEvent *closeEvt );
    void showEvent( QShowEvent* showEvent );

    QAction* quitAction_;
    ControllerPtr controller_;

    QPoint position_;
    TransferFunctionGraphPtr transferFunctionGraphPtr_;
    Ui_transferFunctionEditor_ ui_;
    FloatVector rgba_;
    UInt8Vector histogram_;
};

}
#endif
