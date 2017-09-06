/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
 *                          Grigori Chevtchenko <grigori.chevtchenko@epfl.ch>
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

#include <livre/eq/FrameGrabber.h>

#include <livre/eq/Channel.h>
#include <livre/eq/Event.h>
#include <livre/eq/FrameData.h>
#include <livre/eq/Pipe.h>
#include <livre/eq/settings/FrameSettings.h>

#include <eq/image.h>

#ifdef LIVRE_USE_LIBJPEGTURBO
#include <turbojpeg.h>
#endif

namespace livre
{
FrameGrabber::FrameGrabber()
    : _compressor(nullptr)
{
}

FrameGrabber::~FrameGrabber()
{
#ifdef LIVRE_USE_LIBJPEGTURBO
    if (_compressor)
        tjDestroy(_compressor);
#endif
}

uint8_t* FrameGrabber::_encodeJpeg(const uint32_t width LB_UNUSED,
                                   const uint32_t height LB_UNUSED,
                                   const uint8_t* rawData LB_UNUSED,
                                   unsigned long& dataSize LB_UNUSED)
{
#ifdef LIVRE_USE_LIBJPEGTURBO
    if (!_compressor)
        _compressor = tjInitCompress();

    uint8_t* tjSrcBuffer = const_cast<uint8_t*>(rawData);
    const int32_t pixelFormat = TJPF_BGRA;
    const int32_t color_components = 4; // Color Depth
    const int32_t tjPitch = width * color_components;
    const int32_t tjPixelFormat = pixelFormat;

    uint8_t* tjJpegBuf = 0;
    const int32_t tjJpegSubsamp = TJSAMP_444;
    const int32_t tjJpegQual = 100; // Image Quality
    const int32_t tjFlags = TJXOP_ROT180;

    const int32_t success =
        tjCompress2(_compressor, tjSrcBuffer, width, tjPitch, height,
                    tjPixelFormat, &tjJpegBuf, &dataSize, tjJpegSubsamp,
                    tjJpegQual, tjFlags);

    if (success != 0)
    {
        LBERROR << "libjpeg-turbo image conversion failure" << std::endl;
        dataSize = 0;
        return nullptr;
    }
    return static_cast<uint8_t*>(tjJpegBuf);
#else
    dataSize = 0;
    if (!_compressor) // just to silence unused private field warning
        return nullptr;
    return nullptr;
#endif
}

void FrameGrabber::notifyNewImage(eq::Channel& channel, const eq::Image& image)
{
    const Pipe* pipe = static_cast<Pipe*>(channel.getPipe());
    if (!pipe->getFrameData().getFrameSettings().getGrabFrame())
        return;

    const uint64_t size = image.getPixelDataSize(eq::Frame::Buffer::color);
    const uint8_t* data = image.getPixelPointer(eq::Frame::Buffer::color);
    const eq::PixelViewport& pvp = image.getPixelViewport();

    unsigned long jpegSize = size;
    uint8_t* jpegData = _encodeJpeg(pvp.w, pvp.h, data, jpegSize);

    if (!jpegData)
        LBERROR << "Jpeg compression failed" << std::endl;

    channel.getConfig()->sendEvent(GRAB_IMAGE)
        << uint64_t(jpegSize) << co::Array<const uint8_t>(jpegData, jpegSize);
#ifdef LIVRE_USE_LIBJPEGTURBO
    tjFree(jpegData);
#endif
}
}
