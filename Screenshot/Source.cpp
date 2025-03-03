#include "sockets.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <gdiplus.h>


#include <mfapi.h>
#include <mfobjects.h>
#include <mfidl.h>
#include <mfplay.h>
#include <mferror.h>
#include <mfreadwrite.h>

//All these libraries must be linked. Properties > Linker > Input.
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "mfreadwrite.lib")

//int startSockets();

void PrintMFError(HRESULT hr) {
    if (FAILED(hr)) {
        std::cerr << "Media Foundation Error Code: " << hr << std::endl;
    }
}

int startWebCam() {
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        PrintMFError(hr);
        return -1;
    }


    IMFAttributes* pAttributes = NULL;
    IMFActivate** ppDevices = NULL;
    UINT32 count = 0;

    // Create the attribute store for video capture devices
    hr = MFCreateAttributes(&pAttributes, 1);
    if (FAILED(hr)) {
        PrintMFError(hr);
        return -1;
    }

    // Set the attribute to find video capture devices
    hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr)) {
        PrintMFError(hr);
        pAttributes->Release();
        return -1;
    }

    // Enumerate the video capture devices
    hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
    if (FAILED(hr)) {
        PrintMFError(hr);
        std::cerr << "Failed to enumerate video devices." << std::endl;
        pAttributes->Release();
        return -1;
    }


    // Output the number of video devices found
    std::cout << "Devices found: " << count << std::endl;
    if (count == 0) {
        std::cerr << "No video devices found." << std::endl;
    }
    else {
        for (UINT32 i = 0; i < count; i++) {
            IMFMediaSource* pSource = NULL;
            hr = ppDevices[i]->ActivateObject(IID_PPV_ARGS(&pSource));
            if (SUCCEEDED(hr)) {
                std::cout << "Successfully activated video capture device." << std::endl;
                // Create a media source reader
                IMFSourceReader* pReader = NULL;
                hr = MFCreateSourceReaderFromMediaSource(pSource, pAttributes, &pReader);
                if (FAILED(hr)) {
                    PrintMFError(hr);
                    std::cerr << "Failed to create source reader." << std::endl;
                    pSource->Release();
                    ppDevices[0]->Release();
                    pAttributes->Release();
                    return -1;
                }

                hr = pReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
                hr = pReader->SetStreamSelection(0, TRUE);
                //hr = pReader->SetStreamSelection(1, TRUE);

                if (FAILED(hr)) {
                    PrintMFError(hr);
                    std::cerr << "Failed to select video stream." << std::endl;
                    pReader->Release();
                    pSource->Release();
                    ppDevices[0]->Release();
                    pAttributes->Release();
                    return -1;
                }

                // Start capturing video frames.
                while (true) {
                    IMFSample* pSample = NULL;
                    DWORD dwFlags = 0;
                    LONGLONG llTimestamp = 0;

                    hr = pReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, NULL, &dwFlags, &llTimestamp, &pSample);

                    if (FAILED(hr)) {
                        PrintMFError(hr);
                        std::cerr << "Failed to read sample from the video stream." << std::endl;
                        break;
                    }

                    if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
                        std::cout << "End of stream reached." << std::endl;
                        break;
                    }
                    if (pSample) {
                        std::cout << "Captured frame at timestamp: " << llTimestamp << std::endl;
                        pSample->Release();
                    }
                }
                pReader->Release();
                pSource->Release();
            }
            else {
                std::cerr << "Failed to activate video capture device." << std::endl;
            }
        }
    }

    for (UINT32 i = 0; i < count; i++) {
        ppDevices[i]->Release();
    }
    CoTaskMemFree(ppDevices);
    pAttributes->Release();
    MFShutdown();

    return 0;
}

void TakeScreenshot(const wchar_t* filename) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    int screenX = GetSystemMetrics(SM_CXSCREEN);
    int screenY = GetSystemMetrics(SM_CYSCREEN);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenX, screenY);
    SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, screenX, screenY, hScreenDC, 0, 0, SRCCOPY);

    // Create a GDI+ Bitmap from HBITMAP. This takes ownership of the bitmap.
    Gdiplus::Bitmap bitmap(hBitmap, NULL);

    CLSID clsid;
    // CLSID for PNG Encoder
    HRESULT res = CLSIDFromString(L"{557CF400-1A04-11D3-9A73-0000F81EF32E}", &clsid);
    if (res != S_OK) {
        std::wcerr << L"Failed to get CLSID for PNG encoder.\n";
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return;
    }

    // Save the bitmap to the file
    res = bitmap.Save(filename, &clsid, NULL);
    if (res != Gdiplus::Ok) {
        std::wcerr << L"Failed to save screenshot.\n";
    }
    std::cout << "Cleaning up" << std::endl;
    // Now clean up
    try {
        DeleteDC(hMemoryDC);
    }
    catch (...) {
        std::cout << "Error cleaning DC" << std::endl;
    }
    try {
        ReleaseDC(NULL, hScreenDC);
    }
    catch (...) {
        std::cout << "Error releasing DC" << std::endl;
    }
    //try {
       // Gdiplus::GdiplusShutdown(gdiplusToken);
    //}
    //catch (...) {
    //    std::cout << "Error shuttingdown Gdiplus." << std::endl;
    //}

}

int main() {
    TakeScreenshot(L"screenshot.png");
    std::wcout << L"Screenshot saved as screenshot.png\n";
    int res = startSockets();
    //int res = startWebCam();
    
    return 0;
}

