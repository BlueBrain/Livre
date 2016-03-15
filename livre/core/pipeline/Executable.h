/* Copyright (c) 2011-2016, EPFL/Blue Brain Project
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

#ifndef _Executable_h_
#define _Executable_h_

#include <livre/core/types.h>
#include <livre/core/pipeline/Future.h>

namespace livre
{

/**
 * The Executable class is constructed using ExecutableImpl
 * classes. It wraps the methods for execution of ExecutableImpl.
 * a ExecutableImpl. It provides extra pre/post
 * conditions as futures to retrieve the current situation.
 *
 * According to the preconditions or postconditions,
 * executors can decide on scheduling algorithms.
 */
class Executable
{
public:

    struct ExecutableImpl;

    /**
     * @param impl Executable implementation. T class must be
     * derived from ExecutableImpl and must be copy constructible
     */
    template< class T >
    Executable( const T& impl )
        : _impl( ExecutableImplPtr( new T( impl )))
    {}

    /**
     * @return the implementation
     */
    const ExecutableImpl& getImpl() const { return *_impl; }

    /**
     * Executes the executable
     */
    void execute()
    {
        _impl->execute();
    }

    /**
     * @return the output futures for getting the outputs of the executable. The post
     * conditions has to be fullfilled by the execute() implementation ( at the end of
     * execution all futures should be ready )
     */
    Futures getPostconditions() const
    {
        return _impl->getPostconditions();
    }

    /**
     * @return the input futures which the executable can be queried for the state or
     * data retrieval.
     */
    Futures getPreconditions() const
    {
        return _impl->getPreconditions();
    }

    /**
     * Resets the executable by setting all pre and post conditions to an clean state
     * ( The futures are not ready )
     */
    void reset() { _impl->reset(); }

    /**
     * Executable implementation
     */
    struct ExecutableImpl
    {
        /**
         * @copydoc Executable::execute
         */
        virtual void execute() = 0;

        /**
         * @copydoc Executable::getPostconditions
         */
        virtual Futures getPostconditions() const = 0;

        /**
         * @copydoc Executable::getPreconditions
         */
        virtual Futures getPreconditions() const = 0;

        /**
         * @copydoc Executable::reset
         */
        virtual void reset() {}

        virtual ~ExecutableImpl() {}
    };

private:

    typedef std::shared_ptr< ExecutableImpl > ExecutableImplPtr;
    ExecutableImplPtr _impl;
};

}

#endif // _Executable_h_

