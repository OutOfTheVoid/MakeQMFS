#include <HostFile.h>

HostFile :: HostFile ( FILE * FilePointer, bool Writable ):
	FilePointer ( FilePointer ),
	Writable ( Writable ),
	Open ( true )
{
	
	fseek ( FilePointer, 0, SEEK_END );
	Length = ftell ( FilePointer );
	
};

HostFile :: ~HostFile ()
{
	
	Close ();
	
};

void HostFile :: Close ()
{
	
	if ( Open )
		fclose ( FilePointer );
	
	Open = false;
	
};

void HostFile :: Read ( uint8_t * Buffer, uint32_t Offset, uint32_t Length )
{
	
	fseek ( FilePointer, Offset, SEEK_SET );
	fread ( Buffer, sizeof ( uint8_t ), Length, FilePointer );
	
};

void HostFile :: Write ( const uint8_t * Buffer, uint32_t Offset, uint32_t Length )
{
	
	if ( ! Writable )
		return;
	
	fseek ( FilePointer, Offset, SEEK_SET );
	fwrite ( Buffer, sizeof ( uint8_t ), Length, FilePointer );
	
	if ( Offset + Length > this -> Length )
		this -> Length = Offset + Length;
	
};

void HostFile :: Flush ()
{
	
	fflush ( FilePointer );
	
};

uint32_t HostFile :: GetLength ()
{
	
	return Length;
	
};

HostFile * HostFile :: OpenFile ( const char * Path, bool Writable, bool OverWrite )
{
	
	FILE * FilePointer = fopen ( Path, Writable ? ( OverWrite ? "w+b" : "r+b" ) : "rb" );
	
	if ( FilePointer == NULL )
		return NULL;
	
	return new HostFile ( FilePointer, Writable );
	
};
