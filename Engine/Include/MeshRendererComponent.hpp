#ifndef MESH_RENDERER_COMPONENT
#define MESH_RENDERER_COMPONENT

#include <vector>

namespace ae3d
{
    /// Contains a Mesh. The game object must also contain a TransformComponent to be able to render the mesh.
    class MeshRendererComponent
    {
    public:
        /// \return GameObject that owns this component.
        class GameObject* GetGameObject() const { return gameObject; }

        /// \param enabled True if the component should be rendered, false otherwise.
        void SetEnabled( bool enabled ) { isEnabled = enabled; }

        /// \return Mesh.
        class Mesh* GetMesh() { return mesh; }
        
        /// \param subMeshIndex Sub mesh index.
        /// \return Submesh's material or null if the index is invalid.
        class Material* GetMaterial( int subMeshIndex );

        /// \param material Material.
        /// \param subMeshIndex Sub mesh index.
        void SetMaterial( Material* material, int subMeshIndex );

        /// \param aMesh Mesh.
        void SetMesh( Mesh* aMesh );

        /// \param frame Animation frame. If too high or low, repeats from the beginning using modulo.
        void SetAnimationFrame( int frame ) { animFrame = frame; }
        
        /// \return True, if the mesh will be rendered as a wireframe.
        bool IsWireframe() const { return isWireframe; }

        /// \param enable True, if the mesh will be rendered as a wireframe.
        void EnableWireframe( bool enable ) { isWireframe = enable; }
        
    private:
        friend class GameObject;
        friend class Scene;
        
        enum class RenderType { Opaque, Transparent };
        
        /// \return Component's type code. Must be unique for each component type.
        static int Type() { return 5; }
        
        /// \return Component handle that uniquely identifies the instance.
        static unsigned New();
        
        /// \return Component at index or null if index is invalid.
        static MeshRendererComponent* Get( unsigned index );
        
        /// \param cameraFrustum cameraFrustum
        /// \param localToWorld Local-to-World matrix
        void Cull( const class Frustum& cameraFrustum, const struct Matrix44& localToWorld );
        
        /// \param localToView Model-view matrix.
        /// \param localToClip Model-view-projection matrix.
        /// \param localToWorld Transforms mesh AABB from mesh-local space into world-space.
        void Render( const struct Matrix44& localToView, const Matrix44& localToClip, const Matrix44& localToWorld,
                     const Matrix44& shadowView, const Matrix44& shadowProjection, class Shader* overrideShader, RenderType renderType );

        Mesh* mesh = nullptr;
        std::vector< Material* > materials;
        std::vector< bool > isSubMeshCulled;
        GameObject* gameObject = nullptr;
        int animFrame = 0;
        bool isCulled = false;
        bool isWireframe = false;
        bool isEnabled = true;
    };
}

#endif
