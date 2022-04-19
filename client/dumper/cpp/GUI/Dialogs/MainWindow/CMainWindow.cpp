
#include "stdafx.h"

CMainWindow::CMainWindow( HINSTANCE hInstance )
	: CWindow( hInstance, TEXT( "dumper_class" ) )
{
	Register( CS_DBLCLKS, NULL, LoadCursor( NULL, IDC_ARROW ), (HBRUSH)( COLOR_BTNFACE + 1 ) );

	m_setsCOMPort.dwPortId = 1;
	m_setsCOMPort.uBaudRate = rate_500000;
}

BOOL CMainWindow::OnCreate( LPCREATESTRUCT lpcs )
{
	SetMenu( *this, LoadMenu( GetInstance(), MAKEINTRESOURCE( IDC_PROJECT ) ) );
	return TRUE;
}

VOID CMainWindow::OnNCDestroy()
{

}

VOID CMainWindow::OnCommand( USHORT uCmd, USHORT uId, HWND hCtl )
{
	if ( 1 == uCmd )
	{
		OnButton( uId );
	}
}