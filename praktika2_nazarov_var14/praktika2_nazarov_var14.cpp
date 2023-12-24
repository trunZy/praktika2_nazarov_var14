#include <iostream>
#include <windows.h>
#include <commdlg.h>
#include <codecvt>
#include <fstream>
#include <locale>
#include <string>
#include <vector>

using namespace std;

HBITMAP hBitmap = NULL;

#define IDM_OPEN 1001


struct ImageInfo {
    vector<COLORREF>* pixels;
    int width;
    int height;
};

bool OpenKartinkiBMP(HWND hWnd, wchar_t** selectedFile)
{
    OPENFILENAME ofn;
    wchar_t name_kartinki[MAX_PATH] = L"";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = L"BMP Files\0*.bmp\0";
    ofn.lpstrFile = name_kartinki;
    ofn.lpstrTitle = L"Выберите картинку BMP";
    ofn.nMaxFile = sizeof(name_kartinki);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn))
    {
        *selectedFile = _wcsdup(name_kartinki);
        return true;
    }

    return false;
}

ImageInfo* ReadPixelColorsFromBMPkartinok(const wchar_t* filename) {
    ifstream filekartinkii(filename, ios::binary);
    vector<COLORREF>* pixelColors = new vector<COLORREF>();

    if (filekartinkii.is_open()) {
        char* header = new char[54];
        filekartinkii.read(header, 54);

        int width = *reinterpret_cast<int*>(&header[18]);
        int height = *reinterpret_cast<int*>(&header[22]);
        int SizeOfData = width * height * 3;
        char* DATA = new char[SizeOfData];

        filekartinkii.read(DATA, SizeOfData);

        COLORREF ZADcolor = RGB((unsigned char)DATA[2], (unsigned char)DATA[1], (unsigned char)DATA[0]);

        for (int i = 0; i < SizeOfData; i += 3) {
            COLORREF color = RGB((unsigned char)DATA[i + 2], (unsigned char)DATA[i + 1], (unsigned char)DATA[i]);
            pixelColors->push_back(color);
        }

        delete[] DATA;
        delete[] header;

        filekartinkii.close();

        for (COLORREF& color : *pixelColors) {
            if (color == ZADcolor) {
                color = RGB(255, 255, 255);
            }
        }

        ImageInfo* InfOfImage = static_cast<ImageInfo*>(malloc(sizeof(ImageInfo)));
        InfOfImage->pixels = pixelColors;
        InfOfImage->height = height;
        InfOfImage->width = width;

        return InfOfImage;
    }

    return nullptr;
}

void otrisovka(const ImageInfo* imageinfo, HDC hdcwindow, HWND hwnd) {
    int WidthOfImage = imageinfo->width;
    int HeightOfImage = imageinfo->height;
    vector<COLORREF>* pixelColors = imageinfo->pixels;

    RECT windowRECT;
    GetClientRect(hwnd, &windowRECT);
    int WidthOfWindow = windowRECT.right - windowRECT.left;
    int HeightOfWindow = windowRECT.bottom - windowRECT.top;

    HBRUSH WhiteBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(hdcwindow, &windowRECT, WhiteBrush);
    DeleteObject(WhiteBrush);

    int offsetX = 250; 
    int offsetY = 140; 

    for (int y = 0; y < HeightOfImage; ++y) {
        for (int x = 0; x < WidthOfImage; ++x) {
            SetPixel(hdcwindow, WidthOfWindow - WidthOfImage + x + 
                offsetX, HeightOfWindow - HeightOfImage + (HeightOfImage - y - 1) + 
                offsetY, pixelColors->at(WidthOfImage * y + x));
        }
    }
}

LRESULT CALLBACK WndProc(HWND Hwnd, UINT UintMessage, WPARAM Wparam, LPARAM Lparam) {
    static wchar_t* SelectedKartinka = nullptr; 
    static ImageInfo* ImageInfo = nullptr; 

    switch (UintMessage) {
    case WM_COMMAND:
        switch (LOWORD(Wparam)) {
        case IDM_OPEN:
            if (OpenKartinkiBMP(Hwnd, &SelectedKartinka)) {
                ImageInfo = ReadPixelColorsFromBMPkartinok(SelectedKartinka);
                if (ImageInfo) {
                    InvalidateRect(Hwnd, NULL, TRUE);
                }
            }
            break;
        }
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(Hwnd, &ps);

        int WidthOfWindow = LOWORD(Lparam);
        int HeightOfWindow = HIWORD(Lparam);
        PaintRgn(hdc, CreateRectRgn(0, 0, WidthOfWindow, HeightOfWindow));
        if (ImageInfo != nullptr) {
            otrisovka(ImageInfo, hdc, Hwnd);
        }

        EndPaint(Hwnd, &ps);

        return 0;
    } break;

    case WM_SIZE:
        InvalidateRect(Hwnd, NULL, TRUE);
        break;

    case WM_CLOSE:
        DeleteObject(hBitmap);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(Hwnd, UintMessage, Wparam, Lparam);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE Hinstance, HINSTANCE Hpredinstance, LPSTR LpCmdLine, int intCmdShow) {

    const wchar_t name_classa[] = L"name_window_classa";

    WNDCLASS wndClass = { };
    wndClass.lpfnWndProc = WndProc;
    wndClass.hInstance = Hinstance;
    wndClass.lpszClassName = name_classa;

    RegisterClass(&wndClass);

    HWND hwnd = CreateWindowEx(
        0,
        name_classa,
        L"Отображение картинки BMP",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, Hinstance, NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    HMENU HMenu = CreateMenu();
    HMENU HSubMenu = CreatePopupMenu();
    AppendMenu(HSubMenu, MF_STRING, IDM_OPEN, L"Открыть картинку BMP");
    AppendMenu(HMenu, MF_STRING | MF_POPUP, (UINT)HSubMenu, L"Выбрать картинку");

    SetMenu(hwnd, HMenu);

    ShowWindow(hwnd, intCmdShow);

    MSG message;
    while (GetMessage(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return 0;
}