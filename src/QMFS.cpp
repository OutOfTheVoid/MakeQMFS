#include <QMFS.h>
#include <Directory.h>
#include <string.h>

void QMFS :: WriteHeader ( HostFile * File, uint32_t RootDirectoryOffset )
{
	
	uint8_t Buffer [ 16 ];
	
	Buffer [ 0 ] = 'q';
	Buffer [ 1 ] = 'm';
	Buffer [ 2 ] = 'f';
	Buffer [ 3 ] = 's';
	Buffer [ 4 ] = '.';
	Buffer [ 5 ] = 's';
	Buffer [ 6 ] = 'i';
	Buffer [ 7 ] = 'g';
	
	WriteLittleEndianU32 ( & Buffer [ 8 ], RootDirectoryOffset );
	
	uint32_t Checksum = 0;
	
	for ( uint32_t I = 0; I < 3; I ++ )
	{
		
		uint32_t Val = 0;
		
		Val += Buffer [ I * 4 ];
		Val += Buffer [ I * 4 + 1 ] << 8;
		Val += Buffer [ I * 4 + 2 ] << 16;
		Val += Buffer [ I * 4 + 3 ] << 24;
		
		Checksum += Val;
		
	}
	
	Checksum = ~ Checksum + 1;
	
	WriteLittleEndianU32 ( & Buffer [ 12 ], Checksum );
	
	File -> Write ( const_cast <const uint8_t *> ( Buffer ), 0, 16 );
	
};

uint32_t QMFS :: ReserveHeader ( uint32_t * Position )
{
	
	* Position = 16;
	
	return 0;
	
};

void QMFS :: WriteFS ( HostFile * ImageFile, const char * RootDirectoryPath, const char * RootName )
{
	
	size_t RootDirectoryPathLength = strlen ( RootDirectoryPath );
	
	uint32_t Position = 0;
	
	uint32_t RootDirectoryOffset;
	uint32_t RootDirectoryNameOffset;
	uint32_t RootDirectoryChildOffsetArrayOffset;
	
	ReserveHeader ( & Position );
	
	Vector <const char *> * RootDirectoryEnumeration = Directory :: EnumerateDirectory ( RootDirectoryPath );
	
	if ( RootDirectoryEnumeration == NULL )
		return;
	
	RootDirectoryOffset = ReserveDirectoryNode ( & Position );
	RootDirectoryNameOffset = ReserveNameString ( RootName, & Position );
	
	WriteHeader ( ImageFile, RootDirectoryOffset );
	
	uint32_t ChildOffsets [ RootDirectoryEnumeration -> Length () ];
	uint32_t ChildNameOffsets [ RootDirectoryEnumeration -> Length () ];
	bool IsFile [ RootDirectoryEnumeration -> Length () ];
	
	for ( uint32_t I = 0; I < RootDirectoryEnumeration -> Length (); I ++ )
	{
		
		char TempName [ RootDirectoryPathLength + 1 + strlen ( ( * RootDirectoryEnumeration ) [ I ] ) ];
		
		strcpy ( TempName, RootDirectoryPath );
		TempName [ RootDirectoryPathLength ] = '/';
		strcpy ( & TempName [ RootDirectoryPathLength + 1 ], ( * RootDirectoryEnumeration ) [ I ] );
		
		if ( Directory :: IsDirectory ( const_cast <const char *> ( TempName ) ) )
		{
			
			IsFile [ I ] = false;
			ChildOffsets [ I ] = ReserveDirectoryNode ( & Position );
			ChildNameOffsets [ I ] = ReserveNameString ( ( * RootDirectoryEnumeration ) [ I ], & Position );
			
		}
		else if ( Directory :: IsFile ( const_cast <const char *> ( TempName ) ) )
		{
			
			IsFile [ I ] = true;
			ChildOffsets [ I ] = ReserveFileNode ( & Position );
			ChildNameOffsets [ I ] = ReserveNameString ( ( * RootDirectoryEnumeration ) [ I ], & Position );
			
		}
		else
		{
			
			RootDirectoryEnumeration -> Remove ( I, 1 );
			I --;
			
		}
		
	}
	
	RootDirectoryChildOffsetArrayOffset = ReserveDirectoryChildOffsetArray ( RootDirectoryEnumeration -> Length (), & Position );
	
	WriteDirectoryNode ( ImageFile, RootDirectoryOffset, RootDirectoryEnumeration -> Length (), RootDirectoryChildOffsetArrayOffset, RootDirectoryNameOffset, strlen ( RootName ) );
	WriteName ( ImageFile, RootName, RootDirectoryNameOffset );
	WriteOffsetArray ( ImageFile, ChildOffsets, RootDirectoryEnumeration -> Length (), RootDirectoryChildOffsetArrayOffset );
	
	for ( uint32_t I = 0; I < RootDirectoryEnumeration -> Length (); I ++ )
	{
		
		char TempName [ RootDirectoryPathLength + 1 + strlen ( ( * RootDirectoryEnumeration ) [ I ] ) ];
		
		strcpy ( TempName, RootDirectoryPath );
		TempName [ RootDirectoryPathLength ] = '/';
		strcpy ( & TempName [ RootDirectoryPathLength + 1 ], ( * RootDirectoryEnumeration ) [ I ] );
		
		if ( IsFile [ I ] )
		{
			
			HostFile * ReadFile = HostFile :: OpenFile ( const_cast <const char *> ( TempName ), false );
			
			if ( ReadFile != NULL )
			{
				
				uint32_t FileSize = ReadFile -> GetLength ();
				uint32_t DataOffset = ReserveFileData ( FileSize, & Position );
				
				WriteFileNode ( ImageFile, ChildOffsets [ I ], DataOffset, FileSize, ChildNameOffsets [ I ], strlen ( ( * RootDirectoryEnumeration ) [ I ] ) );
				
				uint8_t ScratchBuffer [ 0x1000 ];
				uint32_t CopiedSize = 0;
				
				while ( CopiedSize < FileSize )
				{
					
					uint32_t SubCopySize = ( FileSize - CopiedSize > 0x1000 ) ? 0x1000 : FileSize - CopiedSize;
					
					ReadFile -> Read ( ScratchBuffer, CopiedSize, SubCopySize );
					ImageFile -> Write ( const_cast <const uint8_t *> ( ScratchBuffer ), CopiedSize + DataOffset, SubCopySize );
					
					CopiedSize += SubCopySize;
					
				}
				
				WriteName ( ImageFile, ( * RootDirectoryEnumeration ) [ I ], ChildNameOffsets [ I ] );
				
			}
			else
			{
				
				WriteFileNode ( ImageFile, ChildOffsets [ I ], 0, 0, ChildNameOffsets [ I ], strlen ( ( * RootDirectoryEnumeration ) [ I ] ) );
				WriteName ( ImageFile, ( * RootDirectoryEnumeration ) [ I ], ChildNameOffsets [ I ] );
				
			}
			
			ReadFile -> Close ();
			
		}
		else
			WriteSubDirectory ( ImageFile, TempName, & Position, ChildOffsets [ I ], ChildNameOffsets [ I ], ( * RootDirectoryEnumeration ) [ I ] );
		
	}
	
	Directory :: FreeEnumerationList ( RootDirectoryEnumeration );
	
};

void QMFS :: WriteSubDirectory ( HostFile * ImageFile, const char * Path, uint32_t * Position, uint32_t DirOffset, uint32_t NameOffset, const char * Name )
{
	
	Vector <const char *> * DirectoryEnumeration = Directory :: EnumerateDirectory ( Path );
	
	if ( DirectoryEnumeration == NULL )
	{
		
		WriteDirectoryNode ( ImageFile, DirOffset, 0, 0, NameOffset, strlen ( Name ) );
		
		return;
		
	}
	
	uint32_t DirectoryPathLength = strlen ( Path );
	
	uint32_t ChildOffsets [ DirectoryEnumeration -> Length () ];
	uint32_t ChildNameOffsets [ DirectoryEnumeration -> Length () ];
	bool IsFile [ DirectoryEnumeration -> Length () ];
	
	for ( uint32_t I = 0; I < DirectoryEnumeration -> Length (); I ++ )
	{
		
		char TempName [ DirectoryPathLength + 1 + strlen ( ( * DirectoryEnumeration ) [ I ] ) ];
		
		strcpy ( TempName, Path );
		TempName [ DirectoryPathLength ] = '/';
		strcpy ( & TempName [ DirectoryPathLength + 1 ], ( * DirectoryEnumeration ) [ I ] );
		
		if ( Directory :: IsDirectory ( const_cast <const char *> ( TempName ) ) )
		{
			
			IsFile [ I ] = false;
			ChildOffsets [ I ] = ReserveDirectoryNode ( Position );
			ChildNameOffsets [ I ] = ReserveNameString ( ( * DirectoryEnumeration ) [ I ], Position );
			
		}
		else if ( Directory :: IsFile ( const_cast <const char *> ( TempName ) ) )
		{
			
			IsFile [ I ] = true;
			ChildOffsets [ I ] = ReserveFileNode ( Position );
			ChildNameOffsets [ I ] = ReserveNameString ( ( * DirectoryEnumeration ) [ I ], Position );
			
		}
		else
		{
			
			DirectoryEnumeration -> Remove ( I, 1 );
			I --;
			
		}
		
	}
	
	uint32_t DirectoryChildOffsetArrayOffset = ReserveDirectoryChildOffsetArray ( DirectoryEnumeration -> Length (), Position );
	
	WriteDirectoryNode ( ImageFile, DirOffset, DirectoryEnumeration -> Length (), DirectoryChildOffsetArrayOffset, NameOffset, strlen ( Name ) );
	WriteName ( ImageFile, Name, NameOffset );
	WriteOffsetArray ( ImageFile, ChildOffsets, DirectoryEnumeration -> Length (), DirectoryChildOffsetArrayOffset );
	
	for ( uint32_t I = 0; I < DirectoryEnumeration -> Length (); I ++ )
	{
		
		char TempName [ DirectoryPathLength + 1 + strlen ( ( * DirectoryEnumeration ) [ I ] ) ];
		
		strcpy ( TempName, Path );
		TempName [ DirectoryPathLength ] = '/';
		strcpy ( & TempName [ DirectoryPathLength + 1 ], ( * DirectoryEnumeration ) [ I ] );
		
		if ( IsFile [ I ] )
		{
			
			HostFile * ReadFile = HostFile :: OpenFile ( const_cast <const char *> ( TempName ), false );
			
			if ( ReadFile != NULL )
			{
				
				uint32_t FileSize = ReadFile -> GetLength ();
				uint32_t DataOffset = ReserveFileData ( FileSize, Position );
				
				WriteFileNode ( ImageFile, ChildOffsets [ I ], DataOffset, FileSize, ChildNameOffsets [ I ], strlen ( ( * DirectoryEnumeration ) [ I ] ) );
				
				uint8_t ScratchBuffer [ 0x1000 ];
				uint32_t CopiedSize = 0;
				
				while ( CopiedSize < FileSize )
				{
					
					uint32_t SubCopySize = ( FileSize - CopiedSize > 0x1000 ) ? 0x1000 : FileSize - CopiedSize;
					
					ReadFile -> Read ( ScratchBuffer, CopiedSize, SubCopySize );
					ImageFile -> Write ( const_cast <const uint8_t *> ( ScratchBuffer ), CopiedSize + DataOffset, SubCopySize );
					
					CopiedSize += SubCopySize;
					
				}
				
				WriteName ( ImageFile, ( * DirectoryEnumeration ) [ I ], ChildNameOffsets [ I ] );
				
			}
			else
			{
				
				WriteFileNode ( ImageFile, ChildOffsets [ I ], 0, 0, ChildNameOffsets [ I ], strlen ( ( * DirectoryEnumeration ) [ I ] ) );
				WriteName ( ImageFile, ( * DirectoryEnumeration ) [ I ], ChildNameOffsets [ I ] );
				
			}
			
		}
		
	}
	
};

void QMFS :: WriteFileNode ( HostFile * ImageFile, uint32_t Offset, uint32_t DataOffset, uint32_t DataSize, uint32_t NameOffset, uint32_t NameLength )
{
	
	uint8_t Buffer [ 20 ];
	
	// Node type.
	WriteLittleEndianU32 ( & Buffer [ 0 ], kQMFSNodeType_File );
	
	// Name tag
	WriteLittleEndianU32 ( & Buffer [ 4 ], NameOffset );
	WriteLittleEndianU32 ( & Buffer [ 8 ], NameLength );
	
	// Child array tag
	WriteLittleEndianU32 ( & Buffer [ 12 ], DataOffset );
	WriteLittleEndianU32 ( & Buffer [ 16 ], DataSize );
	
	ImageFile -> Write ( const_cast <const uint8_t *> ( Buffer ), Offset, 20 );
	
};

uint32_t QMFS :: ReserveFileNode ( uint32_t * Position )
{
	
	uint32_t Reservation = * Position;
	
	* Position += 20;
	
	return Reservation;
	
}

uint32_t QMFS :: ReserveFileData ( uint32_t FileSize, uint32_t * Position )
{
	
	uint32_t Reservation = * Position;
	
	* Position += FileSize;
	
	return Reservation;
	
};

uint32_t QMFS :: ReserveDirectoryNode ( uint32_t * Position )
{
	
	uint32_t Reservation = * Position;
	
	* Position += 20;
	
	return Reservation;
	
};

uint32_t QMFS :: ReserveNameString ( const char * Name, uint32_t * Position )
{
	
	uint32_t Reservation = * Position;
	
	* Position += strlen ( Name );
	
	return Reservation;
	
};

uint32_t QMFS :: ReserveDirectoryChildOffsetArray ( uint32_t Length, uint32_t * Position )
{
	
	uint32_t Reservation = * Position;
	
	* Position += Length * 4;
	
	return Reservation;
	
};

void QMFS :: WriteName ( HostFile * File, const char * Name, uint32_t Position )
{
	
	File -> Write ( reinterpret_cast <const uint8_t *> ( Name ), Position, strlen ( Name ) );
	
};

void QMFS :: WriteOffsetArray ( HostFile * File, uint32_t * Array, uint32_t Count, uint32_t Offset )
{
	
	uint8_t Buffer [ 4 * Count ];
	
	for ( uint32_t I = 0; I < Count; I ++ )
		WriteLittleEndianU32 ( & Buffer [ 4 * I ], Array [ I ] );
	
	File -> Write ( const_cast <const uint8_t *> ( Buffer ), Offset, Count * 4 );
	
};

void QMFS :: WriteDirectoryNode ( HostFile * ImageFile, uint32_t Offset, uint32_t ChildCount, uint32_t ChildOffsetArrayOffset, uint32_t NameOffset, uint32_t NameLength )
{
	
	uint8_t Buffer [ 20 ];
	
	// Node type.
	WriteLittleEndianU32 ( & Buffer [ 0 ], kQMFSNodeType_Directory );
	
	// Name tag
	WriteLittleEndianU32 ( & Buffer [ 4 ], NameOffset );
	WriteLittleEndianU32 ( & Buffer [ 8 ], NameLength );
	
	// Child array tag
	WriteLittleEndianU32 ( & Buffer [ 12 ], ChildOffsetArrayOffset );
	WriteLittleEndianU32 ( & Buffer [ 16 ], ChildCount );
	
	ImageFile -> Write ( const_cast <const uint8_t *> ( Buffer ), Offset, 20 );
	
};
