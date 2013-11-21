
/* Copyright (c) 2011, Maxim Makhinya  <maxmah@gmail.com>
 */

#ifndef MASS_VOLL__VOLUME_INFO_H
#define MASS_VOLL__VOLUME_INFO_H

#include <eq/eq.h>
#include "../renderer/rendererTypes.h"
#include "../renderer/transferFunction.h"
#include "../asyncFetcher/compression/tensorParameters.h"

namespace massVolVis
{

class VolumeInfo : public co::Serializable
{
public:

    VolumeInfo();

    void setModelFileName(    const std::string&      fileName );
    void setTransferFunction( const TransferFunction& tf       );
    void resetTransferFunction();
    void setRendererType(     const RendererType      rType    );
    void setTensorParameters( const TensorParameters& tp       );

    const std::string&      getModelFileName()    const { return _fileName; }
    const TransferFunction& getTransferFunction() const { return _tf;       }
          RendererType      getRendererType()     const { return _rType;    }
    const TensorParameters& getTensorParameters() const { return _tp;       }

    uint32_t getModelFileNameVersion()    const { return _fileNameVersion; }
    uint32_t getTransferFunctionVersion() const { return _tfVersion;       }
    uint32_t getRendererTypeVersion()     const { return _rTypeVersion;    }
    uint32_t getTensorParametersVersion() const { return _tpVersion;       }


protected:
    /** @sa Object::serialize() */
    virtual void serialize(   co::DataOStream& os, const uint64_t dirtyBits );

    /** @sa Object::deserialize() */
    virtual void deserialize( co::DataIStream& is, const uint64_t dirtyBits );

    virtual ChangeType getChangeType() const { return DELTA; }

    /** The changed parts of the data since the last pack(). */
    enum DirtyBits
    {
        DIRTY_FILE_NAME     = co::Serializable::DIRTY_CUSTOM << 0,
        DIRTY_TF            = co::Serializable::DIRTY_CUSTOM << 1,
        DIRTY_RENDERER_TYPE = co::Serializable::DIRTY_CUSTOM << 2,
        DIRTY_TP            = co::Serializable::DIRTY_CUSTOM << 3
    };

private:

    std::string         _fileName;
    TransferFunction    _tf;
    RendererType        _rType;
    TensorParameters    _tp;

    uint32_t            _fileNameVersion;
    uint32_t            _tfVersion;
    uint32_t            _rTypeVersion;
    uint32_t            _tpVersion;
};


}//namespace massVolVis


#endif // MASS_VOLL__VOLUME_INFO_H

