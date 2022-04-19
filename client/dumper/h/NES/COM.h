#pragma once

#include <queue>

typedef enum _tagBAUD_RATE
{
	rate_75					= 75,
	rate_110				= CBR_110,
	rate_300				= CBR_300,
	rate_600				= CBR_600,
	rate_1200				= CBR_1200,
	rate_2400				= CBR_2400,
	rate_4800				= CBR_4800,
	rate_9600				= CBR_9600,
	rate_14400				= CBR_14400,
	rate_19200				= CBR_19200,
	rate_38400				= CBR_38400,
	rate_56000				= CBR_56000,
	rate_57600				= CBR_57600,
	rate_115200				= CBR_115200,
	rate_125000				= 125000L,
	rate_128000				= CBR_128000,
	rate_250000				= 250000L,
	rate_256000				= CBR_256000,
	rate_500000 			= 500000L
} BAUD_RATE, *PBAUD_RATE;

typedef std::vector<BYTE>		DUMPER_COMMAND, *PDUMPER_COMMAND;

typedef struct _tagCOM_SETTINGS
{
	DWORD			dwPortId;
	BAUD_RATE		uBaudRate;
} COM_SETTINGS, *PCOM_SETTINGS;

class CCOMPortError
{
	const DWORD		m_dwLastError;

public:
	CCOMPortError() : m_dwLastError( GetLastError() ) {}
	CCOMPortError( DWORD dwLastError ) : m_dwLastError( dwLastError ) {}

	DWORD			What() const
	{
		return m_dwLastError;
	}

	CString			Error() const
	{
		CString strError;
		LPTSTR pszError = nullptr;
		size_t size = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, m_dwLastError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), (LPTSTR)&pszError, 0, NULL );

		if ( pszError )
		{
			strError = pszError;
			LocalFree( pszError );
		}

		return strError;
	}
};

class CCriticalSection
{
	CRITICAL_SECTION	m_cs;

public:
	CCriticalSection();
	~CCriticalSection();

	VOID				Enter();
	VOID				Leave();

};

class CCOMPort
{
	const BAUD_RATE					m_dwBaudRate;
	const DWORD						m_dwPort;
	CString							m_sPort;
	DCB								m_dcbPortState;
	HANDLE							m_hPort;
	volatile BOOL					m_fTerminating;
	std::queue<DUMPER_COMMAND>		m_vCommandList;
	CCriticalSection				m_cs;

	struct
	{
		HANDLE					hRead;
		HANDLE					hWrite;
	} m_event;

	struct
	{
		HANDLE				hRead;
		DWORD				idRead;
		HANDLE				hWrite;
		DWORD				idWrite;
	} m_thread;

	VOID					StartThreads();

	static DWORD	CALLBACK	_ReadThread( LPVOID pParam );
	static DWORD	CALLBACK	_WriteThread( LPVOID pParam );

	VOID					ReadPort();
	VOID					WritePort();

	virtual VOID			HandleRead( BYTE bReadData ) PURE;
	virtual VOID			Error( DWORD dwError ) PURE;

	operator HANDLE() const;

	VOID					SetSpeed( BAUD_RATE rate );

	BOOL					IsTerminating() const;
	VOID					SetTerminating();

protected:
	CCOMPort( DWORD dwPortId, BAUD_RATE dwBaudRate );
	~CCOMPort();

	VOID					Connect();
	VOID					Disconnect();
	BOOL					IsConnected() const;

	VOID					SendCommand( const DUMPER_COMMAND & cmd );
	VOID					SendCommand( const LPCTSTR pszTextCommand  );
};