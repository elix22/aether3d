#include "System.hpp"
#include <fstream>
#if _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#endif
#include <cstdarg>
#include <cassert>
#include "GfxDevice.hpp"
#include "Renderer.hpp"

extern void nsLog(const char* msg);
extern ae3d::Renderer renderer;

void ae3d::System::Deinit()
{
    ae3d::GfxDevice::ReleaseGPUObjects();
}

void ae3d::System::EnableWindowsMemleakDetection()
{
#if _MSC_VER
    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
}

void ae3d::System::LoadBuiltinAssets()
{
    renderer.builtinShaders.Load();
}

ae3d::System::FileContentsData ae3d::System::FileContents(const char* path)
{
    ae3d::System::FileContentsData outData;

    std::ifstream ifs(path, std::ios::binary);

    outData.data.assign(std::istreambuf_iterator< char >(ifs), std::istreambuf_iterator< char >());
    outData.path = std::string(path);
    outData.isLoaded = ifs.is_open();

    if (!outData.isLoaded)
    {
        Print( "Could not open %s", path );
    }
    
    return outData;
}

void ae3d::System::Print(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);

    static char msg[512];
#if _MSC_VER
    vsnprintf_s(msg, sizeof(msg), format, ap);
#else
    vsnprintf(msg, sizeof(msg), format, ap);
#endif
    va_end(ap);
    std::printf("%s\n", msg);
#if _MSC_VER
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");
#endif
#if __APPLE__
    //nsLog(msg);
#endif
    std::fflush(stdout);
}

void ae3d::System::Assert(bool condition, const char* message)
{
    if (!condition)
    {
        Print("Assertion failed: %s", message);

#ifdef _MSC_VER
        __debugbreak();
#else
        assert(false);
#endif
    }
}
