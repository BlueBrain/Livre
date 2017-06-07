
/* Copyright (c) 2015-2017, Daniel.Nachbaur@epfl.ch
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

#include "communicator.h"

#include <livre/eq/Config.h>
#include <livre/eq/Event.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/settings/CameraSettings.h>
#include <livre/eq/settings/FrameSettings.h>
#include <livre/eq/settings/RenderSettings.h>
#include <livre/eq/settings/VolumeSettings.h>

#include <livre/lib/configuration/ApplicationParameters.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>

#include <livre/data/DataSource.h>
#include <livre/data/VolumeInformation.h>

#include <lexis/lexis.h>
#include <zeroeq/zeroeq.h>
#ifdef ZEROEQ_USE_CPPNETLIB
#include <zeroeq/http/server.h>
#endif
#include <lunchbox/clock.h>

#include <functional>
#include <map>
#include <unordered_map>

namespace livre
{
namespace zeroeq
{
class Communicator::Impl
{
public:
    Impl(Config& config, const int argc, char** argv)
        : _config(config)
    {
        if (!servus::Servus::isAvailable())
            return;

        _setupRequests();
        _setupSubscriber();
        _setupHTTPServer(argc, argv);
    }

    bool publishFrame()
    {
        const auto& frameSettings = _getFrameData().getFrameSettings();
        const auto& params = _getFrameData().getApplicationParameters();

        _frame.setStart(params.frames[0]);
        _frame.setCurrent(frameSettings.getFrameNumber());
        _frame.setEnd(params.frames[1]);
        _frame.setDelta(params.animation);

        return _publisher.publish(_frame);
    }

    ::lexis::render::LookOut _createLookOut(const Matrix4f& livreModelView)
    {
        // this computation does not work if spaces are rotated in respect to
        // each other.
        Matrix4f rotation;
        rotation.setSubMatrix<3, 3>(livreModelView.getSubMatrix<3, 3>(0, 0), 0,
                                    0);

        Vector4f translation = livreModelView.getColumn(3);

        translation = -rotation.inverse() * translation;
        translation[3] = 1.0f;

        const auto& volumeInfo = _config.getVolumeInformation();
        translation =
            -rotation * volumeInfo.dataToLivreTransform.inverse() * translation;
        translation *= (1.0f / volumeInfo.meterToDataUnitRatio);
        translation[3] = 1.0f;

        Matrix4f networkModelView;
        networkModelView.setSubMatrix<3, 3>(rotation.getSubMatrix<3, 3>(0, 0),
                                            0, 0);
        networkModelView.setColumn(3, translation);

        ::lexis::render::LookOut lookOut;

        std::copy(&networkModelView.array[0], &networkModelView.array[0] + 16,
                  lookOut.getMatrix());
        return lookOut;
    }

    bool publishCamera(const Matrix4f& livreModelview)
    {
        return _publisher.publish(_createLookOut(livreModelview));
    }

    void onCamera(const ::lexis::render::LookOut& lookOut)
    {
        // this computation does not work if spaces are rotated in respect to
        // each other.
        float matrixValues[16];
        std::copy(lookOut.getMatrix(), lookOut.getMatrix() + 16, matrixValues);

        Matrix4f networkModelView(&matrixValues[0], &matrixValues[0] + 16);

        Vector4f translation = networkModelView.getColumn(3);
        Matrix4f rotation;
        rotation.setSubMatrix<3, 3>(networkModelView.getSubMatrix<3, 3>(0, 0),
                                    0, 0);

        const auto& volumeInfo = _config.getVolumeInformation();
        translation = -rotation.inverse() * translation;
        translation *= volumeInfo.meterToDataUnitRatio;
        translation[3] = 1.0f;

        translation = -rotation * volumeInfo.dataToLivreTransform * translation;
        translation[3] = 1.0f;

        Matrix4f livreModelView;
        livreModelView.setSubMatrix<3, 3>(rotation.getSubMatrix<3, 3>(0, 0), 0,
                                          0);
        livreModelView.setColumn(3, translation);

        _getFrameData().getCameraSettings().setModelViewMatrix(livreModelView);
    }

    bool publish(const servus::Serializable& serializable)
    {
        return _publisher.publish(serializable);
    }

    bool frameDirty()
    {
        const auto& frameSettings = _getFrameData().getFrameSettings();
        const auto& params = _getFrameData().getApplicationParameters();
        return _frame.getCurrent() != frameSettings.getFrameNumber() ||
               _frame.getDelta() != params.animation ||
               _frame.getStart() != params.frames.x() ||
               _frame.getEnd() != params.frames.y();
    }

    bool onRequest(::lexis::ConstRequestPtr request)
    {
        const auto& i = _requests.find(request->getEvent());
        if (i == _requests.end())
            return false;
        return i->second();
    }

    void updateFrame()
    {
        if (_config.getVolumeInformation().frameRange == INVALID_FRAME_RANGE)
            return;

        auto& frameSettings = _config.getFrameData().getFrameSettings();
        auto& params = _getFrameData().getApplicationParameters();

        if (_frame.getCurrent() == frameSettings.getFrameNumber() &&
            _frame.getDelta() == params.animation &&
            _frame.getStart() == params.frames.x() &&
            _frame.getEnd() == params.frames.y())
        {
            return;
        }

        frameSettings.setFrameNumber(_frame.getCurrent());
        params.animation = _frame.getDelta();
        params.frames = {_frame.getStart(), _frame.getEnd()};
    }

    bool requestExit()
    {
        _config.stopRunning();
        return true;
    }

    void handleEvents()
    {
        while (_subscriber.receive(0))
            _config.sendEvent(REDRAW);
    }

private:
    ::zeroeq::Subscriber _subscriber;
    ::zeroeq::Publisher _publisher;
    typedef std::function<bool()> RequestFunc;
    typedef std::map<::zeroeq::uint128_t, RequestFunc> RequestFuncs;
    RequestFuncs _requests;
#ifdef ZEROEQ_USE_CPPNETLIB
    std::unique_ptr<::zeroeq::http::Server> _httpServer;
    ::lexis::render::Exit _exit;
    ::lexis::render::ImageJPEG _imageJPEG;
    ::lexis::render::LookOut _lookOut;
#endif
    ::lexis::render::Frame _frame;
    Config& _config;

    void _setupRequests()
    {
        _requests[_frame.getTypeIdentifier()] = [&] { return publishFrame(); };
        _requests[_getFrameData().getVRParameters().getTypeIdentifier()] = [&] {
            return _publisher.publish(_getFrameData().getVRParameters());
        };
        _requests[ ::lexis::render::LookOut::ZEROBUF_TYPE_IDENTIFIER()] = [&] {
            return publishCamera(
                _getFrameData().getCameraSettings().getModelViewMatrix());
        };
        _requests
            [_getRenderSettings().getTransferFunction().getTypeIdentifier()] =
                [&] {
                    return _publisher.publish(
                        _getRenderSettings().getTransferFunction());
                };
        _requests[_getRenderSettings().getClipPlanes().getTypeIdentifier()] =
            [&] {
                return _publisher.publish(_getRenderSettings().getClipPlanes());
            };
        _requests[_config.getHistogram().getTypeIdentifier()] = [&] {
            return _publisher.publish(_config.getHistogram());
        };
    }

    void _setupHTTPServer(const int argc LB_UNUSED, char** argv LB_UNUSED)
    {
#ifdef ZEROEQ_USE_CPPNETLIB
        _httpServer = ::zeroeq::http::Server::parse(argc, argv, _subscriber);

        if (!_httpServer)
            return;

        _exit.registerDeserializedCallback([this]() { requestExit(); });
        _httpServer->handlePUT(_exit);

        _imageJPEG.registerSerializeCallback(
            [this]() { _config.renderJPEG(_imageJPEG); });
        _httpServer->handleGET(_imageJPEG);

        _lookOut.registerDeserializedCallback([this]() { onCamera(_lookOut); });
        _lookOut.registerSerializeCallback([this]() {
            const auto& camera = _getFrameData().getCameraSettings();
            const auto lookout = _createLookOut(camera.getModelViewMatrix());
            _lookOut.setMatrix(lookout.getMatrixVector());
        });
        _httpServer->handle(_lookOut);

        _httpServer->handle(_frame);
        _httpServer->handle(_getFrameData().getVRParameters());
        _httpServer->handle(_getRenderSettings().getTransferFunction());
        _httpServer->handle(_getRenderSettings().getClipPlanes());
#endif
    }

    void _setupSubscriber()
    {
        _subscriber.subscribe(::lexis::Request::ZEROBUF_TYPE_IDENTIFIER(),
                              [&](const void* data, const size_t size) {
                                  onRequest(
                                      ::lexis::Request::create(data, size));
                              });

        _subscriber.subscribe(
            ::lexis::render::LookOut::ZEROBUF_TYPE_IDENTIFIER(),
            [&](const void* data, const size_t size) {
                onCamera(*::lexis::render::LookOut::create(data, size));
            });

        _frame.registerSerializeCallback([&] {
            const auto& frameSettings = _getFrameData().getFrameSettings();
            const auto& params = _getFrameData().getApplicationParameters();

            _frame.setStart(params.frames[0]);
            _frame.setCurrent(frameSettings.getFrameNumber());
            _frame.setEnd(params.frames[1]);
            _frame.setDelta(params.animation);
        });
        _frame.registerDeserializedCallback([&] { updateFrame(); });
        _subscriber.subscribe(_frame);

        _subscriber.subscribe(_getFrameData().getVRParameters());
        _subscriber.subscribe(_getRenderSettings().getTransferFunction());
        _subscriber.subscribe(_getRenderSettings().getClipPlanes());
    }

    FrameData& _getFrameData() { return _config.getFrameData(); }
    const FrameData& _getFrameData() const { return _config.getFrameData(); }
    RenderSettings& _getRenderSettings()
    {
        return _getFrameData().getRenderSettings();
    }
    const RenderSettings& _getRenderSettings() const
    {
        return _getFrameData().getRenderSettings();
    }
};

Communicator::Communicator(Config& config, const int argc, char* argv[])
    : _impl(new Impl(config, argc, argv))
{
}

Communicator::~Communicator()
{
}

void Communicator::publishFrame()
{
    if (_impl->frameDirty())
        _impl->publishFrame();
}

bool Communicator::publish(const servus::Serializable& serializable)
{
    return _impl->publish(serializable);
}

void Communicator::publishCamera(const Matrix4f& modelview)
{
    _impl->publishCamera(modelview);
}

void Communicator::handleEvents()
{
    _impl->handleEvents();
}
}
}
