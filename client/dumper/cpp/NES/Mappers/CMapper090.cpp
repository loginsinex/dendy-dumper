
#include "stdafx.h"

CMapper090::CMapper090( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner )
	: CMapper( dwPortId, dwRate, hOwner )
{
	InitSettings( {
			MAPPER_SETTING{ TEXT( "PRG Banks" ), mstIntList, { 8, 16, 32 }, 0 },
			MAPPER_SETTING{ TEXT( "CHR Banks" ), mstIntList, { 16, 32, 64 }, 0 }
				  }
	);
}

BOOL CMapper090::Dump( LPCTSTR pszFile )
{
	NES_HEADER header = { 0 };
	CNESFileStream stream( pszFile, fsmWrite );

	// ResetCartridge();
	DoMirroringDetect();

	header.uSignature = NES_FILE_SIGNATURE;
	header.bChrCount = LOBYTE( GetSetting( 1 ) );;
	header.bPrgCount = LOBYTE( GetSetting( 0 ) );;
	header.fMapperLo = 0x00;
	header.fMirroring = GetMirroring();
	header.fMapperLo = ( 90 & 0x0F );
	header.fMapperHi = ( 90 >> 4 );

	stream.write( header );

	std::vector<BYTE> vBank;
	vBank.reserve( 0x4000 );
	CString str;

	for ( USHORT prg = 0; prg < 8 * static_cast<USHORT>( header.bPrgCount ); prg += 2 )
	{
		str.Format( TEXT( "PRG bank #%d" ), prg );
		WritePRG( 0x8000, LOBYTE( prg ) );
		WritePRG( 0x8001, LOBYTE( prg + 1 ) );
		ReadPRG( str, 0x8000, 0x4000 - 1, vBank );
		stream.write( vBank );
	}
	
	vBank.clear();
	for ( BYTE chr = 0; chr < header.bChrCount; ++chr )
	{
		str.Format( TEXT( "CHR bank #%d" ), chr );
		WritePRG( 0x9000, chr );
		ReadPRG( str, 0x0000, 0x2000 - 1, vBank );
		stream.write( vBank );
	}

	return TRUE;
}