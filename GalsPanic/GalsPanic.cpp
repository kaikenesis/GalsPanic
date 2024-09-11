// GalsPanic.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "GalsPanic.h"
#include <stdio.h>
#include <stack>
#include <vector>
#include <algorithm>
#include <random>

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
void Gdi_Draw(HDC hdc);
void Gdi_End();

Gdiplus::Image* pImg;

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
Gdiplus::PointF playerPos = { 10, 10 };

#define WINDOW_SIZE_X 720
#define WINDOW_SIZE_Y 953
#define IDT_TIMER1 1

int drawOffset = 10;

// System
BOOL IsSafe();
BOOL IsFrame();
BOOL IsInPolygon();
void DrawLine(Graphics& graphics);
void DrawRectangle(HDC hdc);
void UpdateMovePoint(float speed);
void UpdatePolygonPoint();
void InitStartPoint();
void InitPolygon();
void MoveToX(float speed);
void MoveToY(float speed);
void SortArr(std::vector<int>& vec);

enum EMoveDir
{
    None,
    X,
    Y
};

int offset = 10;
float playerSpeed = 10.0f;
std::vector<POINT> movePoints;
std::vector<POINT> bufferPoints;
POINT* drawPoints;
EMoveDir eDir = None;

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
    
    return (int) msg.wParam;
}

//==============================================================================================

void Gdi_Init()
{
    using namespace Gdiplus;

    GdiplusStartupInput gpsi;
    GdiplusStartup(&g_GdiPlusToken, &gpsi, NULL);

    pImg = Image::FromFile(_T("images/sigong.png"));
}

void Gdi_Draw(HDC hdc)
{
    using namespace Gdiplus;

    Graphics graphics(hdc);
    int w, h;

    

    // >> : line
    DrawLine(graphics);
    
    // >> : Image
    ImageAttributes imgAttr;
    imgAttr.SetColorKey(Color(245, 0, 245), Color(255, 10, 255));

    if (pImg)
    {
        w = pImg->GetWidth() / 1.5;
        h = pImg->GetHeight() / 1.5;

        Matrix mat;
        static int rot = 0;

        mat.RotateAt((rot % 360), PointF(playerPos.X, playerPos.Y));
        graphics.SetTransform(&mat);

        if(IsSafe())
        {
            REAL transparency = 1.0f;
            Gdiplus::ColorMatrix colorMatrix =
            {
                10.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, transparency, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            };
            imgAttr.SetColorMatrix(&colorMatrix);
        }
        else
        {
            REAL transparency = 1.0f;
            Gdiplus::ColorMatrix colorMatrix =
            {
                1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 10.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, transparency, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            };
            imgAttr.SetColorMatrix(&colorMatrix);
        }

        graphics.DrawImage(pImg, Rect((int)playerPos.X - w / 2, (int)playerPos.Y - h / 2, w, h), 0, 0, w * 1.5, h * 1.5, UnitPixel, &imgAttr);
        rot -= 20;

        mat.Reset();
        graphics.SetTransform(&mat);
    }

    // monster(��������)
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
    // TODO: ����Է�
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

        // -> �����Ա� ������ ���� Rectangle�׷��ֱ�
        if(bufferPoints.empty() == false)
        {
            HPEN myPen = CreatePen(PS_NULL, 0, RGB(255, 0, 255));
            HGDIOBJ oldPen = SelectObject(hFrontMemDC, myPen);
            
            hBrush = CreateSolidBrush(RGB(255, 0, 255));
            oldBrush = (HBRUSH)SelectObject(hFrontMemDC, hBrush);
            
            DrawRectangle(hFrontMemDC);

            SelectObject(hFrontMemDC, oldPen);
            DeleteObject(myPen);

            SelectObject(hFrontMemDC, oldBrush);
            DeleteObject(hBrush);
            
        }
        
        
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

    if (newTime - oldTime < 20)
        return;
    oldTime = newTime;

    if (GetAsyncKeyState(VK_LEFT) & 0x8000)
    {
        if (playerPos.X - playerSpeed < rectView.left + offset)
            playerPos.X = rectView.left + offset;
        else
            playerPos.X -= playerSpeed;

        UpdateMovePoint(-playerSpeed);
    }
    else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
    {
        if (playerPos.X + playerSpeed > rectView.right - offset)
            playerPos.X = rectView.right - offset;
        else
            playerPos.X += playerSpeed;

        UpdateMovePoint(playerSpeed);
    }
    if (GetAsyncKeyState(VK_UP) & 0x8000)
    {
        if (playerPos.Y - playerSpeed < rectView.top + offset)
            playerPos.Y = rectView.top + offset;
        else
            playerPos.Y -= playerSpeed;

        UpdateMovePoint(-playerSpeed);
    }
    else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
    {
        if (playerPos.Y + playerSpeed > rectView.bottom - offset)
            playerPos.Y = rectView.bottom - offset;
        else
            playerPos.Y += playerSpeed;

        UpdateMovePoint(playerSpeed);
    }

}

BOOL IsSafe()
{
    //if (IsFrame() == false) return false;
    if (IsInPolygon() == false) return false;

    return true;
}

BOOL IsFrame()
{
    if ((playerPos.Y != rectView.top + offset && playerPos.Y != rectView.bottom - offset)
        && (playerPos.X != rectView.left + offset && playerPos.X != rectView.right - offset)) return false;

    return true;
}

BOOL IsInPolygon()
{
    int maxX = bufferPoints[0].x;
    int minX = bufferPoints[0].x;

    for (int i = 0; i < bufferPoints.size(); i++)
    {
        if (bufferPoints[i].x > maxX) maxX = bufferPoints[i].x;
        else if (bufferPoints[i].x < minX) minX = bufferPoints[i].x;
    }

    std::vector<int> yPoints;
    for (int i = minX; i < maxX; i += playerSpeed) // minX -> maxX �� ���پ� �׸��鼭 
    {
        for (int j = 0; j < bufferPoints.size(); j++)
        {
            if (bufferPoints[j].x == i)
                yPoints.push_back(bufferPoints[j].y);
        }

        SortArr(yPoints);
        for (int j = 0; j < yPoints.size(); j += 2)
        {
            if (playerPos.X >= i && playerPos.X <= i + playerSpeed && playerPos.Y >= yPoints[j] && playerPos.Y <= yPoints[j + 1])
                return true;
        }
    }

    return false;
}

void DrawLine(Graphics& graphics)
{
    Pen pen(Color(255, 255, 0, 0));

    if (movePoints.size() >= 2)
    {
        for (int i = 0; i < movePoints.size() - 1; i++)
        {
            graphics.DrawLine(&pen, (INT)movePoints[i].x, (INT)movePoints[i].y, (INT)movePoints[i + 1].x, (INT)movePoints[i + 1].y);
        }
    }
}

void DrawRectangle(HDC hdc)
{
    if (bufferPoints.size() > _msize(drawPoints) / sizeof(drawPoints))
    {
        POINT* newPoints = new POINT[_msize(drawPoints) / sizeof(drawPoints) + 100]();
        delete[](drawPoints);
        drawPoints = newPoints;
        newPoints = NULL;
    }

    for (int i = 0; i < bufferPoints.size(); i++)
    {
        drawPoints[i] = bufferPoints[i];
    }

    Polygon(hdc, drawPoints, bufferPoints.size());
    
    //int maxX = bufferPoints[0].x;
    //int minX = bufferPoints[0].x;

    //for (int i = 0; i < bufferPoints.size(); i++)
    //{
    //    if (bufferPoints[i].x > maxX) maxX = bufferPoints[i].x;
    //    else if (bufferPoints[i].x < minX) minX = bufferPoints[i].x;
    //}
    //
    //std::vector<int> yPoints;
    //for (int i = minX; i < maxX; i+= playerSpeed) // minX -> maxX �� ���پ� �׸��鼭 
    //{
    //    for (int j = 0; j < bufferPoints.size(); j++)
    //    {
    //        if (bufferPoints[j].x == i)
    //            yPoints.push_back(bufferPoints[j].y);
    //    }

    //    SortArr(yPoints);
    //    for (int j = 0; j < yPoints.size(); j += 2)
    //    {
    //        Rectangle(hdc, i, yPoints[j], i + playerSpeed + 1, yPoints[j + 1]);
    //    }
    //}
}

void UpdateMovePoint(float speed)
{
    if (IsSafe() == false)
    {
        MoveToX(speed);
        MoveToY(speed);
    }
    else
    {
        if (movePoints.size() > 2)
        {
            POINT point = { playerPos.X, playerPos.Y };
            movePoints[movePoints.size() - 1] = point;
            UpdatePolygonPoint();
            // ������ point�� ������Ʈ�ϰ�, �ϼ��� point�� �����ؼ� polygon�� ���� �迭�� ����
        }
        else
        {
            POINT point = { playerPos.X, playerPos.Y };
            movePoints[0] = point;
        }
    }
    // ���������� ����� ������� ����, x->y, y->x ������ ���� ������ ���� ��ġ�� stack.top�� ����, ����Ǹ� �ش����� stack�� ����
    
}

void UpdatePolygonPoint()
{
    for (int i = 0; i < movePoints.size(); i++)
    {
        bufferPoints.push_back(movePoints[i]);
    }
    
    while (movePoints.empty() == false)
    {
        movePoints.pop_back();
    }

    if (movePoints.empty())
    {
        InitStartPoint();
    }

    for (int i = 0; i < bufferPoints.size(); i++)
    {
        bufferPoints[i].x -= drawOffset;
        bufferPoints[i].y -= drawOffset;
    }
}

void InitStartPoint()
{
    InitPolygon();

    if (movePoints.empty() == true)
    {
        POINT point = { bufferPoints[0].x + drawOffset, bufferPoints[0].y + drawOffset };
        movePoints.push_back(point);
        playerPos.X = point.x;
        playerPos.Y = point.y;
    }
}

void InitPolygon()
{
    // �������� �簢���� polygon���� �ű⼭���� �÷��̾ ������ �� �ֵ��� ����
    drawPoints = new POINT[1]();

    std::random_device rd;
    std::mt19937 gen(rd());
    
    std::uniform_int_distribution<int> x(rectView.left, rectView.right - 300);
    std::uniform_int_distribution<int> width(5, 20);

    std::uniform_int_distribution<int> y(rectView.top, rectView.bottom - 300);
    std::uniform_int_distribution<int> height(5, 20);

    int xx = (x(gen) / 10) * 10;
    int w = width(gen) * 10;
    int yy = (y(gen) / 10) * 10;
    int h = height(gen) * 10;

    POINT points[4] = { {xx,yy}, {xx + w,yy}, {xx + w,yy + h}, {xx,yy + h} };

    for (int i = 0; i < 4; i++)
    {
        bufferPoints.push_back(points[i]);
        drawPoints[i] = points[i];
    }
}

void MoveToX(float speed)
{
    if (movePoints[movePoints.size() - 1].x != playerPos.X)
    {
        if (eDir == Y || eDir == None)
        {
            if (movePoints[movePoints.size() - 1].y != playerPos.Y || movePoints.size() == 1)
            {
                POINT point = { playerPos.X - speed,playerPos.Y };
                movePoints.push_back(point);
            }
            else
            {
                if (movePoints[movePoints.size() - 2].y == playerPos.Y)
                    movePoints.pop_back();
                else
                {
                    POINT point = { playerPos.X - speed,playerPos.Y };
                    movePoints.push_back(point);
                }
            }
            eDir = X;
        }

        POINT point = { playerPos.X ,movePoints[movePoints.size() - 1].y };
        movePoints[movePoints.size() - 1] = point;
    }
}

void MoveToY(float speed)
{
    if (movePoints[movePoints.size() - 1].y != playerPos.Y)
    {
        if (eDir == X || eDir == None)
        {
            if (movePoints[movePoints.size() - 1].x != playerPos.X || movePoints.size() == 1)
            {
                POINT point = { playerPos.X ,playerPos.Y - speed };
                movePoints.push_back(point);
            }
            else
            {
                if (movePoints[movePoints.size() - 2].x == playerPos.X)
                    movePoints.pop_back();
                else
                {
                    POINT point = { playerPos.X ,playerPos.Y - speed };
                    movePoints.push_back(point);
                }
            }
            eDir = Y;
        }

        POINT point = { movePoints[movePoints.size() - 1].x, playerPos.Y };
        movePoints[movePoints.size() - 1] = point;
    }
}

void SortArr(std::vector<int>& vec)
{
    for (int i = 0; i < vec.size() - 1; i++)
    {
        for (int j = i + 1; j < vec.size(); j++)
        {
            if (vec[j] < vec[i])
            {
                int temp = vec[i];
                vec[i] = vec[j];
                vec[j] = temp;
            }
        }
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

   // Create & Init
   CreateBitmap();
   Gdi_Init();

   RECT rt = { 0, 0, bitBack.bmWidth + 20, bitBack.bmHeight + 40 };
   AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, 0);
   HWND g_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       50, 50, rt.right - rt.left, rt.bottom - rt.top, nullptr, nullptr, hInstance, nullptr);

   if (!g_hWnd)
   {
      return FALSE;
   }

   ShowWindow(g_hWnd, nCmdShow);
   UpdateWindow(g_hWnd);

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
        InitStartPoint();
        
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
    case WM_KEYDOWN:

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
        Gdi_End();
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
