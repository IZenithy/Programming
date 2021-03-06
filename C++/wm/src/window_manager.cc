#include "window_manager.h"
#include "util.h"
#include <X11/X.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <cstring>

// For spacial keys such as audio keys
#include <X11/XF86keysym.h>

// general keys
#include <X11/Xutil.h>



namespace WM
{
    // Init static member
    bool WindowManager::m_wmDetected{};
    std::mutex WindowManager::m_wmDetectedMutex{};

    WindowManager::WindowManager(const std::string& displayName)
                            // Return the default root window for a given X server
        :     m_connection{createConnection(displayName)}, m_rootWindow{DefaultRootWindow(m_connection)},
              WM_PROTOCOLS(XInternAtom(m_connection, "WM_PROTOCOLS", false)),
              WM_DELETE_WINDOW(XInternAtom(m_connection, "WM_DELETE_WINDOW", false))
    {

    }

    WindowManager::~WindowManager()
    {
        // Close the connection with X server
        XCloseDisplay(m_connection);
    }

    // Move copy constructor
    WindowManager::WindowManager(WindowManager&& wm)
    {
        m_connection = wm.m_connection;

        m_rootWindow = wm.m_rootWindow;

        WM_PROTOCOLS = wm.WM_PROTOCOLS;

        wm.m_rootWindow = 0;
        wm.m_connection = nullptr;
    }

    // Move assignment operator
    WindowManager& WindowManager::operator=(WindowManager&& wm)
    {
        // Self assignment guard
        if(&wm == this)
            return *this;

        m_connection = wm.m_connection;

        m_rootWindow = wm.m_rootWindow;

        wm.m_rootWindow = 0;
        wm.m_connection = nullptr;

        return *this;
    }

    Display* WindowManager::createConnection(const std::string& displayName)
    {
        // 1. Open X display.

        //Check if display is specified
        const char* display_c_str = displayName.empty() ? nullptr : displayName.c_str();

        Display* connection= XOpenDisplay(display_c_str);


        if (connection == nullptr)
        {
            throw std::runtime_error("Failed to open X display " + std::string{XDisplayName(nullptr)});
        }

        return connection;
    }

    void WindowManager::Run()
    {
        // 1. Initialization.
        //   a. Select events on root window. Use a special error handler so we can
        //   exit gracefully if another window manager is already running.
        {
            std::lock_guard<std::mutex> lock(m_wmDetectedMutex);
            m_wmDetected= false;

            // Setup temporary error handler
            XSetErrorHandler(&WindowManager::OnWMDetected);

            // Take control over the root window
            //            server,  window, events
            XSelectInput(m_connection, m_rootWindow , SubstructureRedirectMask | SubstructureNotifyMask);

              /* XSelectInput doesn't  send a request to the X server,
               *
               * instead only queues the request and returns
               * we have to explicitly flush the request queue with XSync
               *
               * False means that XSync  will not discard the events
               * */
            XSync(m_connection, false);
            if (m_wmDetected)
            {
                throw std::runtime_error("Detected another window manager on display " +
                        std::string{XDisplayString(m_connection)});
                return;
            }
        }

        //   b. Set error handler.
        XSetErrorHandler(&WindowManager::OnXError);

        //   c. Grab X server to prevent windows from changing under us while we
        //   frame them.
        XGrabServer(m_connection);

        //   d. Frame existing top-level windows.
        //     i. Query existing top-level windows.
        Window returned_root;
        Window returned_parent;

        Window* top_level_windows;
        unsigned int num_top_level_windows;

        if (XQueryTree(m_connection, m_rootWindow, &returned_root, &returned_parent,
                       &top_level_windows, &num_top_level_windows) == 0)
        {
            throw std::runtime_error("We can't query the window list");
        }

        if(returned_root != m_rootWindow)
        {
            throw std::runtime_error("returned_root != m_rootWindow");
        }

        //     ii. Frame each top-level window.
        for (unsigned int i = 0; i < num_top_level_windows; ++i)
        {
            Frame(top_level_windows[i], true /* was_created_before_window_manager */);
        }
        //     iii. Free top-level window array.
        XFree(top_level_windows);

        //   e. Ungrab X server.
        XUngrabServer(m_connection);


        // 2. Main event loop.
        while(true)
        {
            // 1. Get next event.
            XEvent e;
            XNextEvent(m_connection, &e);
            std::cout << "Received event: " << ToString(e);

            // 2. Dispatch event.
            switch (e.type)
            {
                // When a client want to create window
                case CreateNotify:
                    OnCreateNotify(e.xcreatewindow);
                break;

                case ConfigureRequest:
                    OnConfigureRequest(e.xconfigurerequest);
                break;

                case ConfigureNotify:
                    OnConfigureNotify(e.xconfigure);
                break;

                case MapRequest:
                    OnMapRequest(e.xmaprequest);
                break;

                case UnmapNotify:
                    OnUnmapNotify(e.xunmap);
                break;

                case ReparentNotify:
                    OnReparentNotify(e.xreparent);
                break;

                case MapNotify:
                    OnMapNotify(e.xmap);
                break;

                case DestroyNotify:
                    OnDestroyNotify(e.xdestroywindow);
                break;

                case ButtonPress:
                    OnButtonPress(e.xbutton);
                break;

                case ButtonRelease:
                    OnButtonRelease(e.xbutton);
                break;

                case MotionNotify:
                    // Skip any already pending motion events.
                    while (XCheckTypedWindowEvent(m_connection, e.xmotion.window, MotionNotify, &e))
                    {

                    }
                    OnMotionNotify(e.xmotion);
                break;

                case KeyPress:
                    OnKeyPress(e.xkey);
                break;

                case KeyRelease:
                    OnKeyRelease(e.xkey);
                break;

                default:
                std::cerr << "Ignored event";

            }
        }
    }

    // Temporary error handler, to catch errors during this XSync invocation
    int WindowManager::OnWMDetected(Display* display, XErrorEvent* e)
    {
        // In the case of an already running window manager, the error code from
        // XSelectInput is BadAccess. We don't expect this handler to receive any
        // other errors.
        if(static_cast<int>(e->error_code) == BadAccess)
            throw std::runtime_error("A window manager is running!");

        // Set flag.
        m_wmDetected= true;
        // The return value is ignored.
        return 0;
    }

    // Error handler
    int WindowManager::OnXError(Display* display, XErrorEvent* e)
    {
        // Print the Error and continue
        const int MAX_ERROR_TEXT_LENGTH = 1024;

        char error_text[MAX_ERROR_TEXT_LENGTH];

        XGetErrorText(display, e->error_code, error_text, sizeof(error_text));

        std::cerr << "Received X error:\n"
            << "    Request: " << int(e->request_code)
            << " - " <<  XRequestCodeToString(e->request_code) << "\n"
            << "    Error code: " << int(e->error_code)
            << " - " << error_text << "\n"
            << "    Resource ID: " << e->resourceid;

        // The return value is ignored.
        return 0;

    }

    void WindowManager::Frame(Window w, bool was_created_before_window_manager)
    {
        // Visual properties of the frame to create.
        constexpr unsigned int BORDER_WIDTH = 3;
        constexpr unsigned long BORDER_COLOR = 0xff0000;
        constexpr unsigned long BG_COLOR = 0x0000ff;

        // 1. Retrieve attributes of window to frame.
        XWindowAttributes x_window_attrs;

        if(m_clients.count(w))
        {
            throw std::runtime_error("We shouldn't be framing windows we've already framed.");
        }

        if(XGetWindowAttributes(m_connection, w, &x_window_attrs) == 0)
        {
            throw std::runtime_error("We can't get window attributes!");
        }

        // 2. If window was created before window manager started, we should frame
        // it only if it is visible and doesn't set override_redirect.
        if (was_created_before_window_manager)
        {
            // if override_redirect is set to true that's means we don't have to care about it
            // and map_state indicate whether a window is mapped(visible) or not
            if (x_window_attrs.override_redirect || x_window_attrs.map_state != IsViewable)
            {
                return;
            }
        }

        // 3. Create frame.
        const Window frame { XCreateSimpleWindow(
        m_connection,
        m_rootWindow,
        x_window_attrs.x,
        x_window_attrs.y,
        static_cast<unsigned int>(x_window_attrs.width),
        static_cast<unsigned int>(x_window_attrs.height),
        BORDER_WIDTH,
        BORDER_COLOR,
        BG_COLOR)
        };

        // 4. Select events on frame.
        XSelectInput(m_connection, frame, SubstructureRedirectMask | SubstructureNotifyMask);


        // 5. Add client to save set, so that it will be restored and kept alive if we
        // crash.
        XAddToSaveSet(m_connection, w);

        // 6. Reparent client window to the frame
        XReparentWindow(m_connection, w, frame, 0, 0);  // Offset of client window within frame.

        // 7. Map frame, make it visible
        XMapWindow(m_connection, frame);

        // 8. Save frame handle.
        m_clients[w] = frame;

        //   a. Move windows with alt + left button.
        XGrabButton(
            m_connection,
            Button1,
            Mod1Mask,
            w,
            false,
            ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
            GrabModeAsync,
            GrabModeAsync,
            None,
            None
        );
        //   b. Resize windows with alt + right button.
        XGrabButton
        (
            m_connection,
            Button3,
            Mod1Mask,
            w,
            false,
            ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
            GrabModeAsync,
            GrabModeAsync,
            None,
            None
        );

        //   c. Kill windows with alt + f4.
        XGrabKey
        (
            m_connection,
            XKeysymToKeycode(m_connection, XK_F4),
            Mod1Mask,
            w,
            false,
            GrabModeAsync,
            GrabModeAsync);
            //   d. Switch windows with alt + tab.
            XGrabKey(
            m_connection,
            XKeysymToKeycode(m_connection, XK_Tab),
            Mod1Mask,
            w,
            false,
            GrabModeAsync,
            GrabModeAsync
        );


        std::cout << "Framed window " << w << " [" << frame << "]";
    }


    void WindowManager::Unframe(Window w)
    {
        // We reverse the steps taken in Frame().
        const Window frame = m_clients[w];

        // 1. Unmap frame.
        XUnmapWindow(m_connection, frame);

        // 2. Reparent client window back to root window.
        XReparentWindow( m_connection, w, m_rootWindow, 0, 0);  // Offset of client window within root.

        // 3. Remove client window from save set, as it is now unrelated to us.
        XRemoveFromSaveSet(m_connection, w);

        // 4. Destroy frame.
        XDestroyWindow(m_connection, frame);

        // 5. Drop reference to frame handle.
        m_clients.erase(w);

        std::cout  << "Unframed window " << w << " [" << frame << "]";
    }


    //------------------------------------------------------------------//
    //                              EVENTS                              //
    //------------------------------------------------------------------//

    // Ignore window creation events
    void WindowManager::OnCreateNotify(const XCreateWindowEvent& e)
    {

    }

    void WindowManager::OnConfigureRequest(const XConfigureRequestEvent& e)
    {
        XWindowChanges changes;
        // Copy fields from e to changes.
        changes.x = e.x;
        changes.y = e.y;
        changes.width = e.width;
        changes.height = e.height;
        changes.border_width = e.border_width;
        changes.sibling = e.above;
        changes.stack_mode = e.detail;


        // Configure a window that is currently visible
        if (m_clients.count(e.window))
        {
            const Window frame = m_clients[e.window];
            XConfigureWindow(m_connection, frame, e.value_mask, &changes);
            std::cout << "Resize [" << frame << "] to " << Size<int>(e.width, e.height);
        }

        // Grant request by calling XConfigureWindow().
        XConfigureWindow(m_connection, e.window, e.value_mask, &changes);

        std::cout << "Resize " << e.window << " to " << Size<int>(e.width, e.height);
    }

    void WindowManager::OnMapRequest(const XMapRequestEvent& e)
    {
        // 1. Frame or re-frame window.
        Frame(e.window, false /* was_created_before_window_manager */);

        // 2. Actually map window.
        XMapWindow(m_connection, e.window);

    }

    // Ignore re parenting notify
    void WindowManager::OnReparentNotify(const XReparentEvent& e)
    {

    }

    // Ignore mapping notify
    void WindowManager::OnMapNotify(const XMapEvent& e)
    {

    }

    // Ignore configure notify
    void WindowManager::OnConfigureNotify(const XConfigureEvent& e)
    {

    }

    void WindowManager::OnUnmapNotify(const XUnmapEvent& e) {
        // If the window is a client window we manage, unframe it upon UnmapNotify. We
        // need the check because we will receive an UnmapNotify event for a frame
        // window we just destroyed ourselves.
        if (!m_clients.count(e.window))
        {
            std::cout << "Ignore UnmapNotify for non-client window " << e.window;
            return;
        }

        // Ignore event if it is triggered by reparenting a window that was mapped
        // before the window manager started.
        //
        // Since we receive UnmapNotify events from the SubstructureNotify mask, the
        // event attribute specifies the parent window of the window that was
        // unmapped. This means that an UnmapNotify event from a normal client window
        // should have this attribute set to a frame window we maintain. Only an
        // UnmapNotify event triggered by reparenting a pre-existing window will have
        // this attribute set to the root window.
        if (e.event == m_rootWindow)
        {
            std::cout << "Ignore UnmapNotify for reparented pre-existing window " << e.window;
            return;
        }
        Unframe(e.window);
    }

    // Ignore destroy notify
    void WindowManager::OnDestroyNotify(const XDestroyWindowEvent& e)
    {

    }

    void WindowManager::OnButtonPress(const XButtonEvent& e)
    {
        if(!(m_clients.count(e.window)))
        {
            throw std::runtime_error("There is no window!\n");
        }

        const Window frame = m_clients[e.window];

        // 1. Save initial cursor position.
        drag_start_pos_ = Position<int>(e.x_root, e.y_root);

        // 2. Save initial window info.
        Window returned_root;
        int x, y;
        unsigned width, height, border_width, depth;

        if(XGetGeometry(m_connection, frame, &returned_root, &x, &y,
                        &width, &height, &border_width, &depth) == BadDrawable)
        {
            throw std::runtime_error("Drawable argument does not name a defined Window or Pixmap!\n");
        }
        drag_start_frame_pos_ = Position<int>(x, y);
        drag_start_frame_size_ = Size<int>(static_cast<int>(width), static_cast<int>(height));

        // 3. Raise clicked window to top.
        XRaiseWindow(m_connection, frame);
    }

    // Ignore button release
    void WindowManager::OnButtonRelease(const XButtonEvent& e)
    {

    }

    void WindowManager::OnMotionNotify(const XMotionEvent& e)
    {
        if(!(m_clients.count(e.window)))
        {
            throw std::runtime_error("There is no window!\n");
        }
        const Window frame = m_clients[e.window];
        const Position<int> drag_pos(e.x_root, e.y_root);
        const Vector2D<int> delta = drag_pos - drag_start_pos_;

        if (e.state & Button1Mask )
        {
            // alt + left button: Move window.
            const Position<int> dest_frame_pos = drag_start_frame_pos_ + delta;
            XMoveWindow(
            m_connection,
            frame,
            dest_frame_pos.m_x, dest_frame_pos.m_y);
        }
        else if (e.state & Button3Mask)
        {
            // alt + right button: Resize window.
            // Window dimensions cannot be negative.
            const Vector2D<int> size_delta(
            std::max(delta.m_x, -drag_start_frame_size_.m_width),
            std::max(delta.m_y, -drag_start_frame_size_.m_height));
            const Size<int> dest_frame_size = drag_start_frame_size_ + size_delta;
            // 1. Resize frame.
            XResizeWindow(m_connection, frame,
                        static_cast<unsigned int>(dest_frame_size.m_width),
                        static_cast<unsigned int>(dest_frame_size.m_height));

            // 2. Resize client window.
            XResizeWindow(m_connection, e.window,
                        static_cast<unsigned int>(dest_frame_size.m_width),
                        static_cast<unsigned int>(dest_frame_size.m_height));
        }
    }

    void WindowManager::OnKeyPress(const XKeyEvent& e)
    {
        if ((e.state & Mod1Mask) && (e.keycode == XKeysymToKeycode(m_connection, XK_F4)))
        {
            // alt + f4: Close window.
            //
            // There are two ways to tell an X window to close. The first is to send it
            // a message of type WM_PROTOCOLS and value WM_DELETE_WINDOW. If the client
            // has not explicitly marked itself as supporting this more civilized
            // behavior (using XSetWMProtocols()), we kill it with XKillClient().
            Atom* supported_protocols;

            int num_supported_protocols;

            if (XGetWMProtocols(m_connection, e.window, &supported_protocols,
                    &num_supported_protocols) && (::std::find(supported_protocols,
                    supported_protocols + num_supported_protocols, WM_DELETE_WINDOW) !=
                    supported_protocols + num_supported_protocols))
            {
                std::cout << "Gracefully deleting window " << e.window;

                // 1. Construct message.
                XEvent msg;
                std::memset(&msg, 0, sizeof(msg));
                msg.xclient.type = ClientMessage;
                msg.xclient.message_type = WM_PROTOCOLS;
                msg.xclient.window = e.window;
                msg.xclient.format = 32;
                msg.xclient.data.l[0] = static_cast<long>(WM_DELETE_WINDOW);

                // 2. Send message to window to be closed.

                //TODO: Check for BadValue
                if(XSendEvent(m_connection, e.window, false, 0, &msg) == BadWindow)
                {
                    throw std::runtime_error("We can't send close message to the window!");
                }
            }
            else
            {
                std::cout << "Killing window " << e.window;
                XKillClient(m_connection, e.window);
            }
        }
        else if ((e.state & Mod1Mask) && (e.keycode == XKeysymToKeycode(m_connection, XK_Tab)))
        {
            // alt + tab: Switch window.
            // 1. Find next window.
            auto i = m_clients.find(e.window);

            if(i == m_clients.end())
            {
                throw std::runtime_error("We can't find The window!\n");
            }
            ++i;

            if (i == m_clients.end())
            {
                i = m_clients.begin();
            }
            // 2. Raise and set focus.
            XRaiseWindow(m_connection, i->second);
            XSetInputFocus(m_connection, i->first, RevertToPointerRoot, CurrentTime);
        }
    }

    // Ignore key release events
    void WindowManager::OnKeyRelease(const XKeyEvent& e)
    {

    }

}
