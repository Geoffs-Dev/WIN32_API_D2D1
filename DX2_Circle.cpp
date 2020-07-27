#include <windows.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1")

#include "DX2_Circle.h"


template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}



float g_DPIScaleX = 1.0f;
float g_DPIScaleY = 1.0f;

void InitializeDPIScale(ID2D1Factory* pFactory)
{
    FLOAT dpiX, dpiY;
    /******************************************************
    Suppress the Error code C4996 as described on the next line:
    .........
    Error	C4996	'ID2D1Factory::GetDesktopDpi': Deprecated.
    Use DisplayInformation::LogicalDpi for Windows Store Apps or GetDpiForWindow for desktop apps.API_DX2_WIN32
    /******************************************************/
#pragma warning(suppress : 4996 )
    pFactory->GetDesktopDpi(&dpiX, &dpiY);


    g_DPIScaleX = dpiX / 96.0f;
    g_DPIScaleY = dpiY / 96.0f;
}

template <typename T>
float PixelsToDipsX(T x)
{
    return static_cast<float>(x) / g_DPIScaleX;
}

template <typename T>
float PixelsToDipsY(T y)
{
    return static_cast<float>(y) / g_DPIScaleY;
}


class MainWindow : public BaseWindow<MainWindow>
{
    
    ID2D1Factory* pFactory;
    ID2D1HwndRenderTarget* pRenderTarget;
    ID2D1SolidColorBrush* pBrush;
    D2D1_ELLIPSE            ellipse;
    D2D1_RECT_F             rectangle;
    RECT dw_rect;

    D2D1_TRIANGLE           triangle;
    D2D1_POINT_2F point1;
    D2D1_POINT_2F point2;
    D2D1_POINT_2F point3;
    

    void    CalculateLayout();
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();

public:

    MainWindow() : pFactory(NULL), pRenderTarget(NULL), pBrush(NULL)
    {
    }

    PCWSTR  ClassName() const { return L"Circle Window Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// Recalculate drawing layout when the size of the window changes.

void MainWindow::CalculateLayout()
{
    if (pRenderTarget != NULL)
    {
        //setup the paramaters of the circle.
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        const float x = size.width = 100;
        const float y = size.height = 100;
        const float radius = min(x, y);
        ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);
        
        //setup the paramaters of the rectangle.
        const float rl = dw_rect.left = 200;
        const float rt = dw_rect.right = 150;
        const float rr = dw_rect.right = 500;
        const float rb = dw_rect.bottom = 200;
        rectangle = D2D1::RectF(rl, rt, rr, rb);

        //Setup the parameters for the triangle
        D2D1_POINT_2F point1;
        D2D1_POINT_2F point2;
        D2D1_POINT_2F point3;
        
        triangle.point1;
        triangle.point2 = { 50,100 };
        triangle.point2
    }
}

HRESULT MainWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget);

        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F color = D2D1::ColorF(0.93f, 0.94f, 0.96f,1.0f);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);

            if (SUCCEEDED(hr))
            {
                CalculateLayout();
            }
        }
    }
    return hr;
}

void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void MainWindow::OnPaint()
{
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);

        pRenderTarget->BeginDraw();

        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));
        pRenderTarget->FillEllipse(ellipse, pBrush);//Draw the circle
        pRenderTarget->FillRectangle(rectangle, pBrush); //Draw the rectangle

        //Remember to clear the resources.
        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }
        EndPaint(m_hwnd, &ps);
    }
}

void MainWindow::Resize()
{
    if (pRenderTarget != NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        pRenderTarget->Resize(size);
        CalculateLayout();
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    MainWindow win;

    if (!win.Create(L"Direct2D Shapes", WS_OVERLAPPEDWINDOW))
    {
        return 0;
    }

    ShowWindow(win.Window(), nCmdShow);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        if (FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
        {
            return -1;  // Fail CreateWindowEx.
        }
        return 0;

    case WM_DESTROY:
        DiscardGraphicsResources();
        SafeRelease(&pFactory);
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        OnPaint();
        return 0;



    case WM_SIZE:
        Resize();
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}