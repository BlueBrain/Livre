
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *               2013, Ahmet Bilgili  <ahmet.bilgili@epfl.ch>
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

#ifndef _RenderSettings_h_
#define _RenderSettings_h_

#include <livre/lib/types.h>
#include <livre/lib/render/RendererType.h>

#include <livre/core/render/TransferFunction1D.h>

#include <co/serializable.h>

namespace livre
{

class RenderSettings : public co::Serializable
{
    /** The changed parts of the data since the last pack( ). */
    enum DirtyBits
    {
        DIRTY_TF = co::Serializable::DIRTY_CUSTOM << 0u,
        DIRTY_RENDERER_TYPE = co::Serializable::DIRTY_CUSTOM << 1u,
        DIRTY_DEPTH = co::Serializable::DIRTY_CUSTOM << 2u
    };

public:

    /**
     * @brief RenderSettings constructor.
     */
    RenderSettings( );

    /**
     * @brief setTransferFunction Sets the transfer function.
     * @param tf Transfer function.
     */
    void setTransferFunction( const TransferFunction1Dc& tf );

    /**
     * @brief resetTransferFunction Resets the transfer function.
     */
    void resetTransferFunction( );

    /**
     * @brief setRendererType Sets the renderer type.
     * @param rendererType The renderer type.
     */
    void setRendererType( const RendererType rendererType );

    /**
     * @return Returns the transfer function.
     */
    const TransferFunction1D< uint8_t >& getTransferFunction( ) const;

    /**
     * @return Returns the renderer type.
     */
    RendererType getRendererType( ) const;

    /**
     * @param depth Sets the maximum rendering depth.
     */
    void setMaxTreeDepth( const uint8_t depth );

    /**
     * @return Returns the maximum rendering depth.
     */
    uint8_t getMaxTreeDepth( ) const;

    /**
     * @brief adjustQuality Adjusts the quality.
     * @param delta The adjustment factor.
     */
    void adjustQuality( const float delta );

    /**
     * @brief increaseError Increases the error.
     */
    void increaseError( );

    /**
     * @brief decreaseError Decreases the error.
     */
    void decreaseError( );

private:

    virtual void serialize(   co::DataOStream& os, const uint64_t dirtyBits );
    virtual void deserialize( co::DataIStream& is, const uint64_t dirtyBits );

    TransferFunction1Dc transferFunction_;
    RendererType rendererType_;
    uint8_t depth_;
};

}

#endif // _RenderInfo_h_
