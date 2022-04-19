
#include "stdafx.h"

#define COMSETSDLG_IDC_COMID			100
#define COMSETSDLG_IDC_COMID_INT		101
#define COMSETSDLG_IDC_BAUD_RATE		200

CCOMSettingsDlg::CCOMSettingsDlg( HINSTANCE hInstance )
	: CDialog( hInstance, 150, 100, TEXT( "COM Settings" ), TRUE ),
	m_frCOMSettings( hInstance, TEXT( "COM port settings" ), 7, 7, -7, -( 7 + 15 + 7 ) ),
	m_stComId( hInstance, TEXT( "Port number:" ), 15, 22, 75, 10 ),
	m_edComId( hInstance, COMSETSDLG_IDC_COMID, TEXT( "1" ), 95, 20, -( 7 + 7 ), 13 ),
	m_udComId( hInstance, COMSETSDLG_IDC_COMID_INT ),
	m_stBaudRate( hInstance, TEXT( "Baud rate:" ), 15, 42, 75, 10 ),
	m_cbBaudRate( hInstance, COMSETSDLG_IDC_BAUD_RATE, WC_COMBOBOX, nullptr, 95, 40, -( 7 + 7 ), 100, WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | WS_TABSTOP )
{

	pushctl( m_frCOMSettings );
	pushctl( m_stComId );
	pushctl( m_edComId );
	pushctl( m_udComId );
	pushctl( m_stBaudRate );
	pushctl( m_cbBaudRate );

}

BOOL CCOMSettingsDlg::OnInit( LPARAM lParam )
{
	BAUD_RATE values[] =
	{
	rate_75, rate_110, rate_300, rate_600,
	rate_1200, rate_2400, rate_4800, rate_9600,
	rate_14400, rate_19200, rate_38400, rate_56000,
	rate_57600, rate_115200, rate_125000, rate_128000, rate_250000, rate_256000,
	rate_500000
	};

	for ( const auto & rate : values )
	{
		CString sValue;
		sValue.Format( TEXT( "%d" ), rate );
		INT_PTR id = m_cbBaudRate.cSendMessage( CB_ADDSTRING, 0, (LPARAM)sValue.GetString() );
		m_cbBaudRate.cSendMessage( CB_SETITEMDATA, id, rate );
		if ( rate == m_settings.uBaudRate )
		{
			m_cbBaudRate.cSendMessage( CB_SETCURSEL, id, id );
		}
	}

	SetFocus( m_edComId );
	m_udComId.SetRange( 16, 1 );
	m_udComId.Value( m_settings.dwPortId );

	return FALSE;
}

VOID CCOMSettingsDlg::OnOK()
{
	INT_PTR idSel = m_cbBaudRate.cSendMessage( CB_GETCURSEL );
	if ( idSel < 0 ) return;
	if ( m_udComId.IsError() ) return;

	m_settings.uBaudRate = static_cast<BAUD_RATE>( m_cbBaudRate.cSendMessage( CB_GETITEMDATA, idSel ) );
	m_settings.dwPortId = m_udComId.Value();
	Close( TRUE );
}

VOID CCOMSettingsDlg::Let_Settings( const COM_SETTINGS & settings )
{
	m_settings = settings;
}

VOID CCOMSettingsDlg::Get_Settings( COM_SETTINGS & settings )
{
	settings = m_settings;
}