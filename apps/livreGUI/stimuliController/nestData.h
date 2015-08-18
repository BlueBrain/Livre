#ifndef _nestData_h_
#define _nestData_h_


#include <livreGUI/types.h>
#include <livre/core/types.h>

namespace livre
{

/**
 * @return Returns list of NEST generators
 */
const Strings& getGenerators();

/**
 * @param generator NEST generator
 * @return Returns the properties for the given NEST generator
 */
const PropertyList& getGeneratorProperties( const std::string& generator );

/**
 * @param list NEST generator properties
 * @return JSON text for given properties
 */
std::string getJSON( const PropertyList& list );

}


#endif // _nestData_h_

