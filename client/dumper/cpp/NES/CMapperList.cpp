
#include "stdafx.h"

template <class MAPPER> CMapper * __alloc_mapper( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner )
{
	return new MAPPER( dwPortId, dwRate, hOwner );
};

MAPPER_LIST		g_MapperList =
{
	MAPPER_ID{ TEXT( "NROM" ), __alloc_mapper<CNROM> },
	MAPPER_ID{ TEXT( "UNROM" ), __alloc_mapper<CUNROM> },
	MAPPER_ID{ TEXT( "MMC3" ), __alloc_mapper<CMMC3> },
//	MAPPER_ID{ TEXT( "MMC3 / 4 in 1 Super HIK" ), __alloc_mapper<CMMC3ForInOne> },
	MAPPER_ID{ TEXT( "J.Y. Company ASIC #90" ), __alloc_mapper<CMapper090> },
	MAPPER_ID{ TEXT( "Mapper #200" ), __alloc_mapper<CMapper200> },
	MAPPER_ID{ TEXT( "Mapper #205" ), __alloc_mapper<CMapper205> }
};

MAPPER_LIST		GMapperList()
{
	return g_MapperList;
}