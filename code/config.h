#ifndef _CONFIG_BASE_CLASS_H_
#define _CONFIG_BASE_CLASS_H_

#include "stdafx.h"
#include "float3.h"

class ConfigBase
{
 public:
  ConfigBase(HINSTANCE instance, HWND parent, int dialog_id, int hide_button_id);
  virtual ~ConfigBase() { }

  // When the user updates these controls (slides a slider,
  // checks a checkbox, etc), the associated memory location
  // will be automatically updated:
  //void AddSlider(int ctrl_ID, float* value, float fmin, float fmax);
  //void AddCheckBox(int ctrl_ID, bool* value);

  void Show();
  void Hide();
  void Toggle() { if (showing_) Hide(); else Show(); }

  bool IsShowing() { return showing_; }
  HWND GetHwnd() { return hwnd_; }
  int GetHideButtonID() { return hide_button_id_; }

  int  ButtonPushed() { return button_pushed_; }
  void SetButtonPushed(int n) { button_pushed_ = n; }
  
  void SetClick(int x, int y) { clicked_ = true; click_x_ = x; click_y_ = y; }
  bool GetClick(int &x, int &y) { if (!clicked_) return false; x = click_x_; y = click_y_; return true; }
  void ClearClick() { clicked_ = false; }

  void SetDirty() { dirty_ = true; }
  bool IsDirty() { return dirty_; }
  void ClearDirtyFlag() { dirty_ = false; }

  HWND GetParentWindow() { return parent_hwnd_; }

 protected:
  bool showing_;
  bool dirty_;  // if true, it means that the values have changed, and the client needs to pull the new values and update its state.
  HWND hwnd_;
  HWND parent_hwnd_;
  int  hide_button_id_;
  int  button_pushed_;
  bool clicked_;
  int  click_x_;
  int  click_y_;
  //hash_map<int, float*> sliders_;
  //hash_map<int, bool*> checkboxes_;
};

static ConfigBase* g_active_config_dialog = NULL;


#endif  // _CONFIG_BASE_CLASS_H_