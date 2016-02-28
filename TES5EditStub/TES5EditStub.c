/**
 * @file TES5EditStub.c
 * @see README.md
 */
#include <pch.h>
#define TARGET_EXENAME _T("FO4Edit.exe")

/**
 *  Sorta kinda like StringCchVPrintf somewhat not really. Caller responsible
 *  for cleanup.
 */
STATIC LPTSTR StringCchVAPrintf(LPCTSTR lpMessage, va_list lpArgs)
{
	TCHAR *lpResult;
	SIZE_T szDisplayBuf;
	
	// Check the resulting size of the buffer.
	szDisplayBuf = (SIZE_T)_vsctprintf(lpMessage, lpArgs) + 1;

	// Allocate our buffer.
	lpResult = (TCHAR*)calloc(szDisplayBuf, sizeof(TCHAR));
	assert(lpResult != NULL);

	// Finally, fill in the message.
	_vsntprintf(lpResult, szDisplayBuf, lpMessage, lpArgs);
	return lpResult;
}

/**
 *  Sorta kinda like StringCchPrintf somewhat not really. Caller responsible
 *  for cleanup.
 */
STATIC LPTSTR StringCchAPrintf(LPCTSTR lpMessage, ...)
{
	va_list lpArgs = NULL;
	TCHAR* lpResult = NULL;
	// Allocate buffer for our resulting format string.
	va_start(lpArgs, lpMessage);
	lpResult = StringCchVAPrintf(lpMessage, lpArgs);
	va_end(lpArgs);
	return lpResult;
}

#if 0

STATIC NOINLINE INT MyMessageBox(HWND hWnd, LPTSTR lpMessage, LPTSTR lpCaption, UINT uFlags)
{
	STATIC INT(WINAPI* pfnMessageBox)(HWND, LPTSTR, LPTSTR, UINT) = NULL;
	if(pfnMessageBox == NULL) {
		// Load user32.dll
		HMODULE hUser32 = LoadLibraryW(L"user32.dll"); assert(hUser32 != NULL);
		// Grab our MessageBox function
		*((FARPROC*)(&pfnMessageBox)) = GetProcAddress(hUser32, XSTRA(MessageBox));
		assert(pfnMessageBox != NULL);
	}
	// Show message box
	return pfnMessageBox(hWnd, lpMessage, lpCaption, uFlags);
}

#endif

#ifdef BUILD_CUI
#	include <conio.h>
#endif

/**
 *  If we have a console, print the error details there. Otherwise, show
 *  a message box.
 */
STATIC INLINE VOID ShowErrorMessage(LPTSTR lpMessage)
{
#	ifdef BUILD_CUI
	_tprintf(_T("FATAL: %s\nPress any key to continue.\n"), lpMessage);
	_getch();
#	else
	// Show error message box
	//MyMessageBox(NULL, lpMessage, _T("Fatal Error"), MB_ICONERROR);
	MessageBox(NULL, lpMessage, _T("Fatal Error"), MB_ICONERROR);
#	endif
}

/**
 * Fatal error handler. Provides support for formatting the system
 * error message and showing errors with format strings etc blahblah.
 */
STATIC NORETURN FatalError(DWORD dwError, LPTSTR lpMessage, ...)
{
	DWORD dwExit;
	va_list lpArgs = NULL;
	LPTSTR lpDisplayBuf = NULL;
	
	// If dwError has not been set, use the value of GetLastError
	if(dwError == ERROR_SUCCESS) {
		dwError = GetLastError();
	}
	// Format message against args.
	va_start(lpArgs, lpMessage);
	lpDisplayBuf = StringCchVAPrintf(lpMessage, lpArgs);
	va_end(lpArgs);
	
	// If dwError contains an error code, get the system error message for it.
	if(dwError != ERROR_SUCCESS) {
		TCHAR *lpErrMsg = NULL, *lpTemp = NULL;
		dwExit = dwError;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpErrMsg, 0, NULL
		);
		lpTemp = StringCchAPrintf(_T("%s - Windows Error [0x%08X]: %s"), lpDisplayBuf, dwError, lpErrMsg);
		xfree(lpDisplayBuf);
		lpDisplayBuf = lpTemp;
		LocalFree((HLOCAL)lpErrMsg);
	} else {
		dwExit = 1;
	}
	
	ShowErrorMessage(lpDisplayBuf);
	xfree(lpDisplayBuf);
	ExitProcess(dwExit);
}


/** Resolve filepath to target executable and verify it exists. */
STATIC BOOL EnsureExecutable(LPTSTR lpExecutable, SIZE_T* lpszSize)
{
	BOOL bComplete = FALSE;
	SIZE_T szModule = (SIZE_T)GetModuleFileName(NULL, lpExecutable, (DWORD)*lpszSize);
	if(szModule == 0) {
		FatalError(
			ERROR_SUCCESS,
			_T("Failed to get filepath of executable with GetModuleFileName - *lpszSize = %u"),
			*lpszSize
		);
	}
	
	// Zero out everything after the last '\'
	while(lpExecutable[--szModule] != L'\\' && szModule > 0) {
		lpExecutable[szModule] = 0;
	}
	
	// Then add the target filename and record the new length.
	_tcscat(lpExecutable, TARGET_EXENAME);
	*lpszSize = szModule + ZTLEN(TARGET_EXENAME) + 1;
	
	// Lastly, check to make sure the target exe exists.
	return GetFileAttributes(lpExecutable) != INVALID_FILE_ATTRIBUTES;
}

/** Check if lpArg contains a space */
STATIC BOOL HasSpace(LPTSTR lpArg, SIZE_T szLen)
{
	SIZE_T c = 0;
	for(; c < szLen; c++) {
		if(_istspace(lpArg[c])) {
			return TRUE;
		}
	}
	return FALSE;
}

/** Surround arg with quotes. Caller responsible for cleanup. */
STATIC LPTSTR Quote(LPTSTR lpArg, SIZE_T* lpszArg, BOOL bSpace)
{
	LPTSTR lpNewArg;
	*lpszArg += 2;
	if(bSpace) {
		*lpszArg += 1;
	}
	lpNewArg = (LPTSTR)calloc(*lpszArg + 1, sizeof(TCHAR));
	assert(lpNewArg != NULL);
	if(bSpace) {
		_sntprintf(lpNewArg, *lpszArg, _T(" \"%s\""), lpArg);
	} else {
		_sntprintf(lpNewArg, *lpszArg, _T("\"%s\""), lpArg);
	}
	
	return lpNewArg;
}

/** Duplicate arg string with option for prefixing a space */
STATIC LPTSTR DupArg(LPTSTR lpArg, SIZE_T* lpszArg, BOOL bSpace)
{
	LPTSTR lpNewArg;
	if(bSpace) {
		*lpszArg += 1;
	}
	
	lpNewArg = (LPTSTR)calloc(*lpszArg + 1, sizeof(TCHAR));
	assert(lpNewArg != NULL);
	
	if(bSpace) {
		_tcscpy(lpNewArg, _T(" "));
		_tcscat(lpNewArg, lpArg);
	} else {
		_tcscpy(lpNewArg, lpArg);
	}
	
	return lpNewArg;
}

/**
 *  Given the original argv stuff, the length of our target executable, and the
 *  filepath, build a suitable command line for CreateProcess.
 */
STATIC LPTSTR BuildCmdLine(int argc, TCHAR* argv[], SIZE_T szExecutable, LPTSTR lpExecutable)
{
	int i = 1;
	LPTSTR lpCmdLine = NULL;
	SIZE_T szCmdLine = szExecutable;
	TCHAR** lpNewArgs = (TCHAR**)calloc(argc, sizeof(TCHAR*));
	assert(lpNewArgs != NULL);
	
	// Quote the executable path if need be.
	XINFO(_T("Building command line.."));
	if(HasSpace(lpExecutable, szExecutable)) {
		lpNewArgs[0] = Quote(lpExecutable, &szCmdLine, FALSE);
	} else {
		lpNewArgs[0] = DupArg(lpExecutable, &szCmdLine, FALSE);
	}
	
	// Iterate through all args, quoting as necessary.
	for(; i < argc; i++) {
		SIZE_T szArg = (SIZE_T)_tcslen((const TCHAR*)argv[i]);
		if(HasSpace(argv[i], szArg)) {
			lpNewArgs[i] = Quote(argv[i], &szArg, TRUE);
		} else {
			lpNewArgs[i] = DupArg(argv[i], &szArg, TRUE);
		}
		// +1 for space + len(arg)
		szCmdLine += szArg;
	}
	
	// Allocate our cmdline buffer and begin adding the various parts.
	lpCmdLine = (LPTSTR)calloc(szCmdLine + 1, sizeof(TCHAR));
	assert(lpCmdLine != NULL);
	_tcscpy(lpCmdLine, lpNewArgs[0]);
	XINFO(_T("  [0] => \"%s\""), lpNewArgs[0]);
	
	// Cleanup allocated arg.
	xfree(lpNewArgs[0]);
	lpNewArgs[0] = NULL;
	
	for(i = 1; i < argc; i++) {
		XINFO(_T("  [%d] => \"%s\""), i, argv[i]);
		_tcscat(lpCmdLine, lpNewArgs[i]);
		xfree(lpNewArgs[i]);
		lpNewArgs[i] = NULL;
	}
	
	XINFO(_T("Finished building.."));
	XINFO(_T("CmdLine: %s"), lpCmdLine);
	xfree(lpNewArgs);
	return lpCmdLine;
}

STATIC int ExecuteTarget(LPTSTR lpCmdLine)
{
	HWND hWnd = NULL;
	// DWORD dwExitCode = ERROR_SUCCESS;
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { sizeof(STARTUPINFO), 0 };
	
	/* Execute the process */
	if (CreateProcess(
		NULL,	/* No module name (use command line) */
		lpCmdLine,	/* Command line */
		NULL,	/* Process handle not inheritable */
		NULL,	/* Thread handle not inheritable */
		FALSE,	/* Set handle inheritance to FALSE */
		0,	/* No creation flags */
		NULL,	/* Use parent's environment block */
		NULL,	/* Use parent's starting directory */
		&si,	/* Pointer to STARTUPINFO structure */
		&pi	/* Pointer to PROCESS_INFORMATION structure */
	)) {
		DWORD dwProcessId = 0;
		
		/* Wait until target executable has finished its initialization */
		WaitForInputIdle(pi.hProcess, INFINITE);

		/* Find target executable window */
		hWnd = GetTopWindow(0);
		while(hWnd != NULL) {
			GetWindowThreadProcessId(hWnd, &dwProcessId);
			if (dwProcessId == pi.dwProcessId) {
				/* And move it to the top. */
				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				break;
			}
			hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
		}

		/* Wait until target executable exits. */
		// Nah
		//WaitForSingleObject(pi.hProcess, INFINITE);
		//GetExitCodeProcess(pi.hProcess, &dwExitCode);

		/* Close process and thread handles. */
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	} else {
		FatalError(ERROR_SUCCESS, _T("Failed to execute cmdline: %s"), lpCmdLine);
		xfree(lpCmdLine);
	}
	
	return ERROR_SUCCESS;
}

#ifdef BUILD_CUI
int CDECL _tmain(int argc, TCHAR* argv[])
#else
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpOriginalCmdLine, int nCmdShow)
#	define argv __targv
#	define argc __argc
#endif
{
	int iResult = 0;
	LPTSTR lpCmdLine = NULL;
	TCHAR lpExecutable[_MAX_PATH+1] = T_EMPTY;
	SIZE_T szExecutable = _MAX_PATH + 1;
	if(!EnsureExecutable(lpExecutable, &szExecutable)) {
		FatalError(ERROR_SUCCESS, _T("EnsureExecutable failed! No executable found at path:\n\n%s"), lpExecutable);
	}
	
	lpCmdLine = BuildCmdLine(argc, argv, szExecutable, lpExecutable);
	if(lpCmdLine == NULL) {
		FatalError(ERROR_SUCCESS, _T("Failed to build cmdline for executable: %s"), lpExecutable);
	}
	
	iResult = ExecuteTarget(lpCmdLine);
	xfree(lpCmdLine);
	return iResult;
}
