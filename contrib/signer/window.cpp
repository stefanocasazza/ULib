/* window.cpp */

#include "window.h"

ATOM      Window::WindowClassAtom;
HINSTANCE Window::AppInstance;

LRESULT CALLBACK Window::FirstWindowProcReflector(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   U_TRACE(2, "Window::FirstWindowProcReflector(%d,%d,%d,%d)", hwnd, uMsg, wParam, lParam)

   Window* wnd = 0;

   if (uMsg == WM_NCCREATE)
      {
      // This is the first message a window gets (so MSDN says anyway).
      // Take this opportunity to "link" the HWND to the 'this' ptr, steering
      // messages to the class instance's WindowProc().
      wnd = (Window*)(((LPCREATESTRUCT)lParam)->lpCreateParams);

      // Set a backreference to this class instance in the HWND.
      SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)(wnd));

      // Set a new WindowProc now that we have the peliminaries done.
      // We could instead simply do the contents of Window::WindowProcReflector
      // in the 'else' clause below, but this way we eliminate an unnecessary 'if/else' on
      // every message.  Yeah, it's probably not worth the trouble.
      SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR) &Window::WindowProcReflector);
      
      // Finally, store the window handle in the class.
      wnd->WindowHandle = hwnd;
      }
   else
      {
      // Should never get here.
      U_ERROR("Unexpected windows message %u received too early in window initialization", uMsg);
      }

   return wnd->WindowProc(uMsg, wParam, lParam);
}

bool Window::Create(Window* parent, DWORD Style)
{
   U_TRACE(2, "Window::Create(%p,%lld)", parent, Style)

   // First register the window class, if we haven't already
   if (registerWindowClass() == false) return false; // Registration failed

   // Save our parent, we'll probably need it eventually.
   Parent = parent;

   // Create the window instance
   WindowHandle = CreateWindowEx(
                // Extended Style
                0,
                "MainWindowClass",  // MAKEINTATOM(WindowClassAtom), // window class atom (name)
                "Hello", // no title-bar string yet
                // Style bits
                Style,
                // Default positions and size
                CW_USEDEFAULT,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                // Parent Window 
                (parent == 0 ? 0 : parent->GetHWND()),
                // use class menu 
                0,
                // The application instance 
                GetInstance(),
                // The this ptr, which we'll use to set up the WindowProc reflection.
                (LPVOID)this);

   U_RETURN(WindowHandle != 0);
}

bool Window::registerWindowClass()
{
   U_TRACE(2, "Window::registerWindowClass()")

   if (WindowClassAtom == 0)
      {
      // We're not registered yet
      WNDCLASSEX wc;

      wc.cbSize = sizeof(wc);
      // Some sensible style defaults
      wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
      // Our default window procedure.  This replaces itself
      // on the first call with the simpler Window::WindowProcReflector().
      wc.lpfnWndProc = Window::FirstWindowProcReflector;
      // No class bytes
      wc.cbClsExtra = 0;
      // One pointer to REFLECTION_INFO in the extra window instance bytes
      wc.cbWndExtra = 4;
      // The app instance
      wc.hInstance = GetInstance();
      // Use a bunch of system defaults for the GUI elements
      wc.hIcon   = 0;
      wc.hIconSm = 0;
      wc.hCursor = 0;
      wc.hbrBackground = (HBRUSH) (COLOR_BACKGROUND + 1);
      // No menu
      wc.lpszMenuName = 0;
      // We'll get a little crazy here with the class name
      wc.lpszClassName = "MainWindowClass";

      // All set, try to register
      WindowClassAtom = RegisterClassEx(&wc);

      if (WindowClassAtom == 0) U_RETURN(false); // Failed
      }

   // We're registered, or already were before the call,
   // return success in either case.
   U_RETURN(true);
}

bool Window::MessageLoop()
{
   U_TRACE(2, "Window::MessageLoop()")

   MSG msg;

   while (GetMessage(&msg, 0, 0, 0) != 0 &&
          GetMessage(&msg, 0, 0, 0) != -1)
      {
      if (!IsWindow(WindowHandle) ||
          !IsDialogMessage(WindowHandle, &msg))
         {
         TranslateMessage(&msg);
          DispatchMessage(&msg);
         }
      }

   U_RETURN(true);
}

void Window::CenterWindow()
{
   U_TRACE(2, "Window::CenterWindow()")

   POINT p;
   RECT WindowRect, ParentRect;
   int WindowWidth, WindowHeight;

   // Get the window rectangle
   WindowRect = GetWindowRect();

   if (GetParent() == 0) ::GetWindowRect(GetDesktopWindow(),     &ParentRect); // Center on desktop window
   else                     ::GetClientRect(GetParent()->GetHWND(), &ParentRect); // Center on client area of parent

   WindowWidth  = WindowRect.right  - WindowRect.left;
   WindowHeight = WindowRect.bottom - WindowRect.top;

   // Find center of area we're centering on
   p.x = (ParentRect.right  - ParentRect.left) / 2;
   p.y = (ParentRect.bottom - ParentRect.top) / 2;

   // Convert that to screen coords
   if (GetParent() == 0) ClientToScreen(GetDesktopWindow(), &p);
   else                     ClientToScreen(GetParent()->GetHWND(), &p);

   // Calculate new top left corner for window
   p.x -= WindowWidth  / 2;
   p.y -= WindowHeight / 2;

   // And finally move the window
   MoveWindow(p.x, p.y, WindowWidth, WindowHeight);
}

bool Window::SetDlgItemFont(int id, const TCHAR* fontname, int Pointsize, int Weight, bool Italic, bool Underline, bool Strikeout)
{
   U_TRACE(2, "Window::SetDlgItemFont(%d,%S,%d,%d,%b,%b,%b)", id, fontname, Pointsize, Weight, Italic, Underline, Strikeout)

   HWND ctrl = GetDlgItem(id);

   if (ctrl == 0) U_RETURN(false); // Couldn't get that ID

   // We need the DC for the point size calculation.
   HDC hdc = GetDC(ctrl);

   // Create the font.  We have to keep it around until the dialog item
   // goes away - basically until we're destroyed.
   HFONT hfnt = CreateFont(-MulDiv(Pointsize, GetDeviceCaps(hdc, LOGPIXELSY), 72),
                           0, 0, 0, Weight,
                           Italic    ? TRUE : FALSE,
                           Underline ? TRUE : FALSE,
                           Strikeout ? TRUE : FALSE,
                           ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontname);

   if (hfnt == 0) U_RETURN(false); // Font creation failed

   // Set the new font, and redraw any text which was already in the item.
   SendMessage(ctrl, WM_SETFONT, (WPARAM) hfnt, TRUE);

   // Store the handle so that we can DeleteObject() it in dtor
   Fonts.push_back(hfnt);

   U_RETURN(true);
}

RECT Window::ScreenToClient(const RECT &r) const
{
   U_TRACE(2, "Window::ScreenToClient(%p)", &r)

   RECT ret;
   POINT tl, br;

   tl.y = r.top;
   tl.x = r.left;
   ::ScreenToClient(GetHWND(), &tl);

   br.y = r.bottom;
   br.x = r.right;
   ::ScreenToClient(GetHWND(), &br);

   ret.top    = tl.y;
   ret.left   = tl.x;
   ret.bottom = br.y;
   ret.right  = br.x;

   return ret;
}

// initialization of the tooltip capability
void Window::ActivateTooltips()
{
   U_TRACE(2, "Window::ActivateTooltips()")

   if (TooltipHandle != 0) return; // already initialized

   // create a window for the tool tips - will be invisible most of the time
   if ((TooltipHandle = CreateWindowEx(0, (LPCTSTR) TOOLTIPS_CLASS, 0, 
                                       WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT,
                                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, GetHWND(),
                                       0, GetInstance(), 0)) == 0)
      {
      char buffer[4096];

      (void) write(2, buffer,
          u__snprintf(buffer, sizeof(buffer),
                     "Warning: call to CreateWindowEx failed when initializing tooltips. Error = %8.8x\n", GetLastError()));

      return;
      }

   // must be topmost so that tooltips will display on top
   SetWindowPos(TooltipHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

   // some of our tooltips are lengthy, and will disappear before they can be
   // read with the default windows delay, so we set a long (30s) delay here.
   SendMessage(TooltipHandle, TTM_SETDELAYTIME, TTDT_AUTOPOP, (LPARAM) MAKELONG(30000, 0));
}

// adds a tooltip to element 'target' in window 'win'
// note: text is limited to 80 chars (grumble)
void Window::AddTooltip(HWND target, HWND win, const char* text)
{
   U_TRACE(2, "Window::AddTooltip(%d,%d,%S)", target, win, text)

   if (!TooltipHandle) ActivateTooltips();

   TOOLINFO ti;

   memset((void*)&ti, 0, sizeof(ti));

   ti.hwnd     = win;
   ti.cbSize   = sizeof(ti);
   ti.uId      = (UINT_PTR)target;
   ti.uFlags   = TTF_IDISHWND |  // add tool based on handle not ID
                 TTF_SUBCLASS;   // tool is to subclass the window in order to automatically get mouse events
   ti.lpszText = (LPTSTR)text;   // pointer to text or string resource

   SendMessage(TooltipHandle, (UINT)TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
}

// this is the handler for TTN_GETDISPINFO notifications
BOOL Window::TooltipNotificationHandler(LPARAM lParam)
{
   U_TRACE(2, "Window::TooltipNotificationHandler(%d)", lParam)

   int ctrlID;
   NMTTDISPINFO* dispinfo = (NMTTDISPINFO*)lParam;

   if ((dispinfo->uFlags & TTF_IDISHWND) &&
       ((ctrlID = GetDlgCtrlID((HWND)dispinfo->hdr.idFrom)) != 0) &&
       TooltipStrings.find(ctrlID))
      {
      // enable multiple lines
      SendMessage(dispinfo->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, 450);

      // this is quite ugly. Apparently even when using string resources
      // the tooltip length still can't exceed 80 chars.  So, we fetch the
      // resource into our own buffer and use that

      TCHAR buf[2048];
      LoadString(GetInstance(), TooltipStrings.elem(), (LPTSTR)buf, (sizeof(buf) / sizeof(TCHAR)));

      dispinfo->lpszText = buf;

      // set this flag so that the control will not ask for this again
      dispinfo->uFlags |= TTF_DI_SETITEM;
      dispinfo->hinst   = 0;

      U_RETURN(TRUE);
      }

   U_RETURN(FALSE);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* Window::dump(bool reset) const
{
   *UObjectIO::os << "Parent                " << Parent        << '\n'
                  << "WindowHandle          " << WindowHandle  << '\n'
                  << "TooltipHandle         " << TooltipHandle << '\n'
                  << "Fonts (UVector<HFONT> " << (void*)&Fonts << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
