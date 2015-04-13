/**
 * @file    EqEventHandler.h
 * @brief
 * @author  Ahmet Bilgili
 * @date    2013-22-07
 * @remarks Copyright (c) BBP/EPFL 2005-2012; All rights reserved. Do not distribute without further notice.
 */

#ifndef _EqEventHandler_h_
#define _EqEventHandler_h_

#include <livre/core/types.h>
#include <livre/Eq/types.h>
#include <livre/core/Events/EventHandler.h>

namespace livre
{

/**
 * The EqEventHandler class is the base class for Equalizer based event
 */
class EqEventHandler : public EventHandler
{
public:

    /**
     * operator () Handles the event.
     * @param eventInfo The information needed for the event.
     * @return True, if the event handler succeeds.
     */
    virtual bool operator ()( EqEventInfo& eventInfo ) = 0;

private:

    virtual bool operator ()( EventInfo& eventInfo );
};

}

#endif // EQEVENTHANDLER_H
