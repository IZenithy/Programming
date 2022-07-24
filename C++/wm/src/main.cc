#include "window_manager.h"
#include <iostream>
#include <exception>


int main(void)
{

    try
    {
        WM::WindowManager windowManager{};
        windowManager.Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }


    return 0;
}
