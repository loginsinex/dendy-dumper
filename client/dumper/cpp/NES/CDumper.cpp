
#include "stdafx.h"

CDumper::CDumper( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner )
	: CCOMPort( dwPortId, dwRate ), m_hOwner( hOwner ), m_fQueueEmpty( TRUE )
{

}

BOOL CDumper::SendNextCommand()
{
	if ( m_qResult.empty() )
	{
		m_fQueueEmpty = TRUE;
		return FALSE;
	}
	else
	{
		m_fQueueEmpty = FALSE;
		const auto & q = m_qResult.front();
		// _tprintf( TEXT( "(->%c)\n" ), q.query );
		switch ( q.query )
		{
		case dqPRGMode:
		case dqPPUMode:
		case dqReqMirroring:
		case dqResetM2:
			{
				SendCommand( { q.query } );
				break;
			}
		case dqReadMode:
			{
				CString s;
				s.Format( TEXT( "%c%03X" ), q.query, q.uAddress / 0x10 );
				SendCommand( s );
				break;
			}
		case dqWriteMode:
			{
				CString s;
				s.Format( TEXT( "%c%04X%02X" ), q.query, q.uAddress, q.bData );
				SendCommand( s );
				break;
			}
		case dqSetCounter:
			{
				CString s;
				s.Format( TEXT( "%c%04X" ), q.query, q.uAddress );
				SendCommand( s );
				break;
			}
		}

		return TRUE;
	}
}

VOID CDumper::HandleRead( BYTE bReadChr )
{
	// _tprintf( TEXT( "%c [%02X]\n" ), bReadChr, bReadChr );

	m_cs.Enter();
	if ( !m_qResult.empty() )
	{
		if ( daBootUp == bReadChr )	// dumper booted up. Is there are some queue? Flush it
		{
			_tprintf( TEXT( " !!!! WARNING !!!! Controller REBOOTED !!!!\r\n" ) );
			while ( !m_qResult.empty() )
			{
				m_qResult.pop();
			}

			m_fQueueEmpty = TRUE;
			Bootup();
		}
		else
		{
			DUMPER_RESULT & result = m_qResult.front();
			BOOL fModeDone = FALSE;
			switch ( result.query )
			{
			case dqPRGMode: fModeDone = HandlePRGMode( result, bReadChr ); break;
			case dqPPUMode: fModeDone = HandlePPUMode( result, bReadChr ); break;
			case dqReadMode: fModeDone = HandleReadMode( result, bReadChr ); break;
			case dqWriteMode: fModeDone = HandleWriteMode( result, bReadChr ); break;
			case dqSetCounter: fModeDone = HandleSetCounter( result, bReadChr ); break;
			case dqResetM2: fModeDone = HandleResetM2( result, bReadChr ); break;
			case dqReqMirroring: fModeDone = HandleMirroringReq( result, bReadChr ); break;
			}

			if ( fModeDone )
			{
				m_qResult.pop();
				SendNextCommand();
			}
		}
	}
	m_cs.Leave();
}

BOOL CDumper::HandlePRGMode( DUMPER_RESULT & result, BYTE bReadChr )
{
	switch ( bReadChr )
	{
	case daSuccess: _tprintf( TEXT( "Switch PRG Mode SUCCESS\n" ) ); break;
	case daFailed: _tprintf( TEXT( "Switch PRG Mode FAILED\n" ) ); break;
	}

	return TRUE;
}

BOOL CDumper::HandlePPUMode( DUMPER_RESULT & result, BYTE bReadChr )
{
	switch ( bReadChr )
	{
	case daSuccess: _tprintf( TEXT( "Switch PPU Mode SUCCESS\n" ) ); break;
	case daFailed: _tprintf( TEXT( "Switch PPU Mode FAILED\n" ) ); break;
	}

	return TRUE;
}

BOOL CDumper::HandleReadMode( DUMPER_RESULT & result, BYTE bReadChr )
{
	switch ( bReadChr )
	{
	case daSuccess:
		{
			// read done!
			std::vector<BYTE> bin;
			// _tprintf( TEXT( "Got data from 0x%04X: %s\n" ), result.uAddress, CString( result.result.data() ).GetString() );
			DoConvert( result.result, bin );
			Data( bin );
			return TRUE;
		}
	case daFailed:
		{
			// cannot read
			_tprintf( TEXT( "Unexpectedly error while reading ROM from 0x%04X\n" ), result.uAddress );
			return TRUE;
		}
	default:
		{
			if ( bReadChr >= '0' && bReadChr <= '9' || bReadChr >= 'A' && bReadChr <= 'F' || bReadChr >= 'a' && bReadChr <= 'f' )
			{
				result.result.push_back( bReadChr );
			}
			else
			{
				_tprintf( TEXT( "Unexpected char 0x%02X while reading ROM from 0x%04X\n" ), bReadChr, result.uAddress );
				return TRUE;
			}
			break;
		}
	}

	return FALSE;
}

BOOL CDumper::HandleWriteMode( DUMPER_RESULT & result, BYTE bReadChr )
{
	switch ( bReadChr )
	{
	case daSuccess:
		{
			_tprintf( TEXT( "Succeeded write byte 0x%02X to 0x%04X\n" ), result.bData, result.uAddress );
			WriteDone( TRUE );
			break;
		}
	case daFailed:
		{
			_tprintf( TEXT( "Failed write byte 0x%02X to 0x%04X\n" ), result.bData, result.uAddress );
			WriteDone( FALSE );
			break;
		}
	}
	
	return TRUE;
}

BOOL CDumper::HandleSetCounter( DUMPER_RESULT & result, BYTE bReadChr )
{
	switch ( bReadChr )
	{
		case daSuccess:
			{
				_tprintf( TEXT( "Succeeded counter set to 0x%04X\n" ), result.uAddress );
				break;
			}
		case daFailed:
			{
				_tprintf( TEXT( "Failed counter set to 0x%04X\n" ), result.uAddress );
				break;
			}
	}

	return TRUE;
}

BOOL CDumper::HandleResetM2( DUMPER_RESULT & result, BYTE bReadChr )
{
	switch ( bReadChr )
	{
		case daSuccess:
			{
				_tprintf( TEXT( "Succeeded M2 reset\n" ) );
				SendNotifyProgress( CString(), dpsResetM2, TRUE, 0 );
				ResetM2Done( TRUE );
				break;
			}
		case daFailed:
			{
				_tprintf( TEXT( "Failed M2 reset\n" ) );
				SendNotifyProgress( CString(), dpsResetM2, FALSE, 0 );
				ResetM2Done( FALSE );
				break;
			}
	}

	return TRUE;
}

BOOL CDumper::HandleMirroringReq( DUMPER_RESULT & result, BYTE bReadChr )
{
	switch ( bReadChr )
	{
		case 'Z':
		case 'F':
		case 'H':
		case 'V':
			{
				result.bData = bReadChr;
				_tprintf( TEXT( "Mirroring detect result: %c\n" ), bReadChr );
				SendNotifyProgress( CString(), dpsMirroringDetect, bReadChr, 0 );
				return FALSE;	// wait for success/fail
			}
		case daSuccess:
			{
				SendNotifyProgress( CString(), dpsMirroringDetect, 0, 1 );
				ReqMirroringDone( result.bData );
				_tprintf( TEXT( "Detecting mirroring succeeded\n" ) );
				break;
			}
		case daFailed:
			{
				SendNotifyProgress( CString(), dpsMirroringDetect, 0, 0 );
				ReqMirroringDone( 0 );
				_tprintf(TEXT( "Detecting mirroring failed\n" ) );
				break;

			}
	}

	return TRUE;
}

VOID CDumper::ResetM2()
{
	SendM2Reset();
}

VOID CDumper::DetectMirroring()
{
	SendMirroringReq();
}

VOID CDumper::ReadPRGData( USHORT uAddress )
{
	SelectDumperMode( dmPRG );
	RequestDumperRead( uAddress );
}

VOID CDumper::WritePRG( USHORT uAddress, BYTE uByte )
{
	SelectDumperMode( dmPRG );
	RequestDumperWrite( uAddress, uByte );
}

VOID CDumper::ReadPPUData( USHORT uAddress )
{
	SelectDumperMode( dmPPU );
	RequestDumperRead( uAddress );
}

VOID CDumper::WritePPU( USHORT uAddress, BYTE uByte )
{
	SelectDumperMode( dmPPU );
	RequestDumperWrite( uAddress, uByte );
}

VOID CDumper::SetCounter( USHORT uCounter )
{
	m_cs.Enter();
	uCounter = ( uCounter ? uCounter : 1 );
	m_qResult.emplace( DUMPER_RESULT{ static_cast<DUMPER_QUERY>( dqSetCounter ), uCounter, 0, std::vector<BYTE>() } );

	if ( m_fQueueEmpty ) SendNextCommand();

	m_cs.Leave();
}

VOID CDumper::SendM2Reset()
{
	m_cs.Enter();
	m_qResult.emplace( DUMPER_RESULT{ dqResetM2, 0, 0, std::vector<BYTE>() } );

	if ( m_fQueueEmpty ) SendNextCommand();

	m_cs.Leave();
}

VOID CDumper::SendMirroringReq()
{
	m_cs.Enter();
	m_qResult.emplace( DUMPER_RESULT{ dqReqMirroring, 0, 0, std::vector<BYTE>() } );

	if ( m_fQueueEmpty ) SendNextCommand();

	m_cs.Leave();
}

VOID CDumper::SelectDumperMode( DUMPER_MODE mode )
{
	m_cs.Enter();
	m_qResult.emplace( DUMPER_RESULT{ static_cast<DUMPER_QUERY>( mode ), 0, 0, std::vector<BYTE>() }  );

	if ( m_fQueueEmpty ) SendNextCommand();

	m_cs.Leave();
}

VOID CDumper::RequestDumperRead( USHORT uAddress )
{
	m_cs.Enter();
	m_qResult.emplace( DUMPER_RESULT{ dqReadMode, uAddress, 0, std::vector<BYTE>() } );

	if ( m_fQueueEmpty ) SendNextCommand();

	m_cs.Leave();
}

VOID CDumper::RequestDumperWrite( USHORT uAddress, BYTE bData )
{
	m_cs.Enter();
	m_qResult.emplace( DUMPER_RESULT{ dqWriteMode, uAddress, bData, std::vector<BYTE>() } );

	if ( m_fQueueEmpty ) SendNextCommand();

	m_cs.Leave();
}

VOID CDumper::ReadPRGRange( USHORT uFrom, USHORT uEnd )
{
	if ( uEnd > uFrom )
	{
		SelectDumperMode( dmPRG );
		USHORT uStep = 0x10;
		USHORT uSize = uEnd - uFrom + 1;
		if ( uSize >= 0x1000 )
		{
			uStep = 0x1000;
			SetCounter( 0x100 );
		}
		else if ( uSize >= 0x800 )
		{
			uStep = 0x800;
			SetCounter( 0x80 );
		}
		else if ( uSize >= 0x400 )
		{
			uStep = 0x400;
			SetCounter( 0x40 );
		}
		else if ( uSize >= 0x100 )
		{
			uStep = 0x100;
			SetCounter( 0x10 );
		}
		else
		{
			SetCounter( 0x1 );
		}
		for ( DWORD u = uFrom; u <= uEnd; u += uStep )
		{
			RequestDumperRead( LOWORD( u ) );
		}
	}
}

VOID CDumper::ReadPPURange( USHORT uFrom, USHORT uEnd )
{
	if ( uEnd > uFrom )
	{
		SelectDumperMode( dmPPU );
		USHORT uStep = 0x10;
		USHORT uSize = uEnd - uFrom + 1;
		if ( uSize >= 0x800 )
		{
			uStep = 0x800;
			SetCounter( 0x80 );
		}
		else if ( uSize >= 0x400 )
		{
			uStep = 0x400;
			SetCounter( 0x40 );
		}
		else if ( uSize >= 0x100 )
		{
			uStep = 0x100;
			SetCounter( 0x10 );
		}
		else
		{
			SetCounter( 0x1 );
		}

		for ( DWORD u = uFrom; u <= uEnd; u += uStep )
		{
			RequestDumperRead( LOWORD( u ) );
		}
	}
}

VOID CDumper::DoConvert( const std::vector<BYTE>& result, std::vector<BYTE>& bin ) const
{
	bin.reserve( 0x10 );
	if ( !( result.size() % 2 ) )
	{
		BYTE nybble = 0, value = 0;
		for ( const auto & chr : result )
		{
			if ( chr >= '0' && chr <= '9' )
			{
				value += chr - '0';
			}
			else if ( chr >= 'A' && chr <= 'F' )
			{
				value += chr - 'A' + 10;
			}
			else if ( chr >= 'a' && chr <= 'f' )
			{
				value += chr - 'a' + 10;
			}

			nybble++;
			if ( nybble >= 2 )
			{
				bin.push_back( value );
				nybble = value = 0;
			}
			else
			{
				value *= 0x10;
			}
		}
	}
}

VOID CDumper::SendNotifyProgress( const CString & sDescription, DUMP_PROGRESS_STATE dps, DWORD dwCurrent, DWORD dwMaximum )
{
	DUMP_NOTIFY_PROGRESS dnp = { dps, sDescription, dwCurrent, dwMaximum };
	SendMessage( m_hOwner, WM_DUMP_PROGRESS, 0, (LPARAM)&dnp );
}

VOID CDumper::SendNotifyProgress( const std::vector<BYTE>& data )
{
	SendMessage( m_hOwner, WM_DUMP_DATA, data.size(), (LPARAM)data.data() );
}

VOID CDumper::SendNotifyState( DWORD uState, DWORD dwError )
{
	SendMessage( m_hOwner, uState, ( dwError != 0 ), dwError );
}