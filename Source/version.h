#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "27";
	static const char MONTH[] = "03";
	static const char YEAR[] = "2025";
	static const char UBUNTU_VERSION_STYLE[] =  "25.03";
	
	//Software Status
	static const char STATUS[] =  "Release";
	static const char STATUS_SHORT[] =  "r";
	
	//Standard Version Type
	static const long MAJOR  = 2;
	static const long MINOR  = 6;
	static const long BUILD  = 560;
	static const long REVISION  = 560;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT  = 678;
	#define RC_FILEVERSION 2,6,560,560
	#define RC_FILEVERSION_STRING "2, 6, 560, 560\0"
	static const char FULLVERSION_STRING [] = "2.6.560.560";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY  = 60;
	

}
#endif //VERSION_H
