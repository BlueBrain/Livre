/**
 * Copyright (c) BBP/EPFL 2005-2015 Ahmet.Bilgili@epfl.ch
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

#ifndef _FrameData_h_
#define _FrameData_h_

#include <co/objectHandler.h>
#include <livre/eq/types.h>
#include <livre/lib/types.h>
#include <livre/eq/coTypes.h>

namespace livre
{

enum SerializationMapIds
{
    SMI_FRAME_SETTINGS = 1u,
    SMI_RENDER_SETTINGS,
    SMI_CAMERA_SETTINGS,
    SMI_VOLUME_SETTINGS,
    SMI_VR_PARAMETERS
};

/**
 * The FrameData class handles registering/mapping/unmpapping/syncing of dynamic data
 * between client and renderer.
 */
class FrameData
{
public:

    FrameData( );

    /**
     * Initializes the FrameData.
     * @param eqConfig equalizer configuration.
     */
    void initialize( eq::Config* eqConfig );

    /** Set up the parameters. */
    void setup( const VolumeRendererParameters& rendererParams );

    /**
     * Registers objects to the internal \see co::ObjectMap.
     */
    void registerObjects( );

    /**
     * Deregisters objects from the internal \see co::ObjectMap.
     */
    void deregisterObjects( );

    /**
     * Maps objects to the \see co::ObjectMap.
     */
    void mapObjects( );

    /**
     * Unmaps objects from the \see co::ObjectMap.
     */
    void unmapObjects( );

    /**
     * @return The UUID of the wrapped map ptr
     */
    const eq::uint128_t& getID( ) const;

    /**
     * Commits changes.
     * @return The commited version.
     */
    eq::uint128_t commit( );

    /**
     * Syncs with the application node object.
     */
    bool syncToApplicatonNode( const eq::uint128_t& uuid );

    /**
     * Syncs changes.
     * @param version Sync version.
     * @return Synced version.
     */
    eq::uint128_t sync( const eq::uint128_t& version = co::VERSION_HEAD );

    /**
     * Maps frame data to config.
     * @param config eq Config.
     * @param uuid uuid of the data.
     * @return True if mapping is successfull.
     */
    bool map( eq::Config *config , const eq::uint128_t& uuid );

    /**
    * Unmaps frame data from config.
    * @param config eq Config.
    */
    void unmap( eq::Config *config );

    /**
     * Registers frame data to config.
     * @param config eq Config.
     * @return True if registration is successful.
     */
    bool registerToConfig_( eq::Config *config );

    /**
     * Deregisters frame data from config.
     * @param config eq Config.
     * @return True if deregistration is successful.
     */
    bool deregisterFromConfig_( eq::Config *config );

    /**
     * @return The frame settings.
     */
    FrameSettingsPtr getFrameSettings() const;

    /**
     * @return The render settings.
     */
    RenderSettingsPtr getRenderSettings() const;

    /**
     * @return The camera settings.
     */
    CameraSettingsPtr getCameraSettings() const;

    /**
     * @return The volume settings.
     */
    VolumeSettingsPtr getVolumeSettings() const;

    /**
     * @return The volume rendering parameters.
     */
    ConstVolumeRendererParametersPtr getVRParameters() const;
    VolumeRendererParametersPtr getVRParameters();

    virtual ~FrameData();

private:

    CoObjectFactoryPtr _objectFactory;
    CoObjectMapPtr _objectMap;

    FrameSettingsPtr _frameSettings;
    RenderSettingsPtr _renderSettings;
    CameraSettingsPtr _cameraSettings;
    VolumeSettingsPtr _volumeSettings;

    VolumeRendererParametersPtr _vrParameters;
};

}

#endif // _FrameData_h_
