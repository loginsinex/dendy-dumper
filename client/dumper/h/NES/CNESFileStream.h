#pragma once

typedef enum _tagNES_FILE_STREAM_MODE
{
	fsmRead,
	fsmWrite
} NES_FILE_STREAM_MODE, *PNES_FILE_STREAM_MODE;

class CNESFileStream
{
	HANDLE				m_hFile;
	CNESFileStream( const CNESFileStream & )
	{}

public:

	CNESFileStream( const tstring & sFile, NES_FILE_STREAM_MODE fsm )
	{
		switch ( fsm )
		{
		case fsmRead: m_hFile = CreateFile( sFile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr ); break;
		case fsmWrite: m_hFile = CreateFile( sFile.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, nullptr ); break;
		}

		if ( INVALID_HANDLE_VALUE == m_hFile )
		{
			throw std::exception( "Open file failed" );
		}
	}

	~CNESFileStream()
	{
		CloseHandle( m_hFile );
		m_hFile = INVALID_HANDLE_VALUE;
	}

	template <class T>
	VOID		read( T & data )
	{
		DWORD dwRead = 0;
		if ( !ReadFile( m_hFile, &data, sizeof( data ), &dwRead, nullptr ) || dwRead != sizeof( data ) )
		{
			throw std::exception( "Read file failed" );
		}
	}

	template <class T>
	BOOL		read( std::vector<T> & vdata )
	{
		DWORD dwRead = 0;
		DWORD uSize = LODWORD( vdata.size() * sizeof( T ) );
		if ( !ReadFile( m_hFile, vdata.data(), uSize, &dwRead, nullptr ) || dwRead != uSize )
		{
			throw std::exception( "Read file failed" );
		}

		return TRUE;
	}

	template <class T>
	BOOL		write( const T & data )
	{
		DWORD dwWrite = 0;
		if ( !WriteFile( m_hFile, &data, sizeof( data ), &dwWrite, nullptr ) || dwWrite != sizeof( data ) )
		{
			throw std::exception( "Write file failed" );
		}

		return TRUE;
	}

	template <class T>
	BOOL		write( const std::vector<T> & vdata )
	{
		DWORD dwWrite = 0;
		DWORD uSize = LODWORD( vdata.size() * sizeof( T ) );

		if ( !WriteFile( m_hFile, vdata.data(), uSize, &dwWrite, nullptr ) || dwWrite != uSize )
		{
			throw std::exception( "Write file failed" );
		}

		return TRUE;
	}
};
