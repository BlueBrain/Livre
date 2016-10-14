/* Copyright (c) 2011-2016  Ahmet Bilgili <ahmetbilgili@gmail.com>
 *
 * This file is part of Livre <https://github.com/bilgili/Livre>
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

#ifndef _SignalledVariable_h_
#define _SignalledVariable_h_

#include <livre/core/types.h>

namespace livre
{

/**
 * When assigned a value, it can signal with a given function
 * T type should be copyable
 */
template< class T >
class SignalledVariable
{
public:

    typedef std::function< void(const T&) > Functor;

    /**
     * Constructor
     * @param init is the init value
     * @param func_ is the signalled function
     */
    SignalledVariable( const T& init,
                       const Functor func_ =  Functor( ))
        : var( init )
        , func( func_ )
    {}

    /**
     * Assignment
     * @param set is the new value to set
     * @return itself
     */
    SignalledVariable& operator=( const T& set )
    {
        var = set;
        if( func )
            func( var );
        return *this;
    }

    /** @return the variable */
    const T& get() const { return var; }

    /** @return the variable */
    T& get() { return var; }

private:
    T var;
    Functor func;
};

}
#endif // _SignalledVariable_h_
