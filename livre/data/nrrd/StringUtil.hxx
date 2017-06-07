//
//  Library: GetSet
//  c++ library for load/saving *typed* and *named* properties and automatic GUI.
//
//  Copyright (c) by André Aichert (aaichert@gmail.com)
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#ifndef __StringUtil_hxx
#define __StringUtil_hxx

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>

/// Reset value (call constructor or zeros c-types, see specializations)
template <typename T> inline void reset(T& v)
{
	v=T();
}

// Type general conversion to string
template <typename T> inline std::string toString(const T& in)
{
	std::ostringstream strstr;
	strstr << in;
	return strstr.str();
}

// General conversion from string to another type
template <typename T> inline T stringTo(const std::string& in)
{
	T value;
	reset(value);
	std::istringstream strstr(in);
	strstr >> value;
	return value;
}

template <> inline std::string toString<>(const std::string& in) { return in; }
template <> inline std::string stringTo<>(const std::string& in) { return in; }
template <> inline std::string toString<>(const bool& in) { return in ? "true" : "false"; }
template <> inline bool stringTo<>(const std::string& in)
{
	if (in=="" || in=="false" || in=="no") return false;
	if (in=="true" || in=="yes") return true;
	return stringTo<int>(in)>0;
}

// Conversion from vector of any type to string
template <typename T> inline std::string vectorToString(const typename std::vector<T>& in, const std::string& delim=" ")
{
    if (in.empty()) return std::string();
    typename std::vector<T>::const_iterator it=in.begin();
	std::string ret=toString(*it);
	for (++it;it!=in.end();++it)
        ret+=delim+toString(*it);
    return ret;
}

// Conversion of a string to a vector of any type
template <typename T> inline std::vector<T> stringToVector(const std::string& in, const char delim=' ')
{
	std::string item;
	std::vector<T> ret;
	std::istringstream str(in);
	for (;std::getline(str,item,delim);str&&!str.eof())
		ret.push_back(stringTo<T>(item));
	if (item.empty()&&!ret.empty()) ret.pop_back();
	return ret;
}

// Specializations of toString and stringTo for select vector-types assuming seperating semicolon
#define _DEFINE_TYPE(TYPE) \
	template <> inline std::string toString<>(const std::vector<TYPE>& in) {return vectorToString(in,";");} \
	template <> inline std::vector<TYPE> stringTo<>(const std::string& in) {return stringToVector<TYPE>(in,';');}
_DEFINE_TYPE(std::string)
#include "BaseTypes.hxx"

// Specializations
#define _DEFINE_TYPE(X)  template<> inline void reset<X>(X& v) {v=0;}
#include "BaseTypes.hxx"

/// Right trim
inline void rtrim(std::string &str , const std::string& t = " \t")
{
	str.erase(str.find_last_not_of(t)+1);
}

/// Left trim
inline void ltrim(std::string& str, const std::string& t = " \t")
{
	str.erase(0,str.find_first_not_of(t));
}

/// Trim
inline void trim(std::string& str, const std::string& t = " \t")
{
	rtrim(str,t);
	ltrim(str,t);
}

/// Remove the part right of last occurence of delim and return it
inline std::string splitRight(std::string& str, const std::string& delim)
{
	std::string::size_type loc=str.find_last_of(delim);
	std::string right;
	if (loc!=std::string::npos)
	{
		right=str.substr(loc+1,std::string::npos);
		str=str.substr(0,loc); // left
	}
	else
	{
		right=str;
		str.clear();
	}
	return right;
}

/// Remove the part left of first occurence of delim and return it
inline std::string splitLeft(std::string& str, const std::string& delim)
{
	std::string::size_type loc=str.find_first_of(delim);
	std::string left;
	if (loc!=std::string::npos)
	{
		left=str.substr(0,loc);
		str=str.substr(loc+1,std::string::npos);
	}
	else
	{
		left=str;
		str.clear();
	}
	return left;
}

/// Overload of string conversion for specific lengths
template <typename T> inline std::string toString(T in, int width, char fill='0')
{
	std::ostringstream strstr;
	strstr << std::setfill(fill) << std::setw(width) << in;
	return strstr.str();
}

/// write text to file
inline bool fileWriteString(const std::string& filename, const std::string& contents)
{
	std::ofstream file(filename.c_str());
	if (!file) return false;
	file << contents;
	file.close();
	return true;
}

/// Read a complete text file
inline std::string fileReadString(const std::string filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open() && file.good())
		return "";
	std::string all;
	getline(file,all,'\0');
	file.close();
	return all;
}

/// Make all lower-case no whitespace strings
inline void normalize(std::string& name)
{
	std::transform(name.begin(),name.end(),name.begin(),tolower);
	std::replace(name.begin(),name.end(),' ','-');
	std::replace(name.begin(),name.end(),'/','-');
}

/// Parse XML-Style attributes into an std::map of strings
inline void parseAttribs(const std::string& in, std::map<std::string,std::string>& out)
{
	size_t pos=0;
	std::string key,value;
	for (;;)
	{
		size_t next=in.find("=",pos);
		size_t v1=in.find("\"",next);
		size_t v2=in.find("\"",v1+1);
		if (next==std::string::npos||v1==std::string::npos||v2==std::string::npos)
			return;
		if (v2!=std::string::npos)
		{
			key=in.substr(pos,next-pos);
			value=in.substr(v1+1,v2-v1-1);
			trim(key);
			trim(value);
			out[key]=value;
			pos=v2+1;
		}
	}
}


#endif // __StringUtil_hxx
