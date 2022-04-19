
// dumper.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "dumper.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


	INT_PTR retval = 0;
	CMainWindow wnd( hInstance );

	CString s;
	s.LoadString( hInstance, IDS_APP_TITLE, 0 );

	retval = wnd.ShowCustomAccel( IDC_PROJECT, NULL, 0, s );

    return (int) retval;
}

int _tmain( int argc, char ** argv, char ** env )
{
	return wWinMain( GetModuleHandle( nullptr ), nullptr, GetCommandLineW(), SW_SHOW );
}