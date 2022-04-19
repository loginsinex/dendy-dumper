// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")

#pragma warning(disable:4838)

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <Resource.h>
// reference additional headers your program requires here
extern void * __cdecl operator new( size_t size );
extern void __cdecl operator delete( void *ptr );
extern void __cdecl operator delete[]( void *ptr );

#include "h\GUI\Wrapper\Controls.h"

#include "h/NES/dumper.h"

#include "h\GUI\Dialogs\CAboutDlg.h"
#include "h\GUI\Dialogs\CHEXEditorDlg.h"
#include "h\GUI\Dialogs\CCOMSettingsDlg.h"
#include "h\GUI\Dialogs\CDumperConnectDlg.h"
#include "h\GUI\Dialogs\MainWindow\CMainWindow.h"