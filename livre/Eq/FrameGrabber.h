/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
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

#ifndef _FrameGrabber_h_
#define _FrameGrabber_h_

#include <eq/client/resultImageListener.h>

typedef void* tjhandle;

namespace livre
{

class FrameGrabber : public eq::ResultImageListener
{
public:

    FrameGrabber();

    ~FrameGrabber();

    /**
     * Grabs the frame from the specified channel and posts a GRABBED_IMAGE application
     * event containing the dimensions and the buffer of the image
     * @return A pointer to the buffer containing the JPeg image or 0 if encoding failed.
     */
    void notifyNewImage( eq::Channel& channel, const eq::Image& image ) override;

private:

    /**
     * Converts RGBA RAW image into JPeg
     * @return A pointer to the buffer containing the JPeg image
     */
    uint8_t* _encodeJpeg( const uint32_t width, const uint32_t height,
                          const uint8_t* rawData, unsigned long& jpSize );
    tjhandle handleCompress_;

};

}

#endif // _FrameGrabber_h_
