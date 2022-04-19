

#include "stdafx.h"

CMMC3ForInOne::CMMC3ForInOne( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner )
	: CMapper( dwPortId, dwRate, hOwner )
{
	InitSettings(
		{
			MAPPER_SETTING{ TEXT( "PRG Banks" ), mstIntList, { 8, 16 }, 0 },
			MAPPER_SETTING{ TEXT( "CHR Banks" ), mstIntList, { 16, 32 }, 0 }
		}
	);
}

BOOL CMMC3ForInOne::Dump( LPCTSTR pszFile )
{
	NES_HEADER header = { 0 };
	CNESFileStream stream( pszFile, fsmWrite );

	const BYTE bChrCount = LOBYTE( GetSetting( 1 ) );
	const BYTE bPrgCount = LOBYTE( GetSetting( 0 ) );

	ResetCartridge();
	DoMirroringDetect();

	header.uSignature = NES_FILE_SIGNATURE;
	header.bChrCount = ( bChrCount << 2 ); // chr banks
	header.bPrgCount = ( bPrgCount << 2 ); // prg banks
	header.fMapperLo = ( 52 & 0x0F );
	header.fMapperHi = ( ( 52 >> 4 ) & 0x0F );
	header.fMirroring = GetMirroring();

	stream.write( header );
	std::vector<BYTE> vBank;
	std::vector<std::vector<BYTE>> vCHR;
	vBank.reserve( 0x4000 );
	WritePRG( 0xA001, 0x80 );
	
	for ( BYTE buGame = 0; buGame < 4; buGame++ )
	{
		auto uGame = buGame; // -1;
		BYTE bReg = 0X80 | ( ( uGame & 1 ) * 3 ) | ( 0x10 * ( ( uGame >> 1 ) & 1 ) );
		ResetCartridge();
		WritePRG( 0xA001, 0x80 );
		WritePRG( 0x6801, bReg );
		for ( BYTE i = 0; i < 2 * ( bPrgCount - 1 ); i += 2 )
		{
			CString sDesc;
			WritePRG( 0x8000, 0x06 );
			WritePRG( 0x8001, i );
			WritePRG( 0x8002, 0x07 );
			WritePRG( 0x8003, ( i + 1 ) );
			sDesc.Format( TEXT( "Reading game #%d PRG bank #%d" ), uGame, i );
			ReadPRG( sDesc, 0x8000, 0x4000 - 1, vBank );
			stream.write( vBank );
		}
		
		CString sDescLast;
		sDescLast.Format( TEXT( "Last bank of game #%d" ), uGame );
		ReadPRG( sDescLast, 0xC000, 0x4000 - 1, vBank );
		stream.write( vBank );

		for ( BYTE i = 0; i < 0x4 * bChrCount; i += 2 )
		{
			CString sDesc;
			sDesc.Format( TEXT( "Reading game #%d CHR bank %d" ), uGame, i );
			WritePRG( 0x8000, 0x00 );
			WritePRG( 0x8001, 2 * i );
			WritePRG( 0x8000, 0x01 );
			WritePRG( 0x8001, 2 * i + 1 );
			ReadPPU( sDesc, 0, 0x1000 - 1, vBank );
			vCHR.push_back( vBank );
		}
	}

	for ( const auto & chr : vCHR )
	{
		stream.write( chr );
	}

	
	/*
	for ( USHORT i = 0; i < 0x100; i += 2 )
	{
		WritePRG( 0x8000, 0x00 );
		WritePRG( 0x8001, LOBYTE( i ) );

		CString sDesc;
		sDesc.Format( TEXT( "Reading game CHR bank %d" ), i );
		ReadPPU( sDesc, 0, 0x800 - 1, vBank );
		stream.write( vBank );
	}

	header.bChrCount <<= 2;
	header.bPrgCount <<= 2;

	for ( auto & vCHR : vCHR )
	{
		stream.write( vCHR );
	}
	*/
	
	/*
	ReadPRG( TEXT( "test1" ), 0x8000, 0x100 - 1, vBank );
	stream.write( vBank ); vBank.clear();

	WritePRG( 0xA001, 0x80 );
	WritePRG( 0x6801, 0x01 );
	ReadPRG( TEXT( "test2" ), 0x8000, 0x100 - 1, vBank );
	stream.write( vBank ); vBank.clear();

	WritePRG( 0xA001, 0x80 );
	WritePRG( 0x6801, 0x41 );
	ReadPRG( TEXT( "test3" ), 0x8000, 0x100 - 1, vBank );
	stream.write( vBank ); vBank.clear();

	WritePRG( 0xA001, 0x80 );
	WritePRG( 0x6801, 0x81 );
	ReadPRG( TEXT( "test4" ), 0x8000, 0x100 - 1, vBank );
	stream.write( vBank ); vBank.clear();

	WritePRG( 0xA001, 0x80 );
	WritePRG( 0x6801, 0xC1 );
	ReadPRG( TEXT( "test5" ), 0x8000, 0x100 - 1, vBank );
	stream.write( vBank ); vBank.clear();
	*/

	/*
	ReadPRG( TEXT( "Reading initial bank" ), 0x8000, 0x4000 - 1, vBank );
	ReadPPU( TEXT( "Reading initial CHR" ), 0x0000, 0x2000 - 1, vCHRBank );
	*/


	/*
	ReadPRG( TEXT( "Reading PRG" ), 0x8000, 0x8000 - 1, vBank );
	stream.write( vBank );

	vBank.clear();
	ReadPPU( TEXT( "Reading CHR" ), 0x0000, 0x2000 - 1, vBank );
	stream.write( vBank );
	*/

	// do dump mapper 49

	return TRUE;
}