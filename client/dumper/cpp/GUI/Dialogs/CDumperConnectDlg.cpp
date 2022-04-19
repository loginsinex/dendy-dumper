
#include "stdafx.h"
#include <shellapi.h>

#define DLG_SETTING_IDC_VERTICAL_MIRRORING		50

CMapperSettingsDlg::CMapperSettingsDlg( HINSTANCE hInstance, CMapper * pMapper )
	: CDialog( hInstance, WS_VISIBLE | WS_CHILD | DS_CONTROL | DS_FIXEDSYS | DS_SHELLFONT, 0, 0, 0, 400, 400, TEXT( "" ), 0 ),
	m_pMapper( pMapper ),
	m_chkVerticalMirroring( hInstance, DLG_SETTING_IDC_VERTICAL_MIRRORING, TEXT( "Vertical mirroring" ), 7, 7, 100, 10 )
{
	m_pMapper->EnumSettings( m_settings );
	pushctl( m_chkVerticalMirroring );
	
	int Y = 20;
	USHORT id = 100;

	for ( const auto & setting : m_settings )
	{
		DLG_MAPPER_SETTING dlgSetting;
		dlgSetting.pstName = new CStaticControl( hInstance, setting.sSettingName, 7, Y + 2, 75, 10 );
		dlgSetting.pcbValue = new CControl( hInstance, id, WC_COMBOBOX, nullptr, 90, Y, 75, 100, WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | WS_TABSTOP );
		pushctl( *dlgSetting.pstName );
		pushctl( *dlgSetting.pcbValue );
		m_dlgSettings[ id ] = dlgSetting;
		id++;
		Y += 20;
	}
}

BOOL CMapperSettingsDlg::OnInit( LPARAM lParam )
{
	for ( auto & dlgSetting : m_dlgSettings )
	{
		auto set_id = dlgSetting.first - 100;
		size_t k = 0;
		for ( const auto & setting : m_settings[ set_id ].vIntList )
		{
			CString sValue;
			sValue.Format( TEXT( "%d" ), setting );
			INT_PTR id = dlgSetting.second.pcbValue->cSendMessage( CB_ADDSTRING, 0, (LPARAM)sValue.GetString() );
			if ( k == m_settings[ set_id ].value_id )
			{
				dlgSetting.second.pcbValue->cSendMessage( CB_SETCURSEL, id, id );
			}
			k++;
		}
	}

	m_chkVerticalMirroring.Check( m_pMapper->GetMirroring() );

	return FALSE;
}

VOID CMapperSettingsDlg::OnButton( USHORT uId )
{
	switch ( uId )
	{
		case DLG_SETTING_IDC_VERTICAL_MIRRORING:
			{
				m_pMapper->SetMirroring( m_chkVerticalMirroring.Check() );
				break;
			}
	}
}

CMapperSettingsDlg::~CMapperSettingsDlg()
{
	for ( auto & dlgSetting : m_dlgSettings )
	{
		delete dlgSetting.second.pstName;
		delete dlgSetting.second.pcbValue;
	}

	m_dlgSettings.clear();
}

VOID CMapperSettingsDlg::OnCommand( USHORT uCmd, USHORT uId, HWND hCtl )
{
	switch ( uCmd )
	{
		case CBN_SELENDOK:
			{
				OnSettingChange( uId );
				break;
			}
	}
}

VOID CMapperSettingsDlg::OnSettingChange( USHORT uId )
{
	auto it = m_dlgSettings.find( uId );
	if ( it == m_dlgSettings.end() ) return;

	auto & dlgSetting = it->second;
	size_t set_id = uId - 100;
	if ( set_id >= m_settings.size() ) return;
	auto & setting = m_settings[ set_id ];
	setting.value_id = dlgSetting.pcbValue->cSendMessage( CB_GETCURSEL );
	m_pMapper->SetSetting( set_id, setting );
}

CMapper * CMapperSettingsDlg::Mapper()
{
	return m_pMapper;
}

VOID CMapperSettingsDlg::SetEnableState( BOOL fEnableState )
{
	m_chkVerticalMirroring.Enable( fEnableState );
	for ( auto & dlgSetting : m_dlgSettings )
	{
		dlgSetting.second.pcbValue->Enable( fEnableState );
	}
}

VOID CMapperSettingsDlg::SetMirroring( BOOL fMirroring )
{
	m_chkVerticalMirroring.Check( fMirroring );
	m_pMapper->SetMirroring( fMirroring );
}

// main dialog imp

CDumperConnectDlg::CDumperConnectDlg( HINSTANCE hInstance, const COM_SETTINGS & settings )
	: CDialog( hInstance, 200, 325, TEXT( "New dump" ), TRUE ), m_settings( settings ), m_vpCurSel( nullptr ), m_fCloseRequested( FALSE ),
	m_frComSettings( hInstance, TEXT( "Dumper source" ), 7, 7, -7, 60 ),
	m_edCOMName( hInstance, DUMPDLG_IDC_COM_NAME, nullptr, 15, 20, -( 7 + 7 ), 13, FALSE, FALSE, TRUE ),
	m_cbMapper( hInstance, DUMPDLG_IDC_MAPPER, WC_COMBOBOX, nullptr, 15, 40, -( 7 + 7 ), 100, WS_VISIBLE | WS_TABSTOP | CBS_AUTOHSCROLL | CBS_DROPDOWNLIST ),

	m_frTarget( hInstance, TEXT( "Target file" ), 7, 80, -7, 40 ),
	m_edTarget( hInstance, DUMPDLG_IDC_TARGET_FILE, TEXT( "dump.nes" ), 15, 96, -( 7 + 7 + 50 + 7 ), 13 ),
	m_btnBrowseTarget( hInstance, DUMPDLG_IDC_TARGET_FILE_BROWSE, TEXT( "Browse..." ), -( 7 + 7 + 50 ), 95, 50, 15 ),

	m_frMapperSets( hInstance, TEXT( "Mapper settings" ), 7, 130, -7, -( 7 + 15 + 7 + 50 + 7 ) ),
	m_stNoSets( hInstance, TEXT( "There are no settings for this mapper" ), 15, 140, -( 7 + 7 ), -( 7 + 15 + 7 + 7 + 50 + 7 ), FALSE, WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE | SS_SUNKEN, 0, DUMPDLG_IDC_DUMMY ),

	m_frDump( hInstance, TEXT( "Dump progress" ), 7, -( 7 + 15 + 7 + 50 ), -7, 50 ),
	m_stDump( hInstance, nullptr, 15, -( 7 + 15 + 7 + 50 - 10 - 2 ), -( 7 + 7 ), 10, FALSE, WS_VISIBLE | SS_LEFT, 0, 0x1000 ),
	m_prDump( hInstance, DUMPDLG_IDC_PROGRESS, PROGRESS_CLASS, nullptr, 15, -( 7 + 15 + 7 + 50 - 10 - 15 ), -( 7 + 7 ), 10, WS_VISIBLE ),
	m_btShellExec( hInstance, DUMPDLG_IDC_SHELLEXEC, TEXT( "Run ROM" ), 7, -( 7 + 15 ), 50, 15 )
{

	pushctl( m_frComSettings );
	pushctl( m_edCOMName );

	pushctl( m_frTarget );
	pushctl( m_cbMapper );
	pushctl( m_edTarget );
	pushctl( m_btnBrowseTarget );
	pushctl( m_frMapperSets );
	pushctl( m_stNoSets );
	pushctl( m_frDump );
	pushctl( m_stDump );
	pushctl( m_prDump );
	pushctl( m_btShellExec );
}

BOOL CDumperConnectDlg::OnInit( LPARAM lParam )
{
	dSendMessage( IDOK, WM_SETTEXT, 0, (LPARAM)TEXT( "Dump" ) );
	
	const auto & mappers = GMapperList();
	POINT pt = { 0 };
	RECT rc = { 0 };
	GetWindowRect( m_stNoSets, &rc );
	ScreenToClient( *this, &pt );
	OffsetRect( &rc, pt.x, pt.y );

	m_stNoSets.Visible( FALSE );

	for ( const auto & mapper : mappers )
	{
		INT_PTR id = m_cbMapper.cSendMessage( CB_ADDSTRING, 0, (LPARAM)mapper.sMapperName.GetString() );
		CMapperSettingsDlg * pdlg = new CMapperSettingsDlg( GetInstance(), mapper.Mapper( m_settings.dwPortId, m_settings.uBaudRate, *this ) );
		m_vpdlgMapper.push_back( pdlg );
		SetWindowPos( pdlg->Create( this ) , nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0 );
		pdlg->Visible( FALSE );
	}

	m_cbMapper.cSendMessage( CB_SETCURSEL );
	OnMapperChange( 0 );

	CString sSourcePort;
	sSourcePort.Format( TEXT( "Port: COM%d / Baud rate: %d" ), m_settings.dwPortId, m_settings.uBaudRate );
	m_edCOMName.Text( sSourcePort.GetString() );

	SetFocus( m_edTarget );
	m_edTarget.cSendMessage( EM_SETSEL, 0, -1 );
	return FALSE;
}

VOID CDumperConnectDlg::OnButton( USHORT uId )
{
	switch ( uId )
	{
		case DUMPDLG_IDC_TARGET_FILE_BROWSE:
			{
				CString sFile = SaveFile( m_edTarget.Text().c_str(), TEXT( "NES files (*.nes)" ), TEXT( "*.nes" ), 0, 0 ).c_str();
				if ( sFile.GetLength() > 0 )
				{
					m_edTarget.Text( sFile.GetString() );
				}
				break;
			}
		case DUMPDLG_IDC_SHELLEXEC:
			{
				CString strFile = m_edTarget.Text().c_str();
				if ( strFile.GetLength() > 0 )
				{
					ShellExecute( *this, TEXT( "open" ), strFile, nullptr, nullptr, SW_SHOW );
				}
				break;
			}
	}
}

VOID CDumperConnectDlg::OnCommand( USHORT uCmd, USHORT uId, HWND hCtl )
{
	switch ( uCmd )
	{
		case CBN_SELENDOK:
			{
				if ( uId == DUMPDLG_IDC_MAPPER )
				{
					OnMapperChange( m_cbMapper.cSendMessage( CB_GETCURSEL ) );
				}

				break;
			}
	}
}

VOID CDumperConnectDlg::OnMapperChange( size_t id )
{
	auto pOldSel = m_vpCurSel;

	if ( id >= m_vpdlgMapper.size() ) return;

	m_vpCurSel = m_vpdlgMapper[ id ];
	m_vpCurSel->Visible( TRUE );

	if ( pOldSel )
	{
		pOldSel->Visible( FALSE );
	}
}

VOID CDumperConnectDlg::SetEnableState( BOOL fEnableState )
{
	m_edCOMName.Enable( fEnableState );
	m_cbMapper.Enable( fEnableState );

	m_edTarget.Enable( fEnableState );
	m_btnBrowseTarget.Enable( fEnableState );

	EnableAll( fEnableState, IDOK, 0 );

	if ( m_vpCurSel ) m_vpCurSel->SetEnableState( fEnableState );
}

LRESULT CDumperConnectDlg::OnMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandle )
{
	fHandle = TRUE;
	switch ( uMsg )
	{
		case WM_DUMP_START: OnStartDump(); break;
		case WM_DUMP_PROGRESS: OnDumpProgress( (PDUMP_NOTIFY_PROGRESS)lParam ); break;
		case WM_DUMP_FINISHED: OnFinishDump( LODWORD( lParam ) ); break;
		default: fHandle = FALSE; break;
	}

	return 0;
}

VOID CDumperConnectDlg::OnStartDump()
{
	SetEnableState( FALSE );
	m_prDump.cSendMessage( PBM_SETPOS, 0, 0 );
	m_stDump.Text( TEXT( "Waiting for a dumper..." ) );
}

VOID CDumperConnectDlg::OnDumpProgress( PDUMP_NOTIFY_PROGRESS pData )
{
	switch ( pData->dps )
	{
		case dpsStart:
			{
				m_stDump.Text( CString( pData->sDescription + TEXT( "..." ) ).GetString() );
				m_prDump.cSendMessage( PBM_SETBARCOLOR, 0, CLR_DEFAULT );
				m_prDump.cSendMessage( PBM_SETPOS, 0, 0 );
				break;
			}
		case dpsProgress:
			{
				if ( pData->dwMaximum )
				{
					m_prDump.cSendMessage( PBM_SETPOS, 100 * pData->dwCurrent / pData->dwMaximum );
				}
				break;
			}
		case dpsFinish:
			{
				m_stDump.Text( m_stDump.Text() + TEXT( "... OK!" ) );
				m_prDump.cSendMessage( PBM_SETBARCOLOR, 0, RGB( 0, 0xFF, 0 ) );
				break;
			}
		case dpsError:
			{
				m_stDump.Text( m_stDump.Text() + TEXT( "... ERROR!" ) );
				m_prDump.cSendMessage( PBM_SETBARCOLOR, 0, RGB( 0xFF, 0, 0 ) );
				break;
			}
		case dpsResetM2:
			{
				m_stDump.Text( TEXT( "Cartridge resetted!" ) );
				break;
			}
		case dpsMirroringDetect:
			{
				if ( !pData->dwMaximum )
				{
					switch ( pData->dwCurrent )
					{
						case 'H':
							{
								m_stDump.Text( TEXT( "Detected horizontal mirroring" ) );
								if ( m_vpCurSel )
								{
									m_vpCurSel->SetMirroring( FALSE );
								}
								break;
							}
						case 'V':
							{
								m_stDump.Text( TEXT( "Detected vertical mirroring" ) );
								if ( m_vpCurSel )
								{
									m_vpCurSel->SetMirroring( TRUE );
								}
								break;
							}
						case 0:
							{
								m_stDump.Text( TEXT( "Failed mirroring detection!" ) );
								break;
							}
					}
				}
				else
				{

				}
			}
	}
}

VOID CDumperConnectDlg::OnFinishDump( DWORD dwError )
{
	if ( m_fCloseRequested )
	{
		// ignore all errors. Just close self
		Close( 0 );
		return;
	}

	if ( dwError )
	{
		LPTSTR pszError = nullptr;
		size_t size = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
									  NULL, dwError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR)&pszError, 0, NULL );

		if ( pszError )
		{
			ShowErrorF( TEXT( "Dump error!\n\n%s" ), pszError );
			LocalFree( pszError );
		}
		else
		{
			ShowErrorF( TEXT( "Dump error!\n\nError: %d" ), dwError );
		}
	}
	else
	{
		// ShowMessage( TEXT( "Dump completed!" ) );
		MessageBeep( MB_ICONINFORMATION );
	}

	m_stDump.Text( TEXT( "" ) );
	m_prDump.cSendMessage( PBM_SETPOS );
	SetEnableState( TRUE );
	SetFocus( Ctl( IDOK ) );
}

VOID CDumperConnectDlg::OnOK()
{
	if ( m_vpCurSel )
	{
		m_vpCurSel->Mapper()->StartDump( m_edTarget.Text().c_str() );
	}
}

VOID CDumperConnectDlg::OnDestroy()
{
	for ( auto & dlg : m_vpdlgMapper )
	{
		delete dlg->Mapper();
		DestroyWindow( *dlg );
		delete dlg;
	}

	m_vpdlgMapper.clear();
	m_vpCurSel = nullptr;
}

BOOL CDumperConnectDlg::OnClose()
{
	OnCancel();
	return FALSE;
}

VOID CDumperConnectDlg::OnCancel()
{
	if ( m_vpCurSel->Mapper()->IsInProgress() )
	{
		if ( !Confirm( TEXT( "Dumping is in progress. Do you want to abort it?" ) ) )
		{
			return;
		}

		m_fCloseRequested = TRUE;
		m_vpCurSel->Mapper()->AbortDump();
	}
	else
	{
		Close( 0 );
	}
}

CDumperConnectDlg::~CDumperConnectDlg()
{
	OnDestroy();
}