/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <livre/core/defines.h>
#include <livre/core/configuration/Configuration.h>
#include <lunchbox/term.h>

namespace livre
{

Configuration::Configuration()
{}

void Configuration::parseCommandLine( int32_t argc, const char **argv )
{
    // Add group for config file
    ProgramOptionsDescription description;
    processDescriptionMap_( description );
    boost::program_options::command_line_parser clp =
        boost::program_options::command_line_parser( argc, argv).
            options( description ).allow_unregistered();

    boost::program_options::parsed_options filtopts = clp.run( );
    boost::program_options::store( filtopts, _options );
    boost::program_options::notify( _options );
}

void Configuration::addDescription( const Configuration &config )
{
    const auto& descriptionMap = config.getDescriptionMap_();
    for( const auto& description : descriptionMap )
        _descriptions.insert( description );
}

const ProgramOptionsDescriptionMap &Configuration::getDescriptionMap_( ) const
{
    return _descriptions;
}

std::ostream& operator<<( std::ostream& os, const Configuration& configuration )
{
    for( const auto& description : configuration._descriptions )
    {
        description.second.print( os );
        os << std::endl;
    }

    return os;
}

ProgramOptionsDescription& Configuration::getGroup_( const std::string& groupName )
{
    const auto& i = _descriptions.find( groupName );
    if( i != _descriptions.end( ))
        return i->second;
    _descriptions.emplace( groupName,
                           ProgramOptionsDescription{ groupName,
                                              lunchbox::term::getSize().first });
    return _descriptions[ groupName ];
}

void Configuration::processDescriptionMap_( ProgramOptionsDescription& po ) const
{
    for( const auto& description : _descriptions )
        po.add( description.second );
}

Configuration& Configuration::operator = ( const Configuration& rhs )
{
    if( this == &rhs )
        return *this;

    _options = rhs._options;
    _descriptions.clear();
    for( const auto& item : rhs._descriptions )
        _descriptions.insert( item );

    return *this;
}

}
