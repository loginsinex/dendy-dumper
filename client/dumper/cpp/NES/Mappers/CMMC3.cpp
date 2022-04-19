
#include "stdafx.h"

CMMC3::CMMC3( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner )
	: CMapper( dwPortId, dwRate, hOwner )
{
	InitSettings(
		{
			MAPPER_SETTING{ TEXT( "PRG Banks" ), mstIntList, { 8, 16 }, 0 },
			MAPPER_SETTING{ TEXT( "CHR Banks" ), mstIntList, { 16, 32 }, 0 }
		}
	);
}

BOOL CMMC3::Dump( LPCTSTR pszFile )
{
	NES_HEADER header = { 0 };
	CNESFileStream stream( pszFile, fsmWrite );

	// ResetCartridge();
	DoMirroringDetect();

	header.uSignature = NES_FILE_SIGNATURE;
	header.bChrCount = LOBYTE( GetSetting( 1 ) ); // chr banks
	header.bPrgCount = LOBYTE( GetSetting( 0 ) ); // prg banks
	header.fMapperLo = 0x04;
	header.fMirroring = GetMirroring();

	stream.write( header );

	
	std::vector<BYTE> vBank;
	vBank.reserve( 0x4000 );

	
	for ( BYTE i = 0; i < header.bPrgCount; ++i )
	{
		CString sDesc;
		sDesc.Format( TEXT( "Reading PRG bank #%d" ), i );
		vBank.clear();
		WritePRG( 0x8000, 0x06 );
		WritePRG( 0x8001, ( 2 * i ) | ( 0 << 4 ) );
		WritePRG( 0x8000, 0x07 );
		WritePRG( 0x8001, ( 2 * i + 1 ) | ( 0 << 4 ) );
		ReadPRG( sDesc, 0x8000, 0x4000 - 1, vBank );
		stream.write( vBank );
	}

	/*
	vBank.clear();
	ReadPRG( TEXT("Reading last PRG bank"), 0xC000, 0x4000 - 1, vBank );
	stream.write( vBank );
	*/
	
	for ( BYTE i = 0; i < 0x4 * header.bChrCount; i += 2 )
	{
		CString sDesc;
		vBank.clear();
		sDesc.Format( TEXT( "Reading CHR bank %d" ), i );
		WritePRG( 0x8000, 0x00 );
		WritePRG( 0x8001, ( 2 * i ) | ( 0 << 7 ) );
		WritePRG( 0x8000, 0x01 );
		WritePRG( 0x8001, ( 2 * i + 2 ) | ( 0 << 7 ) );
		ReadPPU( sDesc, 0, 0x1000 - 1, vBank );
		stream.write( vBank );
	}
	
	

	/*
	WritePRG( 0x8000, 0x00 );
	WritePRG( 0x8001, 0x4f );
	ReadPPU( 0x0000, 0x2000 - 1, vBank );
	stream.write( vBank );
	*/

	return TRUE;
}