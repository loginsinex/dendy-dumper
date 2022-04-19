
#include "stdafx.h"

VOID CMainWindow::OnButton( USHORT uId )
{
	switch ( uId )
	{
		case ID_FILE_MAKENEWDUMP: Ctl_OnMakeNewDump(); break;
		case ID_FILE_COMSETTINGS: Ctl_OnCOMSettings(); break;
		case ID_EDIT_HEXEDITOR: Ctl_OnHEXEditor(); break;
		case IDM_ABOUT: Ctl_OnAbout(); break;
		case IDM_EXIT: Ctl_OnExit(); break;
	}
}

VOID CMainWindow::Ctl_OnAbout()
{
	CAboutDlg dlgAbout( GetInstance() );
	dlgAbout.Show( this );
}

VOID CMainWindow::OnCancel()
{
	return Ctl_OnExit();
}

VOID CMainWindow::Ctl_OnMakeNewDump()
{
	CDumperConnectDlg dlg( GetInstance(), m_setsCOMPort );
	dlg.Show( this );
}

VOID CMainWindow::Ctl_OnCOMSettings()
{
	CCOMSettingsDlg dlg( GetInstance() );

	dlg.Let_Settings( m_setsCOMPort );
	if ( dlg.Show( this ) )
	{
		dlg.Get_Settings( m_setsCOMPort );
	}
}

VOID CMainWindow::Ctl_OnHEXEditor()
{
	CHEXEditorDlg dlg( GetInstance(), m_setsCOMPort );
	dlg.Show( this );
}

VOID CMainWindow::Ctl_OnExit()
{
	if ( Confirm( TEXT( "Exit now?" ) ) )
	{
		Close( 0 );
	}
}
