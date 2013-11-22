
/* Copyright (c) 2011, Fatih Erol
 *               2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */

#ifndef MASS_VOL__GUI_DOCK_H
#define MASS_VOL__GUI_DOCK_H

#include "guiConnectionDefs.h"

#include <msv/types/nonCopyable.h>

#include <co/types.h>
#include <co/localNode.h>

namespace massVolVis
{

struct TransferFunction;
class Config;
class TensorParameters;

class GUINode : public co::LocalNode, private NonCopyable
{
public:
    GUINode();
    bool init( Config* config );

private:
    bool _cmdSetX(    co::ICommand &command );
    bool _cmdSetFile( co::ICommand &command );
    bool _cmdSetTF(   co::ICommand &command );
    bool _cmdSetTensorParameters( co::ICommand &command );

    Config* _config;

    const std::auto_ptr<TransferFunction> _tfTmpPtr;
    const std::auto_ptr<TensorParameters> _tpTmpPtr;
};

/** Dock for eqGUI clients */
class GUIDock
{
public:
    GUIDock(){};
    ~GUIDock();

    bool init( Config* config );
private:
    typedef lunchbox::RefPtr<GUINode> GUINodePtr;
    GUINodePtr _guiNode;
};

}// namespace massVolVis

#endif //MASS_VOL__GUI_DOCK_H


