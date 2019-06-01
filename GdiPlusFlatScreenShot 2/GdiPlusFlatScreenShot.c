#include<stdio.h>
#include<windows.h>
#include<gdiplus.h>
#include<combaseapi.h>

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
	IStream * stream;
	ULONG_PTR token;
	GdiplusStartupInput gdiinput;
	HGLOBAL global;LPVOID buf;
	HDC hdc,memdc;
	RECT r;HWND window;HBITMAP bitmap;GpBitmap * gdibitmap;CLSID clsid;DWORD len;

	ZeroMemory(&gdiinput,sizeof(gdiinput));
	gdiinput.GdiplusVersion = 1;

	SetProcessDPIAware();

	if(GdiplusStartup(&token,&gdiinput,NULL))
	{
		printf("GdiplusStartup() Failed");return -1;
	}

	if(CreateStreamOnHGlobal(NULL,1,&stream))
	{
		printf("CreateStreamOnHGlobal() Failed");
		return -1;
	}

	window = GetDesktopWindow();

	GetWindowRect(window,&r);

    printf("Y:%d-X:%d\n",r.bottom,r.right);

    hdc = GetDC(window);

    memdc = CreateCompatibleDC(hdc);

    bitmap = CreateCompatibleBitmap(hdc,r.right,r.bottom);

    HBITMAP old = SelectObject(memdc,bitmap);

    BitBlt(memdc,0,0,r.right,r.bottom,hdc,0,0,SRCCOPY);

    if(GdipCreateBitmapFromHBITMAP(bitmap,NULL,&gdibitmap))
    {
        printf("GdipCreateBitmapFromHBITMAP() failed");
        return -1;
    }


    if(-1==GetEncoderClsid(L"image/png",&clsid))
    {
        printf("GetEncoderClsid() Failed");
        return -1;
    }

/*
    if(GdipSaveImageToFile(gdibitmap,L"ScreenShot.png",&clsid,NULL))
    {
        printf("GdipSaveImageToFile() Failed");
        return -1;
    }
*/

    if(GdipSaveImageToStream(gdibitmap,stream,&clsid,NULL))
    {
        printf("GdipSaveImageToStream() Failed");
        return -1;
    }

    GetHGlobalFromStream(stream,&global);
    buf = GlobalLock(global);
	HANDLE file = CreateFileA("ScreenShot2.png", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(file, buf, GlobalSize(global), &len, NULL);
	FlushFileBuffers(file);
	//CloseHandle(file);


    (stream)->lpVtbl->Release(stream);


    SelectObject(memdc, old);
    GdipDisposeImage(gdibitmap);
    GdiplusShutdown(token);
	DeleteObject(bitmap);
	DeleteObject(memdc);
	ReleaseDC(window,hdc);
	CloseWindow(window);

	return 0;

}



