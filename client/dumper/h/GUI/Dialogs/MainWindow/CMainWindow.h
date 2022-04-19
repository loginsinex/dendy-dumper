#pragma once

#define IDCU_MAINWINDOW_STATUS		0x2010
#define IDCU_MAINWINDOW_MENU		0x2020
#define IDCU_MAINWINDOW_REBAR		0x2030

class CMainWindow : public CWindow
{
	COM_SETTINGS	m_setsCOMPort;

	BOOL			OnCreate( LPCREATESTRUCT lpcs ) override;
	VOID			OnNCDestroy() override;
	VOID			OnCommand( USHORT uCmd, USHORT uId, HWND hCtl ) override;
	VOID			OnButton( USHORT uId ) override;
	VOID			OnCancel() override;

	//				Controls
	VOID			Ctl_OnAbout();
	VOID			Ctl_OnExit();

	VOID			Ctl_OnMakeNewDump();
	VOID			Ctl_OnCOMSettings();
	VOID			Ctl_OnHEXEditor();

public:
	CMainWindow( HINSTANCE hInstance );
};