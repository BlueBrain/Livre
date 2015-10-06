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

#include <livre/core/render/TransferFunction1D.h>
#include <livre/core/render/TransferFunctionData.h>
#include <livre/core/util/Utilities.h>

#include <co/dataIStream.h>
#include <co/dataOStream.h>

#include <eq/gl.h>

#include <boost/filesystem/path.hpp>

#include <fstream>
#include <limits>

namespace livre
{

bool sortControlPointsX( const ControlPoint& p1,
                         const ControlPoint& p2 )
{
    return p1[ 0 ] < p2[ 0 ];
}

bool sortControlPointsY( const ControlPoint& p1,
                         const ControlPoint& p2 )
{
    return p1[ 1 ] < p2[ 1 ];
}

class LinearInterpolationAlgorithm::Impl
{
    Impl()
        : _isDirty( true )
    {
        _controlPoints.push_back( ControlPoint( 0.0, 0.0, 0.0 ));
        _controlPoints.push_back( ControlPoint( 1.0, 0.0, 1.0 ));
    }

    void addControlPoint( const ControlPoint& controlPoint )
    {
        BOOST_FOREACH( const ControlPoint& cp, _controlPoints )
        {
            if( std::abs( 1.0f - cp.dot( controlPoint )) <= std::numeric_limits<float>::epsilon( ))
                return; // There is already a control point on that location
        }

        _controlPoints.push_back( controlPoint );
        _isDirty = true;
    }

    void removeControlPoint( const ControlPoint& controlPoint )
    {
        ControlPoints::const_iterator it = controlPoints.begin();
        while( it != controlPoints.end( ))
        {
            if( std::abs( 1.0f - cp.dot( _controlPoints )) <= std::numeric_limits<float>::epsilon( ))
            {
                controlPoints.remove( it );
                _isDirty = true;
                return;
            }
            else
                ++it;
        }
    }

    void compute(  const TransferFunction& transferFunction,
                   FloatVector& result ) const final
    {
        if( !_isDirty )
        {
            result = _rawData;
            return result;
        }

        result.resize( transferFunction.getFormat().nXElements *
                       transferFunction.getFormat().nYElements );

        // Implement based on x, y interpolation
        ControlPoints xSorted;
        if( format.nXElements > 0 )
        {
            xSorted = _controlPoints;
            std::sort( xSorted.begin(), xSorted.end(), sortControlPointsX );
        }

        for( uint32_t elementX = 0; elementX < format.nXElements; ++elementX )
            for( uint32_t channel = 0; channel < format.nChannels; ++nChannels )
            {
                const float x = (float)elementX / (float)format.nXElements;

                ControlPoints::const_iterator lowX =
                        std::lower_bound( xSorted.begin(), xSorted.end(), x );

                // Computes only depending on x dimension
                const float startX = *lowX;
                const float endX = *( lowX + 1 );
                const float distanceRecp = 1.0f / ( endX - startX );

                const size_t index = elementY * format.nXElements * format.nChannels +
                                     elementX * format.nChannels +
                                     channel;

                _rawData[ index ] = ( startX * ( endX - x ) + endX * ( x - beginX )) * distanceRecp;
             }

       result = _rawData;
       _isDirty = false;
    }

    void setControlPoints( const ControlPoints& controlPoints )
    {
        if( controlPoints.size() < 2 )
            LBTHROW( std::runtime_error( "At least two contol points should be provided"))
        _controlPoints = controlPoints;
        _isDirty = true;
    }

    std::string& _name;
    bool _isDirty;
    FloatVector _result;
};

LinearInterpolationAlgorithm::LinearInterpolationAlgorithm()
    : _impl( new LinearInterpolationAlgorithm::Impl())
{}

LinearInterpolationAlgorithm::~LinearInterpolationAlgorithm()
{
    delete _impl;
}

class DiracInterpolationAlgorithm::Impl
{
    Impl()
        : _isDirty( true )
    {
        controlPoint.push_back( ControlPoint( 0.5, 0.0, 1.0 ));
    }

    void compute(  const TransferFunction& transferFunction,
                   FloatVector& result ) const final
    {
        if( !_isDirty )
        {
            result = _rawData;
            return result;
        }

       result = _rawData;
       _isDirty = false;
    }

    void setControlPoint( const ControlPoint& controlPoint )
    {
        if( std::abs( 1.0f - cp.dot( controlPoint[ 0 ] )) <= std::numeric_limits<float>::epsilon( ))
            return; // There is already a control point on that location

        _controlPoints[ 0 ] = controlPoint;
        _isDirty = true;
    }

    bool _isDirty;
};

DiracInterpolationAlgorithm::DiracInterpolationAlgorithm()
    : _impl( new DiracInterpolationAlgorithm::Impl())
{}

DiracInterpolationAlgorithm::~DiracInterpolationAlgorithm()
{
    delete _impl;
}

typedef std::map< std::string, InterpolationAlgorithms > NameInterpolationAlgorithmsMap;
typedef std::pair< std::string, InterpolationAlgorithms > NameInterpolationAlgorithmsPair;

struct TransferFunction::Impl
{
    Impl( const TransferFunction::Format& format )
    {}

    Impl( const TransferFunction& tf )
    {}

    bool addAlgorithm( const std::string& name,
                       InterpolationAlgorithmType type )
    {}

    InterpolationAlgorithmPtr getAlgorithm( const std::string& name,
                                            const size_t channel ) const
    {}

    bool removeAlgorithm( const std::name )
    {}

    void compute( const size_t channel,
                  FloatVector& result ) const
    {}

    void fillTransferFunctionData( TransferFunctionData& data )
    {
        std::vector< uint32_t > algorithms;
        std::vector< std::string > algorithmNames;
        BOOST_FOREACH( const NameInterpolationAlgorithmsPair& pair, _algorithmMap )
        {
            algorithms.push_back( pair.second[0].getType( ));
            algorithmNames.push_back( pair.first );
        }

        data.setAlgorithms( algorithms );
    }

    bool load( const std::string& filename )
    {





    }

    bool save( const std::string& filename )
    {
        TransferFunctionData data;
        fillTransferFunctionData( data );
        std::
    }

    TransferFunction::Format _format;
    NameInterpolationAlgorithmsMap _algorithmMap;
};

TransferFunction::TransferFunction( const Format& format )
{}

TransferFunction::TransferFunction( const TransferFunction& tf )
{}

Format TransferFunction::getFormat() const
{}

bool TransferFunction::addAlgorithm( const std::string& name,
                                     InterpolationAlgorithmType type )
{}

bool TransferFunction::removeAlgorithm( const std::string& name )
{}

virtual void TransferFunction::compute( const size_t channel,
                                        FloatVector& result ) const;

TransferFunction::~TransferFunction()
{}

bool TransferFunction::load( const std::string& filename )
{}

bool TransferFunction::save( const std::string& filename )
{}

struct ControlPointTransferFunction::Impl
{
    Impl( ControlPointTransferFunction& transferFunction,
          const ControlPoints& controlPoints,
          InterpolationMethod method )
        : _transferFunction( transferFunction )
        , _tfTexture( -1 )
        , _isDirty( true )
        , _interpolationMethod( method )
    {
    }

    void fillWithDefaultData()
    {
        _controlPoints.push_back( ControlPoint( 0.0f, 0.0f, 0.0f ));
        _controlPoints.push_back( ControlPoint( 1.0f, 0.0f, 1.0f ));
    }

    bool load( const std::string& filename )
    {
        std::ifstream ofs( filename );
        if( ifs.is_open())
        {
            LBERROR( "Cannot open the file for writing" );
            return false;
        }

        deserialize( ifs );
        ifs.close();
        isDirty = true;
        return true;
    }

    void save( const std::string& filename )
    {
        std::ofstream ofs( filename );
        if( ofs.is_open())
        {
            LBERROR( "Cannot open the file for writing");
            return false;
        }

        serialize( ofs );
        ofs.close();
        return true;
    }


    void computeTransferFunction()
    {
        // TODO : support more types
        const TransferFunction::Format& format = _transferFunction.getFormat();
        if( _rawData.empty( ))
        {
            const size_t elementSize = livre::util::getDataTypeSize( format.dataType );
            const size_t dataSize = elementSize *
                                    format.nXElements *
                                    format.nYElements *
                                    format.nChannels;

            _rawData.reserve( dataSize );
        }

        if( !isDirty )
            return;

        ControlPoints xSorted;
        if( format.nXElements > 0 )
        {
            xSorted = _controlPoints;
            std::sort( xSorted.begin(), xSorted.end(), sortControlPointsX );
        }

        ControlPoints ySorted;
        if( format.nYElements > 0 )
        {
            ySorted = _controlPoints;
            std::sort( ySorted.begin(), ySorted.end(), sortControlPointsY );
        }

        for( uint32_t elementY = 0; elementY < format.nYElements; ++elementY )
            for( uint32_t elementX = 0; elementX < format.nXElements; ++elementX )
                for( uint32_t channel = 0; channel < format.nChannels; ++nChannels )
                {
                    const float x = (float)elementX / (float)format.nXElements;
                    const float y = (float)elementY / (float)format.nYElements;
                    const float z = _interpolationMethod->compute( xSorted,
                                                                   x,
                                                                   ySorted,
                                                                   y );
                    const size_t index = elementY * format.nXElements * format.nChannels +
                                         elementX * format.nChannels +
                                         channel;

                    _rawData[ index ] =
                            y * std::numeric_limits< unsigned char >::max();
                }
    }

    int uploadToGPU( int internalFormat )
    {
        if( internalFormat != GL_RGBA )
            LBTHROW( std::runtime_error( "Only GL_RGBA is format is supported" ));

        computeTransferFunction();

        // TODO : support more types
        // TODO : fill & return a texture object
        // Do not update transfer function texture
        if( _tfTexture >= 0 && !isDirty )
            return _tfTexture;

        const TransferFunction::Format defaultFormat;
        if( _transferFunction.getFormat() != defaultFormat )
            return -1;

        if( _tfTexture == -1 )
        {
            glGenTextures( 1, &_tfTexture );
            glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        }

        glBindTexture( GL_TEXTURE_1D, _tfTexture );
        glTexImage1D(  GL_TEXTURE_1D, 0,
                       internalFormat,
                       GLsizei( _transferFunction.getFormat().nElements ), 0,
                       GL_RGBA, // Should be derived from channel info
                       GL_UNSIGNED_BYTE, // Should be derived from data type
                       &rawData[ 0 ] );

        isDirty = false;
        return _tfTexture;

    }

    std::ostream& serialize( std::ostream& os ) const
    {

    }

    std::istream& deserialize( std::ostream& is )
    {

    }


    co::DataOStream& serialize( co::DataOStream& os ) const
    {

    }

    co::DataIStream& deserialize( co::DataIStream& is )
    {

    }

    GLuint _tfTexture;
    InterpolationAlgorithms _algorithms;
};


std::istream& operator>>( std::istream& is,
                          TransferFunction& tf )
{
    return tf._impl->derialize( os );
}

co::DataOStream& operator<<( co::DataOStream& os,
                             const TransferFunction& tf )
{
    return tf._impl->serialize( os );
}

co::DataIStream& operator>>( co::DataIStream& is,
                             TransferFunction& tf )
{
    return tf._impl->derialize( os );
}

}
