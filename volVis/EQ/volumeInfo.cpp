
/* Copyright (c) 2011, Maxim Makhinya  <maxmah@gmail.com>
 */


#include "volumeInfo.h"


namespace massVolVis
{

VolumeInfo::VolumeInfo()
    : _fileNameVersion( 0 )
    , _tfVersion(       0 )
    , _rTypeVersion(    0 )
{
    _rType = SLICE; // renderer/renderingTypes.h (RenderingTypes::SLICE)
}

void VolumeInfo::resetTransferFunction()
{
    _tf.initDefault();
    ++_tfVersion;
    setDirty( DIRTY_TF );
}


void VolumeInfo::setModelFileName( const std::string& fileName )
{
    _fileName = fileName;
    _fileNameVersion++;
    setDirty( DIRTY_FILE_NAME );
}


void VolumeInfo::setTransferFunction( const TransferFunction& tf )
{
    _tf = tf;
    ++_tfVersion;
    setDirty( DIRTY_TF );
}

void VolumeInfo::setTensorParameters( const TensorParameters& tp )
{
    _tp = tp;
    ++_tpVersion;
    setDirty( DIRTY_TP );
}


void VolumeInfo::setRendererType( const RendererType rType )
{
    _rType = rType;
    _rTypeVersion++;
    setDirty( DIRTY_RENDERER_TYPE );
}


void VolumeInfo::serialize( co::DataOStream& os, const uint64_t dirtyBits )
{
    co::Serializable::serialize( os, dirtyBits );
    if( dirtyBits & DIRTY_FILE_NAME )
        os  << _fileName << _fileNameVersion;

    if( dirtyBits & DIRTY_TF )
        os << _tf << _tfVersion;

    if( dirtyBits & DIRTY_RENDERER_TYPE )
        os << _rType << _rTypeVersion;

    if( dirtyBits & DIRTY_TP )
        os << _tp << _tpVersion;
}


void VolumeInfo::deserialize( co::DataIStream& is, const uint64_t dirtyBits )
{
    co::Serializable::deserialize( is, dirtyBits );
    if( dirtyBits & DIRTY_FILE_NAME )
        is >> _fileName >> _fileNameVersion;

    if( dirtyBits & DIRTY_TF )
        is >> _tf >> _tfVersion;

    if( dirtyBits & DIRTY_RENDERER_TYPE )
        is >> _rType >> _rTypeVersion;

    if( dirtyBits & DIRTY_TP )
        is >> _tp >> _tpVersion;
}

}//namespace massVolVis
