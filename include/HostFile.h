#ifndef HOSTFILE_H
#define HOSTFILE_H

#include <stdio.h>
#include <stdint.h>

class HostFile
{
public:
	
	~HostFile ();
	
	void Read ( uint8_t * Buffer, uint32_t Offset, uint32_t Length );
	void Write ( const uint8_t * Buffer, uint32_t Offset, uint32_t Length );
	
	void Flush ();
	
	uint32_t GetLength ();
	
	void Close ();
	
	static HostFile * OpenFile ( const char * Path, bool Writable, bool OverWrite = false );
	
private:
	
	HostFile ( FILE * FilePointer, bool Writable );
	
	FILE * FilePointer;
	
	uint32_t Length;
	
	bool Writable;
	bool Open;
	
};

#endif
