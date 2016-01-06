/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

namespace livre
{

const char* CONFIGFILE_PARAM ="config-file";

Configuration::Configuration( )
{
    addDescription< std::string>( HIDDEN_PROGRAMDESCRIPTION_STR,
                                  CONFIGFILE_PARAM,
                                  "Config file" );
}

void Configuration::parseCommandLine( int32_t argc, const char **argv )
{
    // Add group for config file
    ProgramOptionsDescription descripton;
    processDescriptionMap_( descripton );
    boost::program_options::command_line_parser clp =
        boost::program_options::command_line_parser( argc, argv).
            options( descripton ).
            allow_unregistered();

    boost::program_options::parsed_options filtopts = clp.run( );
    boost::program_options::store( filtopts, _options );
    boost::program_options::notify( _options );

    // If config file is given, parameters are taken from the config file
    if( _options.count( CONFIGFILE_PARAM ) )
    {
        parseConfigFile( _options[ CONFIGFILE_PARAM ].as< std::string >( )  );
    }
}

void Configuration::parseConfigFile( const std::string &configFile )
{
    ProgramOptionsDescription descripton;
    processDescriptionMap_( descripton );
    std::istringstream configStream( configFile );

    boost::program_options::store(
                boost::program_options::parse_config_file< char >( configStream,
                                                                   descripton, true ),
                                        _options );
    boost::program_options::notify( _options );
}

void Configuration::addDescription( const Configuration &config )
{
    const ProgramOptionsDescriptionMap& descriptionMap = config.getDescriptionMap_();
    for( ProgramOptionsDescriptionMap::const_iterator it = descriptionMap.begin();
         it != descriptionMap.end(); ++it )
    {
        if( it->first == HIDDEN_PROGRAMDESCRIPTION_STR )
            continue;

        _descriptions.insert( *it );
    }
}

const ProgramOptionsDescriptionMap &Configuration::getDescriptionMap_( ) const
{
    return _descriptions;
}

std::ostream& operator<<( std::ostream& os, const Configuration& configuration )
{
    for( ProgramOptionsDescriptionMap::const_iterator it = configuration._descriptions.begin();
         it != configuration._descriptions.end(); ++it )
    {
        os << it->first << std::endl;
        it->second.print( os );
        os << std::endl;
    }

    return os;
}

ProgramOptionsDescription& Configuration::getGroup_( const std::string& groupName )
{
    return _descriptions[ groupName ];
}

void Configuration::processDescriptionMap_( ProgramOptionsDescription& description, bool doNotProcessHidden ) const
{
    for( ProgramOptionsDescriptionMap::const_iterator it = _descriptions.begin();
         it != _descriptions.end(); ++it )
    {
        if( doNotProcessHidden && it->first == HIDDEN_PROGRAMDESCRIPTION_STR )
            continue;

        description.add( it->second );
    }
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
