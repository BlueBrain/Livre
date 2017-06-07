//
//  Library: GetSet
//  c++ library for load/saving *typed* and *named* properties and automatic GUI.
//
//  Copyright (c) by Andr� Aichert (aaichert@gmail.com)
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#ifndef __TypeString_hxx
#define __TypeString_hxx

#include <string>
#include <vector>

/// Get a c++ type name as string
template <typename T> inline std::string typeName()
{
	return typeid(T).name();
}

// Specializations
template<> inline std::string typeName<std::string>() {return "string";}
template<> inline std::string typeName<std::vector<std::string> >() {return "vector<string>";}

#define _DEFINE_TYPE(X) template<> inline std::string typeName<std::vector<X> >() {return "vector<"#X">";}
#include "BaseTypes.hxx"
#define _DEFINE_TYPE(X) template<> inline std::string typeName<X>() {return #X;}
#include "BaseTypes.hxx"

inline bool isTypeStr(const std::string& type)
{
	bool isType=false;
	#define _DEFINE_TYPE(T) \
		if (!isType && type==#T) isType=true;
	#include "BaseTypes.hxx"
	#undef _DEFINE_TYPE
	return isType;
}

#endif // __TypeString_hxx
