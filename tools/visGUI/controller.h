
/* Copyright (c) 2011, Fatih Erol
 *               2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */


#ifndef MASS_VOL__GUI_CONTROLLER_H
#define MASS_VOL__GUI_CONTROLLER_H

#include <msv/types/types.h> // byte

// EQ includes
#include <co/node.h>
#include <co/localNode.h>


class QString;

namespace ivs
{
    struct TransferFunctionPair;
}

namespace massVolGUI
{

/**
 * This is the class that controls communication between the GUI and the Equalizer application.
 */
class Controller
{
public:

    Controller( ivs::TransferFunctionPair* tf, std::vector<byte>* ranks );

    ~Controller();

    bool connect( const std::string &host, const short port );
    void disconnect();

    bool isConnected() const;

    bool setX( const int value );

    bool updateTF();

    bool updateTensorRanks();

    bool loadFile(  const std::string &fileName  );

private:
    void _onConnect(); //!< force sending new data when connection is established

// connection
    std::string      _host;
    short            _port;

    co::LocalNodePtr _localNode;
    co::NodePtr      _applicationNode;

// data to sync
    int     _x;
    bool    _xNew;

    ivs::TransferFunctionPair* _tf;
    bool                       _tfNew;

    std::vector<byte>*         _ranks;
    bool                       _ranksNew;

    std::string _fileName;
    bool        _fileNameNew;
};  // class Controller

}// namespace massVolGUI


#endif  // MASS_VOL__GUI_CONTROLLER_H


