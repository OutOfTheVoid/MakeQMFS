#ifndef QMFS_H
#define QMFS_H

#include <HostFile.h>

#include <stdint.h>

class QMFS
{
public:
	
	static void WriteFS ( HostFile * ImageFile, const char * RootDirectoryPath, const char * RootName );
	
private:
	
	static const uint32_t kQMFSNodeType_File = 0;
	static const uint32_t kQMFSNodeType_Directory = 1;
	
	static void WriteSubDirectory ( HostFile * ImageFile, const char * Path, uint32_t * Position, uint32_t DirOffset, uint32_t NameOffset, const char * Name );
	static void WriteFile ( const char * Path, uint32_t * Position, uint32_t FilePosition );
	
	static void WriteHeader ( HostFile * File, uint32_t RootDirectoryOffset );
	
	static void WriteName ( HostFile * File, const char * Name, uint32_t Position );
	
	static void WriteOffsetArray ( HostFile * File, uint32_t * Array, uint32_t Count, uint32_t Offset );
	
	static void WriteFileNode ( HostFile * ImageFile, uint32_t Offset, uint32_t DataOffset, uint32_t DataSize, uint32_t NameOffset, uint32_t NameLength );
	static void WriteDirectoryNode ( HostFile * ImageFile, uint32_t Offset, uint32_t ChildCount, uint32_t ChildOffsetArrayOffset, uint32_t NameOffset, uint32_t NameLength );
	
	static uint32_t ReserveHeader ( uint32_t * Position );
	
	static uint32_t ReserveNameString ( const char * Name, uint32_t * Position );
	static uint32_t ReserveDirectoryNode ( uint32_t * Position );
	static uint32_t ReserveDirectoryChildOffsetArray ( uint32_t ChildCount, uint32_t * Position );
	
	static uint32_t ReserveFileNode ( uint32_t * Position );
	static uint32_t ReserveFileData ( uint32_t FileSize, uint32_t * Position );
	
	static inline void WriteLittleEndianU32 ( uint8_t * Buff, uint32_t Value )
	{
		
		Buff [ 0 ] = Value & 0xFF;
		Buff [ 1 ] = ( Value >> 8 ) & 0xFF;
		Buff [ 2 ] = ( Value >> 16 ) & 0xFF;
		Buff [ 3 ] = ( Value >> 24 ) & 0xFF;
		
	};
	
};

#endif
