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

HBITMAP hBackgroundImage;
BITMAP bitBackground;
HBITMAP hBackImage;
BITMAP bitBack;
HBITMAP hFrontImage;
BITMAP bitFront;
HBITMAP hDoubleBufferImage;
RECT rectView;
Gdiplus::PointF playerPos = { 0, 0 };

#define WINDOW_SIZE_X 720
#define WINDOW_SIZE_Y 953
#define IDT_TIMER1 1

int drawOffset = 10;

// System
BOOL IsSafe();
BOOL IsInFrame(std::vector<POINT> points, int inX, int inY);
BOOL IsInPolygon(std::vector<POINT> points, int inX, int inY);
void DrawLine(Graphics& graphics);
void DrawRectangle(HDC hdc);
void UpdateMovePoint(float speed);
void UpdatePolygonPoint();
void InitStartPoint(int inX, int inY);
void InitPolygon();
void MoveToX(float speed);
void MoveToY(float speed);

enum EMoveDir
{
    None,
    X,
    Y
};

int offset = 10;
int arrSize = 100;
float playerSpeed = 10.0f;
std::vector<POINT> movePoints;
std::vector<POINT> polygonPoints;
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

        mat.RotateAt((rot % 360), PointF(playerPos.X+ offset, playerPos.Y+ offset));
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

        graphics.DrawImage(pImg, Rect((int)playerPos.X + offset - w / 2, (int)playerPos.Y + offset - h / 2, w, h),
            0, 0, w * 1.5, h * 1.5, UnitPixel, &imgAttr);
        rot -= 20;

        mat.Reset();
        graphics.SetTransform(&mat);
    }

    // monster(나중으로)
}

void Gdi_End()
{
    using namespace Gdiplus;

    GdiplusShutdown(g_GdiPlusToken);
}

void CreateBitmap()
{
    // background image
    hBackgroundImage = (HBITMAP)LoadImage(NULL, _T("images/Maxim.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (hBackgroundImage == NULL)
    {
        DWORD dwError = GetLastError();
        MessageBox(NULL, _T("backGround Image load error"), _T("Error"), MB_OK);
    }
    else
        GetObject(hBackgroundImage, sizeof(BITMAP), &bitBackground);

    // back image
    hBackImage = (HBITMAP)LoadImage(NULL, _T("images/Maxim.bmp"),IMAGE_BITMAP,0,0,LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (hBackImage == NULL)
    {
        DWORD dwError = GetLastError();
        MessageBox(NULL, _T("backGround Image load error"), _T("Error"), MB_OK);
    }
    else
        GetObject(hBackImage, sizeof(BITMAP), &bitBack);
    
    // front Image
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

    // background
    {
        hMemDC = CreateCompatibleDC(hdc);
        hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBackgroundImage);
        bx = bitBackground.bmWidth;
        by = bitBackground.bmHeight; 

        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hMemDC, hBrush);
        Rectangle(hMemDC, 0, 0, bx, by);

        SelectObject(hMemDC, oldBrush);
        DeleteObject(hBrush);

        BitBlt(hdc, 0, 0, bx, by, hMemDC, 0, 0, SRCCOPY);
        StretchBlt(hdc, 0, 0, bx+offset*2, by+offset * 2, hMemDC, 0, 0, bx, by, SRCCOPY);

        SelectObject(hMemDC, hOldBitmap);
        DeleteDC(hMemDC);
    }

    // back
    {
        hMemDC = CreateCompatibleDC(hdc);
        hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBackImage);
        bx = bitBack.bmWidth;
        by = bitBack.bmHeight;

        BitBlt(hdc, offset, offset, bx, by, hMemDC, 0, 0, SRCCOPY);
        StretchBlt(hdc, offset, offset, bx, by, hMemDC, 0, 0, bx, by, SRCCOPY);
        
        SelectObject(hMemDC, hOldBitmap);
        DeleteDC(hMemDC);
    }

    HDC hFrontMemDC;
    HBITMAP hFrontOldBitmap;
    // front
    {
        hFrontMemDC = CreateCompatibleDC(hdc);
        hFrontOldBitmap = (HBITMAP)SelectObject(hFrontMemDC, hFrontImage);
        bx = bitFront.bmWidth;
        by = bitFront.bmHeight;

        HBRUSH hBrush = CreateSolidBrush(RGB(100, 100, 100));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hFrontMemDC, hBrush);
        Rectangle(hFrontMemDC, 0, 0, bx, by);

        // -> 땅따먹기 성공한 구역 Rectangle그려주기
        if(polygonPoints.empty() == false)
        {
            hBrush = CreateSolidBrush(RGB(255, 0, 255));
            oldBrush = (HBRUSH)SelectObject(hFrontMemDC, hBrush);
            
            DrawRectangle(hFrontMemDC);

            SelectObject(hFrontMemDC, oldBrush);
            DeleteObject(hBrush);
            
        }
        
        TransparentBlt(hdc, offset, offset, bx, by, hFrontMemDC, 0, 0, bx, by, RGB(255, 0, 255));
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
        else if(IsInFrame(polygonPoints, playerPos.X - playerSpeed, playerPos.Y) == true
            || IsInPolygon(polygonPoints, playerPos.X - playerSpeed, playerPos.Y) == false)
            playerPos.X -= playerSpeed;

        UpdateMovePoint(-playerSpeed);
    }
    else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
    {
        if (playerPos.X + playerSpeed > rectView.right - offset)
            playerPos.X = rectView.right - offset;
        else if (IsInFrame(polygonPoints, playerPos.X + playerSpeed, playerPos.Y) == true
            || IsInPolygon(polygonPoints, playerPos.X + playerSpeed, playerPos.Y) == false)
            playerPos.X += playerSpeed;

        UpdateMovePoint(playerSpeed);
    }
    if (GetAsyncKeyState(VK_UP) & 0x8000)
    {
        if (playerPos.Y - playerSpeed < rectView.top + offset)
            playerPos.Y = rectView.top + offset;
        else if (IsInFrame(polygonPoints, playerPos.X, playerPos.Y - playerSpeed) == true
            || IsInPolygon(polygonPoints, playerPos.X, playerPos.Y - playerSpeed) == false)
            playerPos.Y -= playerSpeed;

        UpdateMovePoint(-playerSpeed);
    }
    else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
    {
        if (playerPos.Y + playerSpeed > rectView.bottom - offset)
            playerPos.Y = rectView.bottom - offset;
        else if (IsInFrame(polygonPoints, playerPos.X, playerPos.Y + playerSpeed) == true
            || IsInPolygon(polygonPoints, playerPos.X, playerPos.Y + playerSpeed) == false)
            playerPos.Y += playerSpeed;

        UpdateMovePoint(playerSpeed);
    }

}

BOOL IsSafe()
{
    if (IsInFrame(polygonPoints, playerPos.X, playerPos.Y) == false) return false;

    return true;
}

BOOL IsInFrame(std::vector<POINT> points, int inX, int inY)
{
    int maxY = points[0].y;
    int minY = points[0].y;
    int n = points.size();
    for (int i = 0; i < n; i++)
    {
        if (points[i].y > maxY) maxY = points[i].y;
        else if (points[i].y < minY) minY = points[i].y;
    }

    if (inY > maxY || inY < minY) return false;

    int crossCount = 0;
    for (int i = 0; i < n; i++)
    {
        POINT p1 = points[i];
        POINT p2 = points[(i + 1) % n];

        if ((inX == p1.x && inX == p2.x) && ((inY <= p1.y && inY >= p2.y) || (inY <= p2.y && inY >= p1.y)))
            return true;
        else if ((inY == p1.y && inY == p2.y) && ((inX <= p1.x && inX >= p2.x) || (inX <= p2.x && inX >= p1.x)))
            return true;
    }

    return false;
}

BOOL IsInPolygon(std::vector<POINT> points, int inX, int inY)
{
    int maxY = points[0].y;
    int minY = points[0].y;
    int n = points.size();
    for (int i = 0; i < n; i++)
    {
        if (points[i].y > maxY) maxY = points[i].y;
        else if (points[i].y < minY) minY = points[i].y;
    }

    if (inY > maxY || inY < minY) return false;

    int crossCount = 0; 
    for (int i = 0; i < n; i++)
    {
        POINT p1 = points[i];
        POINT p2 = points[(i + 1) % n];

        if ((inY > min(p1.y, p2.y)) && (inY <= max(p1.y, p2.y)) && (inX <= max(p1.x, p2.x)))
        {
            double xIntersect = (inY - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
            if (p1.x == p2.x || inX <= xIntersect)
            {
                crossCount++;
            }
        }
    }
    
    return crossCount % 2 == 1;
}

void DrawLine(Graphics& graphics)
{
    Pen pen(Color(255, 255, 0, 0));

    if (movePoints.size() >= 2)
    {
        for (int i = 0; i < movePoints.size() - 1; i++)
        {
            graphics.DrawLine(&pen, (INT)movePoints[i].x + offset, (INT)movePoints[i].y + offset,
                (INT)movePoints[i + 1].x + offset, (INT)movePoints[i + 1].y + offset);
        }
    }
}

void DrawRectangle(HDC hdc)
{
    if (polygonPoints.size() > arrSize)
    {
        arrSize += 100;
        POINT* newPoints = new POINT[arrSize]();
        delete[](drawPoints);
        drawPoints = newPoints;
    }

    for (int i = 0; i < polygonPoints.size(); i++)
    {
        drawPoints[i] = polygonPoints[i];
    }

    Polygon(hdc, drawPoints, polygonPoints.size());
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
            // 마지막 point를 업데이트하고, 완성된 point를 저장해서 polygon을 만들 배열에 저장
        }
        else
        {
            POINT point = { playerPos.X, playerPos.Y };
            movePoints[0] = point;
            if (movePoints.size() == 2)
            {
                movePoints[1] = point;
            }
        }
    }
    // 안전구역을 벗어나면 출발지점 저장, x->y, y->x 변경이 없는 동안은 현재 위치를 stack.top에 갱신, 변경되면 해당지점 stack에 저장
    
}

void UpdatePolygonPoint()
{
    std::vector<POINT> frontPoints;
    std::vector<POINT> backPoints;
    std::vector<POINT> buffer;

    POINT startPos = movePoints[0];
    POINT endPos = movePoints[movePoints.size() - 1];

    /*
        new폴리곤 점 개수가 5이상일 경우
        기존 폴리곤에서 시작점부터 체크해서 새로 그린 폴리곤의 시작 또는 끝점에 만날 경우
        앞에 체크된 기존 폴리곤의 점들과 새로 그린 폴리곤의 점들로 새로 그려지는 영역을 그리고
        해당 영역에 체크가 안된 기존 폴리곤의 점들이 포함 된다면 체크된 점들을 저장,
        포함되지 않는다면 체크가 안된 점들을 저장해서 새로운 폴리곤의 점들과 함께 그리기

        new폴리곤 점 개수가 4이하일 경우
        처음만난 점에서 다른 점까지의 사이에 기존 폴리곤 점들은 제외
        대신 만난 점이 시작점인지 끝점인지 판별하고 new폴리곤 점을 넣어줄때 방향에 맞춰서 조정해야함
    */ 
    bool bStart = false;
    bool bEnd = false;
    bool bReverse = false;

    if (movePoints.size() <= 4) // new폴리곤 점 개수 4이하
    {
        for (int i = 0; i < polygonPoints.size(); i++)
        {
            POINT p1 = polygonPoints[i % polygonPoints.size()];
            POINT p2 = polygonPoints[(i + 1) % polygonPoints.size()];

            // 세로 모서리
            if (p1.x == p2.x)
            {
                if (startPos.x == p1.x && endPos.x == p1.x)
                {
                    if (abs(startPos.y - p1.y) > abs(endPos.y - p1.y))
                        bReverse = true;
                    bStart = true;
                    bEnd = true;
                }
                else if (startPos.x == p1.x)
                {
                    if (bStart == false)
                        bReverse = true;
                    bEnd = true;
                }
                else if (endPos.x == p1.x)
                {
                    bStart = true;
                }
            }
            // 가로 모서리
            else if (p1.y == p2.y)
            {
                if (startPos.y == p1.y && endPos.y == p1.y)
                {
                    if (abs(startPos.x - p1.x) > abs(endPos.x - p1.x))
                        bReverse = true;
                    bStart = true;
                    bEnd = true;
                }
                else if (startPos.y == p1.y)
                {
                    bStart = true;
                }
                else if (endPos.y == p1.y)
                {
                    if (bStart == false)
                        bReverse = true;
                    bEnd = true;
                }
            }

            if ((p1.x == startPos.x && p1.y == startPos.y) || (p1.x == endPos.x && p1.y == endPos.y)) continue;

            if (bStart == false && bEnd == false)
                frontPoints.push_back(polygonPoints[i]);
            else if(bStart == true && bEnd == true)
                backPoints.push_back(polygonPoints[i]);
        }

        for (int i = 0; i < frontPoints.size(); i++)
        {
            POINT point = { frontPoints[i].x, frontPoints[i].y };
            buffer.push_back(point);
        }
        for (int i = 0; i < movePoints.size(); i++)
        {
            POINT point;
            if (bReverse == false)
                point = { movePoints[i].x, movePoints[i].y };
            else
                point = { movePoints[(movePoints.size() - 1) - i].x, movePoints[(movePoints.size() - 1) - i].y };
            buffer.push_back(point);
        }
        for (int i = 0; i < backPoints.size(); i++)
        {
            POINT point = { backPoints[i].x, backPoints[i].y };
            buffer.push_back(point);
        }
    }
    else // new폴리곤 점 개수 5이상
    {
        std::vector<POINT> newPolygon;
        std::vector<POINT> leastPoints;

        for (int i = 0; i < polygonPoints.size(); i++)
        {
            POINT p1 = polygonPoints[i % polygonPoints.size()];
            POINT p2 = polygonPoints[(i + 1) % polygonPoints.size()];

            // 세로 모서리
            if (p1.x == p2.x)
            {
                if (startPos.x == p1.x && endPos.x == p1.x)
                {
                    if (abs(startPos.y - p1.y) > abs(endPos.y - p1.y))
                        bReverse = true;
                    bStart = true;
                    bEnd = true;
                }
                else
                {
                    if (startPos.x == p1.x)
                    {
                        if (bStart == false)
                            bReverse = true;
                        bEnd = true;
                    }
                    else if (endPos.x == p1.x)
                    {
                        bStart = true;
                    }

                    if ((bStart == true && bEnd == false) || (bStart == false && bEnd == true))
                    {
                        for (int i = 0; i < frontPoints.size(); i++)
                        {
                            POINT point = { frontPoints[i].x, frontPoints[i].y };
                            newPolygon.push_back(point);
                        }
                        for (int i = 0; i < movePoints.size(); i++)
                        {
                            POINT point;
                            if (bReverse == false)
                                point = { movePoints[i].x, movePoints[i].y };
                            else
                                point = { movePoints[(movePoints.size() - 1) - i].x, movePoints[(movePoints.size() - 1) - i].y };
                            newPolygon.push_back(point);
                        }

                        // new폴리곤이 그려지는 방향 판별 :
                        // true -> i+1부터 new폴리곤의 끝점에 도달할때까지의 점들을 제외
                        // false -> new폴리곤의 시작점이 폴리곤을 그리는 시작점으로 변경되고 i+1 이전의 점들과,
                        // new폴리곤의 끝점 이후의 점들을 제외
                        if (IsInPolygon(newPolygon, p2.x, p2.y) || IsInFrame(newPolygon, p2.x, p2.y))
                        {
                            for (int i = 0; i < frontPoints.size(); i++)
                            {
                                POINT point = { frontPoints[i].x, frontPoints[i].y };
                                buffer.push_back(point);
                            }
                            for (int i = 0; i < movePoints.size(); i++)
                            {
                                POINT point;
                                if (bReverse == false)
                                    point = { movePoints[i].x, movePoints[i].y };
                                else
                                    point = { movePoints[(movePoints.size() - 1) - i].x, movePoints[(movePoints.size() - 1) - i].y };
                                buffer.push_back(point);
                            }
                            break;
                        }
                        else
                        {
                            for (int i = 0; i < movePoints.size(); i++)
                            {
                                POINT point;
                                if (bReverse == false)
                                    point = { movePoints[i].x, movePoints[i].y };
                                else
                                    point = { movePoints[(movePoints.size() - 1) - i].x, movePoints[(movePoints.size() - 1) - i].y };
                                buffer.push_back(point);
                            }
                            for (int j = i + 1; j < polygonPoints.size(); j++)
                                buffer.push_back(polygonPoints[j]);
                            break;
                        }
                    }
                }
                
            }
            // 가로 모서리
            else if (p1.y == p2.y)
            {
                if (startPos.y == p1.y && endPos.y == p1.y)
                {
                    if (abs(startPos.x - p1.x) > abs(endPos.x - p1.x))
                        bReverse = true;
                    bStart = true;
                    bEnd = true;
                }
                else
                {
                    if (startPos.y == p1.y)
                    {
                        bStart = true;
                    }
                    else if (endPos.y == p1.y)
                    {
                        if (bStart == false)
                            bReverse = true;
                        bEnd = true;
                    }

                    if ((bStart == true && bEnd == false) || (bStart == false && bEnd == true))
                    {
                        for (int i = 0; i < frontPoints.size(); i++)
                        {
                            POINT point = { frontPoints[i].x, frontPoints[i].y };
                            newPolygon.push_back(point);
                        }
                        for (int i = 0; i < movePoints.size(); i++)
                        {
                            POINT point;
                            if (bReverse == false)
                                point = { movePoints[i].x, movePoints[i].y };
                            else
                                point = { movePoints[(movePoints.size() - 1) - i].x, movePoints[(movePoints.size() - 1) - i].y };
                            newPolygon.push_back(point);
                        }

                        if (IsInPolygon(newPolygon, p2.x, p2.y) || IsInFrame(newPolygon, p2.x, p2.y))
                        {
                            
                        }
                        else
                        {
                            
                            break;
                        }
                    }
                }
            }
            if ((p1.x == startPos.x && p1.y == startPos.y) || (p1.x == endPos.x && p1.y == endPos.y)) continue;

            if (bStart == false && bEnd == false)
                frontPoints.push_back(polygonPoints[i]);
            else if (bStart == true && bEnd == true)
                backPoints.push_back(polygonPoints[i]);
        }

        for (int i = 0; i < frontPoints.size(); i++)
        {
            POINT point = { frontPoints[i].x, frontPoints[i].y };
            buffer.push_back(point);
        }
        for (int i = 0; i < movePoints.size(); i++)
        {
            POINT point;
            if (bReverse == false)
                point = { movePoints[i].x, movePoints[i].y };
            else
                point = { movePoints[(movePoints.size() - 1) - i].x, movePoints[(movePoints.size() - 1) - i].y };
            buffer.push_back(point);
        }
        for (int i = 0; i < backPoints.size(); i++)
        {
            POINT point = { backPoints[i].x, backPoints[i].y };
            buffer.push_back(point);
        }
    }

    polygonPoints.clear();
    movePoints.clear();
    
    for (int i = 0; i < buffer.size(); i++)
    {
        int x1 = buffer[(i - 1 + buffer.size()) % buffer.size()].x;
        int y1 = buffer[(i - 1 + buffer.size()) % buffer.size()].y;
        int x2 = buffer[(i + 1) % buffer.size()].x;
        int y2 = buffer[(i + 1) % buffer.size()].y;

        if ((buffer[i].x != x1 || buffer[i].x != x2) && (buffer[i].y != y1 || buffer[i].y != y2))
        {
            polygonPoints.push_back(buffer[i]);
        }
    }

    if (movePoints.empty())
    {
        InitStartPoint(playerPos.X, playerPos.Y);
    }
    eDir = None;
}

void InitStartPoint(int inX, int inY)
{
    if (movePoints.empty() == true)
    {
        POINT point = { inX, inY };
        movePoints.push_back(point);
        playerPos.X = point.x;
        playerPos.Y = point.y;
    }
}

void InitPolygon()
{
    // 랜덤으로 사각형의 polygon생성 거기서부터 플레이어가 시작할 수 있도록 세팅
    drawPoints = new POINT[arrSize]();

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
        polygonPoints.push_back(points[i]);
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

   RECT rt = { 0, 0, bitBack.bmWidth + offset * 2, bitBack.bmHeight + offset * 4 };
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
        SetTimer(hWnd, IDT_TIMER1, 33, NULL);
        GetClientRect(hWnd, &rectView);
        InitPolygon();
        InitStartPoint(polygonPoints[0].x, polygonPoints[0].y);

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

            TCHAR str[128];
            wsprintf(str, _T("Position : (%04d,%04d)"), (int)playerPos.X, (int)playerPos.Y);
            TextOut(hdc, 20, 20, str, lstrlen(str));

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
