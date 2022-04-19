
#include "stdafx.h"

CMapperEvent::CMapperEvent()
	: m_hEvent( CreateEvent( nullptr, TRUE, FALSE, nullptr ) )
{
	ResetEvent( m_hEvent );
}

CMapperEvent::~CMapperEvent()
{
	SetEvent( m_hEvent );
	CloseHandle( m_hEvent );
}

VOID CMapperEvent::Reset()
{
	ResetEvent( m_hEvent );
}

VOID CMapperEvent::Raise()
{
	SetEvent( m_hEvent );
}

VOID CMapperEvent::Wait( DWORD uMilliseconds )
{
	WaitForSingleObject( m_hEvent, uMilliseconds );
}

// ----


CMapper::CMapper( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner )
	: CDumper( dwPortId, dwRate, hOwner ), m_fErrorState( FALSE ), m_fMirroring( FALSE ), m_fDumpInProgress( FALSE )
{

}

VOID CMapper::Data( const std::vector<BYTE>& data )
{
	m_data.insert( m_data.end(), data.begin(), data.end() );
	SendNotifyProgress( CString(), dpsProgress, LODWORD( m_data.size() ), m_dwExpected );
	_tprintf( TEXT( "." ) );
	if ( m_data.size() >= m_dwExpected )
	{
		m_event.Raise();
	}
}

VOID CMapper::Error( DWORD dwError )
{
	m_fErrorState = TRUE;
	m_event.Raise();
}

VOID CMapper::Bootup()
{
	m_fErrorState = TRUE;
	m_event.Raise();
}

VOID CMapper::ResetM2Done( BOOL fStatus )
{
	m_fErrorState = !fStatus;
	m_event.Raise();
}

VOID CMapper::ReqMirroringDone( BYTE bMirroring )
{
	switch ( bMirroring )
	{
		case 'V': SetMirroring( TRUE ); break;
		case 'H': SetMirroring( FALSE ); break;
	}

	m_event.Raise();
}

VOID CMapper::WriteDone( BOOL fSucceeded )
{
	m_fErrorState = !fSucceeded;
	m_event.Raise();
}

VOID CMapper::ReadPRG( const CString & sDescription, USHORT uAddress, USHORT uSize, std::vector<BYTE>& data )
{
	m_cs.Enter();
	m_data.clear();
	m_event.Reset();
	m_fErrorState = FALSE;
	USHORT uEnd = LOWORD( min( DWORD( uAddress ) + uSize, 0xFFFF ) );
	m_dwExpected = uEnd - uAddress + 1;
	SendNotifyProgress( sDescription, dpsStart, 0, m_dwExpected );
	m_data.reserve( m_dwExpected + 0x10 );
	ReadPRGRange( uAddress, uEnd );
	m_event.Wait( INFINITE );
	data = m_data;
	SendNotifyProgress( sDescription, dpsFinish, m_dwExpected, m_dwExpected );
	m_cs.Leave();

	if ( IsErrorState() )
	{
		SendNotifyProgress( sDescription, dpsError, m_dwExpected, m_dwExpected );
		throw CCOMPortError( ERROR_IO_INCOMPLETE );
	}
}

VOID CMapper::ResetCartridge()
{
	m_cs.Enter();
	m_event.Reset();
	CDumper::ResetM2();
	m_event.Wait( INFINITE );
	m_cs.Leave();
}

VOID CMapper::DoMirroringDetect()
{
	m_cs.Enter();
	m_event.Reset();
	CDumper::DetectMirroring();
	m_event.Wait( INFINITE );
	m_cs.Leave();
}

VOID CMapper::WritePRG( USHORT uAddress, BYTE bData )
{
	m_cs.Enter();
	m_event.Reset();
	CDumper::WritePRG( uAddress, bData );
	m_event.Wait( INFINITE );
	m_cs.Leave();
}

VOID CMapper::ReadPPU( const CString & sDescription, USHORT uAddress, USHORT uSize, std::vector<BYTE>& data )
{
	m_cs.Enter();
	m_data.clear();
	m_event.Reset();
	m_fErrorState = FALSE;
	USHORT uEnd = LOWORD( min( DWORD( uAddress ) + uSize, 0xFFFF ) );
	m_dwExpected = uEnd - uAddress + 1;
	m_data.reserve( m_dwExpected + 0x10 );
	SendNotifyProgress( sDescription, dpsStart, 0, m_dwExpected );
	ReadPPURange( uAddress, uEnd );
	m_event.Wait( INFINITE );
	data = m_data;
	SendNotifyProgress( sDescription, dpsFinish, m_dwExpected, m_dwExpected );
	m_cs.Leave();

	if ( IsErrorState() )
	{
		SendNotifyProgress( sDescription, dpsError, m_dwExpected, m_dwExpected );
		throw CCOMPortError( ERROR_IO_INCOMPLETE );
	}
}

VOID CMapper::WritePPU( USHORT uAddress, BYTE bData )
{
	m_cs.Enter();
	m_event.Reset();
	CDumper::WritePPU( uAddress, bData );
	m_event.Wait( INFINITE );
	m_cs.Leave();
}

BOOL CMapper::IsErrorState() const
{
	return m_fErrorState;
}

VOID CMapper::InitSettings( const MAPPER_SETTINGS & settings )
{
	m_settings = settings;
}

MAPPER_SETTINGS & CMapper::Settings()
{
	return m_settings;
}

VOID CMapper::EnumSettings( MAPPER_SETTINGS & settings ) const
{
	settings = m_settings;
}

VOID CMapper::SetSetting( size_t set_id, MAPPER_SETTING & setting )
{
	if ( set_id >= m_settings.size() ) return;

	auto & mysetting = m_settings[ set_id ];

	if ( setting.value_id < mysetting.vIntList.size() )
	{
		mysetting.value_id = setting.value_id;
	}
}

USHORT CMapper::GetSetting( const MAPPER_SETTING & setting ) const
{
	if ( setting.value_id < setting.vIntList.size() )
	{
		return setting.vIntList[ setting.value_id ];
	}

	return 0;
}

USHORT CMapper::GetSetting( size_t set_id ) const
{
	if ( set_id < m_settings.size() ) return GetSetting( m_settings[ set_id ] );

	return 0;
}

VOID CMapper::SetMirroring( BOOL fVertical )
{
	m_fMirroring = fVertical;
}

BOOL CMapper::GetMirroring() const
{
	return m_fMirroring;
}

VOID CMapper::AbortDump()
{
	m_fErrorState = TRUE;
	m_event.Raise();
	Sleep( 20 );
}

DWORD __stdcall CMapper::__Dump( LPVOID pParam )
{
	PDUMP_THREAD_DATA pData = (PDUMP_THREAD_DATA)pParam;
	pData->pThis->ThreadDump( pData->sFile.GetString() );
	delete pData;
	return 0;
}

DWORD __stdcall CMapper::__DumpManual( LPVOID pParam )
{
	PDUMP_MANUAL_THREAD_DATA pData = (PDUMP_MANUAL_THREAD_DATA)pParam;
	pData->pThis->ThreadDumpManual( pData->fPPU, pData->fWrite, pData->uAddress, pData->uLength, pData->vWriteData );
	delete pData;
	return 0;
}

VOID CMapper::ThreadDump( LPCTSTR pszFilename )
{
	SendNotifyState( WM_DUMP_START, 0 );
	try
	{
		BOOL fReconnect = IsConnected();
		if ( !fReconnect ) Connect();
		Dump( pszFilename );
		if ( !fReconnect ) Disconnect();
		SendNotifyState( WM_DUMP_FINISHED, IsErrorState() ? ERROR_IO_INCOMPLETE : 0 );
	}
	catch ( CCOMPortError & err )
	{
		SendNotifyState( WM_DUMP_FINISHED, err.What() );
	}
	catch ( std::exception & file_error )
	{
		file_error;
		SendNotifyState( WM_DUMP_FINISHED, ERROR_FILE_INVALID );
	}
	
	m_fDumpInProgress = FALSE;
}

VOID CMapper::StartDump( LPCTSTR pszFile )
{
	if ( m_fDumpInProgress ) return;

	m_fDumpInProgress = TRUE;

	DWORD tid = 0;
	PDUMP_THREAD_DATA pData = new DUMP_THREAD_DATA;
	pData->pThis = this;
	pData->sFile = pszFile;
	HANDLE hThread = CreateThread( nullptr, 0, CMapper::__Dump, pData, 0, &tid );
	if ( INVALID_HANDLE_VALUE == hThread )
	{
		m_fDumpInProgress = FALSE;
		delete pData;
	}
}

BOOL CMapper::IsInProgress() const
{
	return m_fDumpInProgress;
}

VOID CMapper::ThreadDumpManual( BOOL fPPU, BOOL fWrite, USHORT uAddress, USHORT uLength, const std::vector<BYTE> & vWriteData )
{
	SendNotifyState( WM_DUMP_START, 0 );
	try
	{
		if ( !fPPU )
		{
			if ( !fWrite )
			{
				std::vector<BYTE> vData( uLength, 0 );
				ReadPRG( CString(), uAddress, uLength, vData );
				SendNotifyProgress( vData );
			}
			else
			{
				for ( const auto & chr : vWriteData )
				{
					WritePRG( uAddress++, chr );
				}
			}
		}
		else
		{
			if ( !fWrite )
			{
				std::vector<BYTE> vData( uLength, 0 );
				ReadPPU( CString(), uAddress, uLength, vData );
				SendNotifyProgress( vData );
			}
			else
			{
				for ( const auto & chr : vWriteData )
				{
					WritePPU( uAddress++, chr );
				}
			}
		}
		SendNotifyState( WM_DUMP_FINISHED, IsErrorState() ? ERROR_IO_INCOMPLETE : 0 );
	}
	catch ( CCOMPortError & err )
	{
		SendNotifyState( WM_DUMP_FINISHED, err.What() );
	}
	catch ( std::exception & file_error )
	{
		file_error;
		SendNotifyState( WM_DUMP_FINISHED, ERROR_FILE_INVALID );
	}

	m_fDumpInProgress = FALSE;
}

VOID CMapper::ManualDump( PDUMP_MANUAL_THREAD_DATA pData )
{
	m_fDumpInProgress = TRUE;
	pData->pThis = this;

	DWORD tid = 0;
	HANDLE hThread = CreateThread( nullptr, 0, CMapper::__DumpManual, pData, 0, &tid );
	if ( INVALID_HANDLE_VALUE == hThread )
	{
		m_fDumpInProgress = FALSE;
		delete pData;
	}
}

VOID CMapper::DumpPRGData( USHORT uAddress, USHORT uLength )
{
	PDUMP_MANUAL_THREAD_DATA pData = new DUMP_MANUAL_THREAD_DATA;
	pData->fPPU = FALSE;
	pData->fWrite = FALSE;
	pData->uAddress = uAddress;
	pData->uLength = uLength;
	ManualDump( pData );
}

VOID CMapper::DumpPPUData( USHORT uAddress, USHORT uLength )
{
	PDUMP_MANUAL_THREAD_DATA pData = new DUMP_MANUAL_THREAD_DATA;
	pData->fPPU = TRUE;
	pData->fWrite = FALSE;
	pData->uAddress = uAddress;
	pData->uLength = uLength;
	ManualDump( pData );
}

VOID CMapper::SendPRGByte( USHORT uAddress, const std::vector<BYTE> & vData )
{
	PDUMP_MANUAL_THREAD_DATA pData = new DUMP_MANUAL_THREAD_DATA;
	pData->fPPU = FALSE;
	pData->fWrite = TRUE;
	pData->uAddress = uAddress;
	pData->uLength = 0;
	pData->vWriteData = vData;
	ManualDump( pData );
}

VOID CMapper::SendPPUByte( USHORT uAddress, const std::vector<BYTE> & vData )
{
	PDUMP_MANUAL_THREAD_DATA pData = new DUMP_MANUAL_THREAD_DATA;
	pData->fPPU = TRUE;
	pData->fWrite = TRUE;
	pData->uAddress = uAddress;
	pData->uLength = 0;
	pData->vWriteData = vData;
	ManualDump( pData );
}

VOID CMapper::MapperReset()
{
	CDumper::ResetM2();
}