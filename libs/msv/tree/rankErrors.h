
#ifndef MASS_VOL__RANK_ERRORS_H
#define MASS_VOL__RANK_ERRORS_H

#include <msv/types/types.h>
#include <iostream>

// Errors are given for ranks 8, 10, 12, 16, 24, 32
class RankErrors
{
public:
    RankErrors();
    byte getValidRank( uint16_t error ) const;

    void set( double err1, double err2, double err3,
              double err4, double err5, double err6, double multiplier );

    bool isQualityTooLow(  uint16_t error ) const { return _minError > error; }
    bool isQualityTooHigh( uint16_t error ) const { return _maxError < error; }

private:
    uint16_t _minError;
    uint16_t _maxError;
    static const byte _ranks[6];

    friend std::ostream& operator<< ( std::ostream& ostr, const RankErrors& rErr )
    {
        ostr << "[" << rErr._minError << ", " << rErr._maxError << "] ";
        return ostr;
    }
};

typedef std::vector< RankErrors > RankErrorsVec;


class RankErrorsNearest
{
public:
    RankErrorsNearest();
    byte getValidRank( uint16_t error ) const;

    void set( double err1, double err2, double err3,
              double err4, double err5, double err6, double multiplier );

    bool isQualityTooLow(  uint16_t error ) const { return _errs[_minErrorPos] > error; }
    bool isQualityTooHigh( uint16_t error ) const { return _errs[_maxErrorPos] < error; }

private:
    uint16_t _errs[6];
    byte _minErrorPos;
    byte _maxErrorPos;
    static const byte _ranks[6];

    friend std::ostream& operator<< ( std::ostream& ostr, const RankErrorsNearest& rErr )
    {
        ostr << "[" << rErr._errs[0] << ", " << rErr._errs[1] << ", " << rErr._errs[2] << ", "
                    << rErr._errs[3] << ", " << rErr._errs[4] << ", " << rErr._errs[5] << "] ";
        return ostr;
    }
};

class RankErrorsFloat
{
public:
    RankErrorsFloat();
    byte getValidRank( float error ) const;

    void set( float err1, float err2, float err3,
              float err4, float err5, float err6 );

    bool isQualityTooLow(  float error ) const { return _errs[_minErrorPos] > error; }
    bool isQualityTooHigh( float error ) const { return _errs[_maxErrorPos] < error; }

private:
    float _errs[6];
    byte _minErrorPos;
    byte _maxErrorPos;
    static const byte _ranks[6];

    friend std::ostream& operator<< ( std::ostream& ostr, const RankErrorsFloat& rErr )
    {
        ostr << "[" << rErr._errs[0] << ", " << rErr._errs[1] << ", " << rErr._errs[2] << ", "
                    << rErr._errs[3] << ", " << rErr._errs[4] << ", " << rErr._errs[5] << "] ";
        return ostr;
    }
};


class RankErrorsLinear
{
public:
    RankErrorsLinear();
    byte getValidRank( float error ) const;

    void set( float err1, float err2, float err3,
              float err4, float err5, float err6 );

    bool isQualityTooLow(  float error ) const { return _errs[5] > error; }
    bool isQualityTooHigh( float error ) const { return _errs[0] < error; }

private:
    float _errs[6];
    static const float _ranks[6];

    friend std::ostream& operator<< ( std::ostream& ostr, const RankErrorsLinear& rErr )
    {
        ostr << "[" << rErr._errs[0] << ", " << rErr._errs[1] << ", " << rErr._errs[2] << ", "
                    << rErr._errs[3] << ", " << rErr._errs[4] << ", " << rErr._errs[5] << "] ";
        return ostr;
    }
};

#endif //MASS_VOL__RANK_ERRORS_H
