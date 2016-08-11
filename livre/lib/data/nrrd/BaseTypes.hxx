// 
// Disabling types:
// _DEFINE_TYPE_NO_BOOL		<> bool
// _DEFINE_TYPE_NO_CHAR		<> char (UNSIGNED CHAR STILL USED)
// _DEFINE_TYPE_NO_INT		<> short, int, long, unsigned short, unsigned int, unsigned long
// _DEFINE_TYPE_NO_SHORT	<> short, unsigned short
// _DEFINE_TYPE_NO_LONG		<> long unsigned long
// _DEFINE_TYPE_NO_FLOATING_POINT <> float, double
// _DEFINE_TYPE_NO_DOUBLE	<> double
// _DEFINE_TYPE_NO_UNSIGNED <> unsigned anything


#ifndef _DEFINE_TYPE_NO_BOOL
							_DEFINE_TYPE(bool)
#endif

#ifndef _DEFINE_TYPE_NO_CHAR
							_DEFINE_TYPE(char)
#endif
#ifndef _DEFINE_TYPE_NO_UNSIGNED
							_DEFINE_TYPE(unsigned char)
#endif


#ifndef _DEFINE_TYPE_NO_INT
	#ifndef _DEFINE_TYPE_NO_SHORT
							_DEFINE_TYPE(short)
		#ifndef _DEFINE_TYPE_NO_UNSIGNED
							_DEFINE_TYPE(unsigned short)
		#endif
	#endif
							_DEFINE_TYPE(int)
		#ifndef _DEFINE_TYPE_NO_UNSIGNED
							_DEFINE_TYPE(unsigned int)
		#endif
	#ifndef _DEFINE_TYPE_NO_LONG
							_DEFINE_TYPE(long)
		#ifndef _DEFINE_TYPE_NO_UNSIGNED
							_DEFINE_TYPE(unsigned long)
		#endif
	#endif
#endif

#ifndef _DEFINE_TYPE_NO_FLOATING_POINT
							_DEFINE_TYPE(float)
	#ifndef _DEFINE_TYPE_NO_DOUBLE
                            _DEFINE_TYPE(double)
	#endif
#endif

#undef _DEFINE_TYPE
