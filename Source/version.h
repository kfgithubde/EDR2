#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "10";
	static const char MONTH[] = "05";
	static const char YEAR[] = "2025";
	static const char UBUNTU_VERSION_STYLE[] =  "25.05";
	
	//Software Status
	static const char STATUS[] =  "Release";
	static const char STATUS_SHORT[] =  "r";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 8;
	static const long BUILD  = 607;
	static const long REVISION  = 607;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 740;
	#define RC_FILEVERSION 2,8,607,607
	#define RC_FILEVERSION_STRING "2, 8, 607, 607\0"
	static const char FULLVERSION_STRING [] = "2.8.607.607";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 7;
	

}
#endif //VERSION_H
