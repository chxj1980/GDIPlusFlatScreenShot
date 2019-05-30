
#include <stdio.h>
#include <Windows.h>
#include <ShellScalingApi.h>
#include "gdiflat.h"

int GetEncoderClsid(const WCHAR * format, CLSID * pClsid)
{
	UINT num = 0;	// number of image encoders
	UINT size = 0;	// size of the image encoder array in bytes

	ImageCodecInfo *pImageCodecInfo = NULL;

	GdipGetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;	// Failure

	pImageCodecInfo = GdipAlloc(size);
	if (pImageCodecInfo == NULL)
		return -1;	// Failure

	GdipGetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			GdipFree(pImageCodecInfo);
			return j;	// Success
		}
	}

	GdipFree(pImageCodecInfo);
	return -1;	// Failure
}


int main()
{
	int x, y; UINT dpi;
	HWND window;
	GdiplusStartupInput startupinp;
	ULONG_PTR token;
	CLSID Clsid;
	HDC hdc, memdc; HBITMAP bitmap,old; GpBitmap * gdibitmap;
	HGLOBAL global; LPVOID buf;
	LPSTREAM stream;

	if (CreateStreamOnHGlobal(NULL,1,&stream)!=S_OK)
	{
		printf("CreateStreamOnHGlobal() Failed");
		return -1;
	}

	ZeroMemory(&startupinp, sizeof(startupinp));
	startupinp.GdiplusVersion = 1;

	GdiplusStartup(&token, &startupinp, NULL);


	window = GetDesktopWindow();
	
	dpi = GetDpiForWindow(window);
	if (dpi != 96)
	{
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
	}

	x = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	y = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	//x = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, dpi);
	//y = GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, dpi);
	
	printf("DPI-%d\n", dpi);
	printf("X:%d-Y:%d\n", x, y);
	
	hdc = GetDC(window);
	memdc = CreateCompatibleDC(hdc);
	bitmap = CreateCompatibleBitmap(hdc, x, y);
	old = SelectObject(memdc, bitmap);

	BitBlt(memdc, 0, 0, x, y, hdc, 0, 0, SRCCOPY);

	if (GdipCreateBitmapFromHBITMAP(bitmap, NULL, &gdibitmap))
	{
		printf("GdipCreateBitmapFromHBITMAP() Failed");
		return -1;
	}

	GetEncoderClsid(L"image/png", &Clsid);
	/*
	if (GdipSaveImageToFile(gdibitmap, L"ScreenShot.png", &Clsid, NULL))
	{
		printf("GdipSaveImageToFile() Failed");
		return -1;
	}
	*/

	if (GdipSaveImageToStream(gdibitmap, stream, &Clsid, NULL))
	{
		printf("GdipSaveImageToStream() Failed");
		return -1;
	}

	GetHGlobalFromStream(stream, &global);
	buf = GlobalLock(global);
	HANDLE file = CreateFileA("ScreenShot2.png", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(file, buf, GlobalSize(global), NULL, NULL);
	CloseHandle(file);

	GlobalUnlock(global);
	GlobalFree(global);


	GdipDisposeImage(gdibitmap);
	GdiplusShutdown(token);
	
	SelectObject(memdc, old);
	
	DeleteObject(bitmap);
	DeleteObject(memdc);
	ReleaseDC(window,hdc);
	CloseWindow(window);

	if (dpi != 96)
	{
		SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);
	}


	return 0;
}