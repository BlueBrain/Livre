
/* Copyright (c) 2007-2010, Stefan Eilemann <eile@equalizergraphics.com>
 *                    2011, Maxim Makhinya  <maxmah@gmail.com>
 *                    2013, Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
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

#ifndef _Node_h_
#define _Node_h_

#include <livre/core/Dash/DashContextTrait.h>
#include <livre/core/dashTypes.h>
#include <livre/Lib/types.h>
#include <livre/Eq/types.h>
#include <eq/node.h>

namespace livre
{

/**
 * The Node class is a standard EQ abstraction for a process. It manages the data loaders and keeps the
 * data cache.
 */
class Node : public eq::Node, public DashContextTrait
{
public:

    /**
     * @param parent Parent config that owns the Node.
     */
    Node( eq::Config* parent );

    /**
     * @return The raw data cache.
     */
    RawDataCache& getRawDataCache();

    /**
     * @return The texture data cache.
     */
    TextureDataCache& getTextureDataCache();

    /**
     * @return The volume data source.
     */
    ConstVolumeDataSourcePtr getVolumeDataSource( ) const;

    /**
     * @return The dash tree.
     */
    DashTreePtr getDashTree( );

private:

    bool initializeVolume_();
    void releaseVolume_();

    void initializeCaches_();
    void releaseCaches_();

    FrameData& getFrameData_();

    virtual bool configInit( const eq::uint128_t& initId );
    virtual void frameStart(  const eq::uint128_t& frameId, const uint32_t frameNumber );
    virtual bool configExit();

    VolumeSettingsPtr volumeSettingsPtr_;
    ConstVolumeRendererParametersPtr vrRenderParametersPtr_;
    RawDataCachePtr rawDataCachePtr_;
    TextureDataCachePtr textureDataCachePtr_;
    VolumeDataSourcePtr dataSourcePtr_;
    DashTreePtr dashTreePtr_;
    DataSourceFactoryPtr dataSourceFactoryPtr_;
};


}

#endif //_Node_h_
