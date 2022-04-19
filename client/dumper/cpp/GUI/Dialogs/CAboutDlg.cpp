
#include "stdafx.h"

CAboutDlg::CAboutDlg( HINSTANCE hInstance )
	: CDialog( hInstance, MAKEINTRESOURCE( IDD_ABOUTBOX ) )
{

}

VOID CAboutDlg::OnOK()
{
	Close( 0 );
}