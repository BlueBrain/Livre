
/* Copyright (c) 2006-2014, Stefan Eilemann <eile@equalizergraphics.com>
 *               2007-2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#include "initData.h"

namespace massVolVis
{

InitData::InitData()
    : _frameDataId()
    , _volumeInfoId()
#ifdef AGL
    , _windowSystem( "AGL" ) // prefer over GLX
#else
    , _windowSystem()
#endif
    , _filename( "" )
{}


InitData::~InitData()
{}


void InitData::getInstanceData( co::DataOStream& os )
{
    os << _frameDataId << _volumeInfoId << _windowSystem << _filename;
}


void InitData::applyInstanceData( co::DataIStream& is )
{
    is >> _frameDataId >> _volumeInfoId >> _windowSystem >> _filename;

    LBASSERT( _frameDataId != 0 );
}


}//namespace massVolVis
