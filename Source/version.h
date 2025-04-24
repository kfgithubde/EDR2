#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "23";
	static const char MONTH[] = "04";
	static const char YEAR[] = "2025";
	static const char UBUNTU_VERSION_STYLE[] =  "25.04";
	
	//Software Status
	static const char STATUS[] =  "Release";
	static const char STATUS_SHORT[] =  "r";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 6;
	static const long BUILD  = 586;
	static const long REVISION  = 586;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 705;
	#define RC_FILEVERSION 2,6,586,586
	#define RC_FILEVERSION_STRING "2, 6, 586, 586\0"
	static const char FULLVERSION_STRING [] = "2.6.586.586";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 86;
	

}
#endif //VERSION_H
