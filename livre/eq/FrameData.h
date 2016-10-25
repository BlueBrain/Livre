/**
 * Copyright (c) BBP/EPFL 2005-2016 Ahmet.Bilgili@epfl.ch
 *                                  Stefan.Eilemann@epfl.ch
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

#include <livre/eq/types.h>
#include <livre/lib/types.h>
#include <co/objectFactory.h> // member

namespace livre
{

/** Handles frame-specific, distributed data. */
class FrameData
{
public:
    FrameData();

    /**
     * Initializes the FrameData.
     * @param eqConfig equalizer configuration.
     */
    void initialize( eq::Config* eqConfig );

    /** Set up the parameters. */
    void setup( const RendererParameters& rendererParams );

    /**
     * Registers objects to the internal \see co::ObjectMap.
     */
    void registerObjects();

    /**
     * Deregisters objects from the internal \see co::ObjectMap.
     */
    void deregisterObjects();

    /**
     * Maps objects to the \see co::ObjectMap.
     */
    void mapObjects();

    /**
     * Unmaps objects from the \see co::ObjectMap.
     */
    void unmapObjects();

    /**
     * @return The UUID of the wrapped map ptr
     */
    const eq::uint128_t& getID() const;

    /**
     * Commits changes.
     * @return The commited version.
     */
    eq::uint128_t commit();

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
    bool registerToConfig( eq::Config *config );

    /**
     * Deregisters frame data from config.
     * @param config eq Config.
     * @return True if deregistration is successful.
     */
    bool deregisterFromConfig( eq::Config *config );

    /** @return The frame settings. */
    FrameSettings& getFrameSettings();
    const FrameSettings& getFrameSettings() const;

    /** @return The render settings. */
    RenderSettings& getRenderSettings();
    const RenderSettings& getRenderSettings() const;

    /** @return The camera settings. */
    CameraSettings& getCameraSettings();
    const CameraSettings& getCameraSettings() const;

    /** @return The volume settings. */
    VolumeSettings& getVolumeSettings();
    const VolumeSettings& getVolumeSettings() const;

    /** @return The application settings. */
    ApplicationSettings& getApplicationSettings();
    const ApplicationSettings& getApplicationSettings() const;

    /** @return The volume rendering parameters. */
    RendererParameters& getVRParameters();
    const RendererParameters& getVRParameters() const;

    virtual ~FrameData();

private:
    class Impl;
    std::unique_ptr< Impl > _impl;
    co::ObjectFactory _factory;
};

}

#endif // _FrameData_h_
