#pragma once

class CMapper090 : public CMapper
{
	BOOL	Dump( LPCTSTR pszFile ) override;

public:
	CMapper090( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner );
};