#pragma once

#include <queue>

#define			WM_DUMP_START				( WM_USER + 0x122 )
#define			WM_DUMP_PROGRESS			( WM_USER + 0x123 )
#define			WM_DUMP_FINISHED			( WM_USER + 0x124 )
#define			WM_DUMP_DATA				( WM_USER + 0x125 )

typedef enum _tagDUMPER_QUERY : BYTE
{
	dqPRGMode		= 'G',
	dqPPUMode		= 'P',
	dqReadMode		= 'R',
	dqWriteMode		= 'W',
	dqSetCounter	= 'C',
	dqResetM2		= 'Y',
	dqReqMirroring	= 'M'
} DUMPER_QUERY, *PDUMPER_QUERY;


typedef enum _tagDUMPER_MODE : BYTE
{
	dmPRG			= dqPRGMode,
	dmPPU			= dqPPUMode
} DUMPER_MODE, *PDUMPER_MODE;

typedef enum _tagDUMPER_ANSWER : BYTE
{
	daBootUp		= '!',
	daSuccess		= '.',
	daFailed		= '?'
} DUMPER_ANSWER, *PDUMPER_ANSWER;

typedef struct _tagDUMPER_RESULT
{
	const DUMPER_QUERY		query;
	USHORT					uAddress;
	BYTE					bData;
	std::vector<BYTE>		result;
} DUMPER_RESULT, *PDUMPER_RESULT;

typedef enum _tagDUMP_PROGRESS_STATE
{
	dpsStart,
	dpsProgress,
	dpsFinish,
	dpsError,
	dpsMirroringDetect,
	dpsResetM2
} DUMP_PROGRESS_STATE, *PDUMP_PROGRESS_STATE;

typedef struct _tagDUMP_NOTIFY_PROGRESS
{
	DUMP_PROGRESS_STATE	dps;
	CString				sDescription;
	DWORD				dwCurrent;
	DWORD				dwMaximum;
} DUMP_NOTIFY_PROGRESS, *PDUMP_NOTIFY_PROGRESS;

class CDumper: public CCOMPort
{
	CCriticalSection				m_cs;
	const HWND						m_hOwner;
	std::queue<DUMPER_RESULT>		m_qResult;
	BOOL			m_fQueueEmpty;

	VOID			HandleRead( BYTE bReadChr ) override;
	BOOL			SendNextCommand();

	BOOL			HandlePRGMode( DUMPER_RESULT & result, BYTE bReadChr );
	BOOL			HandlePPUMode( DUMPER_RESULT & result, BYTE bReadChr );
	BOOL			HandleReadMode( DUMPER_RESULT & result, BYTE bReadChr );
	BOOL			HandleWriteMode( DUMPER_RESULT & result, BYTE bReadChr );
	BOOL			HandleSetCounter( DUMPER_RESULT & result, BYTE bReadChr );
	BOOL			HandleResetM2( DUMPER_RESULT & result, BYTE bReadChr );
	BOOL			HandleMirroringReq( DUMPER_RESULT & result, BYTE bReadChr );


	VOID			DoConvert( const std::vector<BYTE> & result, std::vector<BYTE> & bin ) const;

	VOID			SetCounter( USHORT uCounter );
	VOID			SelectDumperMode( DUMPER_MODE mode );
	VOID			RequestDumperRead( USHORT uAddress );
	VOID			RequestDumperWrite( USHORT uAddress, BYTE bData );
	VOID			SendM2Reset();
	VOID			SendMirroringReq();

protected:
	CDumper( DWORD dwPortId, BAUD_RATE dwRate, HWND hOwner );

	VOID			ReadPRGData( USHORT uAddress );
	VOID			WritePRG( USHORT uAddress, BYTE uByte );
	VOID			ReadPPUData( USHORT uAddress );
	VOID			WritePPU( USHORT uAddress, BYTE uByte );
	VOID			ReadPRGRange( USHORT uFrom, USHORT uEnd );
	VOID			ReadPPURange( USHORT uFrom, USHORT uEnd );
	VOID			SendNotifyProgress( const CString & sDescription, DUMP_PROGRESS_STATE dps, DWORD dwCurrent, DWORD dwMaximum );
	VOID			SendNotifyProgress( const std::vector<BYTE> & data );
	VOID			SendNotifyState( DWORD uState, DWORD dwError );
	VOID			ResetM2();
	VOID			DetectMirroring();

	virtual VOID	Data( const std::vector<BYTE> & data ) PURE;
	virtual VOID	WriteDone( BOOL fSucceeded ) PURE;
	virtual VOID	Bootup() PURE;
	virtual VOID	Error( DWORD dwError ) PURE;
	virtual VOID	ResetM2Done( BOOL fStatus ) PURE;
	virtual VOID	ReqMirroringDone( BYTE bMirroring ) PURE;
};