
#include "stdafx.h"

CNROM::CNROM( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner )
	: CMapper( dwPortId, dwRate, hOwner )
{
	InitSettings( {} );
}

BOOL CNROM::Dump( LPCTSTR pszFile )
{
	NES_HEADER header = { 0 };
	CNESFileStream stream( pszFile, fsmWrite );

	// ResetCartridge();
	DoMirroringDetect();

	header.uSignature = NES_FILE_SIGNATURE;
	header.bChrCount = 1;
	header.bPrgCount = 0x02;
	header.fMapperLo = 0x00;
	header.fMirroring = GetMirroring();

	stream.write( header );

	std::vector<BYTE> vBank;
	vBank.reserve( 0x4000 );

	ReadPRG( TEXT("Reading PRG"), 0x8000, 0x8000 - 1, vBank );
	stream.write( vBank );

	vBank.clear();
	ReadPPU( TEXT("Reading CHR"), 0x0000, 0x2000 - 1, vBank );
	stream.write( vBank );

	return TRUE;
}