#include "Console.h"

#include <Windows.h>
#include <io.h>
#include <stdio.h>


bool Console::Create(const Vector2f& consoleSize, const Vector2f& bufferSize)
{
    bool success = true;

    FreeConsole();

    // Creation of a console 
    // https://learn.microsoft.com/en-us/windows/console/creation-of-a-console?source=recommendations

    if (AllocConsole())
    {
        FILE* fileBuf;

        HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
        HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        HANDLE errorHandle = GetStdHandle(STD_ERROR_HANDLE);

        SMALL_RECT cRect = { 0, 0, static_cast<short>(consoleSize.x), static_cast<short>(consoleSize.y) };
        SetConsoleWindowInfo(outputHandle, true, &cRect);

        COORD bSize = { static_cast<short>(bufferSize.x), static_cast<short>(bufferSize.y) };
        SetConsoleScreenBufferSize(outputHandle, bSize);


        // Redirect console output
        // https://stackoverflow.com/questions/191842/how-do-i-get-console-output-in-c-with-a-windows-program

        // input handle
        if (inputHandle != INVALID_HANDLE_VALUE)
        {
            errno_t err = freopen_s(&fileBuf, "CONIN$", "r", stdin);

            if (err == 0)
                setvbuf(stdin, NULL, _IONBF, 0);
            else
                success = false;
        }

        // output handle
        if (outputHandle != INVALID_HANDLE_VALUE)
        {
            errno_t err = freopen_s(&fileBuf, "CONOUT$", "w", stdout);

            if (err == 0)
                setvbuf(stdout, NULL, _IONBF, 0);
            else
                success = false;
        }

        // error handle
        if (errorHandle != INVALID_HANDLE_VALUE)
        {
            errno_t err = freopen_s(&fileBuf, "CONOUT$", "w", stderr);

            if (err == 0)
                setvbuf(stdout, NULL, _IONBF, 0);
            else
                success = false;
        }

        // Make C++ standard streams point to console as well.
        std::ios::sync_with_stdio(true);

        // Clear the error state for each of the C++ standard streams.
        std::cout.clear();
        std::cerr.clear();
        std::cin.clear();

        std::wcout.clear();
        std::wcerr.clear();
        std::wcin.clear();
    }

    return success;
}

bool Console::Release()
{
    bool success = true;
    success = FreeConsole();

    return success;
}