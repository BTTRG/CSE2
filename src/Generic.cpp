#include "Generic.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "WindowsWrapper.h"

#include "CommonDefines.h"
#include "Tags.h"

void GetCompileDate(int *year, int *month, int *day)
{
	int i;
	const char *months[13];
	char month_string[0x10];

	months[0] = "XXX";
	months[1] = "Jan";
	months[2] = "Feb";
	months[3] = "Mar";
	months[4] = "Apr";
	months[5] = "May";
	months[6] = "Jun";
	months[7] = "Jul";
	months[8] = "Aug";
	months[9] = "Sep";
	months[10] = "Oct";
	months[11] = "Nov";
	months[12] = "Dec";
	sscanf(__DATE__, "%s %d %d", &month_string, day, year);

	for (i = 0; i < 12; ++i)	// This being 12 instead of 13 might be a bug, but it works anyway by accident
		if (!memcmp(&month_string, months[i], 3))
			break;

	*month = i;
}

#ifdef WINDOWS
// TODO - Inaccurate stack frame
BOOL GetCompileVersion(int *v1, int *v2, int *v3, int *v4)
{
	unsigned int puLen;
	VS_FIXEDFILEINFO *lpBuffer;
	DWORD dwHandle;
	DWORD dwLen;
	char path[PATH_LENGTH];
	LPVOID lpData;
	BOOL bResult;

	lpData = NULL;
	bResult = FALSE;

	GetModuleFileNameA(NULL, path, sizeof(path));
	dwLen = GetFileVersionInfoSizeA(path, &dwHandle);

	if (dwLen == 0)
	{
		
	}
	else
	{
		lpData = malloc(dwLen);

		if (lpData == NULL)
		{
			
		}
		else
		{
			if (!GetFileVersionInfoA(path, 0, dwLen, lpData))
			{
				
			}
			else
			{
				if (!VerQueryValueA(lpData, "\\", (LPVOID*)&lpBuffer, &puLen))
				{
					
				}
				else
				{
					*v1 = (unsigned short)(lpBuffer->dwFileVersionMS >> 16);
					*v2 = (unsigned short)(lpBuffer->dwFileVersionMS & 0xFFFF);
					*v3 = (unsigned short)(lpBuffer->dwFileVersionLS >> 16);
					*v4 = (unsigned short)(lpBuffer->dwFileVersionLS & 0xFFFF);
					bResult = TRUE;
				}
			}
		}
	}

	if (lpData)
		free(lpData);

	return bResult;
}
#else
BOOL GetCompileVersion(int *v1, int *v2, int *v3, int *v4)
{
	*v1 = 1;
	*v2 = 0;
	*v3 = 0;
	*v4 = 6;
	return TRUE;
}
#endif

#ifdef WINDOWS
// This seems to be broken in recent Windows (Sndvol32.exe was renamed 'SndVol.exe')
// TODO - Inaccurate stack frame
BOOL OpenVolumeConfiguration(HWND hWnd)
{
	char path[PATH_LENGTH];
	char path2[PATH_LENGTH];
	char path3[PATH_LENGTH];
	int error1;
	int error2;
	size_t i;

	GetSystemDirectoryA(path, sizeof(path));
	sprintf(path2, "%s\\Sndvol32.exe", path);

	i = strlen(path);
	while (path[i] != '\\')
		--i;

	path[i] = '\0';
	sprintf(path3, "%s\\Sndvol32.exe", path);

	error1 = (int)ShellExecuteA(hWnd, "open", path2, NULL, NULL, SW_SHOW);
	error2 = (int)ShellExecuteA(hWnd, "open", path3, NULL, NULL, SW_SHOW);

	if (error1 <= 32 && error2 <= 32)
		return FALSE;
	else
		return TRUE;
}
#endif

#ifdef WINDOWS
void DeleteDebugLog(void)
{
	char path[PATH_LENGTH];

	sprintf(path, "%s\\debug.txt", gModulePath);
	DeleteFileA(path);
}

BOOL PrintDebugLog(const char *string, int value1, int value2, int value3)
{
	char path[PATH_LENGTH];
	FILE *fp;

	sprintf(path, "%s\\debug.txt", gModulePath);
	fp = fopen(path, "a+t");

	if (fp == NULL)
		return FALSE;

	fprintf(fp, "%s,%d,%d,%d\n", string, value1, value2, value3);
	fclose(fp);
	return TRUE;
}
#endif

#ifdef WINDOWS
/*
This function is a mystery. It seems to check if the system time is within
a certain range, specified by the two parameters. Nothing in the original game
uses this code.

This is just speculation, but this *might* have been used in those prototypes
Pixel released to testers, to prevent them from running after a certain date.
*/
int CheckTime(SYSTEMTIME *system_time_low, SYSTEMTIME *system_time_high)
{
	FILETIME FileTime1;
	FILETIME FileTime2;
	SYSTEMTIME SystemTime;

	GetSystemTime(&SystemTime);
	SystemTimeToFileTime(&SystemTime, &FileTime1);
	SystemTimeToFileTime(system_time_low, &FileTime2);

	if (CompareFileTime(&FileTime2, &FileTime1) >= 0)
		return -1;	// Return if actual time is lower than system_time_low

	SystemTimeToFileTime(system_time_high, &FileTime2);

	if (CompareFileTime(&FileTime2, &FileTime1) <= 0)
		return 1;	// Return if actual time is higher than system_time_high
	else
		return 0;
}
#endif

BOOL CheckFileExists(const char *name)
{
	char path[PATH_LENGTH];

#ifdef NONPORTABLE
	sprintf(path, "%s\\%s", gModulePath, name);
#else
	sprintf(path, "%s/%s", gModulePath, name);
#endif

	FILE *file = fopen(path, "rb");

	if (file == NULL)
		return FALSE;

	fclose(file);
	return TRUE;
}

long GetFileSizeLong(const char *path)
{
#ifdef NONPORTABLE
	DWORD len;
	HANDLE hFile;

	len = 0;

	hFile = CreateFileA(path, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1;

	len = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	return len;
#else
	long len;
	FILE *fp;

	len = 0;

	fp = fopen(path, "rb");
	if (fp == NULL)
		return -1;

	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fclose(fp);
	return len;
#endif
}

#ifdef WINDOWS
BOOL PrintBitmapError(char *string, int value)
{
	char path[PATH_LENGTH];
	FILE *fp;

	sprintf(path, "%s\\%s", gModulePath, "error.log");

	if (GetFileSizeLong(path) > 0x19000)	// Purge the error log if it gets too big, I guess
		DeleteFileA(path);

	fp = fopen(path, "a+t");
	if (fp == NULL)
		return FALSE;

	fprintf(fp, "%s,%d\n", string, value);
	fclose(fp);
	return TRUE;
}
#endif

BOOL IsShiftJIS(unsigned char c)
{
	if (c >= 0x81 && c <= 0x9F)
		return TRUE;

	if (c >= 0xE0 && c <= 0xEF)
		return TRUE;

	return FALSE;
}

// TODO - Inaccurate stack frame
BOOL CenterWindow(HWND hWnd)
{
	RECT window_rect;
	HWND parent_hwnd;
	RECT parent_rect;
	int x;
	int y;
	RECT child_rect;

	SystemParametersInfoA(SPI_GETWORKAREA, 0, &child_rect, 0);

	GetWindowRect(hWnd, &window_rect);

	parent_hwnd = GetParent(hWnd);
	if (parent_hwnd)
		GetWindowRect(parent_hwnd, &parent_rect);
	else
		SystemParametersInfoA(SPI_GETWORKAREA, 0, &parent_rect, 0);

	x = parent_rect.left + (parent_rect.right - parent_rect.left - (window_rect.right - window_rect.left)) / 2;
	y = parent_rect.top + (parent_rect.bottom - parent_rect.top - (window_rect.bottom - window_rect.top)) / 2;

	if (x < child_rect.left)
		x = child_rect.left;

	if (y < child_rect.top)
		y = child_rect.top;

	if (window_rect.right - window_rect.left + x > child_rect.right)
		x = child_rect.right - (window_rect.right - window_rect.left);

	if (window_rect.bottom - window_rect.top + y > child_rect.bottom)
		y = child_rect.bottom - (window_rect.bottom - window_rect.top);

	return SetWindowPos(hWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);
}

// TODO - Inaccurate stack frame
BOOL LoadWindowRect(HWND hWnd, char *filename, BOOL unknown)
{
	char path[PATH_LENGTH];
	int min_window_width;
	int min_window_height;
	int max_window_width;
	int max_window_height;
	FILE *fp;
	RECT Rect;
	int showCmd;
	RECT pvParam;

	showCmd = SW_SHOWNORMAL;

	sprintf(path, "%s\\%s", gModulePath, filename);

	fp = fopen(path, "rb");
	if (fp)
	{
		fread(&Rect, sizeof(RECT), 1, fp);
		fread(&showCmd, sizeof(int), 1, fp);
		fclose(fp);

		SystemParametersInfoA(SPI_GETWORKAREA, 0, &pvParam, 0);

		max_window_width = GetSystemMetrics(SM_CXMAXIMIZED);
		max_window_height = GetSystemMetrics(SM_CYMAXIMIZED);
		min_window_width = GetSystemMetrics(SM_CXMIN);
		min_window_height = GetSystemMetrics(SM_CYMIN);

		if (Rect.right - Rect.left < min_window_width)
			Rect.right = min_window_width + Rect.left;
		if (Rect.bottom - Rect.top < min_window_height)
			Rect.bottom = min_window_height + Rect.top;
		if (Rect.right - Rect.left > max_window_width)
			Rect.right = max_window_width + Rect.left;
		if (Rect.bottom - Rect.top > max_window_height)
			Rect.bottom = max_window_width + Rect.top;

		if (Rect.left < pvParam.left)
		{
			Rect.right += pvParam.left - Rect.left;
			Rect.left = pvParam.left;
		}
		if (Rect.top < pvParam.top)
		{
			Rect.bottom += pvParam.top - Rect.top;
			Rect.top = pvParam.top;
		}
		if (Rect.right > pvParam.right)
		{
			Rect.left -= Rect.right - pvParam.right;
			Rect.right -= Rect.right - pvParam.right;
		}
		if (Rect.bottom > pvParam.bottom)
		{
			Rect.top -= Rect.bottom - pvParam.bottom;
			Rect.bottom -= Rect.bottom - pvParam.bottom;
		}

		if (unknown)
			MoveWindow(hWnd, Rect.left, Rect.top, Rect.right - Rect.left, Rect.bottom - Rect.top, 0);
		else
			SetWindowPos(hWnd, HWND_TOP, Rect.left, Rect.top, 0, 0, SWP_NOSIZE);
	}

	if (showCmd == SW_MAXIMIZE)
	{
		if (!ShowWindow(hWnd, SW_MAXIMIZE))
			return FALSE;
	}
	else
	{
		ShowWindow(hWnd, SW_SHOWNORMAL);
	}

	return TRUE;
}

BOOL SaveWindowRect(HWND hWnd, const char *filename)
{
	char path[PATH_LENGTH];
	WINDOWPLACEMENT wndpl;
	FILE *fp;
	RECT rect;

	if (!GetWindowPlacement(hWnd, &wndpl))
		return FALSE;

	if (wndpl.showCmd == SW_SHOWNORMAL)
	{
		if (!GetWindowRect(hWnd, &rect))
			return FALSE;

		wndpl.rcNormalPosition = rect;
	}

	sprintf(path, "%s\\%s", gModulePath, filename);

	fp = fopen(path, "wb");
	if (fp == NULL)
		return FALSE;

	fwrite(&wndpl.rcNormalPosition, sizeof(RECT), 1, fp);
	fwrite(&wndpl.showCmd, sizeof(int), 1, fp);
	fclose(fp);

	return TRUE;
}

BOOL IsEnableBitmap(const char *path)
{
	char str[16];
	static const char *extra_text = "(C)Pixel";

	const long len = (long)strlen(extra_text);

	FILE *fp = fopen(path, "rb");

	if (fp == NULL)
		return FALSE;

	fseek(fp, len * -1, SEEK_END);
	fread(str, 1, len, fp);
	fclose(fp);

	if (memcmp(str, extra_text, len) != 0)
		return FALSE;
	else
		return TRUE;
}
