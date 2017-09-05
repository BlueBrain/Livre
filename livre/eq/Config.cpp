/* Copyright (c) 2006-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          David Steiner   <steiner@ifi.uzh.ch>
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

#include <livre/eq/Config.h>

#include <livre/eq/Client.h>
#include <livre/eq/Event.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/serialization.h>
#include <livre/eq/settings/CameraSettings.h>
#include <livre/eq/settings/FrameSettings.h>
#include <livre/eq/settings/RenderSettings.h>
#include <livre/lib/configuration/ApplicationParameters.h>
#include <livre/lib/configuration/VolumeRendererParameters.h>

#include <livre/core/util/FrameUtils.h>
#include <livre/data/VolumeInformation.h>

#ifdef LIVRE_USE_ZEROEQ
#include <livre/eq/zeroeq/communicator.h>
#endif

#include <eq/eq.h>

namespace livre
{
namespace
{
const float ROTATE_AND_ZOOM_SPEED = 0.005f;
const float PAN_SPEED = 0.0005f;
const float ADVANCE_SPEED = 0.05f;

class ViewHistogram
{
public:
    ViewHistogram(const Histogram& histogram_, const float area_,
                  const uint32_t id_)
        : histogram(histogram_)
        , area(area_)
        , id(id_)
    {
    }

    ViewHistogram& operator+=(const ViewHistogram& hist)
    {
        if (this == &hist)
            return *this;

        histogram += hist.histogram;
        area += hist.area;
        return *this;
    }

    bool isComplete() const
    {
        const float eps = 0.0001f;
        return std::abs(1.0f - area) <= eps;
    }

    Histogram histogram;
    float area;
    uint32_t id;
};

using ViewHistogramQueue = std::deque<ViewHistogram>;
}

class Config::Impl
{
public:
    explicit Impl(Config& config_)
        : config(config_)
        , frameStart(config.getTime())
        , volumeBBox(Boxf::makeUnitBox())
    {
    }

    void switchLayout(const int32_t increment)
    {
        const eq::Canvases& canvases = config.getCanvases();
        if (canvases.empty())
            return;

        auto currentCanvas = canvases.front();
        size_t index = currentCanvas->getActiveLayoutIndex() + increment;
        const eq::Layouts& layouts = currentCanvas->getLayouts();
        LBASSERT(!layouts.empty());

        index = (index % layouts.size());
        currentCanvas->useLayout(uint32_t(index));
        activeLayout = currentCanvas->getActiveLayout();
    }

    void gatherHistogram(const Histogram& histogram, const float area,
                         const uint32_t currentId)
    {
        // If we get a very old frame skip it
        if (!histogramQueue.empty() && currentId < histogramQueue.back().id)
            return;

        const ViewHistogram viewHistogram(histogram, area, currentId);
        for (auto it = histogramQueue.begin(); it != histogramQueue.end();)
        {
            auto& data = *it;
            bool dataMerged = false;

            if (currentId == data.id)
            {
                dataMerged = true;
                data += viewHistogram;
            }
            else if (currentId > data.id)
            {
                dataMerged = true;
                it = histogramQueue.emplace(it, viewHistogram);
            }

            if (data.isComplete()) // Send histogram & remove all old ones
            {
                if (config.getHistogram() != data.histogram)
                {
                    config.setHistogram(data.histogram);
#ifdef LIVRE_USE_ZEROEQ
                    config.publish(data.histogram);
#endif
                }
                histogramQueue.erase(it, histogramQueue.end());
                return;
            }

            if (dataMerged)
                break;
            ++it;
        }

        if (histogramQueue.empty() && !viewHistogram.isComplete())
        {
            histogramQueue.push_back(viewHistogram);
            return;
        }

        if (viewHistogram.isComplete())
        {
            if (config.getHistogram() == viewHistogram.histogram)
                return;
            config.setHistogram(viewHistogram.histogram);
#ifdef LIVRE_USE_ZEROEQ
            config.publish(viewHistogram.histogram);
#endif
            return;
        }

        if (histogramQueue.size() > config.getLatency() + 1)
            histogramQueue.pop_back();
    }

    Config& config;
    uint32_t latency = 0;
    FrameData framedata;
#ifdef LIVRE_USE_ZEROEQ
    std::unique_ptr<zeroeq::Communicator> communicator;
#endif
    bool redraw = true;
    VolumeInformation volumeInfo;
    int64_t frameStart;

    eq::Layout* activeLayout = nullptr;
    Histogram _histogram;

    Boxf volumeBBox;
    ViewHistogramQueue histogramQueue;
};

Config::Config(eq::ServerPtr parent)
    : eq::Config(parent)
    , _impl(new Impl(*this))
{
    _impl->framedata.initialize(this);
}

Config::~Config()
{
}

FrameData& Config::getFrameData()
{
    return _impl->framedata;
}

const FrameData& Config::getFrameData() const
{
    return _impl->framedata;
}

void Config::renderJPEG(::lexis::render::ImageJPEG& target)
{
    getFrameData().getFrameSettings().setGrabFrame(true);
    frame();

    for (;;)
    {
        eq::EventICommand event = getNextEvent();
        if (!event.isValid())
            continue;

        if (event.getEventType() == GRAB_IMAGE)
        {
            const uint64_t size = event.read<uint64_t>();
            const uint8_t* data = reinterpret_cast<const uint8_t*>(
                event.getRemainingBuffer(size));

            target.setData(data, size);
            return;
        }

        handleEvent(event);
    }
}

void Config::setHistogram(const Histogram& histogram)
{
    _impl->_histogram = histogram;
}

const Histogram& Config::getHistogram() const
{
    return _impl->_histogram;
}

const VolumeInformation& Config::getVolumeInformation() const
{
    return _impl->volumeInfo;
}

VolumeInformation& Config::getVolumeInformation()
{
    return _impl->volumeInfo;
}

void Config::mapFrameData(const eq::uint128_t& initId)
{
    _impl->framedata.map(this, initId);
    _impl->framedata.mapObjects();
}

void Config::unmapFrameData()
{
    _impl->framedata.unmapObjects();
    _impl->framedata.unmap(this);
}

void Config::resetCamera()
{
    _impl->framedata.getCameraSettings().setCameraPosition(
        _impl->framedata.getApplicationParameters().cameraPosition);
    _impl->framedata.getCameraSettings().setCameraLookAt(
        _impl->framedata.getApplicationParameters().cameraLookAt);
}

bool Config::init()
{
    resetCamera();
    FrameData& framedata = _impl->framedata;
    FrameSettings& frameSettings = framedata.getFrameSettings();
    const ApplicationParameters& params = framedata.getApplicationParameters();
    frameSettings.setFrameNumber(params.frames.x());

    RenderSettings& renderSettings = framedata.getRenderSettings();
    const TransferFunction1D tf(params.transferFunction);
    renderSettings.setTransferFunction(tf);

    _impl->framedata.registerObjects();

    if (!_impl->framedata.registerToConfig(this))
        return false;

    if (!eq::Config::init(_impl->framedata.getID()))
    {
        _impl->framedata.deregisterObjects();
        _impl->framedata.deregisterFromConfig(this);
        return false;
    }

    _impl->switchLayout(0); // update active layout
    _impl->latency = getLatency();
    return true;
}

void Config::initCommunicator(const int argc LB_UNUSED, char** argv LB_UNUSED)
{
#ifdef LIVRE_USE_ZEROEQ
    _impl->communicator.reset(new zeroeq::Communicator(*this, argc, argv));

    _impl->framedata.getCameraSettings().registerNotifyChanged(
        std::bind(&zeroeq::Communicator::publishCamera,
                  _impl->communicator.get(), std::placeholders::_1));
#endif
}

bool Config::frame()
{
    if (_impl->volumeInfo.frameRange == INVALID_FRAME_RANGE)
        return false;

    ApplicationParameters& params = _impl->framedata.getApplicationParameters();
    FrameSettings& frameSettings = _impl->framedata.getFrameSettings();

    const FrameUtils frameUtils(params.frames, _impl->volumeInfo.frameRange);
    params.frames = frameUtils.getFrameRange();

    // Set current frame (start/end may have changed)
    const bool keepToLatest = params.animation == LATEST_FRAME;
    const uint32_t current =
        frameUtils.getCurrent(frameSettings.getFrameNumber(), keepToLatest);

    frameSettings.setFrameNumber(current);
    const eq::uint128_t& version = _impl->framedata.commit();

    if (_impl->framedata.getVRParameters().getSynchronousMode())
        setLatency(0);
    else
        setLatency(_impl->latency);

    // reset data and advance current frame
    frameSettings.setGrabFrame(false);

    if (!keepToLatest && !_keepCurrentFrame(params.animationFPS))
    {
        frameSettings.setFrameNumber(
            frameUtils.getNext(current, params.animation));
        // reset starting time for new frame
        _impl->frameStart = getTime();
    }
    _impl->redraw = false;

#ifdef LIVRE_USE_ZEROEQ
    if (_impl->communicator)
        _impl->communicator->publishFrame();
#endif

    eq::Config::startFrame(version);
    eq::Config::finishFrame();
    return true;
}

void Config::postRedraw()
{
    _impl->redraw = true;
}

bool Config::needRedraw()
{
    return _impl->redraw ||
           _impl->framedata.getApplicationParameters().animation != 0;
}

bool Config::publish(const servus::Serializable& serializable)
{
#ifdef LIVRE_USE_ZEROEQ
    if (_impl->communicator)
        return _impl->communicator->publish(serializable);
#endif
    return false;
}

void Config::switchLayout(const int32_t increment)
{
    return _impl->switchLayout(increment);
}

eq::Layout* Config::getActiveLayout()
{
    return _impl->activeLayout;
}

bool Config::exit()
{
    bool ret = eq::Config::exit();
    _impl->framedata.deregisterObjects();
    if (!_impl->framedata.deregisterFromConfig(this))
        ret = false;
    return ret;
}

void Config::handleNetworkEvents()
{
#ifdef LIVRE_USE_ZEROEQ
    if (_impl->communicator)
        _impl->communicator->handleEvents();
#endif
}

bool Config::_keepCurrentFrame(const uint32_t fps) const
{
    if (fps == 0)
        return false;

    const double desiredTime = 1.0 / fps;
    const int64_t end = getTime();

    // If the frame duration is shorter than the desired frame time then the
    // current frame should be kept until the duration matches (or exceeds) the
    // expected. Otherwise, the frame number should be normally increased.
    // This means that no frames are artificially skipped due to the fps limit
    const double frameDuration = (end - _impl->frameStart) / 1e3;
    return frameDuration < desiredTime;
}

bool Config::handleEvent(const eq::EventType type, const eq::Event& event)
{
    switch (type)
    {
    case eq::EVENT_WINDOW_EXPOSE:
    case eq::EVENT_EXIT:
        postRedraw();
        return true;
    default:
        return eq::Config::handleEvent(type, event);
    }
}

bool Config::handleEvent(const eq::EventType type, const eq::KeyEvent& event)
{
    if (type != eq::EVENT_KEY_PRESS)
        return eq::Config::handleEvent(type, event);

    FrameSettings& frameSettings = getFrameData().getFrameSettings();

    switch (event.key)
    {
    case ' ':
        resetCamera();
        return true;

    case 's':
    case 'S':
        frameSettings.toggleStatistics();
        return true;

    case 'a':
    case 'A':
    {
        auto& vrParams = getFrameData().getVRParameters();
        vrParams.setShowAxes(!vrParams.getShowAxes());
        publish(vrParams);
        return true;
    }

    case 'i':
    case 'I':
        frameSettings.toggleInfo();
        return true;

    case 'l':
        switchLayout(1);
        return true;

    case 'L':
        switchLayout(-1);
        return true;

    default:
        return eq::Config::handleEvent(type, event);
    }
}

bool Config::handleEvent(const eq::EventType type,
                         const eq::PointerEvent& event)
{
    CameraSettings& camera = getFrameData().getCameraSettings();

    switch (type)
    {
    case eq::EVENT_CHANNEL_POINTER_BUTTON_PRESS:
        getFrameData().getFrameSettings().setIdle(false);
        return true;
    case eq::EVENT_CHANNEL_POINTER_BUTTON_RELEASE:
        getFrameData().getFrameSettings().setIdle(true);
        return true;
    case eq::EVENT_CHANNEL_POINTER_MOTION:
        switch (event.buttons)
        {
        case eq::PTR_BUTTON1:
            camera.spinModel(-ROTATE_AND_ZOOM_SPEED * event.dy,
                             -ROTATE_AND_ZOOM_SPEED * event.dx);
            return true;

        case eq::PTR_BUTTON2:
            camera.moveCamera(0.f, 0.f, ROTATE_AND_ZOOM_SPEED * -event.dy);
            return true;

        case eq::PTR_BUTTON3:
            camera.moveCamera(PAN_SPEED * event.dx, -PAN_SPEED * event.dy, 0.f);
            return true;
        }
        return eq::Config::handleEvent(type, event);

    case eq::EVENT_CHANNEL_POINTER_WHEEL:
        camera.moveCamera(-ADVANCE_SPEED * event.xAxis, 0.f,
                          ADVANCE_SPEED * event.yAxis);
        return true;

    default:
        return eq::Config::handleEvent(type, event);
    }
}

bool Config::handleEvent(eq::EventICommand command)
{
    switch (command.getEventType())
    {
    case HISTOGRAM_DATA:
    {
        const Histogram& histogram = command.read<Histogram>();
        const float area = command.read<float>();
        const uint32_t id = command.read<uint32_t>();
        _impl->gatherHistogram(histogram, area, id);
        return false;
    }

    case VOLUME_INFO:
        command >> getVolumeInformation();
        return false;

    case REDRAW:
        postRedraw();
        return true;
    }

    if (!eq::Config::handleEvent(command))
        return false;

    postRedraw();
    return true;
}
}
