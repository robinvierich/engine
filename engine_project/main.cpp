#include <windows.h>
#include <tchar.h>

#include "utils.h"
#include "frame.h"
#include "renderer.h"

int windowWidth, windowHeight = 0;

void resizeWindow(int w, int h) {
	windowWidth = w;
	windowHeight = h;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_SIZE: // If our window is resizing  
        {
            // resize our opengl context
            resizeWindow(LOWORD(lParam), HIWORD(lParam));
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {

    static TCHAR szWindowClass[] = _T("win32app");
    static TCHAR szTitle[] = _T("OpenGL app");

    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szWindowClass;

    if (!RegisterClass(&wc)) {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("ERROR"),
            NULL);

        return 1;
    }

    HWND hWnd = CreateWindow(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        400, 225,
        NULL,
        NULL,
        hInstance,
        NULL
        );
    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("ERROR"),
            NULL);

        return 1;
    }

    
	initRenderer(hWnd);

    ShowWindow(hWnd, nCmdShow);
    
    resizeWindow(400, 225);

    UpdateWindow(hWnd);

    MSG msg;
    bool running = true; // keep track of our own 'running' var

    double t = 0;
    double dt = 0;

    double desiredDt = 1 / 75.0;
	
    LARGE_INTEGER prevCount;
    LARGE_INTEGER count;
    LARGE_INTEGER secondsPerCount;
    
    QueryPerformanceCounter(&prevCount);

	Frame currFrame;

    while (running) { 
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { // if there is a message, and handle it (why don't we need to consume it?! -- as is done in GetMessage)
            if (msg.message == WM_QUIT) {
                running = false;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } else { // if no message, render!
            QueryPerformanceFrequency(&secondsPerCount);
            QueryPerformanceCounter(&count);

            if (count.QuadPart < prevCount.QuadPart) {
                // overflow - worst case we skip 1 frame.. not ideal
                prevCount = count;
                continue;
            }

            dt = (double)(count.QuadPart - prevCount.QuadPart) / (double)secondsPerCount.QuadPart;
            if (dt < desiredDt) {
                continue;
            }

            updateRenderer(currFrame, windowWidth, windowHeight);

            t += dt;
            prevCount = count;
			currFrame.frameIndex++;
        }
    }

	shutdownRenderer();

    return (int)msg.wParam;
}
