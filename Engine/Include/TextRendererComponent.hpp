#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <type_traits>
#include <string>

namespace ae3d
{
    /// Contains text.
    class TextRendererComponent
    {
      public:
        /// Shader type for rendering text.
        enum class ShaderType { Sprite, SDF };
        
        /// Constructor.
        TextRendererComponent();

        /// \param other Other.
        TextRendererComponent( const TextRendererComponent& other );
        
        /// Destructor.
        ~TextRendererComponent();

        /// \param other Other.
        TextRendererComponent& operator=( const TextRendererComponent& other );

        /// \return GameObject that owns this component.
        class GameObject* GetGameObject() const { return gameObject; }

        /// \param enabled True if the component should be rendered, false otherwise.
        void SetEnabled( bool enabled ) { isEnabled = enabled; }

        /// \param color Color in range 0-1.
        void SetColor( const struct Vec4& color );

        /// \param font Font.
        void SetFont( class Font* font );
        
        /// \param text Text. Characters not in font are rendered empty.
        void SetText( const char* text );

        /// \param shaderType Shader type.
        void SetShader( ShaderType shaderType );

        /// \return Textual representation of component.
        std::string GetSerialized() const;

      private:
        friend class GameObject;
        friend class Scene;

        /** \return Component's type code. Must be unique for each component type. */
        static int Type() { return 4; }
        
        /** \return Component handle that uniquely identifies the instance. */
        static unsigned New();
        
        /** \return Component at index or null if index is invalid. */
        static TextRendererComponent* Get( unsigned index );

        /** \param localToClip Transforms screen-space coordinates to clip space. */
        void Render( const float* localToClip );

        struct Impl;
        Impl& m() { return reinterpret_cast<Impl&>(_storage); }
        Impl const& m() const { return reinterpret_cast<Impl const&>(_storage); }
        
        static const std::size_t StorageSize = 1384;
        static const std::size_t StorageAlign = 16;
        
        std::aligned_storage<StorageSize, StorageAlign>::type _storage = {};
        
        GameObject* gameObject = nullptr;
        bool isEnabled = true;
    };
}

#endif
