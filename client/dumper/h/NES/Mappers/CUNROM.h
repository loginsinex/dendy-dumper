#pragma once

class CUNROM : public CMapper
{
	BOOL	Dump( LPCTSTR pszFile ) override;

public:
	CUNROM( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner );
};