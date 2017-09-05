
/* Copyright (c) 2006-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          David Steiner   <steiner@ifi.uzh.ch>
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
 */

#ifndef _Pipe_h_
#define _Pipe_h_

#include <livre/eq/api.h>
#include <livre/eq/types.h>

#include <eq/pipe.h> // base class

namespace livre
{
/**
 * The Pipe class is a standard EQ abstraction for OpenGL contexts.
 */
class Pipe : public eq::Pipe
{
public:
    /**
     * @param parent Parent node that owns the pipe.
     */
    LIVREEQ_API Pipe(eq::Node* parent);
    LIVREEQ_API ~Pipe();

    /** @return The frame data. */
    const FrameData& getFrameData() const;

private:
    void frameStart(const eq::uint128_t& frameId,
                    const uint32_t frameNumber) final;
    bool configInit(const eq::uint128_t& initId) final;
    bool configExit() final;

    struct Impl;
    std::unique_ptr<Impl> _impl;
};
}

#endif //_Pipe_h_
