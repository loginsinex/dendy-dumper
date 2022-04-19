#pragma once

class CMapper200 : public CMapper
{
	BOOL	Dump( LPCTSTR pszFile ) override;

public:
	CMapper200( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner );
};