#ifndef HOSTDIRECTORY_H
#define HOSTDIRECTORY_H

#include <Vector.h>

class Directory
{
public:
	
	static Vector <const char *> * EnumerateDirectory ( const char * Path );
	static void FreeEnumerationList ( Vector <const char *> * List );
	
	static bool IsDirectory ( const char * Path );
	static bool IsFile ( const char * Path );
	
};

#endif
