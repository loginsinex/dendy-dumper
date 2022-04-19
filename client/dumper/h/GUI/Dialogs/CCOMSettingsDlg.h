#pragma once

class CCOMSettingsDlg : public CDialog
{
	COM_SETTINGS				m_settings;

	CFrameControl				m_frCOMSettings;
	CStaticControl				m_stComId;
	CEditControl				m_edComId;
	CUpDownControl				m_udComId;
	CStaticControl				m_stBaudRate;
	CControl					m_cbBaudRate;

	BOOL			OnInit( LPARAM lParam ) override;
	VOID			OnOK() override;

public:
	CCOMSettingsDlg( HINSTANCE hInstance );

	VOID			Let_Settings( const COM_SETTINGS & settings );
	VOID			Get_Settings( COM_SETTINGS & settings );
};