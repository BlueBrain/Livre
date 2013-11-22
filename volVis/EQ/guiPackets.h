
/* Copyright (c) 2011, Fatih Erol
 *               2011, Maxim Makhinya <maxmah@gmail.com>
 *               2012, David Steiner  <steiner@ifi.uzh.ch>
 *
 */


#ifndef MASS_VOL__GUI_PACKETS_H
#define MASS_VOL__GUI_PACKETS_H

#include "guiConnectionDefs.h"

#include <co/commands.h>
#include <msv/types/types.h>


namespace massVolVis
{

enum GUICommand
{
    CMD_GUI_SETX     = co::CMD_NODE_CUSTOM,
    CMD_GUI_SET_FILE,
    CMD_GUI_SET_TF,
    CMD_GUI_SET_TENSOR_PARAMETERS
};

/*
struct GUISetXPacket : public co::NodePacket
{
    GUISetXPacket() :
        NodePacket(),
        x( 0 )
    {
        command = CMD_GUI_SETX;
        size = sizeof( GUISetXPacket );
    }
    int x;
};


struct GUISetFilePacket : public co::NodePacket
{
    GUISetFilePacket() : NodePacket()
    {
        memset( str, 0, VOL_VIS_GUI_MAX_PATH_LEN );

        command = CMD_GUI_SET_FILE;
        size = sizeof( GUISetFilePacket );
    }
    char str[VOL_VIS_GUI_MAX_PATH_LEN];
};


struct GUISetTFPacket : public co::NodePacket
{
    GUISetTFPacket() : NodePacket()
    {
        command = CMD_GUI_SET_TF;
        size = sizeof( GUISetTFPacket );
    }
    byte rgba[256*4];
    byte  sda[256*3];
};


struct GUISetTensorRanksPacket : public co::NodePacket
{
    GUISetTensorRanksPacket() : NodePacket()
    {
        command = CMD_GUI_SET_TENSOR_PARAMETERS;
        size = sizeof( GUISetTensorRanksPacket );
    }
    byte ranks[10]; // rank[0..9]
};
*/

}// namespace massVolVis


#endif  // MASS_VOL__GUI_PACKETS_H


