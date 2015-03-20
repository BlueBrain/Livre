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

#include <livre/core/DashPipeline/DashProcessor.h>
#include <livre/core/DashPipeline/DashProcessorInput.h>
#include <livre/core/DashPipeline/DashProcessorOutput.h>

#include <dash/dash.h>

namespace livre
{

DashProcessor::DashProcessor()
{
}

bool DashProcessor::onPreApply_( const uint32_t connection )
{
    if( !getProcessorInput_< DashProcessorInput >()->getConnection( connection ) )
        return false;

    return true;
}

bool DashProcessor::onPreCommit_(const uint32_t connection)
{
    if( !getProcessorOutput_< DashProcessorOutput >()->getConnection( connection ) )
        return false;

    return true;
}

void DashProcessor::onSetDashContext_()
{
    processorInputPtr_.reset( new DashProcessorInput( *this ) );
    processorOutputPtr_.reset( new DashProcessorOutput( *this ) );
}

bool DashProcessor::initializeThreadRun_()
{
    if( !getDashContext().get( ))
        return false;

    getDashContext()->setCurrent();
    return true;
}

}
