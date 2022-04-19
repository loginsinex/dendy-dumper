
#include "stdafx.h"

CDummyMapper::CDummyMapper( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner )
	: CMapper( dwPortId, dwRate, hOwner ), m_fPPUMode( FALSE )
{
	Connect();
}

BOOL CDummyMapper::Dump( LPCTSTR pszFile )
{
	CNESFileStream stream( pszFile, fsmWrite );

	std::vector<BYTE> vData;
	vData.reserve( 0x4000 );
	if ( m_fPPUMode )
	{
		ReadPPU( TEXT( "Reading PPU" ), 0x0000, 0x2000 - 1, vData );
	}
	else
	{
		ReadPRG( TEXT( "Reading PRG" ), 0x8000, 0x8000 - 1, vData );
	}

	stream.write( vData );

	return TRUE;
}

VOID CDummyMapper::SetPPUMode( BOOL fPPUMode )
{
	m_fPPUMode = fPPUMode;
}

CDummyMapper::~CDummyMapper()
{
	Disconnect();
}