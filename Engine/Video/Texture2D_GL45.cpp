#include "Texture2D.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.c"
#include "System.hpp"
#include <algorithm>
#include <map>
#include <GL/glxw.h>
#include "GfxDevice.hpp"
#include "FileWatcher.hpp"
#include "DDSLoader.hpp"

extern ae3d::FileWatcher fileWatcher;

namespace Texture2DGlobal
{
    std::map< std::string, ae3d::Texture2D > pathToCachedTexture;
}

void TexReload( const std::string& path )
{
    Texture2DGlobal::pathToCachedTexture[ path ].Load( ae3d::System::FileContents( path.c_str() ));
}

bool HasStbExtension( const std::string& path )
{    
    // Checks for uncompressed formats in texture's file name.
    static const std::string extensions[] =
    {
        ".png", ".PNG", ".jpg", ".JPG", ".tga", ".TGA",
        ".bmp", ".BMP", ".gif", ".GIF"
    };

    const bool extensionFound = std::any_of( std::begin( extensions ), std::end( extensions ),
        [&]( const std::string& extension ) { return path.find( extension ) != std::string::npos; } );
    
    return extensionFound;
}

void ae3d::Texture2D::Load( const System::FileContentsData& fileContents )
{
    const bool isCached = Texture2DGlobal::pathToCachedTexture.find( fileContents.path ) != Texture2DGlobal::pathToCachedTexture.end();

    if (isCached && id == 0)
    {
        *this = Texture2DGlobal::pathToCachedTexture[ fileContents.path ];
        return;
    }
    
    const bool isDDS = fileContents.path.find( ".dds" ) != std::string::npos || fileContents.path.find( ".DDS" ) != std::string::npos;
    
    if (isDDS)
    {
        LoadDDS( fileContents.path.c_str() );
    }
    else if (HasStbExtension( fileContents.path ))
    {
        LoadSTB( fileContents );
    }

    Texture2DGlobal::pathToCachedTexture[ fileContents.path ] = *this;
}

void ae3d::Texture2D::LoadDDS( const char* path )
{
#pragma message("TODO [2015-03-26] Combine similar parts in LoadDDS and LoadSTB when DDS loader enables it.")

    // First load.
    if (id == 0)
    {
        id = GfxDevice::CreateTextureId();
        fileWatcher.AddFile( path, TexReload );
    }
    
    glBindTexture( GL_TEXTURE_2D, id );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    const bool success = DDSLoader::Load( path, 0, width, height, opaque );

    if (!success)
    {
        ae3d::System::Print( "DDS Loader could not load %s", path );
    }
}

void ae3d::Texture2D::LoadSTB( const System::FileContentsData& fileContents )
{
    int components;
    unsigned char* data = stbi_load_from_memory( &fileContents.data[ 0 ], static_cast<int>(fileContents.data.size()), &width, &height, &components, 4 );

    if (data == nullptr)
    {
        const std::string reason( stbi_failure_reason() );
        System::Print( "%s failed to load. stb_image's reason: %s", fileContents.path.c_str(), reason.c_str() );
        return;
    }

    // First load.
    if (id == 0)
    {
        id = GfxDevice::CreateTextureId();
        fileWatcher.AddFile( fileContents.path, TexReload );
    }

    glBindTexture( GL_TEXTURE_2D, id );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
    stbi_image_free( data );
}