#pragma once

#define			NES_FILE_SIGNATURE		0x1A53454E

#pragma pack(push, 1)
typedef struct _tagNES_HEADER
{
	union
	{
		BYTE		bSignature[ 4 ];
		DWORD		uSignature;
	};

	BYTE		bPrgCount;
	BYTE		bChrCount;
	union
	{
		BYTE	bFlag6;
		struct
		{
			BYTE fMirroring : 1;
			BYTE fPrgRAMPresent : 1;
			BYTE fTrainerPresent : 1;
			BYTE fSquareVRAM : 1;
			BYTE fMapperLo : 4;
		};
	};
	union
	{
		BYTE	bFlag7;
		struct
		{
			BYTE fVSUnisystem : 1;
			BYTE fPlayChoice10 : 1;	// ROM growed up in 8 kB after chr
			BYTE fHeaderFormat : 2;	// 0x02 - this is iNES2.0
			BYTE fMapperHi : 4;
		};
	};
	BYTE	bPrgRamSize;

	union
	{
		BYTE	bFlag9;
		struct
		{
			BYTE fTVSystem : 1;	// 0 - NTSC, 1 - PAL
			BYTE fReserved : 7;
		};
	};

	union
	{
		BYTE	bFlag10;
		struct
		{
			BYTE fTVSystemUO : 2;
			BYTE fReserved2 : 2;
			BYTE fPrgRamPresentUO : 1;
			BYTE fBusConflicts : 1;
			BYTE fReserved3 : 2;
		};
	};

	BYTE	bReserved[ 5 ];
} NES_HEADER, *PNES_HEADER;
#pragma pack(pop)

typedef enum _tagMAPPER_SETTING_TYPE
{
	mstIntList
} MAPPER_SETTING_TYPE, *PMAPPER_SETTING_TYPE;

typedef struct _tagMAPPER_SETTING
{
	CString 				sSettingName;
	MAPPER_SETTING_TYPE		mst;
	std::vector<USHORT>		vIntList;

	size_t					value_id;
} MAPPER_SETTING, *PMAPPER_SETTING;

typedef std::vector<MAPPER_SETTING>		MAPPER_SETTINGS, *PMAPPER_SETTINGS;

class CMapper;

typedef struct _tagDUMP_THREAD_DATA
{
	CMapper *		pThis;
	CString			sFile;
} DUMP_THREAD_DATA, *PDUMP_THREAD_DATA;

typedef struct _tagDUMP_MANUAL_THREAD_DATA
{
	CMapper *			pThis;
	BOOL				fPPU;
	BOOL				fWrite;
	USHORT				uAddress;
	USHORT				uLength;
	std::vector<BYTE>	vWriteData;
} DUMP_MANUAL_THREAD_DATA, *PDUMP_MANUAL_THREAD_DATA;

class CMapperEvent
{
	HANDLE			m_hEvent;

public:
	CMapperEvent();
	~CMapperEvent();

	VOID			Reset();
	VOID			Raise();
	VOID			Wait( DWORD uMilliseconds );
};

class CMapper : public CDumper
{
	CCriticalSection	m_cs;
	CMapperEvent		m_event;
	volatile BOOL		m_fErrorState;
	volatile BOOL		m_fDumpInProgress;
	BOOL				m_fMirroring;
	std::vector<BYTE>	m_data;
	DWORD				m_dwExpected;
	
	MAPPER_SETTINGS		m_settings;

	VOID	Data( const std::vector<BYTE> & data ) override;
	VOID	WriteDone( BOOL fSucceeded ) override;
	VOID	Error( DWORD dwError ) override;
	VOID	Bootup() override;
	VOID	ResetM2Done( BOOL fStatus ) override;
	VOID	ReqMirroringDone( BYTE bMirroring ) override;
	static DWORD WINAPI __Dump( LPVOID pParam );
	static DWORD WINAPI __DumpManual( LPVOID pParam );
	VOID			ThreadDump( LPCTSTR pszFilename );
	VOID			ThreadDumpManual( BOOL fPPU, BOOL fWrite, USHORT uAddress, USHORT uLength, const std::vector<BYTE> & vWriteData );
	VOID			ManualDump( PDUMP_MANUAL_THREAD_DATA pData );

protected:
	CMapper( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner );

	VOID	ReadPRG( const CString & sDescription, USHORT uAddress, USHORT uSize, std::vector<BYTE> & data );
	VOID	WritePRG( USHORT uAddress, BYTE bData );
	VOID	ReadPPU( const CString & sDescription, USHORT uAddress, USHORT uSize, std::vector<BYTE> & data );
	VOID	WritePPU( USHORT uAddress, BYTE bData );
	BOOL	IsErrorState() const;
	VOID	InitSettings( const MAPPER_SETTINGS & settings );
	MAPPER_SETTINGS & Settings();
	USHORT  GetSetting( const MAPPER_SETTING & setting ) const;
	USHORT	GetSetting( size_t set_id ) const;
	virtual BOOL	Dump( LPCTSTR pszFile ) PURE;
	VOID	ResetCartridge();
	VOID	DoMirroringDetect();

public:
	VOID			EnumSettings( MAPPER_SETTINGS & settings ) const;
	VOID			SetSetting( size_t set_id, MAPPER_SETTING & setting );
	VOID			SetMirroring( BOOL fVertical );
	BOOL			GetMirroring() const;
	VOID			AbortDump();
	VOID			StartDump( LPCTSTR pszFile );
	BOOL			IsInProgress() const;

	VOID			DumpPRGData( USHORT uAddress, USHORT uLength );
	VOID			DumpPPUData( USHORT uAddress, USHORT uLength );
	VOID			SendPRGByte( USHORT uAddress, const std::vector<BYTE> & vData );
	VOID			SendPPUByte( USHORT uAddress, const std::vector<BYTE> & vData );
	VOID			MapperReset();
};
