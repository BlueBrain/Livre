/* Copyright (c) 2011-2017, EPFL/Blue Brain Project
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
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

#include <livre/core/api.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * The Configuration class is used to have a dictionary of variadic data types with names.
 */
class Configuration
{
public:
    LIVRECORE_API Configuration();

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
        getGroup_( groupName ).add_options() ( shortDesc.c_str(),
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
        getGroup_( groupName ).add_options()( shortDesc.c_str(),
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
        getGroup_( groupName ).add_options()( shortDesc.c_str(),
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
        getGroup_( groupName ).add_options()( shortDesc.c_str(),
           boost::program_options::bool_switch()->default_value( defaultValue ),
                                              longDesc.c_str() );
    }

    /**
     * Parses command line.
     * @param argc Number of arguments.
     * @param argv Arguments array.
     */
    LIVRECORE_API void parseCommandLine( int32_t argc, const char **argv );

    /**
     * Gets the value of a key from the dictionary.
     * @param key Key to the value in the dictionary.
     * @param defaultValue The default value to use if key not in dictionary
     * @return The value if found in the dictionary, defaultValue otherwise
     */
    template< class T >
    T getValue( const std::string& key, const T& defaultValue ) const
    {
        if( !_options.count( key ))
            return defaultValue;

        return _options[ key ].as< T >( );
    }

    /**
     * Gets the value of a key from the disctionary.
     * @param key Key to the value in the dictionary.
     * @return True if key is in the dictionary.
     */
    LIVRECORE_API bool getValue( const std::string& key ) const
    {
        return _options.count( key );
    }

    /**
     * @return True if dictionary is empty.
     */
    LIVRECORE_API bool empty() const { return _descriptions.empty(); }

    /**
     * Adds global parameters ( help, config file ... to global configuration )
     * @param config Configuration that has the description.
     */
    LIVRECORE_API void addDescription(const Configuration& config );

    /**
     * @return The description map.
     */
    LIVRECORE_API const ProgramOptionsDescriptionMap& getDescriptionMap_( ) const;

    /**
     * Prints description map for parameters.
     * @return the output stream.
     */
    LIVRECORE_API friend std::ostream& operator<<( std::ostream& ostream,
                                                   const Configuration& configuration );

    LIVRECORE_API Configuration& operator = ( const Configuration& );

private:
    ProgramOptionsMap _options;
    ProgramOptionsDescriptionMap _descriptions;

    LIVRECORE_API ProgramOptionsDescription& getGroup_( const std::string& groupName );
    void processDescriptionMap_( ProgramOptionsDescription& description ) const;
};



}

#endif // _Configuration_h_
