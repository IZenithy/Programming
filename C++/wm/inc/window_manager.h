#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

// C libraries
extern "C"
{
    #include <X11/Xlib.h>
}


#include "util.h"
#include <memory>
#include <unordered_map>

namespace WM
{
    class WindowManager
    {
    private: // Private variables

        // Handle to the underlying Xlib connection struct.
        // TODO: rename it to connection
        Display* m_connection;
        // Handle to root window.
        Window m_rootWindow;

        // Whether an existing window manager has been detected. Set by OnWMDetected,
        // and hence must be static.
        static bool m_wmDetected;

        // NEW
        // A mutex for protecting m_wmDetected. It's not strictly speaking needed as
        // this program is single threaded, but better safe than sorry.
        static std::mutex m_wmDetectedMutex;

        // Maps top-level windows to their frame windows.
        std::unordered_map<Window, Window> m_clients;

        // The cursor position at the start of a window move/resize.
        Position<int> drag_start_pos_;
        // The position of the affected window at the start of a window
        // move/resize.
        Position<int> drag_start_frame_pos_;
        // The size of the affected window at the start of a window move/resize.
        Size<int> drag_start_frame_size_;

        // Atom constants.
        Atom WM_PROTOCOLS;
        Atom WM_DELETE_WINDOW;




    public: // Public variables


    private: // Private methods

        // Create connection to X server
        Display* createConnection(const std::string& displayName);

        // Remove copy semantics
        WindowManager(const WindowManager&) = delete;
        WindowManager& operator=(const WindowManager&) = delete;

        // Move copy constructor
        WindowManager(WindowManager&& wm);
        // Move assignment operator
        WindowManager& operator=(WindowManager&& wm);

        // Xlib error handler. It must be static as its address is passed to Xlib.
        static int OnXError(Display* display, XErrorEvent* e);

        // Xlib error handler used to determine whether another window manager is
        // running. It is set as the error handler right before selecting substructure
        // redirection mask on the root window, so it is invoked if and only if
        // another window manager is running. It must be static as its address is
        // passed to Xlib.
        static int OnWMDetected(Display* display, XErrorEvent* e);

        // Frame top level window
        void Frame(Window w, bool was_created_before_window_manager);

        // Unframe top level window
        void Unframe(Window w);


    //------------------------------------------------------------------//
    //                              Events                              //
    //------------------------------------------------------------------//

    // When a client want to create window
    void OnCreateNotify(const XCreateWindowEvent& e);

    // When a client want to configure a window, set size, position...
    void OnConfigureRequest(const XConfigureRequestEvent& e);

    // Make a client window visible
    void OnMapRequest(const XMapRequestEvent& e);

    // when a client re parent window
    void OnReparentNotify(const XReparentEvent& e);

    // Tell me if a client window has been mapped
    void OnMapNotify(const XMapEvent& e);

    // Notify me if when a window is re configured
    void OnConfigureNotify(const XConfigureEvent& e);

    // Notify me if a window want to hide
    void OnUnmapNotify(const XUnmapEvent& e);

    // Notify me if a client window is destroyed
    void OnDestroyNotify(const XDestroyWindowEvent& e);

    void OnButtonPress(const XButtonEvent& e);
    void OnButtonRelease(const XButtonEvent& e);
    void OnMotionNotify(const XMotionEvent& e);
    void OnKeyPress(const XKeyEvent& e);
    void OnKeyRelease(const XKeyEvent& e);



    public: // Public methods

        WindowManager(const std::string& displayName = std::string{});

        // Disconnects from the X server.
        ~WindowManager();
        // The entry point to this class. Enters the main event loop.
        void Run();

    };
}
#endif
