
#include "rankErrors.h"

#include <msv/util/debug.h>

#include <math.h> // fabs
#include <stdlib.h> // abs

#include <assert.h>
#include <cstring>
#include <limits>

const byte RankErrors::_ranks[6] = {8, 10, 12, 16, 24, 32};

RankErrors::RankErrors()
    : _minError( 0 )
    , _maxError( 0 )
{
}


void RankErrors::set( double err1, double err2, double err3,
                      double err4, double err5, double err6, double multiplier )
{
    uint16_t errs[6];
    const double maxErr = 1000;

    errs[0] = std::max( 0., std::min( err1*multiplier, maxErr ));
    errs[1] = std::max( 0., std::min( err2*multiplier, maxErr ));
    errs[2] = std::max( 0., std::min( err3*multiplier, maxErr ));
    errs[3] = std::max( 0., std::min( err4*multiplier, maxErr ));
    errs[4] = std::max( 0., std::min( err5*multiplier, maxErr ));
    errs[5] = std::max( 0., std::min( err6*multiplier, maxErr ));

    _minError = errs[0];
    _maxError = errs[0];

    for( int i = 1; i < 6; ++i )
    {
        if( _minError > errs[i] || (_minError == 0 && errs[i] > 0 )) _minError = errs[i]; else
        if( _maxError < errs[i] ) _maxError = errs[i];
    }
    std::cout << "min: " << _minError << " max: " << _maxError << " diff: " << _maxError-_minError << std::endl;
}


byte RankErrors::getValidRank( uint16_t error ) const
{
    const byte minRank = _ranks[ 0 ];
    const byte maxRank = _ranks[ 5 ];
    if( isQualityTooLow( error ))
        return maxRank;
    if( isQualityTooHigh( error ))
        return minRank;
    if( _minError == _maxError )
        return minRank;

    byte rank = minRank + (_maxError - error )*(maxRank-minRank)/(_maxError-_minError);
    return std::max( minRank, std::min( rank, maxRank ));
}


//----------------- RankErrorsNearest -----------------

const byte RankErrorsNearest::_ranks[6] = {8, 10, 12, 16, 24, 32};

RankErrorsNearest::RankErrorsNearest()
    : _minErrorPos( 0 )
    , _maxErrorPos( 0 )
{
    assert( sizeof( _errs ) == sizeof(_errs[0])*6 );

    memset( _errs, 0, sizeof( _errs ));
};


void RankErrorsNearest::set( double err1, double err2, double err3,
                      double err4, double err5, double err6, double multiplier )
{
    const double maxErr = 1000;

    _errs[0] = std::max( 0., std::min( err1*multiplier, maxErr ));
    _errs[1] = std::max( 0., std::min( err2*multiplier, maxErr ));
    _errs[2] = std::max( 0., std::min( err3*multiplier, maxErr ));
    _errs[3] = std::max( 0., std::min( err4*multiplier, maxErr ));
    _errs[4] = std::max( 0., std::min( err5*multiplier, maxErr ));
    _errs[5] = std::max( 0., std::min( err6*multiplier, maxErr ));

    _minErrorPos = 0;
    _maxErrorPos = 0;

    for( int i = 1; i < 6; ++i )
    {
        if( _errs[_minErrorPos] > _errs[i] ) _minErrorPos = i; else
        if( _errs[_maxErrorPos] < _errs[i] ) _maxErrorPos = i;
    }
}


byte RankErrorsNearest::getValidRank( uint16_t error ) const
{
    if( isQualityTooLow( error ))
        return _ranks[_minErrorPos];
    if( isQualityTooHigh( error ))
        return _ranks[_maxErrorPos];

    int32_t minErrDiff = abs( static_cast<int32_t>(error) - _errs[0] );
    byte  minErrDiffPos = 0;
    for( int i = 1; i < 6; ++i )
    {
        int32_t errDiff = abs( static_cast<int32_t>(error) - _errs[i] );
        if( minErrDiff > errDiff )
        {
            minErrDiff = errDiff;
            minErrDiffPos = i;
        }
    }
    return _ranks[minErrDiffPos];
}


//----------------- RankErrorsFloat -----------------

const byte RankErrorsFloat::_ranks[6] = {8, 10, 12, 16, 24, 32};

RankErrorsFloat::RankErrorsFloat()
    : _minErrorPos( 0 )
    , _maxErrorPos( 0 )
{
    assert( sizeof( _errs ) == sizeof(_errs[0])*6 );

    memset( _errs, 0, sizeof( _errs ));
};


void RankErrorsFloat::set( float err1, float err2, float err3,
                      float err4, float err5, float err6 )
{
    _errs[0] = err1; _errs[1] = err2; _errs[2] = err3;
    _errs[3] = err4; _errs[4] = err5; _errs[5] = err6;

    _minErrorPos = 0;
    _maxErrorPos = 0;

    for( int i = 1; i < 6; ++i )
    {
        if( _errs[_minErrorPos] > _errs[i] ) _minErrorPos = i; else
        if( _errs[_maxErrorPos] < _errs[i] ) _maxErrorPos = i;
    }
}


byte RankErrorsFloat::getValidRank( float error ) const
{
    if( isQualityTooLow( error ))
        return _ranks[_minErrorPos];
    if( isQualityTooHigh( error ))
        return _ranks[_maxErrorPos];

    float minErrDiff = fabs( error - _errs[0] );
    byte  minErrDiffPos = 0;
    for( int i = 1; i < 6; ++i )
    {
        float errDiff = fabs( error - _errs[i] );
        if( minErrDiff > errDiff )
        {
            minErrDiff = errDiff;
            minErrDiffPos = i;
        }
    }
    return _ranks[minErrDiffPos];
}

//----------------- RankErrorsLinear -----------------

const float RankErrorsLinear::_ranks[6] = {8, 10, 12, 16, 24, 32};

RankErrorsLinear::RankErrorsLinear()
{
    assert( sizeof( _errs ) == sizeof(_errs[0])*6 );

    memset( _errs, 0, sizeof( _errs ));
};


void RankErrorsLinear::set( float err1, float err2, float err3,
                      float err4, float err5, float err6 )
{
    _errs[0] = err1; _errs[1] = err2; _errs[2] = err3;
    _errs[3] = err4; _errs[4] = err5; _errs[5] = err6;
    for( int i = 5; i > 0; --i )
        if( _errs[i-1] <= _errs[i] )
        {
            LOG_ERROR << "wrong rank errors: [" << i << "] " << _errs[i-1] << ", " << _errs[i] << std::endl;
            _errs[i-1] = _errs[i] + 0.001;
        }
}


byte RankErrorsLinear::getValidRank( float error ) const
{
    if( isQualityTooLow( error ))
        return 32;
    if( isQualityTooHigh( error ))
        return 8;

    for( int i = 1; i < 6; ++i )
        if( error >= _errs[i] )
        {
            assert( error <= _errs[i-1] );
            assert( _errs[i-1] > _errs[i] );
            float a = (_errs[i-1]-error)/(_errs[i-1]-_errs[i]);
            return _ranks[i-1]*a + _ranks[i]*(1.f-a) + 0.9999f; // selecting next sufficient rank
        }
    LOG_ERROR << "Wrong error to rank matching" << std::endl;
    return 32;
}
