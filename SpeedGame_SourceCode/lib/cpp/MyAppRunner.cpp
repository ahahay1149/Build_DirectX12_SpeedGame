﻿#include <windowsx.h>
#include "MyAppRunner.h"

HWND MyAppRunner::m_hWnd = nullptr; //staticメンバの初期化がないとリンクエラー
bool MyAppRunner::m_fullscreenMode = false;
RECT MyAppRunner::m_windowRect;

int MyAppRunner::Run(MyGameEngine* pMyEngine, HINSTANCE hInstance, int nCmdShow)
{
    try
    {

        // Initialize the window class.
        WNDCLASSEX windowClass = { 0 };
        windowClass.cbSize = sizeof(WNDCLASSEX);
        windowClass.style = CS_HREDRAW | CS_VREDRAW;
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = hInstance;
        windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        windowClass.lpszClassName = L"MyGameEngineSample";
        RegisterClassEx(&windowClass);

        m_windowRect = { 0, 0, static_cast<LONG>(pMyEngine->GetWidth()), static_cast<LONG>(pMyEngine->GetHeight()) };
        AdjustWindowRect(&m_windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        // Create the window and store a handle to it.
        m_hWnd = CreateWindow(
            windowClass.lpszClassName,
            pMyEngine->GetTitle(),
            m_windowStyle,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            m_windowRect.right - m_windowRect.left,
            m_windowRect.bottom - m_windowRect.top,
            nullptr,        // We have no parent window.
            nullptr,        // We aren't using menus.
            hInstance,
            pMyEngine);

        // Initialize the sample. OnInit is defined in each child-implementation of DXSample.
        if (FAILED(pMyEngine->InitMyGameEngine(hInstance, m_hWnd))) return 0;

        ShowWindow(m_hWnd, nCmdShow);

        // Main sample loop.
        MSG msg = {};
        while (msg.message != WM_QUIT)
        {
            // Process any messages in the queue.
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        pMyEngine->CleanupDevice();
        // Return this part of the WM_QUIT message to Windows.
        return static_cast<char>(msg.wParam);
    }
    catch (std::exception& e)
    {
        OutputDebugString(L"Application hit a problem: ");
        OutputDebugStringA(e.what());
        OutputDebugString(L"\nTerminating.\n");

        pMyEngine->CleanupDevice();
        return EXIT_FAILURE;
    }
}

LRESULT MyAppRunner::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //WM_CREATEで登録したメモリの取得
    MyGameEngine* pEngine = reinterpret_cast<MyGameEngine*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_COMMAND:
    {
    }
    break;

    case WM_CREATE:
    {
        //CreateWindowで渡したWPARAMの登録
        // Save the MyGameEngine* passed in to CreateWindow.
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    break;
    case WM_PAINT:
        //BeginPaint系のコードはWM_PAINTベースでDirectXを動かす場合は絶対に駄目。注意
        pEngine->FrameUpdate();
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

        //Keyboard
    case WM_KEYDOWN:
        pEngine->GetInputManager()->keyPress(wParam);
        break;

    case WM_KEYUP:
        pEngine->GetInputManager()->keyUp(wParam);
        break;

        //Mouse
    case WM_MOUSEMOVE:
        pEngine->GetInputManager()->mouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_MOUSEWHEEL:
        pEngine->GetInputManager()->mouseWheel(GET_WHEEL_DELTA_WPARAM(wParam), GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_RBUTTONDOWN:
        pEngine->GetInputManager()->mouseButtonDown(1);
        break;

    case WM_RBUTTONUP:
        pEngine->GetInputManager()->mouseButtonUp(1);
        break;

    case WM_MBUTTONDOWN:
        pEngine->GetInputManager()->mouseButtonDown(2);
        break;

    case WM_MBUTTONUP:
        pEngine->GetInputManager()->mouseButtonUp(2);
        break;

    case WM_LBUTTONDOWN:
        pEngine->GetInputManager()->mouseButtonDown(0);
        break;

    case WM_LBUTTONUP:
        pEngine->GetInputManager()->mouseButtonUp(0);
        break;

    case WM_XBUTTONDOWN:
        pEngine->GetInputManager()->mouseButtonDown(2 + GET_XBUTTON_WPARAM(wParam));    //3 4
        break;

    case WM_XBUTTONUP:
        pEngine->GetInputManager()->mouseButtonUp(2 + GET_XBUTTON_WPARAM(wParam));
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
