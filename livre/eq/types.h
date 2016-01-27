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

#ifndef _livreEqTypes_h_
#define _livreEqTypes_h_

#include <livre/lib/types.h>

#include <eq/types.h>

#include <lunchbox/refPtr.h>
#include <boost/shared_ptr.hpp>

namespace livre
{

class CameraSettings;
class Channel;
class Client;
class CommandConnection;
class CommandNode;
class Config;
class EqEventHandler;
class EqEventHandlerFactory;
class FrameData;
class FrameSettings;
class GuiConnectionDefs;
class GuiPackets;
class LocalInitData;
class Node;
class Pipe;
class RayCastRenderer;
class RenderSettings;
class ScreenGrabber;
class SerializableMap;
class VolumeSettings;
class Window;
struct EqEventInfo;

/**
 * SmartPtr definitions
 */
typedef lunchbox::RefPtr< CommandNode > CommandNodePtr;
typedef lunchbox::RefPtr< Client > ClientPtr;

typedef boost::shared_ptr< Config > ConfigPtr;
typedef boost::shared_ptr< const Config > ConstConfigPtr;
typedef boost::shared_ptr< SerializableMap > SerializableMapPtr;
typedef boost::shared_ptr< const SerializableMap > ConstSerializableMapPtr;
typedef boost::shared_ptr< FrameData > FrameDataPtr;
typedef boost::shared_ptr< LocalInitData > LocalInitDataPtr;
typedef boost::shared_ptr< const FrameData > ConstFrameDataPtr;
typedef boost::shared_ptr< const LocalInitData > ConstLocalInitDataPtr;
typedef boost::shared_ptr< VolumeSettings > VolumeSettingsPtr;
typedef boost::shared_ptr< FrameSettings > FrameSettingsPtr;
typedef boost::shared_ptr< const FrameSettings > ConstFrameSettingsPtr;
typedef boost::shared_ptr< CameraSettings > CameraSettingsPtr;
typedef boost::shared_ptr< const CameraSettings > ConstCameraSettingsPtr;
typedef boost::shared_ptr< RenderSettings > RenderSettingsPtr;
typedef boost::shared_ptr< const RenderSettings > ConstRenderSettingsPtr;
typedef boost::shared_ptr< RayCastRenderer > RayCastRendererPtr;

}

#endif // _livreEqTypes_h_
