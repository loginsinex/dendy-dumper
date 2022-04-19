
#include "stdafx.h"

CMapper205::CMapper205( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner )
	: CMapper( dwPortId, dwRate, hOwner )
{
	InitSettings( {} );
}

BOOL CMapper205::Dump( LPCTSTR pszFile )
{
	NES_HEADER header = { 0 };
	CNESFileStream stream( pszFile, fsmWrite );
	const BYTE uGamesCount = 3;
	const BYTE uGameReg[] = { 0, 2, 3 };
	const BYTE uPRGCount[] = { 16, 8 ,8 };
	const BYTE uCHRCount[] = { 32, 16, 16 };

	BYTE uPRGTotal = 0, uCHRTotal = 0;
	for ( const auto & c : uPRGCount ) uPRGTotal += c;
	for ( const auto & c : uCHRCount ) uCHRTotal += c;

	ResetCartridge();
	DoMirroringDetect();

	header.uSignature = NES_FILE_SIGNATURE;
	header.bChrCount = uCHRTotal;
	header.bPrgCount = uPRGTotal;
	header.fMapperLo = ( 205 & 0x0F );
	header.fMapperHi = ( ( 205 >> 4 ) & 0x0F );
	header.fMirroring = LOBYTE( GetMirroring() );

	stream.write( header );

	std::vector<std::vector<BYTE>> vCHR;
	std::vector<BYTE> vBank;
	vBank.reserve( 0x8000 );

	WritePRG( 0xE000, 0x00 );
	WritePRG( 0xA001, 0x80 );

	for ( BYTE i = 0; i < uGamesCount; ++i )
	{
		if ( i ) WritePRG( 0x6800 | uGameReg[ i ], uGameReg[ i ] );
		CString sDesc;

		for ( BYTE prg = 0; prg < uPRGCount[ i ] - 1; ++prg )
		{
			BOOL fIsLastBank = ( ( prg + 2 ) == uPRGCount[ i ] );
			const USHORT uSize = ( fIsLastBank ? 0x8000 : 0x4000 );
			sDesc.Format( TEXT( "Game #%d, PRG #%d" ), i, prg );

			WritePRG( 0x8000, 0x06 );
			WritePRG( 0x8001, 2 * prg );
			WritePRG( 0x8000, 0x07 );
			WritePRG( 0x8001, 2 * prg + 1 );

			ReadPRG( sDesc, 0x8000, uSize - 1, vBank );
			stream.write( vBank );
		}

		for ( BYTE chr = 0; chr < 0x4 * uCHRCount[ i ]; chr += 2 )
		{
			sDesc.Format( TEXT( "Game #%d, CHR #%d" ), i, chr );
			WritePRG( 0x8000, 0x00 );
			WritePRG( 0x8001, ( 2 * chr ) );
			WritePRG( 0x8000, 0x01 );
			WritePRG( 0x8001, ( 2 * chr + 2 ) );
			ReadPPU( sDesc, 0, 0x1000 - 1, vBank );
			vCHR.push_back( vBank );
		}
	}

	for ( const auto & chr : vCHR )
	{
		stream.write( chr );
	}

	return TRUE;
}