#include "get_string.h"
#include "resource.h"

const char* g_szPrompt = NULL;
char* g_szText[5] = { NULL, NULL, NULL, NULL, NULL };
const char* g_szLabel[5] = { NULL, NULL, NULL, NULL, NULL };
int   g_nTextMaxChars = 0;

void InitText(HWND dlg, 
              DWORD label, 
              DWORD text, 
              const char* szLabel, 
              const char* szText) {
  HWND hLabel = GetDlgItem(dlg, label);
  HWND hText  = GetDlgItem(dlg, text);

  ShowWindow(hLabel, (szText != NULL) ? SW_SHOW : SW_HIDE);
  ShowWindow(hText, (szText != NULL) ? SW_SHOW : SW_HIDE);

  if (szText != NULL) {
    SetWindowTextA(hLabel, szLabel ? szLabel : "");
    SetWindowTextA(hText, szText);
    SetFocus(hText);
    SendMessage(hText, EM_SETSEL, 0, -1 );
  }
}

void GetText(char* szText, HWND ctrl) {
  GetWindowTextA( ctrl, szText, g_nTextMaxChars );
}

INT_PTR CALLBACK GetStringWndProc(
  __in  HWND hwndDlg,  __in  UINT uMsg,  
  __in  WPARAM wParam,  __in  LPARAM lParam) 
{
  if (uMsg == WM_INITDIALOG)
  {
    SetWindowTextA(GetDlgItem(hwndDlg, IDC_PROMPT), g_szPrompt ? g_szPrompt : "");
    InitText(hwndDlg, IDC_LABEL5, IDC_TEXT5, g_szLabel[4], g_szText[4]);
    InitText(hwndDlg, IDC_LABEL4, IDC_TEXT4, g_szLabel[3], g_szText[3]);
    InitText(hwndDlg, IDC_LABEL3, IDC_TEXT3, g_szLabel[2], g_szText[2]);
    InitText(hwndDlg, IDC_LABEL2, IDC_TEXT2, g_szLabel[1], g_szText[1]);
    InitText(hwndDlg, IDC_LABEL1, IDC_TEXT , g_szLabel[0], g_szText[0]);
  }
  else if (uMsg == WM_COMMAND)
  {
    if (LOWORD(wParam) == IDOK) {
      GetText(g_szText[0], GetDlgItem(hwndDlg, IDC_TEXT));
      GetText(g_szText[1], GetDlgItem(hwndDlg, IDC_TEXT2));
      GetText(g_szText[2], GetDlgItem(hwndDlg, IDC_TEXT3));
      GetText(g_szText[3], GetDlgItem(hwndDlg, IDC_TEXT4));
      GetText(g_szText[4], GetDlgItem(hwndDlg, IDC_TEXT5));
      EndDialog(hwndDlg, 0);
    }
    if (LOWORD(wParam) == IDCANCEL) {
      EndDialog(hwndDlg, 1);
    }
  }

  return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
}

// Note: these pointers must refer to memory locations that will
//   persist during the blocking call to GetStringFromUser[].
// 'max_chars' is the # of bytes available for szText1[], 2, etc.
bool GetStringFromUser(HINSTANCE hInstance, HWND parent, 
                       const char* szPrompt, 
                       const char* szLabel1, char* szText1, 
                       const char* szLabel2, char* szText2, 
                       const char* szLabel3, char* szText3, 
                       const char* szLabel4, char* szText4, 
                       const char* szLabel5, char* szText5, 
                       int max_chars)
{
  g_szPrompt = szPrompt;
  g_szLabel[0] = szLabel1;
  g_szLabel[1] = szLabel2;
  g_szLabel[2] = szLabel3;
  g_szLabel[3] = szLabel4;
  g_szLabel[4] = szLabel5;
  g_szText[0] = szText1;
  g_szText[1] = szText2;
  g_szText[2] = szText3;
  g_szText[3] = szText4;
  g_szText[4] = szText5;
  g_nTextMaxChars = max_chars;
  
  //HWND h = CreateWindowExA(0, NULL, "Text Entry", WS_POPUP, 16, 16, 256, 256, parent, NULL, hInstance, NULL);

  int ret = DialogBox(hInstance, MAKEINTRESOURCE(IDD_GET_STRING), parent, GetStringWndProc);

  return (ret == 0);
}
