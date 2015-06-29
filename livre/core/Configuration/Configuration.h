/* Copyright (c) 2011-2014, EPFL/Blue Brain Project
 *                     Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#ifndef _Configuration_h_
#define _Configuration_h_

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

#include <livre/core/types.h>

namespace livre
{

typedef boost::program_options::variables_map ProgramOptionsMap;
typedef boost::program_options::options_description ProgramOptionsDescription;
typedef std::map< std::string, ProgramOptionsDescription > ProgramOptionsDescriptionMap;

/**
 * The Configuration class is used to have a dictionary of variadic data types with names.
 */
class Configuration
{
public:

     Configuration( );

    /**
     * Adds description for a dictionary item.
     * @param groupName The item group.
     * @param shortDesc Short description.
     * @param longDesc Long description.
     * @warning Before file parsing and/or command line parsing descriptions should be added
     */
    void addDescription( const std::string& groupName,
                         const std::string& shortDesc,
                         const std::string& longDesc )
    {
        getGroup_( groupName ).add_options( )( shortDesc.c_str(),
                                               longDesc.c_str( ));
    }

    /**
     * Adds description for a dictionary item.
     * @param groupName The item group.
     * @param shortDesc Short description.
     * @param longDesc Long description.
     * @warning Before file parsing and/or command line parsing descriptions should be added
     */
    template< class T >
    void addDescription( const std::string& groupName,
                         const std::string& shortDesc,
                         const std::string& longDesc )
    {
        getGroup_( groupName ).add_options( ) ( shortDesc.c_str(),
                                           boost::program_options::value< T >(),
                                                longDesc.c_str() );
    }

    /**
     * Add description for a dictionary item.
     * @param groupName The item group.
     * @param shortDesc Short description.
     * @param longDesc Long description.
     * @param defaultValue Add default value to parameter.
     * @warning Before file parsing and/or command line parsing descriptions
     * should be added. The bool type specialization is different and in the cpp
     * file.
     */
    template< class T >
    void addDescription( const std::string& groupName,
                         const std::string& shortDesc,
                         const std::string& longDesc,
                         T defaultValue )
    {
        getGroup_( groupName ).add_options( )( shortDesc.c_str(),
            boost::program_options::value< T >()->default_value( defaultValue ),
                                               longDesc.c_str() );
    }

    /**
     * Add description for a dictionary item.
     * @param groupName The item group.
     * @param shortDesc Short description.
     * @param longDesc Long description.
     * @param defaultValue Default value for parameter (arg not given).
     * @param implicitValue Implicit value for paramter (arg with no value)
     * @warning Before file parsing and/or command line parsing descriptions
     * should be added. The bool type specialization is different and in the cpp
     * file.
     */
    template< class T >
    void addDescription( const std::string& groupName,
                         const std::string& shortDesc,
                         const std::string& longDesc,
                         T defaultValue, T implicitValue )
    {
        getGroup_( groupName ).add_options( )( shortDesc.c_str(),
                                           boost::program_options::value< T >()
                                               ->default_value( defaultValue )
                                               ->implicit_value( implicitValue),
                                               longDesc.c_str() );
    }

    // Specialization for boolean parameters
    void addDescription( const std::string& groupName,
                         const std::string& shortDesc,
                         const std::string& longDesc,
                         bool defaultValue )
    {
        getGroup_( groupName ).add_options( )( shortDesc.c_str(),
           boost::program_options::bool_switch()->default_value( defaultValue ),
                                               longDesc.c_str() );
    }

    /**
     * Parses command line.
     * @param argc Number of arguments.
     * @param argv Arguments array.
     */
    void parseCommandLine( int32_t argc, char **argv );

    /**
     * Parses a config file.
     * @param configFile Config file name.
     */
    void parseConfigFile( const std::string& configFile );

    /**
     * Gets the value of a key from the disctionary.
     * @param key Key to the value in the dictionary.
     * @param value The value to read into.
     * @return True if key is in the dictionary.
     */
    template< class T >
    bool getValue( const std::string& key, T& value  ) const
    {
        if( !programOptionsMap_.count( key ))
            return false;

        value = programOptionsMap_[ key ].as< T >( );
        return true;
    }

    /**
     * Gets the value of a key from the disctionary.
     * @param key Key to the value in the dictionary.
     * @return True if key is in the dictionary.
     */
    bool getValue( const std::string& key ) const
    {
        return programOptionsMap_.count( key );
    }

    /**
     * @return True if dictionary is empty.
     */
    bool empty( ) const { return descriptionMap_.empty(); }

    /**
     * Adds global parameters ( help, config file ... to global configuration )
     * @param config Configuration that has the description.
     */
    void addDescription(const Configuration& config );

    /**
     * @return The description map.
     */
    const ProgramOptionsDescriptionMap& getDescriptionMap_( ) const;

    /**
     * Prints description map for parameters.
     * @return the output stream.
     */
    friend std::ostream& operator<<( std::ostream& ostream,
                                     const Configuration& configuration );

private:
    ProgramOptionsMap programOptionsMap_;
    ProgramOptionsDescriptionMap descriptionMap_;

    ProgramOptionsDescription& getGroup_( const std::string& groupName );
    void processDescriptionMap_( ProgramOptionsDescription& description ,
                                 bool doNotProcessHidden = false ) const;
};



}

#endif // _Configuration_h_
