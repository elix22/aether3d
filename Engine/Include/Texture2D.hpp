#ifndef TEXTURE_2D_H
#define TEXTURE_2D_H

#include "TextureBase.hpp"
#if RENDERER_VULKAN
#include <vulkan/vulkan.h>
#endif

namespace DDSLoader
{
    struct Output;
}

namespace ae3d
{
    namespace FileSystem
    {
        struct FileContentsData;
    }
    
    /// 2D texture.
    class Texture2D : public TextureBase
    {
    public:
        /// Gets a default texture that is always available after System::LoadBuiltinAssets().
        static Texture2D* GetDefaultTexture();
        
        /// \param textureData Texture image data. File format must be dds, png, tga, jpg, bmp or bmp.
        /// \param wrap Wrap mode.
        /// \param filter Filter mode.
        /// \param mipmaps Mipmaps. If set to Generate and the image already contains mipmaps, they are used, otherwise mipchain is generated.
        /// \param colorSpace Color space.
        /// \param anisotropy Anisotropy. Value range is 1-16 depending on support. On Metal the value is bucketed into 1, 2, 4, 8 and 16.
        void Load( const FileSystem::FileContentsData& textureData, TextureWrap wrap, TextureFilter filter, Mipmaps mipmaps, ColorSpace colorSpace, Anisotropy anisotropy );
        
        /// \param atlasTextureData Atlas texture image data. File format must be dds, png, tga, jpg, bmp or bmp.
        /// \param atlasMetaData Atlas metadata. Format is Ogre/CEGUI. Example atlas tool: Texture Packer.
        /// \param textureName Name of the texture in atlas.
        /// \param wrap Wrap mode.
        /// \param filter Filter mode.
        /// \param colorSpace Color space.
        /// \param anisotropy Anisotropy. Value range is 1-16 depending on support. On Metal the value is bucketed into 1, 2, 4, 8 and 16.
        void LoadFromAtlas( const FileSystem::FileContentsData& atlasTextureData, const FileSystem::FileContentsData& atlasMetaData, const char* textureName, TextureWrap wrap, TextureFilter filter, ColorSpace colorSpace, Anisotropy anisotropy );

        /// \param imageData Raw pixel data
        /// \param width Width in pixels
        /// \param height Height in pixels
        /// \param channels Channels in the image, must be 3 or 4
        /// \param debugName Null-terminated string of texture's debug name that is visible in graphics debugging tools
        void LoadFromData( const void* imageData, int width, int height, int channels, const char* debugName );
        
#if RENDERER_VULKAN
        VkImageView& GetView() { return view; }
#endif
        /// Destroys all textures. Called internally at exit.
        static void DestroyTextures();

    private:
        /// \param path Path.
        void LoadDDS( const char* path );
        
        /**
          Loads texture from stb_image.c supported formats.

          \param textureData Texture data.
          */
        void LoadSTB( const FileSystem::FileContentsData& textureData );
#if RENDERER_METAL
        void LoadPVRv2( const char* path );
        void LoadPVRv3( const char* path );
#endif
#if RENDERER_VULKAN
        void CreateVulkanObjects( const DDSLoader::Output& mipChain, int bytesPerPixel, VkFormat format );
        void CreateVulkanObjects( void* data, int bytesPerPixel, VkFormat format );
        VkImage image = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
#endif
    };
}
#endif
