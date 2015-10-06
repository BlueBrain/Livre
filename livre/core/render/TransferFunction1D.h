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

#ifndef _TransferFunction_h_
#define _TransferFunction_h_

#include <livre/core/types.h>

namespace livre
{

const size_t N_CHANNELS = 4;

enum Channel
{
    RED = 0,
    GREEN,
    BLUE,
    ALPHA
};

enum InterpolationAlgorithmType
{
    IAT_LINEAR,
    IAT_DIRAC
};

typedef Vector2f ControlPoint;
typedef std::list< Vector2f > ControlPoints;

class InterpolationAlgorithm1D
{
public:

    InterpolationAlgorithm1D() {}
    virtual ~InterpolationAlgorithm1D() {}

    virtual InterpolationAlgorithmType getType() = 0;

    virtual void compute( const TransferFunction& transferFunction,
                          Channel channel,
                          FloatVector& result ) const = 0;

    const ControlPoints& getControlPoints( Channel channel ) const { return _controlPoints[ channel ]; }

protected:

    ControlPoints _controlPoints[ N_CHANNELS ];
};

class LinearInterpolationAlgorithm1D : public InterpolationAlgorithm1D
{
public:

    LinearInterpolationAlgorithm1D();

    InterpolationAlgorithmType getType() final { return IAT_LINEAR; }

    void compute(  const TransferFunction& transferFunction,
                   Channel channel,
                   FloatVector& result ) const final;

    void addControlPoint( const ControlPoint& controlPoint, Channel channel );
    void removeControlPoint( const ControlPoint& controlPoint, Channel channel );
    void setControlPoints( const ControlPoints& controlPoints, Channel channel );

private:

    struct Impl;
    Impl* _impl;
};

class DiracInterpolationAlgorithm1D : public InterpolationAlgorithm1D
{
public:

    DiracInterpolationAlgorithm1D();

    InterpolationAlgorithmType getType() final { return IAT_DIRAC; }

    void compute(  const TransferFunction& transferFunction,
                   Channel channel,
                   FloatVector& result ) const final;

    void setControlPoint( const ControlPoint& controlPoint, Channel channel ) final;

private:

    struct Impl;
    Impl* _impl;
};

class TransferFunction
{
public:

    struct Header
    {
        Header( DataType dataType_ = DT_UINT8,
                size_t nElements = 256 )
            : dataType( dataType_ )
            , nElements( nElements_ )
        {}

        DataType dataType;
        size_t nElements;
    };

    LIVRECORE_API explicit TransferFunction( const Header& format );
    LIVRECORE_API explicit TransferFunction( const TransferFunction& tf );

    ~TransferFunction();

    TransferFunctionData1D getTransferFunctionData() const;

    LIVRECORE_API Format getFormat() const;

    LIVRECORE_API bool addAlgorithm( const std::string& name,
                                     InterpolationAlgorithmType type );

    InterpolationAlgorithmPtr getAlgorithm( const std::string& name ) const;

    LIVRECORE_API bool removeAlgorithm( const std::string& name );

    LIVRECORE_API void compute( Channel channel,
                                FloatVector& result ) const;

    LIVRECORE_API bool load( const std::string& filename );
    LIVRECORE_API bool save( const std::string& filename );

    int uploadToGPU( int internalFormat ) const;

private:

    struct Impl;
    Impl* _impl;
};

std::ostream& operator<<( std::ostream& os,
                         const TransferFunctionData1D& tf );

std::istream& operator>>( std::istream& is,
                          TransferFunctionData1D& tf );

co::DataOStream& operator<<( co::DataOStream& os,
                             const TransferFunctionData1D& tf );

co::DataIStream& operator>>( co::DataIStream& is,
                             TransferFunctionData1D& tf );
}

#endif // _RawTransferFunction_h_
