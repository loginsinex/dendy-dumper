
#include "stdafx.h"

CMapper200::CMapper200( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner )
	: CMapper( dwPortId, dwRate, hOwner )
{
	InitSettings( 
		{
			MAPPER_SETTING{ TEXT( "Games count" ), mstIntList, { 8, 16 }, 0 },
		}
	);
}

BOOL CMapper200::Dump( LPCTSTR pszFile )
{
	NES_HEADER header = { 0 };
	CNESFileStream stream( pszFile, fsmWrite );

	ResetCartridge();
	DoMirroringDetect();

	header.uSignature = NES_FILE_SIGNATURE;
	header.bChrCount = LOBYTE( GetSetting( 0 ) );
	header.bPrgCount = LOBYTE( GetSetting( 0 ) );
	header.fMapperLo = ( 200 & 0x0F );
	header.fMapperHi = ( ( 200 >> 4 ) & 0x0F );
	header.fMirroring = LOBYTE( GetMirroring() );

	const BYTE uGamesCount = LOBYTE( GetSetting( 0 ) );

	stream.write( header );

	std::vector<std::vector<BYTE>> vCHR;
	std::vector<BYTE> vBank;
	vBank.reserve( 0x8000 );

	WritePRG( 0xF008, 0x8D );
	WritePRG( 0xF000, 0 );

	for ( BYTE i = 0; i < uGamesCount; ++i )
	{
		CString str;
		vBank.clear();
		WritePRG( 0xF080 | i, 0 );
		str.Format( TEXT( "PRG Game #%d" ), i + 1 );
		ReadPRG( str, 0x8000, 0x4000 - 1, vBank );
		stream.write( vBank );
		vBank.clear();
		str.Format( TEXT( "CHR Game #%d" ), i + 1 );
		ReadPPU( str, 0x0000, 0x2000 - 1, vBank );
		vCHR.push_back( vBank );
		
	}

	for ( const auto & chr : vCHR )
	{
		stream.write( chr );
	}

	/*
	for ( BYTE i = 0; i < header.bPrgCount / 2; ++i )
	{
		WritePRG( 0xA000, 0x80 | ( i << 1 ) );
		ReadPRG( TEXT( "Reading PRG" ), 0x8000, 0x8000 - 1, vBank );
		stream.write( vBank );

		vBank.clear();
		ReadPPU( TEXT( "Reading CHR" ), 0x0000, 0x2000 - 1, vBank );
		vCHR.push_back( vBank );
	}

	for ( const auto & chr : vCHR )
	{
		stream.write( chr );
	}
	*/
	
	return TRUE;
}