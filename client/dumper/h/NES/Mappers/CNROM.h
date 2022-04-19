#pragma once

class CNROM : public CMapper
{
	BOOL	Dump( LPCTSTR pszFile ) override;

public:
	CNROM( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner );

};