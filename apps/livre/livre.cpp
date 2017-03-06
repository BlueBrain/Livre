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

#include <livre/eq/Engine.h>

#include <eq/client.h>
#include <eq/init.h>
#include <stdlib.h>

int main(const int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (std::string("--help") == argv[i])
        {
            std::cout << livre::Engine::getHelp() << eq::getHelp()
                      << eq::Client::getHelp() << std::endl;
            return EXIT_SUCCESS;
        }
        if (std::string("--version") == argv[i])
        {
            std::cout << livre::Engine::getVersion() << std::endl;
            return EXIT_SUCCESS;
        }
    }

    livre::Engine engine(argc, argv);
    engine.run(argc, argv);
    return EXIT_SUCCESS;
}
