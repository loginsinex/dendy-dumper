
#include "stdafx.h"

CCriticalSection::CCriticalSection()
{
	InitializeCriticalSection( &m_cs );
}

VOID CCriticalSection::Leave()
{
	LeaveCriticalSection( &m_cs );
}

VOID CCriticalSection::Enter()
{
	EnterCriticalSection( &m_cs );
}

CCriticalSection::~CCriticalSection()
{
	DeleteCriticalSection( &m_cs );
}

CCOMPort::CCOMPort( DWORD dwPortId, BAUD_RATE dwBaudRate )
	: m_dwPort( dwPortId ), m_dwBaudRate( dwBaudRate ), m_fTerminating( FALSE ), m_hPort( INVALID_HANDLE_VALUE )
{
	m_sPort.Format( TEXT( "COM%d" ), dwPortId );
	m_thread.hRead = m_thread.hWrite = m_event.hRead = m_event.hWrite = INVALID_HANDLE_VALUE;
	m_thread.idRead = m_thread.idWrite = 0;
}

CCOMPort::~CCOMPort()
{
	Disconnect();

	_tprintf( TEXT( "\nTerminated!\n" ) );
}

VOID CCOMPort::Connect()
{
	if ( IsConnected() ) return;

	m_hPort = CreateFile( m_sPort, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr );
	if ( INVALID_HANDLE_VALUE == m_hPort )
	{
		throw CCOMPortError();
	}

	m_event.hRead = CreateEvent( nullptr, TRUE, FALSE, nullptr );
	if ( INVALID_HANDLE_VALUE == m_event.hRead )
	{
		CloseHandle( m_hPort );
		throw CCOMPortError();
	}

	m_event.hWrite = CreateEvent( nullptr, TRUE, FALSE, nullptr );
	if ( INVALID_HANDLE_VALUE == m_event.hWrite )
	{
		CloseHandle( m_hPort );
		CloseHandle( m_event.hRead );
		throw CCOMPortError();
	}

	SecureZeroMemory( &m_dcbPortState, sizeof( m_dcbPortState ) );
	m_dcbPortState.DCBlength = sizeof( DCB );
	if ( !GetCommState( m_hPort, &m_dcbPortState ) )
	{
		CloseHandle( m_hPort );
		CloseHandle( m_event.hRead );
		CloseHandle( m_event.hWrite );
		throw CCOMPortError();
	}

	m_fTerminating = FALSE;
	SetSpeed( m_dwBaudRate );
	StartThreads();
}

VOID CCOMPort::Disconnect()
{
	SetTerminating();
	if ( IsConnected() )
	{
		SetEvent( m_event.hRead );
		SetEvent( m_event.hWrite );

		WaitForSingleObject( m_thread.hRead, INFINITE );
		WaitForSingleObject( m_thread.hWrite, INFINITE );

		CloseHandle( m_thread.hWrite );
		CloseHandle( m_thread.hRead );
		CloseHandle( m_event.hWrite );
		CloseHandle( m_event.hRead );
		CloseHandle( m_hPort );

		m_hPort = INVALID_HANDLE_VALUE;
	}
}

CCOMPort::operator HANDLE() const
{
	return m_hPort;
}

BOOL CCOMPort::IsConnected() const
{
	return ( m_hPort != INVALID_HANDLE_VALUE );
}

BOOL CCOMPort::IsTerminating() const
{
	return m_fTerminating;
}

VOID CCOMPort::SetTerminating()
{
	m_fTerminating = TRUE;
}

VOID CCOMPort::SetSpeed( BAUD_RATE rate )
{
	m_dcbPortState.BaudRate = rate;
	m_dcbPortState.ByteSize = 8;
	m_dcbPortState.Parity = NOPARITY;
	m_dcbPortState.StopBits = ONESTOPBIT;
	SetCommState( *this, &m_dcbPortState );
}

VOID CCOMPort::StartThreads()
{
	m_thread.hRead = CreateThread( nullptr, 0, CCOMPort::_ReadThread, this, 0, &m_thread.idRead );
	m_thread.hWrite = CreateThread( nullptr, 0, CCOMPort::_WriteThread, this, 0, &m_thread.idWrite );
}

DWORD CCOMPort::_ReadThread( LPVOID pParam )
{
	( (CCOMPort*)pParam )->ReadPort();
	return 0;
}

DWORD CCOMPort::_WriteThread( LPVOID pParam )
{
	( (CCOMPort*)pParam )->WritePort();
	return 0;
}

VOID CCOMPort::ReadPort()
{
	ResetEvent( m_event.hRead );
	OVERLAPPED ovRead = { 0 };
	ovRead.hEvent = m_event.hRead;
	BYTE bReadChr = 0;
	BOOL fError = FALSE;
	DWORD dwRead = 0;

	while( !fError )
	{
		SetEvent( m_event.hRead );

		BOOL fSuccess = ReadFile( *this, &bReadChr, 1, &dwRead, &ovRead );
		if ( fSuccess )
		{
			HandleRead( bReadChr );
		}
		else
		{
			switch ( GetLastError() )
			{
			case ERROR_IO_PENDING:
				{
					BOOL fHandled = FALSE;
					do
					{
						DWORD dwResult = WaitForSingleObject( m_event.hRead, 2 );
						switch ( dwResult )
						{
						case WAIT_TIMEOUT:
							{
								if ( IsTerminating() )	// check if we're terminating
								{
									return;
								}
								break;
							}
						case WAIT_OBJECT_0:
							{
								// check if we're got some result
								if ( !GetOverlappedResult( *this, &ovRead, &dwRead, 0 ) )
								{
									if ( ERROR_IO_INCOMPLETE != GetLastError() )
									{
										fError = TRUE;	// bail out
									}
								}
								else
								{
									HandleRead( bReadChr );
									fHandled = TRUE;
								}
								break;
							}
						default:
							{
								fError = TRUE;
								break;
							}
						}
					} while ( !fHandled && !fError && !IsTerminating() );
					if ( IsTerminating() ) return;

					break;
				}
			case ERROR_SUCCESS:
				{
					HandleRead( bReadChr );
					break;
				}
			default:
				{
					fError = TRUE;
				}
			}
		}
	};

	// there are some error occured
	SetTerminating();
	Error( GetLastError() );
	_tprintf( TEXT( "Read error: %d\n" ), GetLastError() );
}

VOID CCOMPort::WritePort()
{
	BOOL fError = FALSE;
	ResetEvent( m_event.hWrite );
	
	while ( !fError )
	{
		if ( !m_vCommandList.empty() )
		{
			DUMPER_COMMAND cmd;
			OVERLAPPED ovWrite = { 0 };
			DWORD dwWrite = 0;
			m_cs.Enter();
			cmd = m_vCommandList.front();
			m_vCommandList.pop();
			m_cs.Leave();

			// okay, we've got a command, now try to send it
			ovWrite.hEvent = m_event.hWrite;
			SetEvent( m_event.hWrite );
			BOOL fSuccess = WriteFile( *this, cmd.data(), LODWORD( cmd.size() ), &dwWrite, &ovWrite );
			if ( !fSuccess )
			{
				switch ( GetLastError() )
				{
				case ERROR_IO_PENDING:
					{
						DWORD dwResult = WaitForSingleObject( m_event.hWrite, 10 );
						switch ( dwResult )
						{
						case WAIT_TIMEOUT:
							{
								if ( IsTerminating() )
								{
									return;
								}
								break;
							}
						case WAIT_OBJECT_0:
							{
								// write ok!
								break;
							}
						default:
							{
								fError = TRUE;
								break;
							}
						}
						break;
					}
				default:
					{
						fError = TRUE;
						break;
					}

				}
			}
		}
		else
		{
			Sleep( 5 );
			if ( IsTerminating() )
			{
				return;
			}
		}
	}

	// thread terminated
	SetTerminating();
	Error( GetLastError() );
	_tprintf( TEXT( "Write error: %d\n" ), GetLastError() );
}

VOID CCOMPort::SendCommand( const DUMPER_COMMAND & cmd )
{
	if ( IsTerminating() || !IsConnected() ) return;

	_tprintf( TEXT( "-> " ) );
	for ( const auto & chr : cmd ) _tprintf( TEXT( "%c" ), chr );
	_tprintf( TEXT( "\n" ) );

	m_cs.Enter();
	m_vCommandList.push( cmd );
	m_cs.Leave();
}

VOID CCOMPort::SendCommand( const LPCTSTR pszTextCommand )
{
	if ( IsTerminating() || !IsConnected() ) return;

	DUMPER_COMMAND cmd( lstrlen( pszTextCommand ), 0 );
	if ( cmd.empty() ) return;

#ifdef UNICODE
	WideCharToMultiByte( CP_ACP, 0, pszTextCommand, LODWORD( cmd.size() ), (LPSTR) cmd.data(), LODWORD( cmd.size() ), nullptr, nullptr );
#else
	CopyMemory( cmd.data(), pszTextCommand, LODWORD( cmd.size() ) );
#endif
	SendCommand( cmd );
}