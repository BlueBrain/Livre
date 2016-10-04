/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

#include <eq/eq.h>

#include <livre/core/types.h>

#include <livre/eq/Client.h>
#include <livre/eq/Config.h>
#include <livre/eq/Node.h>
#include <livre/eq/Pipe.h>
#include <livre/eq/Window.h>
#include <livre/eq/Channel.h>
#include <livre/eq/Error.h>

#include <stdlib.h>

class NodeFactory : public eq::NodeFactory
{
public:
    NodeFactory()
    {}

    virtual eq::Config* createConfig( eq::ServerPtr parent )
        { return new livre::Config( parent ); }

    virtual eq::Node* createNode( eq::Config* parent )
        { return new livre::Node( parent ); }

    virtual eq::Pipe* createPipe( eq::Node* parent )
        { return new livre::Pipe( parent ); }

    virtual eq::Window* createWindow( eq::Pipe* parent )
        { return new livre::Window( parent ); }

    virtual eq::Channel* createChannel( eq::Window* parent )
        { return new livre::Channel( parent ); }
};

int main( const int argc, char** argv )
{
    for( int i = 1; i < argc; ++i )
    {
        if( std::string( "--help" ) == argv[i] )
        {
            std::cout << livre::Client::getHelp() << std::endl;
            return EXIT_SUCCESS;
        }
        if( std::string( "--version" ) == argv[i] )
        {
            std::cout << livre::Client::getVersion() << std::endl;
            return EXIT_SUCCESS;
        }
    }

    // Equalizer initialization
    NodeFactory nodeFactory;
    livre::initErrors( );

    if( !eq::init( argc, argv, &nodeFactory ))
    {
        LBERROR << "Equalizer init failed" << std::endl;
        eq::exit();
        // cppcheck-suppress unreachableCode
        livre::exitErrors();
        return EXIT_FAILURE;
    }

    // Run local client node
    lunchbox::RefPtr< livre::Client > client = new livre::Client();
    const int32_t ret = client->run( argc, argv );

    LBASSERTINFO( client->getRefCount() == 1, "Client still referenced by " <<
                  client->getRefCount() - 1 );

    eq::exit();
    // cppcheck-suppress unreachableCode
    livre::exitErrors();
    return ret;
}
