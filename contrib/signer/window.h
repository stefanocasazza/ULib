/* window.h */

#ifndef U_WINDOW_H
#define U_WINDOW_H

// This is the header for the Window class. It serves both as a window class
// in its own right and as a base class for other window-like classes (e.g. PropertyPage, PropSheet).

#include <ulib/container/vector.h>
#include <ulib/container/gen_hash_map.h>

#include <commctrl.h>
#include "RECTWrapper.h"

// how to override the default...

template <> inline void u_destroy(const HFONT** ptr, uint32_t n)
{
   U_TRACE(0, "u_destroy<HFONT>(%p,%u)", ptr, n)
}

class U_EXPORT Window {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // COSTRUTTORI

   Window()
      {
      U_TRACE_REGISTER_OBJECT(2, Window, "", 0)

      Parent        = 0;
      WindowHandle  = 0;
      TooltipHandle = 0;
      }

   virtual ~Window()
      {
      U_TRACE_UNREGISTER_OBJECT(2, Window)

      // Delete any fonts we created.
      for (unsigned i = 0; i < Fonts.size(); ++i) DeleteObject(Fonts[i]);

      // shut down the tooltip control, if activated
      if (TooltipHandle) DestroyWindow(TooltipHandle);
      }

   // This only has to be called once in the entire app, before any Windows are created.
   static void SetAppInstance(HINSTANCE h) { AppInstance = h; }

   void Show(int State) { ::ShowWindow(WindowHandle, State); }

   // Ideally this could be hidden from the user completely.
   HWND GetHWND() const { return WindowHandle; }

   Window*   GetParent() const         { return Parent; }
   HINSTANCE GetInstance() const       { return AppInstance; }
   HWND      GetDlgItem(int id) const  { return ::GetDlgItem(GetHWND(), id); }

   UINT IsButtonChecked(int nIDButton) const { return ::IsDlgButtonChecked(GetHWND(), nIDButton); }

   void PostMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0) { ::PostMessage(GetHWND(), uMsg, wParam, lParam); }

   bool SetDlgItemFont(int id, const TCHAR * fontname, int Pointsize, int Weight = FW_NORMAL,
                       bool Italic = false, bool Underline = false, bool Strikeout = false);

   RECT GetWindowRect() const
      {
      U_TRACE(2, "Window::GetWindowRect()")

      RECT retval;

      ::GetWindowRect(WindowHandle, &retval);

      return retval;
      }

   RECT GetClientRect() const
      {
      U_TRACE(2, "Window::GetClientRect()")

      RECT retval;

      ::GetClientRect(WindowHandle, &retval);

      return retval;
      }

   // Center the window on the parent, or on screen if no parent.
   void CenterWindow ();

   // Reposition the window
   bool MoveWindow(long x, long y, long w, long h, bool Repaint = true) { return ::MoveWindow(WindowHandle, x, y, w, h, Repaint); }
   bool MoveWindow(const RECTWrapper &r, bool Repaint = true) { return ::MoveWindow(WindowHandle, r.left, r.top, r.width(), r.height(), Repaint); }

   // Set the title of the window
   void SetWindowText(const char* s) { ::SetWindowText(WindowHandle, s); }

   RECT ScreenToClient(const RECT &r) const;

   void ActivateTooltips();

   // enable/disable tooltips
   void SetTooltipState(bool b) { SendMessage(TooltipHandle, (UINT)TTM_ACTIVATE, (WPARAM)(BOOL)b, 0); }

   void AddTooltip(HWND target, HWND win, const char* text);

   // adds a tooltip to a control identified by its ID
   void AddTooltip(int id, const char* text)
      {
      U_TRACE(2, "Window::AddTooltip(%d,%S)", id, text)

      HWND target, parent;

      if ((target = GetDlgItem(id)) != 0 &&
          (parent = ::GetParent(target)) != 0) AddTooltip(target, parent, text);
      }

   // adds a tooltip that's represented by a string resource
   // this also allows for tooltips greater than 80 characters
   // we do this by setting the lpszText to LPSTR_TEXTCALLBACK 
   // and then responding to the TTN_GETDISPINFO notification
   // in order to do this we store a list of (control ID, string ID) pairs
   void AddTooltip(int id, int string_resource) { AddTooltip(id, (const char*)LPSTR_TEXTCALLBACK); TooltipStrings[id] = string_resource; }

   BOOL TooltipNotificationHandler(LPARAM lParam);

   // VIRTUAL

   virtual bool    MessageLoop();
   virtual bool    Create(Window* Parent = 0, DWORD Style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN);
   virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) { return DefWindowProc(WindowHandle, uMsg, wParam, lParam); }

   // Not processed by default. Override in derived classes to do something with app messages if you need to.
   virtual bool OnMessageCmd(int id, HWND hwndctl, UINT code)         { return false; }
   virtual bool OnMessageApp(UINT uMsg, WPARAM wParam, LPARAM lParam) { return false; }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   void SetHWND(HWND h)             { WindowHandle = h; }
   void setParent(Window* aParent)  { Parent = aParent; }

   static LRESULT CALLBACK FirstWindowProcReflector(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
   Window* Parent;
   HWND WindowHandle;                        // Our Windows(tm) window handle.
   HWND TooltipHandle;                       // if we have activated tool tips this will contain the handle
   UVector<HFONT> Fonts;                     // contains handles to fonts we've created that are to be deleted in our dtor
   UGenericHashMap<int,int> TooltipStrings;  // maps a control ID to a string resource ID

   static ATOM WindowClassAtom;
   static HINSTANCE AppInstance;

   virtual bool registerWindowClass();

   static LRESULT CALLBACK WindowProcReflector(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
      { return ((Window*)GetWindowLongPtr(hwnd, GWL_USERDATA))->WindowProc(uMsg, wParam, lParam); }

   Window(const Window&)            {}
   Window& operator=(const Window&) { return *this; }
};

#endif
