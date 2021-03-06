#include "FileSystem.hpp"
#include "System.hpp"
#include <algorithm>
#include <fstream>
#include <vector>
#if VK_USE_PLATFORM_ANDROID_KHR
#include <android/asset_manager.h>
#endif

#if RENDERER_METAL
const char* GetFullPath( const char* fileName )
{
    std::string nameWithSlash( "/" );
    nameWithSlash += fileName;
    std::replace( std::begin( nameWithSlash ), std::end( nameWithSlash ), '\\', '/' );

    NSBundle *b = [NSBundle mainBundle];
    NSString *dir = [b resourcePath];
    NSString* fName = [NSString stringWithUTF8String: nameWithSlash.c_str()];
    dir = [dir stringByAppendingString:fName];
    return [dir fileSystemRepresentation];
}
#else
const char* GetFullPath( const char* fileName )
{
    static std::string fName;
    fName = fileName;
    std::replace( std::begin( fName ), std::end( fName ), '\\', '/' );
    return fName.c_str();
}
#endif

struct PakFile
{
    struct FileEntry
    {
        std::string path;
        std::vector< unsigned char > contents;
    };

    std::vector< FileEntry > entries;
    std::string path;
};

namespace Global
{
    std::vector< PakFile > pakFiles;
}

#if VK_USE_PLATFORM_ANDROID_KHR
extern AAssetManager* assetManager;

ae3d::FileSystem::FileContentsData ae3d::FileSystem::FileContents( const char* path )
{
    ae3d::FileSystem::FileContentsData outData;
    outData.path = outData.path = path == nullptr ? "" : std::string( GetFullPath( path ) );

    AAsset* file = AAssetManager_open( assetManager, path, AASSET_MODE_BUFFER );

    if (file != nullptr)
    {
        size_t fileLength = AAsset_getLength( file );
        outData.data.resize( fileLength );
        outData.isLoaded = true;
        AAsset_read( file, outData.data.data(), fileLength );
    }

    return outData;
}
#else
ae3d::FileSystem::FileContentsData ae3d::FileSystem::FileContents( const char* path )
{
    ae3d::FileSystem::FileContentsData outData;
    outData.path = path == nullptr ? "" : std::string( GetFullPath( path ) );

    for (const auto& pakFile : Global::pakFiles)
    {
        for (const auto& entry : pakFile.entries)
        {
            if (entry.path == outData.path)
            {
                outData.data = entry.contents;
                outData.isLoaded = true;
                return outData;
            }
        }
    }

    std::ifstream in( outData.path.c_str(), std::ifstream::ate | std::ifstream::binary );
    outData.isLoaded = in.is_open();

    if (!outData.isLoaded)
    {
        System::Print( "FileSystem: Could not open %s.\n", outData.path.c_str() );
        return outData;
    }

    const std::size_t size = (std::size_t)in.tellg();
    outData.data.resize( size );
    in.seekg( std::ifstream::beg );
    in.read( (char*)outData.data.data(), outData.data.size() );

    return outData;
}
#endif

void ae3d::FileSystem::LoadPakFile( const char* path )
{
    if (path == nullptr)
    {
        System::Print( "LoadPakFile: path is null\n" );
        return;
    }

    unsigned entryCount = 0;
    std::ifstream ifs( path );
    if (!ifs.is_open())
    {
        System::Print( "LoadPakFile: Could not open %s\n", path );
        return;
    }

    ifs.read( (char*)&entryCount, 4 );
    Global::pakFiles.emplace_back( PakFile() );
    auto& pakFile = Global::pakFiles.back();
    pakFile.entries.resize( entryCount );
    pakFile.path = path;

    for (unsigned i = 0; i < entryCount; ++i)
    {
        auto& entry = pakFile.entries[i];
        char entryPath[ 128 ];
        ifs.read( &entryPath[ 0 ], 128 );
        entry.path = entryPath;
        unsigned entrySize = 0;
        ifs.read( (char*)&entrySize, 4 );
        entry.contents.resize( entrySize );
        ifs.read( (char*)entry.contents.data(), entrySize );
    }
}

void ae3d::FileSystem::UnloadPakFile( const char* path )
{
    int i = 0;

    for (const PakFile& pakFile : Global::pakFiles)
    {
        if (pakFile.path == std::string( path ))
        {
            Global::pakFiles.erase( std::begin( Global::pakFiles ) + i );
            return;
        }
        ++i;
    }
}
