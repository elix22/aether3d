#ifndef RENDER_TEXTURE_H
#define RENDER_TEXTURE_H

#if RENDERER_VULKAN
#include <vulkan/vulkan.h>
#endif
#include "TextureBase.hpp"

namespace ae3d
{
    /// Render texture. Can be 2D or cube map.
    class RenderTexture : public TextureBase
    {
  public:
        /// Data type.
        enum class DataType
        {
            UByte,
            Float,
            R32G32
        };

        /// Destroys graphics API objects.
        static void DestroyTextures();

        /// \return Data type.
        DataType GetDataType() const { return dataType; }
        
        /// \param width Width.
        /// \param height Height.
        /// \param dataType Data type.
        /// \param wrap Wrapping mode.
        /// \param filter Filtering mode.
        /// \param debugName Debug name that is visible in graphics debugging tools.
        void Create2D( int width, int height, DataType dataType, TextureWrap wrap, TextureFilter filter, const char* debugName );

        /// \param dimension Dimension.
        /// \param dataType Data type.
        /// \param wrap Wrapping mode.
        /// \param filter Filtering mode.
        /// \param debugName Debug name that is visible in graphics debugging tools.
        void CreateCube( int dimension, DataType dataType, TextureWrap wrap, TextureFilter filter, const char* debugName );

#if RENDERER_OPENGL
        /// \return FBO.
        unsigned GetFBO() const { return fboId; }
#endif
#if RENDERER_VULKAN
        /// \return frame buffer.
        VkFramebuffer GetFrameBuffer() { return frameBuffer; }

        /// \return color view.
        VkImageView GetColorView() { return color.view; }

        /// \return render pass.
        VkRenderPass GetRenderPass() { return renderPass; }

        /// \return Color image.
        VkImage GetColorImage() { return color.image; }
#endif

  private:

#if RENDERER_OPENGL
        unsigned rboId = 0;
        unsigned fboId = 0;
#endif
#if RENDERER_VULKAN
        void CreateRenderPass();

        VkFramebuffer frameBuffer = VK_NULL_HANDLE;

        struct FrameBufferAttachment
        {
            VkImage image;
            VkDeviceMemory mem;
            VkImageView view;
        };

        FrameBufferAttachment color;
        FrameBufferAttachment depth;
        VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
        VkRenderPass renderPass = VK_NULL_HANDLE;
#endif
        DataType dataType = DataType::UByte;
    };
}
#endif
