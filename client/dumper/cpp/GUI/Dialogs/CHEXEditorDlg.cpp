
#include "stdafx.h"

#define HEX_EDITOR_LINE_LEN							0x10

#define HEXEDITOR_DLG_IDC_WRITE_PRG					0x100
#define HEXEDITOR_DLG_IDC_WRITE_PPU					0x101
#define HEXEDITOR_DLG_IDC_WRITE_ADDRESS				0x102
#define HEXEDITOR_DLG_IDC_WRITE_ADDRESS_VALUE		0x103
#define HEXEDITOR_DLG_IDC_WRITE_DATA				0x104
#define HEXEDITOR_DLG_IDC_SEND_DATA					0x105
#define HEXEDITOR_DLG_IDC_READ_PRG					0x106
#define HEXEDITOR_DLG_IDC_READ_PPU					0x107
#define HEXEDITOR_DLG_IDC_DUMPED_DATA				0x108
#define HEXEDITOR_DLG_IDC_SCROLL					0x109
#define HEXEDITOR_DLG_IDC_READ_ADDR					0x10A
#define HEXEDITOR_DLG_IDC_CMD_DUMP					0x10B
#define HEXEDITOR_DLG_IDC_DUMP_PROGRESS				0x10C
#define HEXEDITOR_DLG_IDC_RESET						0x10D

CHEXEditorDlg::CHEXEditorDlg( HINSTANCE hInstance, const COM_SETTINGS & com )
	: CDialog( hInstance, 780, 300, TEXT( "HEX Editor" ) ), m_pMapper( nullptr ), m_com( com ),
	m_fnt( -16, 0, 0, 0, FW_EXTRABOLD, 0, 0, 0, 0, 0, 0, 0, 0, TEXT( "Courier New" ) ),
	m_frWriteArea( hInstance, TEXT( "Data to send" ), 7, 7, 140, -( 7 + 15 + 7 ) ),
	m_rdWritePRG( hInstance, HEXEDITOR_DLG_IDC_WRITE_PRG, TEXT( "Write to PRG" ), 20, 20, 100, 13, FALSE ),
	m_rdWritePPU( hInstance, HEXEDITOR_DLG_IDC_WRITE_PPU, TEXT( "Write to PPU" ), 20, 40, 100, 13, FALSE ),
	m_stAddress( hInstance, TEXT( "Address:" ), 20, 62, 50, 10 ),
	m_edAddress( hInstance, HEXEDITOR_DLG_IDC_WRITE_ADDRESS, TEXT( "8000" ), 30, 75, 50, 13 ),
	m_stData( hInstance, TEXT( "Data to send:" ), 20, 102, 50, 10 ),
	m_edData( hInstance, HEXEDITOR_DLG_IDC_WRITE_DATA, nullptr, 30, 115, 100, 13 ),
	m_btnSend( hInstance, HEXEDITOR_DLG_IDC_SEND_DATA, TEXT( "Send..." ), 30, 130, 50, 15, TRUE ),
	m_btnReset( hInstance, HEXEDITOR_DLG_IDC_RESET, TEXT( "Reset" ), 30, 145, 50, 15, FALSE ),

	m_frReadArea( hInstance, TEXT( "Cartridge data" ), 160, 7, -7, -( 7 + 15 + 7 ) ),
	m_rdReadPRG( hInstance, HEXEDITOR_DLG_IDC_READ_PRG, TEXT( "Read from PRG" ), 180, 20, 100, 13, FALSE ),
	m_rdReadPPU( hInstance, HEXEDITOR_DLG_IDC_READ_PPU, TEXT( "Read from PPU" ), 180, 40, 100, 13, FALSE ),
	m_edDumpedData( hInstance, HEXEDITOR_DLG_IDC_DUMPED_DATA, nullptr, 180, 60, -( 7 + GetSystemMetrics( SM_CXVSCROLL ) + 7 ), -( 7 + 15 + 7 + 7 ), FALSE, TRUE ),
	m_tkScroll( hInstance, HEXEDITOR_DLG_IDC_SCROLL, WC_SCROLLBAR, nullptr, -( 7 + GetSystemMetrics( SM_CXVSCROLL ) + 7 ), 60, GetSystemMetrics( SM_CXVSCROLL ), -( 7 + 15 + 7 + 7 ), WS_VISIBLE | WS_TABSTOP | SBS_VERT ),
	m_stPos( hInstance, nullptr, -( 7 + 15 + 100 ), 22, -( 7 + 15 ), 15, FALSE, WS_VISIBLE | SS_CENTER | SS_SUNKEN, 0, HEXEDITOR_DLG_IDC_READ_ADDR ),
	m_btnDump( hInstance, HEXEDITOR_DLG_IDC_CMD_DUMP, TEXT( "Dump..." ), 360, 40, 50, 15 ),
	m_prgDump( hInstance, HEXEDITOR_DLG_IDC_DUMP_PROGRESS, PROGRESS_CLASS, nullptr, 420, 40, -( 7 + 15 ), 10, WS_VISIBLE ),

	m_btnClose( hInstance, IDCANCEL, MB_GetString( IDCLOSE - 1 ).c_str(), -( 7 + 50 + 7 ), -( 7 + 15 ), 50, 15 )
{
	pushctl( m_frWriteArea );
	pushctl( m_rdWritePRG );
	pushctl( m_rdWritePPU );
	pushctl( m_stAddress );
	pushctl( m_edAddress );
	pushctl( m_stData );
	pushctl( m_edData );
	pushctl( m_btnSend );
	pushctl( m_btnReset );

	pushctl( m_frReadArea );
	pushctl( m_rdReadPRG );
	pushctl( m_rdReadPPU );
	pushctl( m_edDumpedData );
	pushctl( m_tkScroll );
	pushctl( m_stPos );
	pushctl( m_btnDump );
	pushctl( m_prgDump );

	pushctl( m_btnClose );
}

BOOL CHEXEditorDlg::OnInit( LPARAM lParam )
{
	try
	{
		m_pMapper = new CDummyMapper( m_com.dwPortId, m_com.uBaudRate, *this );
	}
	catch ( CCOMPortError & com_error )
	{
		ShowErrorF( TEXT( "Connection to COM port failed.\n\n%s" ), com_error.Error().GetString() );
		Close( 0 );
		return FALSE;
	}
	catch ( std::exception & other_error )
	{
		CString err = other_error.what();
		ShowErrorF( TEXT( "Connection to COM port failed.\n\n%s" ), err.GetString() );
		Close( 0 );
		return FALSE;
	}

	
	m_data.data.insert( m_data.data.end(), 0x100, 0 );
	m_data.uAddr = 0x8000;
	m_data.range.uMin = 0x5000;
	m_data.range.uMax = 0xFFFF;
	m_data.fReadMode = FALSE;
	m_data.fWriteMode = FALSE;
	m_data.fDumpMode = FALSE;
	m_data.fResetMode = FALSE;
	m_data.fBusyMode = FALSE;

	m_rdWritePRG.Check( TRUE );
	m_rdReadPRG.Check( TRUE );
	m_tkScroll.cSendMessage( SBM_SETRANGE, m_data.range.uMin, m_data.range.uMax );
	m_tkScroll.cSendMessage( SBM_SETPOS, m_data.uAddr, TRUE );
	OnHEXScroll( m_data.uAddr, SB_THUMBTRACK );
	m_edDumpedData.cSendMessage( WM_SETFONT, (WPARAM)(HFONT)m_fnt, TRUE );

	SetFocus( m_edAddress );

	DoDump( m_data.uAddr );

	return FALSE;
}

VOID CHEXEditorDlg::OnDestroy()
{
	if ( m_pMapper )
	{
		m_pMapper->AbortDump();
		delete m_pMapper;
		m_pMapper = nullptr;
	}
}

VOID CHEXEditorDlg::OnButton( USHORT uId )
{
	switch ( uId )
	{
	case HEXEDITOR_DLG_IDC_WRITE_PRG:
	case HEXEDITOR_DLG_IDC_WRITE_PPU:
		{
			m_data.fWriteMode = ( HEXEDITOR_DLG_IDC_WRITE_PPU == uId );
			CheckRadioButton( *this, HEXEDITOR_DLG_IDC_WRITE_PRG, HEXEDITOR_DLG_IDC_WRITE_PPU, uId );
			break;
		}
	case HEXEDITOR_DLG_IDC_READ_PRG:
	case HEXEDITOR_DLG_IDC_READ_PPU:
		{
			m_data.fReadMode = ( HEXEDITOR_DLG_IDC_READ_PPU == uId );
			m_pMapper->SetPPUMode( m_data.fReadMode );

			if ( m_data.fReadMode )
			{
				m_data.range.uMin = 0x0000;
				m_data.range.uMax = 0x1FFF;
				m_data.uAddr = m_data.range.uMin;
			}
			else
			{
				m_data.range.uMin = 0x5000;
				m_data.range.uMax = 0xFFFF;
				m_data.uAddr = 0x8000;
			}
			
			m_tkScroll.cSendMessage( SBM_SETRANGE, m_data.range.uMin, m_data.range.uMax );
			m_tkScroll.cSendMessage( SBM_SETPOS, m_data.uAddr, TRUE );
			CheckRadioButton( *this, HEXEDITOR_DLG_IDC_READ_PRG, HEXEDITOR_DLG_IDC_READ_PPU, uId );
			DoDump( m_data.uAddr );
			break;
		}
	case HEXEDITOR_DLG_IDC_SEND_DATA:
		{
			if ( ! lstrlen( m_edData.Text().c_str() ) )
			{
				MessageBeep( MB_ICONERROR );
				SetFocus( m_edData );
				break;
			}

			USHORT uAddr = LOWORD( wcstol( m_edAddress.Text().c_str(), nullptr, 0x10 ) );
			CString strData = m_edData.Text().c_str();
			std::vector<BYTE> vData;
			vData.reserve( 0x100 );

			TCHAR pszByte[ 3 ] = { 0 };
			if ( strData.GetLength() % 2 )
			{
				strData += _T( '0' );
			}

			m_data.uWritingCounter = 0;

			for ( USHORT i = 0; i < strData.GetLength(); i += 2 )
			{
				pszByte[ 0 ] = strData[ i ];
				pszByte[ 1 ] = strData[ i + 1 ];

				vData.push_back( LOBYTE( wcstol( pszByte, nullptr, 0x10 ) ) );
				m_data.uWritingCounter++;
			}

			if ( !m_data.fWriteMode )
			{
				m_pMapper->SendPRGByte( uAddr++, vData );
			}
			else
			{
				m_pMapper->SendPPUByte( uAddr++, vData );
			}

			break;
		}
	case HEXEDITOR_DLG_IDC_CMD_DUMP:
		{
			if ( !m_pMapper->IsInProgress() )
			{
				CString sFile = SaveFile( TEXT( "dump.bin" ), TEXT( "BIN files (*.bin)" ), TEXT( "*.bin" ), 0, 0 ).c_str();
				if ( sFile.GetLength() > 0 )
				{
					m_data.fDumpMode = TRUE;
					m_pMapper->StartDump( sFile );
				}
			}
			break;
		}
	case HEXEDITOR_DLG_IDC_RESET:
		{
			m_data.fResetMode = TRUE;
			m_btnDump.Enable( FALSE );
			m_btnSend.Enable( FALSE );
			m_btnReset.Enable( FALSE );

			m_pMapper->MapperReset();
			break;
		}
	}
}

VOID CHEXEditorDlg::OnScroll( UINT vScrollType, USHORT uPos, USHORT uRequest, HWND hCtl )
{
	if ( GetWindowLongPtr( hCtl, GWL_ID ) == HEXEDITOR_DLG_IDC_SCROLL && WM_VSCROLL == vScrollType )
	{
		OnHEXScroll( uPos, uRequest );
	}
}

VOID CHEXEditorDlg::OnHEXScroll( USHORT uPos, USHORT uRequest )
{
	if ( m_data.fBusyMode )
	{
		return;
	}

	USHORT uNewPos = m_data.uAddr;
	switch ( uRequest )
	{
	case SB_THUMBTRACK:
		{
			CString sPos;
			sPos.Format( TEXT( "Position: 0x%04X" ), uPos );
			m_stPos.Text( sPos.GetString() );
			break;
		}
	case SB_THUMBPOSITION:
		{
			uNewPos = ( uPos & 0xFFF0 );
			break;
		}
	case SB_BOTTOM:
		{
			uNewPos = ( m_data.range.uMax & 0xFF00 );
			break;
		}
	case SB_TOP:
		{
			uNewPos = m_data.range.uMin;
			break;
		}
	case SB_LINEUP:
		{
			if ( uNewPos >= m_data.range.uMin + 0x10 )
			{
				uNewPos -= 0x10;
			}
			break;
		}
	case SB_LINEDOWN:
		{
			if ( uNewPos <= m_data.range.uMax - 0x10 )
			{
				uNewPos += 0x10;
			}
			break;
		}
	case SB_PAGEDOWN:
		{
			if ( uNewPos <= m_data.range.uMax - 0x100 )
			{
				uNewPos += 0x100;
			}
			else
			{
				uNewPos = m_data.range.uMax - 0x10;
			}
			break;
		}
	case SB_PAGEUP:
		{
			if ( uNewPos >= m_data.range.uMin + 0x100 )
			{
				uNewPos -= 0x100;
			}
			else
			{
				uNewPos = m_data.range.uMin;
			}
			break;
		}
	}

	if ( uNewPos != m_data.uAddr )
	{
		CString sPos;
		sPos.Format( TEXT( "Position: 0x%04X" ), uNewPos );
		m_stPos.Text( sPos.GetString() );

		m_pMapper->AbortDump();
		DoDump( uNewPos );
		m_tkScroll.cSendMessage( SBM_SETPOS, uNewPos, TRUE );
	}
}

BOOL CHEXEditorDlg::OnClose()
{
	OnCancel();
	return FALSE;
}

VOID CHEXEditorDlg::OnCancel()
{
	if ( m_pMapper->IsInProgress() )
	{
		if ( !Confirm( TEXT( "Do you want to interrupt current operation?" ) ) )
		{
			return;
		}
		m_pMapper->AbortDump();
		Sleep( 300 );
	}
	
	Close( 0 );
}

VOID CHEXEditorDlg::UpdateHex()
{
	DWORD dw1, dw2;
	m_edDumpedData.cSendMessage( EM_GETSEL, (WPARAM) &dw1, (LPARAM) &dw2 );

	size_t k = 0;
	BYTE b = 0;
	USHORT uAddr = m_data.uAddr;
	CString sResult, sLeftPart, sRightPart, sLine, sByte;
	for ( const auto & chr : m_data.data )
	{
		if ( !k )
		{
			sLine.Format( TEXT( "%04X | " ), uAddr );
			sLeftPart = CString();
			sRightPart = CString();
		}

		sByte.Format( TEXT( "%02X " ), chr );
		sLeftPart += sByte;
		b = ( ( chr >= ' ' &&  chr <= '~' ) ? chr : '.' );
		sRightPart += b;
		
		k++;

		if ( k >= HEX_EDITOR_LINE_LEN )
		{
			k = 0;
			sLine += sLeftPart + TEXT( "| " ) + sRightPart + TEXT( " |" );
			sResult += sLine + TEXT( "\r\n" );
			uAddr += HEX_EDITOR_LINE_LEN;
		}
	}

	m_edDumpedData.Text( sResult.GetString() );
	m_edDumpedData.cSendMessage( EM_SETSEL, dw1, dw2 );
	// MessageBeep( 0 );
}

LRESULT CHEXEditorDlg::OnMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandle )
{
	fHandle = TRUE;
	switch ( uMsg )
	{
	case WM_DUMP_START: OnStartDump(); break;
	case WM_DUMP_PROGRESS: OnDumpProgress( (PDUMP_NOTIFY_PROGRESS)lParam ); break;
	case WM_DUMP_DATA: OnDumpData( wParam, (PBYTE)lParam );
	case WM_DUMP_FINISHED: OnFinishDump( LODWORD( lParam ) ); break;
	default: fHandle = FALSE; break;
	}

	return 0;
}

VOID CHEXEditorDlg::OnStartDump()
{
	// m_btnClose.Enable( FALSE );
	m_btnSend.Enable( FALSE );
	// m_tkScroll.Enable( FALSE );
	m_btnDump.Enable( FALSE );
	m_btnReset.Enable( FALSE );

	if ( !m_data.fDumpMode )
	{
		m_data.data.clear();
	}
}

VOID CHEXEditorDlg::OnDumpProgress( PDUMP_NOTIFY_PROGRESS pData )
{
	// nothing to do?
	if ( m_data.fDumpMode )
	{
		DWORD dwProgress = ( pData->dwMaximum ? ( 100 * pData->dwCurrent / pData->dwMaximum ) : 0 );
		m_prgDump.cSendMessage( PBM_SETPOS, dwProgress, dwProgress );
	}
	else if ( m_data.fResetMode )
	{
		m_btnSend.Enable( TRUE );
		m_btnDump.Enable( TRUE );
		m_btnReset.Enable( TRUE );
		m_data.fResetMode = FALSE;
		DoDump( m_data.uAddr );
	}
}

VOID CHEXEditorDlg::OnDumpData( size_t len, PBYTE pData )
{
	size_t begin = m_data.data.size();
	m_data.data.insert( m_data.data.begin(), len, 0 );
	CopyMemory( m_data.data.data() + begin, pData, len );
	// CopyMemory( m_data.data.data() - len, pData, LODWORD( len ) );
}

VOID CHEXEditorDlg::OnFinishDump( DWORD dwError )
{
	m_tkScroll.Enable( TRUE );
	m_btnSend.Enable( TRUE );
	m_btnDump.Enable( TRUE );
	m_btnReset.Enable( TRUE );
	m_data.fBusyMode = FALSE;

	if ( m_data.fDumpMode )
	{
		if ( dwError )
		{
			ShowErrorF( TEXT( "Dump error!\nError: %d" ), dwError );
		}

		m_data.fDumpMode = FALSE;
		m_prgDump.cSendMessage( PBM_SETPOS );
	}
	else if ( m_data.uWritingCounter > 0 )
	{
		m_data.uWritingCounter = 0;
		DoDump( m_data.uAddr );
	}
	else
	{
		if ( !dwError )
		{
			UpdateHex();
		}
	}
}

VOID CHEXEditorDlg::DoDump( USHORT uAddress )
{
	m_data.uAddr = uAddress;
	m_data.fBusyMode = TRUE;
	if ( !m_data.fReadMode )
	{
		m_pMapper->DumpPRGData( uAddress, 0x100 - 1 );
	}
	else
	{
		m_pMapper->DumpPPUData( uAddress, 0x100 - 1 );
	}
}