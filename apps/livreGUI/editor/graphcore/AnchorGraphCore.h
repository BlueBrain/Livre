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

#ifndef _AnchorGraphCore_h_
#define _AnchorGraphCore_h_

#ifndef Q_MOC_RUN
#include <livre/core/render/TransferFunction1D.h>
#include <livre/core/mathTypes.h>
#endif // Q_MOC_RUN

#include <livreGUI/editor/graphcore/TransferFunctionGraphCore.h>

namespace livre
{

/**
 * Abstract interface for a class of graphs that are defined by anchor points.
 **/
class AnchorGraphCore : public TransferFunctionGraphCore
{
public:

    typedef std::vector< Vector2f > AnchorVector;
    typedef std::vector< Vector2f > ParameterVector;

    /**
     * Initializes the the transfer function with anchors.
     * @param transferFunction Transfer function.
     */
    virtual void initialize( TransferFunction1Df& transferFunction );

    /**
     * @param channel The color channel.
     * @return The anchor array.
     */
    virtual const AnchorVector& getAnchorVector( const ColorChannel channel ) const;

     /**
     * @param channel Color channel.
     * @param anchorIndex Anchor index.
     * @return The anchox x,y position.
     */
    virtual const Vector2f& getAnchor( const ColorChannel channel,
                                       const uint32_t index ) const;
    /**
     * Sets the anchors for a color channel.
     * @param channel Color channel.
     * @param anchors Anchor array.
     */
    virtual void setAnchor( TransferFunction1Df& transferFunction,
                            const ColorChannel channel,
                            const uint32_t anchorIndex,
                            const Vector2f& anchor ) = 0;
    /**
     * Sets the anchors for a color channel.
     * @param channel Color channel.
     * @param anchors Anchor array.
     */
    virtual void setAnchorVector( TransferFunction1Df& transferFunction,
                                  const ColorChannel channel,
                                  const AnchorVector& anchors );

    /**
     * @param channel The color channel.
     * @return The transformed anchor array.
     */
    virtual AnchorVector getCurve( const ColorChannel channel ) const = 0;


    /**
     * Sets the anchors vector, transforms and modifiestransfer function from a given list of anchors.
     * @param transferFunction The transfer function to modify.
     * @param channel The color channel.
     * @param anchors Anchor array.
     */
    virtual void setCurve( TransferFunction1Df& transferFunction,
                           const ColorChannel channel,
                           const AnchorVector &anchors ) = 0;

protected:


    void updateCurve_( TransferFunction1Df& transferFunction,
                       const ColorChannel channel );

    virtual void writeCurve_( TransferFunction1Df& transferFunction,
                              const ColorChannel channel,
                              const AnchorVector &anchors ) = 0;

    typedef std::vector< AnchorVector > AnchorVectors;
    AnchorVectors anchorLists_;
};


}

#endif // _AnchorGraphCore_h_
