#include "Application.h"
#include "Util/Console.h"

#include "Core.h"


namespace BioLab
{
    int Main(int argc, char** argv)
    {
#ifdef _DEBUG
        Console::Create();
#endif

        Application* app = new Application();
        app->Run();
        delete app;

#ifdef _DEBUG
        Console::Release();
#endif

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