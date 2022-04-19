#pragma once

class CMMC3ForInOne : public CMapper
{
	BOOL	Dump( LPCTSTR pszFile ) override;

public:
	CMMC3ForInOne( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner );
};