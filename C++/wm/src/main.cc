#include "window_manager.h"
#include <iostream>
#include <exception>


int main(void)
{

    try
    {
        std::unique_ptr<WM::WindowManager> window_manager(WM::WindowManager::Create());
        window_manager->Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }


    return 0;
}
