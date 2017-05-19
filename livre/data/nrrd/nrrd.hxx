// Created by A. Aichert in Aug 2013
#ifndef __NRRD_HXX
#define __NRRD_HXX

// Simple header-only implementation of the NRRD file format
// Does not implement:  line skip, byte skip, orientations, encodings other than raw, kinds other than domain.
// Converts all type strings to C-type equivalent i.e. "uint8" to "unsigned char"
// Always writes "endian" tag (required for FIJI/ImageJ loading)
// Can open files of Big and little endian but saves files only in native format.

#include "StringUtil.hxx"
#include "TypeString.hxx"

#include <map>
#include <string>
#include <fstream>
#include <sstream>

#ifdef NRRD
#undef NRRD
#endif

// Important Functions:
//
//template <typename T>
//bool NRRD::save(const std::string& file, const T* data, int n, const int *size,
//	std::map<std::string,std::string> hdr_keys=std::map<std::string,std::string>,
//	std::map<std::string,std::string> hdr_fields=std::map<std::string,std::string>)
//
//template <typename T>
//bool NRRD::load(const std::string& file, T** data, std::vector<int>* size,
//	std::map<std::string,std::string>* hdr_keys=0x0,
//	std::map<std::string,std::string>* hdr_fields=0x0)
//
// Advanced:
//
//inline std::string NRRD::getDataType(const std::string& file)
//
//inline int NRRD::parseHeader(const std::string& path,
//	std::map<std::string,std::string>& hdr_fields,
//	std::map<std::string,std::string>* hdr_keys=0x0)
//
// Simpler interface for 3D data:
//
//template <typename T>
//std::map<std::string,std::string> NRRD::load3D(const std::string& file, T** data,
//	int *nx, int *ny, int *nz=0x0, // size in voxels
//	double *sx=0x0, double *sy=0x0, double *sz=0x0) // spacing
//
//template <typename T>
//inline bool NRRD::save3D(const std::string& file, const T* data, int w, int h, int d, std::map<std::string,std::string> hdr=std::map<std::string,std::string>())
//

namespace NRRD
{

    /// Endian conversion is not fully implemented. But it is at least asserted.
    inline bool is_cpu_BIG_endian()
    {
       unsigned int u=255;
       unsigned char *c=(unsigned char*)&u;
       return 0!=(*(c+3));
    }

    // Template for endian conversion of one element
    template <int N> inline void swapBytes(void* v);

    /// swap 2 bytes (convert short endian)
    template <> inline void swapBytes<2>(void* v) {
        char *d=(char*)v;
        std::swap(d[0],d[1]);
    }

    /// swap 4 bytes (convert int or float endian)
    template <> inline void swapBytes<4>(void* v) {
        short *d=(short*)v;
        std::swap(d[0],d[1]);
        swapBytes<2>(d);
        swapBytes<2>(d+1);
    }

    /// swap 8 bytes (convert double endian)
    template <> inline void swapBytes<8>(void* v) {
        int *d=(int*)v;
        std::swap(d[0],d[1]);
        swapBytes<4>(d);
        swapBytes<4>(d+1);
    }

    // Template for endian conversion
    template <typename T> inline void convertEndian(T& v) {
        swapBytes<sizeof(v)>((void*)&v);
    }

    /// Helper function to allocate an array of a specific type and fill it with values from an array of a different type
    template <typename S, typename T>
    T* convert(S* src, int n)
    {
        T* tgt=new T[n];
        for (int i=0;i<n;i++)
            tgt[i]=(T)src[i];
        return tgt;
    }

	/// Save raw data in NRRD file.
	template <typename T>
	bool save(const std::string& file, const T* data, int n, const int *size,
		std::map<std::string,std::string> hdr_keys=std::map<std::string,std::string>(),
		std::map<std::string,std::string> hdr_fields=std::map<std::string,std::string>())
	{
		std::ofstream nrrd(file.c_str(),std::ios::binary);
		if (!nrrd||!nrrd.good())
			return false;
		// Magic number v.4
		nrrd << "NRRD0004\n";
		// Type. special case: char maps to int8.
		hdr_fields["type"]=typeName<T>();
		if (hdr_fields["type"]=="char")
			hdr_fields["type"]="int8";
		if (hdr_fields["type"]=="unsigned"||hdr_fields["type"]=="unsigned int")
			hdr_fields["type"]=std::string("uint")+toString(sizeof(T)*8);
		// Dimension and size
		hdr_fields["dimension"]=toString(n);
		hdr_fields["sizes"]=toString(size[0]);
		for (int i=1;i<n;i++)
			hdr_fields["sizes"]=hdr_fields["sizes"]+" "+toString(size[i]);
		// Encoding
		hdr_fields["encoding"]="raw";
		if (is_cpu_BIG_endian()) hdr_fields["endian"]="big";
		else hdr_fields["endian"]="little";
		// Standard fields defined by NRRD format
		for (auto it=hdr_fields.begin();it!=hdr_fields.end();++it)
			nrrd << it->first << ": " << it->second << std::endl;
		// User defined Key/Value pairs
		int ignored=0;
		for (auto it=hdr_keys.begin();it!=hdr_keys.end();++it)
			if (it->second.find('\n') == std::string::npos)
				nrrd << it->first << ":=" << it->second << std::endl;
			else ignored++;
		if (ignored) std::cerr <<  "NRRD::save<T>(...): Ignored " << ignored << " keys in meta info because of invalid characters (e.g. line breaks)\n";
		// Finally, add a comment to allow a used to read the ascii header and manually load raw data
		nrrd << "# Offset to raw data: ";
		std::string raw_read_info_end=" bytes.\n\n";
		int offset=(int)nrrd.tellp();
		offset+=(int)raw_read_info_end.length()+8;
		nrrd << toString(offset,8,' ')+raw_read_info_end;
		int ne=1;
		for (int i=0;i<n;i++) ne*=size[i];
		nrrd.write((char*)data,sizeof(T)*ne);
		return true;
	}

	/// Parse header of NRRD file. Returns number of bytes in header (beginning of raw data)
	/// Type strings are converted to match C-types. Null is returned on error.
	/// Note the order of fields / keys / comments versus keys / fields in all other functions.
	inline int parseHeader(const std::string& path,
		std::map<std::string,std::string>& hdr_fields,
		std::map<std::string,std::string>* hdr_keys=0x0)
	{
		int header_offset=0;
		std::ifstream file(path.c_str(), std::ios::binary);
		if (!file || !file.good())
		{
			std::cerr << "NRRD::parseHeader<T>(...): File access error.\n";
			return 0;
		}
		// 2do: check magic number
		file.ignore(1000,'\n');// ignore first line, hopefully "NRRD000X" X>1
		int line_num=-1;
		while (!file.eof() && file.good())
		{
			line_num++;
			std::string line, key,value;
			std::getline(file,line);
            if( (int)file.tellg() != -1 )
                header_offset=(int)file.tellg();
			if (line.empty()) // beginning of data chunk
				break;
			if (line[0]=='#') // comment
				continue;
			std::istringstream strstr(line);
			std::getline(strstr,key,':');
			char eqsp=0;
			strstr.get(eqsp);
			std::getline(strstr,value,'\n');
			// key or field?
			if (eqsp==' ')
				hdr_fields[key]=value;
			else if (eqsp=='=')
			{
				if (hdr_keys)
					(*hdr_keys)[key]=value;
			}
			else
			{
				if (line.length()>1000) line="<too many characters>";
				std::cerr << "NRRD::parseHeader<T>(...): Error parsing line "<< line_num << ":\n" << line << std::endl;
				return 1;
			}
		}
		file.close();
		// Map all possible NRRD-types to the corresponding C-type to avoid confusion.
		static std::map<std::string,std::string> types;
		if (types.empty())
		{
			// Supports non-standard type name "char" for loading but always writes "int8".
			types["int8_t"]=types["int8"]=types["signed char"]=types["char"]="char";
			types["uchar"]=types["uint8_t"]=types["uint8"]=types["unsigned char"]= "unsigned char";
			types["int16_t"]=types["int16"]=types["signed short int"]=types["signed signed"]=types["short int"]=types["short"]="short";
			types["ushort"]=types["uint16_t"]=types["uint16"]=types["unsigned short int"]=types["unsigned short"]="unsigned short";
			types["int32_t"]=types["int32"]=types["signed int"]=types["int"]="int";
			types["uint32_t"]=types["uint32"]=types["uint"]=types["unsigned int"]="unsigned int";
			types["float"]="float";
			types["double"]="double";
		}
		auto t=types.find(hdr_fields["type"]);
		if (t==types.end())
		{
			std::cerr << "NRRD::parseHeader<T>(...): Unsopported type: " << hdr_fields["type"] << std::endl;
			return 0;
		}
		hdr_fields["type"]=t->second;
		return header_offset;
	}

	/// Try to load raw data from file and convert to T. Use getHeader(...) and check for data type if you want to avoid conversion.
	template <typename T>
	bool load(const std::string& file, T** data, std::vector<int>* size,
		std::map<std::string,std::string>* hdr_keys=0x0,
		std::map<std::string,std::string>* hdr_fields=0x0)
	{
		// Parse ascii header
		std::map<std::string,std::string>	h_fields;
		if (!hdr_fields) hdr_fields=&h_fields;
		int header_offset=parseHeader(file, *hdr_fields, hdr_keys);
		if (header_offset<=0) return false;

		// Make sure the data is uncompressed
		if ((*hdr_fields)["encoding"]!="raw")
		{
			std::cerr << "NDDR::load<T>(...): File encoding not supported\n";
			return false;
		}

		// Make sure the endian matches
		// (this is against the standard: we always have the "endian" tag, because Fiji needs it.)
		if ((*hdr_fields)["endian"]=="") (*hdr_fields)["endian"]="little";
		bool endian_file=(*hdr_fields)["endian"]!="little";
		bool endian_machine=is_cpu_BIG_endian();

		// Make sure the file type matches
		if ((*hdr_fields)["type"]!=typeName<T>())
		{
			int n=1;
			if ((*hdr_fields)["type"]=="")
			{
				std::cerr << "NDDR::load<T>(...): Unknown type.\n";
				return false;
			}
			if (false) /*nop*/;
			#define _DEFINE_TYPE(X)												\
				else if ((*hdr_fields)["type"]==#X) {							\
					X* tmp=0x0;													\
					if(!load<X>(file,&tmp,size,hdr_keys, hdr_fields)) {			\
						std::cerr << "NDDR::load<T>(...): Conversion failed.";	\
						return false;}											\
					for (int i=0;i<(int)size->size();i++)						\
						n*=(*size)[i];											\
					*data=convert<X,T>(tmp, n);									\
					delete [] tmp; }
			#define _DEFINE_TYPE_NO_BOOL
            #include "BaseTypes.hxx"
			#undef _DEFINE_TYPE
		}
		else
		{
			// Get number of dimensions and size
			int n=stringTo<int>((*hdr_fields)["dimension"]);
			*size=stringToVector<int>((*hdr_fields)["sizes"]);
			if (n<=0 || n>10 || (int)size->size()!=n)
			{
				std::cerr << "NDDR::load<T>(...): Bad dimension/size.\n";
				return false;
			}

			// Allocate data
			n=1;
			for (int i=0;i<(int)size->size();i++)
				n*=(*size)[i];
			*data=new T[n];

			// Read data after first blank line
			std::ifstream raw(file.c_str(),std::ios::binary);
			raw.seekg(header_offset,std::ios::beg);
			raw.read((char*)*data,n*sizeof(T));
			int nbytes=(int)(n*sizeof(T)-raw.gcount());
			if (!raw || nbytes)
			{
				std::cerr << "NDDR::load<T>(...): Failed to read complete image chunk. (" << nbytes << ")\n";
				return false;
			}
			// Convert endian if we have to.
			if (endian_file!=endian_machine)
				for (int i=0;i<n;i++)
					convertEndian((*data)[i]);
		}
		return true;
	}

	/// Parse the NRRD header for the "type" field. Always returns C-style type string eg. "int" and neither "int32" nor "signed int"
	inline std::string getDataType(const std::string& file)
	{
		std::map<std::string,std::string>	h_fields;
		parseHeader(file, h_fields);
		return h_fields["type"];
	}

    /// Load 3D NRRD data, size and spacing. Can also be used for 2D. Higher dimensional data use the other overload.
	template <typename T>
	std::map<std::string,std::string> load3D(const std::string& file, T** data,
		int *nx, int *ny, int *nz=0x0,
		double *sx=0x0, double *sy=0x0, double *sz=0x0)
	{
		std::vector<int> size;
		std::map<std::string,std::string> hdr_keys;
		std::map<std::string,std::string> hdr_fields;
		NRRD::load(file,data,&size,&hdr_keys,&hdr_fields);
		if (!data || size.size()<2 || size.size()>3 || hdr_fields.empty())
		{
			hdr_keys.clear();
			return hdr_keys;
		}
		*nx=size[0];
		*ny=size[1];
		if (nz && size.size()==3)
			*nz=size[2];
		else if(nz) *nz=1;

		std::istringstream sp(hdr_fields["spacing"]);
		if (sx) {*sx=1; sp >> *sx;}
		if (sy) {*sy=1; sp >> *sy;}
		if (sz) {*sz=1; sp >> *sz;}

		return hdr_keys;
	}

    /// Overload 3D data. Higher dimensional data and 2D use the other overload.
	template <typename T>
	inline bool save3D(const std::string& file, const T* data, int w, int h, int d, std::map<std::string,std::string> hdr=std::map<std::string,std::string>())
	{
		int size[]={w,h,d};
		return save(file,data,3,size,hdr);
	}
} // namespace NRRD


#endif // __NRRD_HXX
