#include <Directory.h>

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string.h>

Vector <const char *> * Directory :: EnumerateDirectory ( const char * Path )
{
	
	Vector <const char *> * List;
	
	try
	{
		
		List = new Vector <const char *> ();
		
	}
	catch ( ... )
	{
		
		return NULL;
		
	}
	
	DIR * DirectoryPointer = opendir ( Path );
	struct dirent * Entry;
	
	if ( DirectoryPointer == NULL )
	{
		
		delete List;
		
		return NULL;
		
	}
	
	while ( ( Entry = readdir ( DirectoryPointer ) ) != NULL )
	{
		
		size_t NameSize = strlen ( Entry -> d_name );
		char * Name = NULL;
		
		if ( ( strcmp ( Entry -> d_name, "." ) != 0 ) && ( strcmp ( Entry -> d_name, ".." ) != 0 ) )
		{
			
			try
			{
				
				Name = new char [ NameSize + 1 ];
				
			}
			catch ( ... )
			{
				
				closedir ( DirectoryPointer );
				
				while ( List -> Length () != 0 )
					delete const_cast <char *> ( List -> Pop () );
				
				delete List;
				
				return NULL;
				
			}
			
			strcpy ( Name, Entry -> d_name );
			Name [ NameSize ] = '\0';
			
			List -> Push ( const_cast <const char *> ( Name ) );
			
		}
		
	}
	
	return List;
	
};

void Directory :: FreeEnumerationList ( Vector <const char *> * List )
{
	
	while ( List -> Length () != 0 )
		delete const_cast <char *> ( List -> Pop () );
	
	delete List;
	
};

bool Directory :: IsDirectory ( const char * Path )
{
	
	struct stat FStatus;
	
	stat ( Path, & FStatus );
	
	return S_ISDIR ( FStatus.st_mode );
	
};

bool Directory :: IsFile ( const char * Path )
{
	
	struct stat FStatus;
	
	stat ( Path, & FStatus );
	
	return S_ISREG ( FStatus.st_mode );
	
};


