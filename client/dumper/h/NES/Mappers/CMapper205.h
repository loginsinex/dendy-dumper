#pragma once

class CMapper205 : public CMapper
{
	BOOL	Dump( LPCTSTR pszFile ) override;

public:
	CMapper205( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner );
};