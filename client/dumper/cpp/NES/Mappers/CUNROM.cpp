
#include "stdafx.h"

CUNROM::CUNROM( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner )
	: CMapper( dwPortId, dwRate, hOwner )
{
	InitSettings(
		{
			MAPPER_SETTING{ TEXT( "PRG Banks" ), mstIntList, { 8, 16 }, 0 }
		}
	);
}

BOOL CUNROM::Dump( LPCTSTR pszFile )
{
	NES_HEADER header = { 0 };
	CNESFileStream stream( pszFile, fsmWrite );

	DoMirroringDetect();

	header.uSignature = NES_FILE_SIGNATURE;
	header.bChrCount = 0; // always zero for unrom
	header.bPrgCount = LOBYTE( GetSetting( 0 ) );	// prg banks
	header.fMapperLo = 0x02;
	header.fMirroring = GetMirroring();
	
	stream.write( header );

	std::vector<BYTE> vLastBank, vBank;
	vBank.reserve( 0x4000 );
	vLastBank.reserve( 0x4000 );

	ReadPRG( TEXT("Reading last PRG bank"), 0xC000, 0x4000, vLastBank );
	_tprintf( TEXT( "Last bank size: 0x%04X\n" ), vLastBank.size() );
	// now try to switch banks
	for ( BYTE i = 0; i < 7; ++i )
	{
		auto it = std::find( vLastBank.begin(), vLastBank.end(), i );
		if ( it != vLastBank.end() )
		{
			CString sDesc;
			sDesc.Format( TEXT( "Reading PRG bank #%d" ), i );
			USHORT bank_byte_pos = 0xC000 + LOWORD( it - vLastBank.begin() );
			WritePRG( bank_byte_pos, i );
			ReadPRG( sDesc, 0x8000, 0x4000 - 1, vBank );
			_tprintf( TEXT( "Bank %d size: 0x%04X\n" ), i, vBank.size() );
			stream.write( vBank );
			vBank.clear();
		}
	}

	stream.write( vLastBank );

	return TRUE;
}