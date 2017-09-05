/* Copyright (c) 2006-2017, Stefan Eilemann <eile@equalizergraphics.com>
 *                          Maxim Makhinya  <maxmah@gmail.com>
 *                          David Steiner   <steiner@ifi.uzh.ch>
 *                          Ahmet Bilgili   <ahmet.bilgili@epfl.ch>
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

#ifndef _Client_h_
#define _Client_h_

#include <livre/eq/api.h>
#include <livre/eq/types.h>

#include <eq/client.h> // base class

namespace livre
{
typedef std::function<void()> IdleFunc;

/**
 * The client application class.
 */
class Client : public eq::Client
{
public:
    LIVREEQ_API Client(int argc, char** argv, bool isResident);
    LIVREEQ_API ~Client();

    /** @return the new chosen config if successful, nullptr otherwise. */
    LIVREEQ_API Config* chooseConfig();

    /** Release the config from chooseConfig(). */
    LIVREEQ_API void releaseConfig(Config* config);

    /**
     * @param idleFunc function which is called every frame, e.g. for updating
     *                 information like frame ranges in data sources
     */
    LIVREEQ_API void setIdleFunction(const IdleFunc& idleFunc);

    /** @return the currently set idle function. */
    LIVREEQ_API const IdleFunc& getIdleFunction() const;

    /**
     * Process all pending commands.
     *
     * @return true if there were commands to process, false otherwise
     */
    LIVREEQ_API bool processCommands();

protected:
    /** @override eq::Client::initLocal() */
    bool initLocal(const int argc, char* argv[]) final;

    /** Infinite loop on remote render client. */
    void clientLoop() final;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};
}
#endif // _Client_h_
