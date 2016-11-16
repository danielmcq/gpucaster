
#include "stdafx.h"

#include "config.h"
#include <assert.h>
#include <windowsx.h>

BOOL CALLBACK ConfigProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
  if (g_active_config_dialog == NULL)
    return FALSE;

  switch (message) 
  { 
    case WM_LBUTTONDOWN:
      g_active_config_dialog->SetClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      return TRUE;

    /*case WM_KEYDOWN:
      if (g_active_config_dialog->GetParentWindow())
        PostMessage(g_active_config_dialog->GetParentWindow(), message, wParam, lParam);
      return TRUE;*/

    case WM_INITDIALOG: 
      return TRUE;

    case WM_NOTIFY:
      // slider moved...
      g_active_config_dialog->SetDirty();
      break;

    case WM_COMMAND:
      if (wParam == g_active_config_dialog->GetHideButtonID())
        g_active_config_dialog->Hide();
      else if (lParam != 0)
        g_active_config_dialog->SetButtonPushed( LOWORD(wParam) );
      break;

    /*
    case WM_INITDIALOG: 
      CheckDlgButton(hwndDlg, ID_ABSREL, fRelative); 
      return TRUE; 

    case WM_COMMAND: 
      switch (LOWORD(wParam)) 
      { 
        case IDOK: 
          fRelative = IsDlgButtonChecked(hwndDlg, ID_ABSREL); 
          iLine = GetDlgItemInt(hwndDlg, ID_LINE, &fError, fRelative); 
          if (fError) 
          { 
              MessageBox(hwndDlg, SZINVALIDNUMBER, SZGOTOERR, MB_OK); 
              SendDlgItemMessage(hwndDlg, ID_LINE, EM_SETSEL, 0, -1L); 
          } 
          else return TRUE; // Notify the owner window to carry out the task. 

        case IDCANCEL: 
          DestroyWindow(hwndDlg); 
          hwndGoto = NULL; 
          return TRUE; 
      }
    */
  } 
  return FALSE; 
} 

ConfigBase::ConfigBase(HINSTANCE instance, HWND parent, int dialog_id, int hide_button_id)
{
  hwnd_ = CreateDialog(instance, 
                       MAKEINTRESOURCE(dialog_id), 
                       parent, 
                       (DLGPROC)ConfigProc); 
  dirty_ = false;
  showing_ = false;
  hide_button_id_ = hide_button_id;
  parent_hwnd_ = parent;
  button_pushed_ = 0;
  clicked_ = false;
}

void ConfigBase::Show() 
{
  if (g_active_config_dialog == NULL)
  {
    assert(!showing_);
    ShowWindow(hwnd_, SW_SHOW); 
    showing_ = true;
    g_active_config_dialog = this;
  }
}

void ConfigBase::Hide() 
{
  if (showing_)
  {
    assert(g_active_config_dialog);
    ShowWindow(hwnd_, SW_HIDE); 
    showing_ = false;
    g_active_config_dialog = NULL;
  }
}

