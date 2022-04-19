#pragma once


#define DUMPDLG_IDC_COM_NAME			100
#define DUMPDLG_IDC_TARGET_FILE			200
#define DUMPDLG_IDC_TARGET_FILE_BROWSE	201
#define DUMPDLG_IDC_MAPPER				300
#define DUMPDLG_IDC_DUMMY				400
#define DUMPDLG_IDC_PROGRESS			500
#define DUMPDLG_IDC_SHELLEXEC			600

typedef struct _tagDLG_MAPPER_SETTING
{
	CStaticControl			* pstName;
	CControl				* pcbValue;
} DLG_MAPPER_SETTING, *PDLG_MAPPER_SETTING;

typedef std::map<USHORT, DLG_MAPPER_SETTING>  DLG_MAPPER_SETTINGS, *PDLG_MAPPER_SETTINGS;

class CMapperSettingsDlg : public CDialog
{
	CMapper					* m_pMapper;
	MAPPER_SETTINGS			m_settings;
	DLG_MAPPER_SETTINGS		m_dlgSettings;
	CCheckBoxControl		m_chkVerticalMirroring;

	BOOL				OnInit( LPARAM lParam ) override;
	VOID				OnButton( USHORT uId ) override;
	VOID				OnCommand( USHORT uCmd, USHORT uId, HWND hCtl ) override;
	VOID				OnSettingChange( USHORT uId );

public:
	CMapperSettingsDlg( HINSTANCE hInstance, CMapper * pMapper );
	~CMapperSettingsDlg();

	CMapper *			Mapper();
	VOID				SetEnableState( BOOL fEnableState );
	VOID				SetMirroring( BOOL fMirroring );

};

class CDumperConnectDlg : public CDialog
{
	const COM_SETTINGS						m_settings;

	CFrameControl							m_frComSettings;
	CEditControl							m_edCOMName;
	CControl								m_cbMapper;

	CFrameControl							m_frTarget;
	CEditControl							m_edTarget;
	CButtonControl							m_btnBrowseTarget;

	CFrameControl							m_frMapperSets;
	CStaticControl							m_stNoSets;

	CFrameControl							m_frDump;
	CStaticControl							m_stDump;
	CControl								m_prDump;

	CButtonControl							m_btShellExec;

	std::vector<CMapperSettingsDlg*>		m_vpdlgMapper;
	CMapperSettingsDlg *					m_vpCurSel;
	volatile BOOL	m_fCloseRequested;

	BOOL			OnInit( LPARAM lParam ) override;
	VOID			OnButton( USHORT uId ) override;
	VOID			OnCommand( USHORT uCmd, USHORT uId, HWND hCtl ) override;
	VOID			OnDestroy() override;
	VOID			OnOK() override;

	VOID			OnMapperChange( size_t id );

	VOID			SetEnableState( BOOL fEnableState );

	LRESULT			OnMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandle ) override;
	VOID			OnStartDump();
	VOID			OnDumpProgress( PDUMP_NOTIFY_PROGRESS pData );
	VOID			OnFinishDump( DWORD dwError );
	VOID			OnCancel() override;
	BOOL			OnClose() override;

public:
	CDumperConnectDlg( HINSTANCE hInstance, const COM_SETTINGS & sets );
	~CDumperConnectDlg();

};