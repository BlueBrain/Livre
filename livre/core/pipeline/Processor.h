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

#ifndef _Processor_h_
#define _Processor_h_

#include <livre/core/api.h>
#include <livre/core/types.h>
#include <livre/core/lunchboxTypes.h>
#include <livre/core/dashTypes.h>

#include <livre/core/pipeline/ProcessorInput.h>
#include <livre/core/pipeline/ProcessorOutput.h>

namespace livre
{

/**
 * The Processor class is the base class of the processing step in a pipeline. It can listen
 * the input connections and can behave accordingly. It has input connections and output connections
 * ( \see Connection ). When underlying is started with start() method it starts the infinite loop.
 */
class Processor : public lunchbox::Thread
{
    friend class ProcessorInput;
    friend class ProcessorOutput;

public:
    LIVRECORE_API Processor();
    LIVRECORE_API virtual ~Processor();

    /**
     * @return The \see ProcessorInput object that is responsible for receiving data from the incoming
     * connections and delivering data.
     */
    LIVRECORE_API ProcessorInputPtr getProcessorInput_( ) { return processorInputPtr_; }

   /**
     * @return The \see ProcessorInput object that is responsible for receiving data from the incoming
     * connections and delivering data.
     * @warning Casts the object to the right Connection type. If returned object is not type T, an empty
     * shared_ptr is returned.
     */
    template< class T >
    boost::shared_ptr< T > getProcessorInput_( )
    {
        return boost::dynamic_pointer_cast< T >( processorInputPtr_ );
    }
    /**
     * @return The \see ProcessorOutput object that is responsible for sending data to the outgoing
     * connections and delivering data.
     * @warning Casts the object to the right Connection type. If returned object is not type T, an empty
     * shared_ptr is returned.
     */
    LIVRECORE_API ProcessorOutputPtr getProcessorOutput_( ) { return processorOutputPtr_; }

    /**
     * @return The \see ProcessorInput object that is responsible for receiving data from the incoming
     * connections and delivering data.
     * @warning Casts the object to the right Connection type. If returned object is not type T, an empty
     * shared_ptr is returned.
     */
    template< class T >
    boost::shared_ptr< T > getProcessorOutput_( )
    {
        return boost::dynamic_pointer_cast< T >( processorOutputPtr_ );
    }

protected:

    /**
     * Is called before commiting.
     * @param connection Connection number.
     * @return False if commit is abandoned.
     */
    virtual bool onPreCommit_( const uint32_t connection LB_UNUSED ) { return true; }

    /**
     * Is called after commiting.
     * @param connection Connection number.
     * @param state State of the commit. \see livre::CommitState
     */
    virtual void onPostCommit_( const uint32_t connection LB_UNUSED, const CommitState state LB_UNUSED ) { }

    /**
     * Is called after applying changes.
     * @param connection Connection number.
     * @return False if apply is abandoned.
     */
    virtual bool onPreApply_( const uint32_t connection LB_UNUSED ) { return true; }

    /**
     * Is called after changes are applied.
     * @param connection Connection number.
     * @param applySuccessful True if apply is successfull on connection.
     */
    virtual void onPostApply_( const uint32_t connection LB_UNUSED, const bool applySuccessful LB_UNUSED ) { }

    /** The parent thread waits for this to finish when start() is invoked. */
    LIVRECORE_API bool init() override;

    /**
     * Is called when start() method is invoked for the thread.
     */
    LIVRECORE_API void run() override;

    /**
     * Is called in case any initialization is needed before starting the infinite loop.
     * @return True if initialization is successfull.
     */
    virtual bool initializeThreadRun_( ) { return true; }

    /**
     * Is called in the infinite loop. Most of the application logic goes into this function and its callees.
     */
    virtual void runLoop_( ) { }

    /**
     * The incoming connections to the processor object
     */
    ProcessorInputPtr processorInputPtr_;

    /**
     * The outgoing connections from the processor object.
     */
    ProcessorOutputPtr processorOutputPtr_;

    /**
     * Dash context of the thread.
     */
    dash::Context context_;
};


}

#endif // _Processor_h_
