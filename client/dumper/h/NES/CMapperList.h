#pragma once

typedef std::function<CMapper*( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner )> MAKE_MAPPER, *PMAKE_MAPPER;

typedef struct _tagMAPPER_ID
{
	const CString		sMapperName;
	MAKE_MAPPER			Mapper;
} MAPPER_ID, *PMAPPER_ID;

typedef std::vector<MAPPER_ID>		MAPPER_LIST, *PMAPPE_LIST;

MAPPER_LIST		GMapperList();