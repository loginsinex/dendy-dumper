#pragma once

class CMMC3: public CMapper
{
	BOOL	Dump( LPCTSTR pszFile ) override;

public:
	CMMC3( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner );
};