/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                     Philipp Schlegel <schlegel@ifi.uzh.ch>
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

#ifndef _TransferFunctionGraph_h_
#define _TransferFunctionGraph_h_

#include <QPoint>
#include <QColor>
#include <QWidget>

#ifndef Q_MOC_RUN
#include <livre/core/render/TransferFunction1D.h>
#include <livreGUI/types.h>
#include <livreGUI/qtTypes.h>
#include <livreGUI/editor/graphcore/TransferFunctionGraphCore.h>
#endif // Q_MOC_RUN

namespace livre
{

/**
 * The transfer function graph widget. A value array for the color mapping and  a value array for the material
 * mapping can be provided. The transfer function graph widget shows these mappings and allows editing them. Editing
 * directly effects the arrays provided. No local copy is stored.
 **/
class TransferFunctionGraph: public QWidget
{
    Q_OBJECT

public:

    /**
     * @param parentWgt Parent widget.
     */
    explicit TransferFunctionGraph( QWidget *parentWgt = 0 );

    virtual ~TransferFunctionGraph( );

    /**
     * @return The transfer function.
     */
    TransferFunction1DfPtr getTransferFunction( ) const;

    /**
     * @param transferFunctionPtr Sets the transfer function.
     */
    void setTransferFunction( TransferFunction1DfPtr transferFunctionPtr );

    /**
     * Sets the histogram.
     * @param histogram The histogram array.
     */
    void setHistogram( const UInt8Vector& histogram );

    /**
     * @return The operational part of the graph widget.
     */
    virtual GraphCorePtr getGraphCore( ) const = 0;

    /**
     * Sets the graph core.
     * @param graphCore Operational part of the graph widget.
     */
    virtual void setGraphCore( GraphCorePtr graphCore) = 0;

    /**
     * Initializes the editor.
     */
    virtual void initialize( ) = 0;

Q_SIGNALS:

    /**
     * Signals the update of the curve.
     * @param immediate If true curve is updated immediately.
     */
    void curveUpdate( const bool immediate = false);

public Q_SLOTS:

    /**
     * Set by the checkbox for corresponding channel to be editable.
     * @param checked If true corresponding channel is editable
     */
    void setStateR( const bool checked );
    void setStateG( const bool checked );
    void setStateB( const bool checked );
    void setStateA( const bool checked );

protected:

    /**
     * Paints the transfer function curve.
     * @param painter Painter object for the widget.
     * @param colorChannel Color channel.
     * @param count The number of elements in the curve.
     */
    void paintCurve_( QPainter &painter,
                      const ColorChannel channel,
                      const uint32_t count );

    typedef std::vector< QColor > ColorVector;

    BoolVector colorChannelEnabled_; //!< Keeps the enabled/disabled state of the color channel.
    UInt8Vector histogram_; //!< Histogram array.
    ColorVector colors_; //!< Color array.
    TransferFunction1DfPtr transferFunctionPtr_; //!< Transfer function.

private:

    QPoint getClippedPoint_( const QPoint& point ) const;
};

}

#endif // _TransferFunctionGraph_h_
