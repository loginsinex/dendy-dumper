#pragma once

#pragma once

class CDummyMapper : public CMapper
{
	BOOL	m_fPPUMode;
	BOOL	Dump( LPCTSTR pszFile ) override;

public:
	CDummyMapper( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner );
	VOID	SetPPUMode( BOOL fPPUMode );
	~CDummyMapper();

};