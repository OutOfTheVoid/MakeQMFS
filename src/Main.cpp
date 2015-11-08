#include <iostream>
#include <stdint.h>

#include <HostFile.h>
#include <Directory.h>
#include <QMFS.h>

#include <string.h>

#define USAGE_STRING "Usage: "BIN_NAME" [-in <directory in>, -out <file out>, -rootname <name of root folder>]"

#define QMFS_SIGNATURE "qmfs.sig"

int main ( int argc, const char * argv [] )
{
	
	bool InDirFound = false;
	const char * InDirectory = NULL;
	
	bool OutFileFound = false;
	const char * OutFile = NULL;
	
	bool RootNameFound = false;
	const char * RootName = NULL;
	
	for ( int32_t I = 0; I < argc; I ++ )
	{
		
		if ( strcmp ( argv [ I ], "-dir" ) == 0 )
		{
			
			if ( InDirFound || ( argc == I + 1 ) )
			{
				
				std :: cerr << USAGE_STRING << std :: endl;
				
				return 1;
				
			}
			
			InDirectory = argv [ I + 1 ];
			InDirFound = true;
			
			I ++;
			
			continue;
			
		}
		
		if ( strcmp ( argv [ I ], "-out" ) == 0 )
		{
			
			if ( OutFileFound || ( argc == I + 1 ) )
			{
				
				std :: cerr << USAGE_STRING << std :: endl;
				
				return 1;
				
			}
			
			OutFile = argv [ I + 1 ];
			OutFileFound = true;
			
			I ++;
			
			continue;
			
		}
		
		if ( strcmp ( argv [ I ], "-rootname" ) == 0 )
		{
			
			if ( RootName || ( argc == I + 1 ) )
			{
				
				std :: cerr << USAGE_STRING << std :: endl;
				
				return 1;
				
			}
			
			RootName = argv [ I + 1 ];
			RootNameFound = true;
			
			I ++;
			
			continue;
			
		}
		
	}
	
	if ( ! OutFileFound )
		OutFile = "fs.img";
	
	if ( ! InDirFound )
		InDirectory = "./imgdir";
	
	if ( ! RootNameFound )
		RootName = "qmfs_root";
	
	HostFile * OutputFile = HostFile :: OpenFile ( OutFile, true, true );
	
	if ( OutputFile == NULL )
	{
		
		std :: cerr << "Filed to open \"" << OutFile << "\" for writing. Either you have insufficient permissions, or the name is not a valid failename." << std :: endl;
		std :: cerr << USAGE_STRING << std :: endl;
		
		return 3;
		
	}
	
	char InDirReplacement [ strlen ( InDirectory ) + 2 ];
	
	if ( InDirectory [ 0 ] != '/' )
	{
		
		if ( InDirectory [ 0 ] != '.' )
		{
			
			strcpy ( InDirReplacement, "./" );
			strcpy ( & InDirReplacement [ 2 ], InDirectory );
			
			InDirectory = InDirReplacement;
			
		}
		
	}
	
	QMFS :: WriteFS ( OutputFile, InDirectory, RootName );
	
};
