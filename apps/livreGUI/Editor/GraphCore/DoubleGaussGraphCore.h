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

#ifndef _DoubleGaussGraphCore_h_
#define _DoubleGaussGraphCore_h_

#include <livreGUI/types.h>
#include <livreGUI/Editor/GraphCore/GaussGraphCore.h>

namespace livre
{

/**
 * A graph where the transfer function is defined by two modifiable Gaussian.
 **/
class DoubleGaussGraphCore: public GaussGraphCore
{
public:

    DoubleGaussGraphCore( );

    virtual ~DoubleGaussGraphCore( );

    /**
     * Initializes the the transfer function with anchors.
     * @param transferFunction Transfer function.
     */
    virtual void initialize( TransferFunction1Df& transferFunction );

    /**
     * @param channel The color channel.
     * @return The anchor array.
     */
    virtual AnchorVector getCurve( const ColorChannel channel) const;

    /**
     * Sets the anchors vector, transforms and modifiestransfer function from a given list of anchors.
     * @param transferFunction The transfer function to modify.
     * @param channel The color channel.
     * @param anchors Anchor array.
     */
    virtual void setCurve( TransferFunction1Df& transferFunction,
                           const ColorChannel channel,
                           const AnchorVector &anchors );

private:

    /**
     * Sets the transfer function from a given list of anchors.
     * @param transferFunction The transfer function to modify.
     * @param channel The color channel.
     * @param anchors Anchor array.
     */
    virtual void writeCurve_( TransferFunction1Df& transferFunction,
                              const ColorChannel channel,
                              const AnchorVector &anchors );
};


}

#endif // _DoubleGaussGraphCore_h_
