#pragma once

class CHEXEditorDlg : public CDialog
{
	const COM_SETTINGS		& m_com;
	CFont					m_fnt;
	CDummyMapper			* m_pMapper;
	CFrameControl			m_frWriteArea;
	CRadioControl			m_rdWritePRG;
	CRadioControl			m_rdWritePPU;

	CStaticControl			m_stAddress;
	CEditControl			m_edAddress;
	CStaticControl			m_stData;
	CEditControl			m_edData;
	CButtonControl			m_btnSend;
	CButtonControl			m_btnReset;

	CFrameControl			m_frReadArea;
	CRadioControl			m_rdReadPRG;
	CRadioControl			m_rdReadPPU;

	CMultilineEditControl	m_edDumpedData;
	CControl				m_tkScroll;
	CStaticControl			m_stPos;
	CButtonControl			m_btnDump;
	CControl				m_prgDump;

	CButtonControl			m_btnClose;

	struct
	{
		std::vector<BYTE>	data;
		USHORT				uAddr;
		struct
		{
			USHORT			uMin;
			USHORT			uMax;
		} range;

		int					uWritingCounter;

		BOOL				fReadMode;
		BOOL				fWriteMode;
		BOOL				fDumpMode;
		BOOL				fBusyMode;
		BOOL				fResetMode;
	} m_data;

	BOOL		OnInit( LPARAM lParam ) override;
	VOID		OnDestroy() override;
	VOID		OnCancel() override;
	BOOL		OnClose() override;
	VOID		OnButton( USHORT uId ) override;
	VOID		OnScroll( UINT vScrollType, USHORT uPos, USHORT uRequest, HWND hCtl ) override;
	VOID		OnHEXScroll( USHORT uPos, USHORT uRequest );

	LRESULT		OnMessage( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & fHandle ) override;
	VOID		OnStartDump();
	VOID		OnDumpProgress( PDUMP_NOTIFY_PROGRESS pData );
	VOID		OnFinishDump( DWORD dwError );
	VOID		OnDumpData( size_t len, PBYTE pData );

	VOID		DoDump( USHORT uAddress );

	VOID		UpdateHex();

public:
	CHEXEditorDlg( HINSTANCE hInstance, const COM_SETTINGS & com );

};
