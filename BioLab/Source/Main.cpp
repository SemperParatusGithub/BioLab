#include "Application.h"
#include "Console.h"

#include <Windows.h>


int APIENTRY WinMain(
    HINSTANCE   hInst,
    HINSTANCE   hInstPrev, 
    LPSTR       cmdline, 
    int         cmdshow)
{
#ifdef _DEBUG
    Console::Create();
#endif

    Application* app = new Application();
    app->Run();
    delete app;

#ifdef _DEBUG
    std::cin.get();
    Console::Release();
#endif

    return EXIT_SUCCESS;
}