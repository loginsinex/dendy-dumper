#include "stdafx.h"

void * __cdecl operator new( size_t size )
{
	PVOID pParam = GlobalAlloc( GMEM_ZEROINIT | GMEM_FIXED, size );

	if ( !pParam )
		FatalAppExit( 0, TEXT( "Can't allocate memory\n\nProgram will abnormal closed" ) );
	return (void *)pParam;
}

void __cdecl operator delete( void *ptr )
{
	GlobalFree( ptr );
}

void __cdecl operator delete[]( void *ptr )
{
	GlobalFree( ptr );
}
