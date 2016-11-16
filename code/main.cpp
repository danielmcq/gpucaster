#include "stdafx.h"
#include <windowsx.h>
#include <assert.h>

// Local header files
#include "main.h"
#include "demo.h"
#include "resource.h"

// Settings
const TCHAR szWindowClass[] = L"gpucaster";
const bool kShowMenu = false;

// Global Variables:
HINSTANCE g_hInst = 0;
HWND      g_hWnd = 0;
gpucaster::Demo* g_demo = NULL;

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

// Window callback procedure.
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
    case WM_KEYDOWN:
      if (g_demo && g_demo->On_WM_KEYDOWN(wParam, lParam))
        return false;
		  return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_KEYUP:
      if (g_demo && g_demo->On_WM_KEYUP(wParam, lParam))
        return false;
		  return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_CHAR:
      if (g_demo && g_demo->On_WM_CHAR(wParam, lParam))
        return false;
		  return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_EXITSIZEMOVE:
      if (g_demo) {
        g_demo->OnResize(-1, -1, -1);
      }
      return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_MOUSEMOVE:
      if (g_demo) g_demo->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (wParam & MK_SHIFT) != 0, (wParam & MK_CONTROL) != 0);
      return FALSE;

    case WM_LBUTTONDOWN: 
      SetCapture(hWnd);
      if (g_demo) g_demo->OnButtonDown(0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (wParam & MK_SHIFT) != 0, (wParam & MK_CONTROL) != 0);
      return FALSE;
    case WM_MBUTTONDOWN: 
      SetCapture(hWnd);
      if (g_demo) g_demo->OnButtonDown(1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (wParam & MK_SHIFT) != 0, (wParam & MK_CONTROL) != 0);
      return FALSE;
    case WM_RBUTTONDOWN: 
      SetCapture(hWnd);
      if (g_demo) g_demo->OnButtonDown(2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (wParam & MK_SHIFT) != 0, (wParam & MK_CONTROL) != 0);
      return FALSE;

    case WM_LBUTTONUP:   
      ReleaseCapture();
      if (g_demo) g_demo->OnButtonUp(0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (wParam & MK_SHIFT) != 0, (wParam & MK_CONTROL) != 0);
      return FALSE;
    case WM_MBUTTONUP:   
      ReleaseCapture();
      if (g_demo) g_demo->OnButtonUp(1, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (wParam & MK_SHIFT) != 0, (wParam & MK_CONTROL) != 0);
      return FALSE;
    case WM_RBUTTONUP:   
      ReleaseCapture();
      if (g_demo) g_demo->OnButtonUp(2, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (wParam & MK_SHIFT) != 0, (wParam & MK_CONTROL) != 0);
      return FALSE;

	  case WM_COMMAND:
		  wmId    = LOWORD(wParam);
		  wmEvent = HIWORD(wParam);
		  // Parse the menu selections:
		  switch (wmId)
		  {
		  case IDM_ABOUT:
			  DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			  break;
		  case IDM_EXIT:
			  DestroyWindow(hWnd);
			  break;
		  default:
			  return DefWindowProc(hWnd, message, wParam, lParam);
		  }
		  break;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
  		// TODO: Add any drawing code here...
			EndPaint(hWnd, &ps);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	//UNREFERENCED_PARAMETER(lpCmdLine);

  g_hInst = hInstance; // Store instance handle in our global variable

  // Register window class
  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style          = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc    = WndProc;
  wcex.cbClsExtra     = 0;
  wcex.cbWndExtra     = 0;
  wcex.hInstance      = hInstance;
  wcex.hIcon          = LoadIcon(hInstance, (LPCTSTR)IDI_GPUCASTER);
  wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
  wcex.lpszMenuName	  = kShowMenu ? MAKEINTRESOURCE(IDC_GPUCASTER) : NULL;
  wcex.lpszClassName  = szWindowClass;
  wcex.hIconSm        = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);
  if( !RegisterClassEx(&wcex) ) {
    // TODO: messagebox...
    return 1;
  }

  // Parse commandline.
  int tile_edge_len_pixels = 0;
  if (swscanf(lpCmdLine, L"%d", &tile_edge_len_pixels) != 1 || 
      tile_edge_len_pixels < 8 ||
      tile_edge_len_pixels > 400) {
    MessageBoxA(g_hWnd, 
        "Invalid commandline parameters.  Usage:\n"
        "\n"
        "    gpucaster <tile_size>\n"
        "\n"
        "where <tile_size> is one of:\n"
        "\n"
        "    [8, 16, 24, 32, 48, 64, 96, 128, 160, 192, 224, 256].\n"
        "\n"
        "Please start with a low value and work your way up.  If you choose a value too low, "
        "the rendering speed will be greatly reduced; however, if you choose a value too high, "
        "your system might become very bogged down and the UI can be slow to respond. "
        "Therefore, start with small values, and as your system stays highly responsive, "
        "try the next higher value.  If/when the system (overall) begins to feel less responsive, "
        "drop back down a notch.\n"
        "\n"
        "Good starting guesses (as of January 2014):\n"
        "\n"
        "    new $500 desktop GPU       tile_size -> 160\n"
        "    new $100 desktop GPU       tile_size -> 64\n"
        "    good laptop GPU                tile_size -> 32\n"
        "    poor laptop GPU                 tile_size -> 16\n"
        "\n"
        "...and for every 2 years after 2014, multiply the value by 2 (up to 256).\n"
        "\n",
        "Error: Invalid Parameters", MB_ICONERROR);    
    exit(1);
  }

  int target_client_width  = 720;
  int target_client_height = 720;
  HMONITOR hMon = MonitorFromWindow(0, MONITOR_DEFAULTTOPRIMARY);
  MONITORINFO mi;
  mi.cbSize = sizeof(mi);
  if (GetMonitorInfo(hMon, &mi)) {
    int w = mi.rcWork.right - mi.rcWork.left;
    int h = mi.rcWork.bottom - mi.rcWork.top;
    int size = min(w, h) * 7 / 8;
    target_client_width = size;
    target_client_height = size;
  }
  target_client_width = (target_client_width / tile_edge_len_pixels) * tile_edge_len_pixels;
  target_client_height = (target_client_height / tile_edge_len_pixels) * tile_edge_len_pixels;

  // Create window
  RECT rc = { 0, 0, target_client_width, target_client_height };
  DWORD style = WS_OVERLAPPEDWINDOW;
  AdjustWindowRect( &rc, style, kShowMenu ? TRUE : FALSE );
  g_hWnd = CreateWindow( szWindowClass, L"GpuCaster", style,
                         8, 8, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL,
                         hInstance, NULL);
  if( !g_hWnd ) {
    MessageBoxA(NULL, "CreateWindow() failed.", "Error", MB_ICONERROR);
    return 1;
  }
  ShowWindow( g_hWnd, nCmdShow );
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GPUCASTER));

  // Make sure window client size came out right.
  GetClientRect(g_hWnd, &rc);
  if (rc.right - rc.left != target_client_width ||
      rc.bottom - rc.top != target_client_height) {
    MessageBoxA(g_hWnd, "Unable to create a window with the desired client size.", "Error", MB_ICONERROR);
    return 1;
  }
  if ( ((rc.right - rc.left) % tile_edge_len_pixels) != 0 ||
       ((rc.bottom - rc.top) % tile_edge_len_pixels) != 0) {
    MessageBoxA(g_hWnd, "Initial window size must be a multiple of tile_edge_len_pixels_.", "Error", MB_ICONERROR);
    return 1;
  }

  // TODO: use seed from settings.
  srand(713);

  // Create demo object with a DX11 context.
  gpucaster::Demo* demo = new gpucaster::Demo(g_hWnd, hInstance, tile_edge_len_pixels);
  HRESULT hr = demo->Init();
  if (FAILED(hr)) {
    // Skip messagebox - Init() will create one if it returns failure.
    delete demo;
    return 1;
  }

  // Hook up messages.
  g_demo = demo;
  
  // Main message loop
  MSG msg = {0};
  while ( WM_QUIT != msg.message )
  {
    if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
    {
        //KIV
        if (!IsDialogMessage(g_demo->GetLightingDialogHwnd(), &msg)) 
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }
    else
    {
      g_demo->Render();
    }
  }

  // Cleanup
  delete g_demo;

  return 0;
}
