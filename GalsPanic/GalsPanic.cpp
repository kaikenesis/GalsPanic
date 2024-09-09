// GalsPanic.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "GalsPanic.h"
#include <stdio.h>

#define MAX_LOADSTRING 100

//==============================================================================================
// GalsPanic

// Gdi
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib,"Gdiplus.lib")

#pragma comment(lib,"msimg32.lib")

using namespace Gdiplus;

ULONG_PTR g_GdiPlusToken;
void Gdi_Init();
void Gdi_Create();
void Gdi_Draw(HDC hdc);
void Gdi_End();


//Gdiplus::Image playerImg((WCHAR*)_T("images/sigong.png"));

// DoubleBuffer
void CreateBitmap();
void DrawBitmap(HWND hWnd, HDC hdc);
void DrawBitmapDoubleBuffering(HWND hWnd, HDC hdc);
void DeleteBitmap();
void UpdatePlayerPos();

HBITMAP hBackImage;
BITMAP bitBack;
HBITMAP hFrontImage;
BITMAP bitFront;
HBITMAP hDoubleBufferImage;
RECT rectView;
Gdiplus::PointF playerPos = { 300,300 };

#define WINDOW_SIZE_X 720
#define WINDOW_SIZE_Y 953
#define IDT_TIMER1 1

//==============================================================================================

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GALSPANIC, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GALSPANIC));

    MSG msg;

    Gdi_Init();
    // Main message loop:
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            UpdatePlayerPos();
        }
    }

    Gdi_End();
    return (int) msg.wParam;
}

//==============================================================================================

void Gdi_Init()
{
    using namespace Gdiplus;

    GdiplusStartupInput gpsi;
    GdiplusStartup(&g_GdiPlusToken, &gpsi, NULL);
}

void Gdi_Create()
{
}

void Gdi_Draw(HDC hdc)
{
    using namespace Gdiplus;

    Graphics graphics(hdc);
    int w, h;
    
    // >> : text
    SolidBrush brush(Color(255, 255, 0, 0));
    FontFamily fontFamily(_T("Times New Roman"));
    Font font(&fontFamily, 24, FontStyleRegular, UnitPixel);
    PointF pointF(10.0f, 20.0f);
    graphics.DrawString(_T("Hello GDI+!"), -1, &font, pointF, &brush);

    // >> : line
    Pen pen(Color(128, 0, 255, 255));
    graphics.DrawLine(&pen, 0, 0, 200, 100);

    Gdiplus::Image* pImg = Image::FromFile(_T("images/sigong.png"));
    if (pImg)
    {
        w = pImg->GetWidth();
        h = pImg->GetHeight();

        Matrix mat;
        static int rot = 0;

        mat.RotateAt((rot % 360), PointF(playerPos.X + (float)(w / 2), playerPos.Y + (float)(h / 2)));
        graphics.SetTransform(&mat);
        graphics.DrawImage(pImg, (int)playerPos.X, (int)playerPos.Y, w, h);
        rot -= 20;

        mat.Reset();
        graphics.SetTransform(&mat);
    }

    if (pImg) delete pImg;
    // player, monster(나중으로)
    //backImg = (Gdiplus::Image)LoadImage(NULL, _T("images/Maxim.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
}

void Gdi_End()
{
    using namespace Gdiplus;

    GdiplusShutdown(g_GdiPlusToken);
}

void CreateBitmap()
{
    // backGround image
    hBackImage = (HBITMAP)LoadImage(NULL, _T("images/Maxim.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (hBackImage == NULL)
    {
        DWORD dwError = GetLastError();
        MessageBox(NULL, _T("backGround Image load error"), _T("Error"), MB_OK);
    }
    else
        GetObject(hBackImage, sizeof(BITMAP), &bitBack);

    // front Image
    // TODO: 경로입력
    hFrontImage = (HBITMAP)LoadImage(NULL, _T("images/FrontMaxim.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (hFrontImage == NULL)
    {
        DWORD dwError = GetLastError();
        MessageBox(NULL, _T("front Image load error"), _T("Error"), MB_OK);
    }
    else
        GetObject(hFrontImage, sizeof(BITMAP), &bitFront);

    // Player
}

void DrawBitmap(HWND hWnd, HDC hdc)
{
    HDC hMemDC;
    HBITMAP hOldBitmap;
    int bx, by;

    // backGround
    {
        hMemDC = CreateCompatibleDC(hdc);
        hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBackImage);
        bx = bitBack.bmWidth;
        by = bitBack.bmHeight;

        BitBlt(hdc, 10, 10, bx, by, hMemDC, 0, 0, SRCCOPY);
        StretchBlt(hdc, 10, 10, bx, by, hMemDC, 0, 0, bx, by, SRCCOPY);
        
        SelectObject(hMemDC, hOldBitmap);
        DeleteDC(hMemDC);
    }

    HDC hFrontMemDC;
    HBITMAP hFrontOldBitmap;
    // front
    {
        hFrontMemDC = CreateCompatibleDC(hdc);
        hFrontOldBitmap = (HBITMAP)SelectObject(hFrontMemDC, hFrontImage);
        
        HBRUSH hBrush = CreateSolidBrush(RGB(100, 100, 100));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hFrontMemDC, hBrush);
        Rectangle(hFrontMemDC, 0, 0, bx, by);

        //SelectObject(hFrontMemDC, oldBrush);
        //DeleteObject(hBrush);

        // -> 땅따먹기 성공한 구역 Polygon그려주기
        hBrush = CreateSolidBrush(RGB(255, 0, 255));
        oldBrush = (HBRUSH)SelectObject(hFrontMemDC, hBrush);
        Rectangle(hFrontMemDC, 10, 10, 500, 500);
        
        SelectObject(hFrontMemDC, oldBrush);
        DeleteObject(hBrush);
        
        TransparentBlt(hdc, 10, 10, bx, by, hFrontMemDC, 0, 0, bx, by, RGB(255, 0, 255));
        SelectObject(hFrontMemDC, hFrontOldBitmap);
        DeleteDC(hFrontMemDC);
    }

    Gdi_Draw(hdc);
}

void DrawBitmapDoubleBuffering(HWND hWnd, HDC hdc)
{
    HDC hDoubleBufferDC;
    HBITMAP hOldDoubleBufferBitmap;

    hDoubleBufferDC = CreateCompatibleDC(hdc);
    if (hDoubleBufferImage == NULL)
    {
        hDoubleBufferImage = CreateCompatibleBitmap(hdc, rectView.right, rectView.bottom);
    }
    hOldDoubleBufferBitmap = (HBITMAP)SelectObject(hDoubleBufferDC, hDoubleBufferImage);

    DrawBitmap(hWnd, hDoubleBufferDC);

    BitBlt(hdc, 0, 0, rectView.right, rectView.bottom, hDoubleBufferDC, 0, 0, SRCCOPY);
    SelectObject(hDoubleBufferDC, hOldDoubleBufferBitmap);
    DeleteDC(hDoubleBufferDC);
}

void DeleteBitmap()
{
    DeleteObject(hBackImage);
    DeleteObject(hFrontImage);
    DeleteObject(hDoubleBufferImage);
}

void UpdatePlayerPos()
{
    DWORD newTime = GetTickCount();
    static DWORD oldTime = newTime;

    if (newTime - oldTime < 100)
        return;
    oldTime = newTime;

    if (GetAsyncKeyState(VK_LEFT) & 0x8000)
    {
        playerPos.X -= 10.0f;
    }
    else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
    {
        playerPos.X += 10.0f;
    }
    else if (GetAsyncKeyState(VK_UP) & 0x8000)
    {
        playerPos.Y -= 10.0f;
    }
    else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
    {
        playerPos.Y += 10.0f;
    }
    else
    {
    }
}


//==============================================================================================
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GALSPANIC));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GALSPANIC);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   /*HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);*/

   RECT rt = { 0, 0, WINDOW_SIZE_X, WINDOW_SIZE_Y };
   AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, 0);
   HWND g_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       50, 50, rt.right - rt.left, rt.bottom - rt.top, nullptr, nullptr, hInstance, nullptr);

   if (!g_hWnd)
   {
      return FALSE;
   }

   ShowWindow(g_hWnd, nCmdShow);
   UpdateWindow(g_hWnd);

   /*if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);*/

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, IDT_TIMER1, 16, NULL);
        GetClientRect(hWnd, &rectView);
        Gdi_Create();
        CreateBitmap();
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_TIMER:
    {
        switch(wParam)
        {
        case IDT_TIMER1:
            InvalidateRgn(hWnd, NULL, FALSE);
            break;
        }
    }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...

            DrawBitmapDoubleBuffering(hWnd, hdc);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        DeleteBitmap();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
