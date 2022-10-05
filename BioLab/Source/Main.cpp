#include "Application.h"
#include "Console.h"

#include <Windows.h>
#include <iostream>


int APIENTRY WinMain(
    HINSTANCE   hInst,
    HINSTANCE   hInstPrev, 
    LPSTR       cmdline, 
    int         cmdshow)
{
    Console::Create();

    Application* app = new Application();
    app->Run();
    delete app;

	std::cin.get();

    Console::Release();

    return EXIT_SUCCESS;
}