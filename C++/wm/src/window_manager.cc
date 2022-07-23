#include "window_manager.h"
#include <iostream>
#include <stdexcept>
#include <string>



namespace WM
{
    // Init static member
    bool WindowManager::wm_detected_{false};

    WindowManager::WindowManager(Display* display)
                            // Return the default root window for a given X server
        : display_{display}, root_{DefaultRootWindow(display_)}
    {

    }

    WindowManager::~WindowManager()
    {
        // Close the connection with X server
        XCloseDisplay(display_);
    }

    // Move copy constructor
    WindowManager::WindowManager(WindowManager&& wm)
    {
        display_ = wm.display_;

        root_ = wm.root_;

        wm.root_ = 0;
        wm.display_ = nullptr;
    }

    // Move assignment operator
    WindowManager& WindowManager::operator=(WindowManager&& wm)
    {
        // Self assignment guard
        if(&wm == this)
            return *this;

        display_ = wm.display_;

        root_ = wm.root_;

        wm.root_ = 0;
        wm.display_ = nullptr;

        return *this;
    }

    std::unique_ptr<WindowManager> WindowManager::Create() {

        // 1. Open X display.
        Display* display = XOpenDisplay(nullptr);

        if (display == nullptr)
        {
            throw std::runtime_error("Failed to open X display " + std::string{XDisplayName(nullptr)});
            return nullptr;
        }

        // 2. Construct WindowManager instance.
        return std::unique_ptr<WindowManager>(new WindowManager(display));

    }

    void WindowManager::Run()
    {
        // 1. Initialization.
        //   a. Select events on root window. Use a special error handler so we can
        //   exit gracefully if another window manager is already running.
        wm_detected_ = false;

        // Setup temporary error handler
        XSetErrorHandler(&WindowManager::OnWMDetected);

        // Take control over the root window
        //            server,  window, events
        XSelectInput(display_, root_, SubstructureRedirectMask | SubstructureNotifyMask);

          /* XSelectInput doesn't  send a request to the X server,
           *
           * instead only queues the request and returns
           * we have to explicitly flush the request queue with XSync
           *
           * False means that XSync  will not discard the events
           * */
        XSync(display_, false);
        if (wm_detected_)
        {
            throw std::runtime_error("Detected another window manager on display " +
                    std::string{XDisplayString(display_)});
            return;
        }

        //   b. Set error handler.
        XSetErrorHandler(&WindowManager::OnXError);

        // 2. Main event loop.
        while(true)
        {
            // 1. Get next event.
            XEvent e;
            XNextEvent(display_, &e);
            // std::cout << "Received event: " << ToString(e);

            // 2. Dispatch event.
            switch (e.type)
            {
                case CreateNotify:
                // OnCreateNotify(e.xcreatewindow);
                break;

                case DestroyNotify:
                // OnDestroyNotify(e.xdestroywindow);
                break;

                case ReparentNotify:
                // OnReparentNotify(e.xreparent);
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
        wm_detected_ = true;
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
            // << " - " <<  XRequestCodeToString(e->request_code) << "\n" TODO: uncomment when you include utlis
            << "    Error code: " << int(e->error_code)
            << " - " << error_text << "\n"
            << "    Resource ID: " << e->resourceid;

        // The return value is ignored.
        return 0;

    }

}
