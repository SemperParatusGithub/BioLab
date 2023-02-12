#include "Application.h"
#include "Util/Console.h"

#include "Core.h"


namespace BioLab
{
    int Main(int argc, char** argv)
    {
        Console::Create();

        Application* app = new Application();
        app->Run();
        delete app;

        Console::Release();

        return 0;
    }
}


#ifdef BIOLAB_PLATFORM_WINDOWS

INT WINAPI WinMain(
    HINSTANCE hInstance, 
    HINSTANCE hPrevInstance,
    PSTR lpCmdLine, 
    INT nCmdShow)
{
    return BioLab::Main(__argc, __argv);
}

#else
    #error "Unsupported Platform"
#endif