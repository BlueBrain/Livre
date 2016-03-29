/* Copyright (c) 2014, EPFL/Blue Brain Project
 *                     Stefan.Eilemann@epfl.ch
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

#ifndef LIVRE_REMOTE_DATASOURCE_H
#define LIVRE_REMOTE_DATASOURCE_H

#include <livre/core/mathTypes.h>
#include <livre/core/data/DataSourcePlugin.h>

namespace livre
{
namespace remote
{

/**
 * Proxy for a remote ZeroEQ-based data source service
 *
 * Used for volume URIs with a schema starting with remote. The remainder of the
 * URI is defined by the remoted data source, e.g., remoteuvf://filename.
 *
 * If specified, uses the host and port to connect to a concrete livreService in
 * place of zeroconf auto-discovery. If specified, uses a 'bind=address'
 * key-value pair in the URI query part to bind the local publisher to a fixed
 * address instead of INADDR_ANY.
 */
class DataSource : public DataSourcePlugin
{
public:
    DataSource();
    explicit DataSource( const DataSourcePluginData& initData );
    virtual ~DataSource();

    MemoryUnitPtr getData( const LODNode& node ) final;
    static bool handles( const DataSourcePluginData& initData );

private:

    class Impl;
    std::unique_ptr< Impl > _impl;
};

}
}

#endif
