#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

// C libraries
extern "C"
{
    #include <X11/Xlib.h>
}

// STD
#include <memory>

namespace WM
{
    class WindowManager
    {
    private: // Private variables

        // Handle to the underlying Xlib connection struct.
        // TODO: rename it to connection
        Display* display_;
        // Handle to root window.
        Window root_;

        // Whether an existing window manager has been detected. Set by OnWMDetected,
        // and hence must be static.
        static bool wm_detected_;

    public: // Public variables


    private: // Private methods

        // Invoked internally by Create().
        WindowManager(Display* display);

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


    public: // Public methods

        // Factory method for establishing a connection to an X server and creating a
        // WindowManager instance.
        static std::unique_ptr<WindowManager> Create();

        // Disconnects from the X server.
        ~WindowManager();
        // The entry point to this class. Enters the main event loop.
        void Run();

    };
}
#endif
