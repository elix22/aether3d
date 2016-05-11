#include "SceneWidget.hpp"
#include <vector>
#include <cmath>
#include <QKeyEvent>
#include <QSurfaceFormat>
#include <QApplication>
#include <QDir>
#include <QGLFormat>
#include <QMessageBox>
#include "System.hpp"
#include "FileSystem.hpp"
#include "TransformComponent.hpp"
#include "Matrix.hpp"
#include "MeshRendererComponent.hpp"
#include "Mesh.hpp"
#include "Vec3.hpp"
#include "MainWindow.hpp"

#include <iostream>

using namespace ae3d;
ae3d::Material* gCubeMaterial = nullptr;

std::string AbsoluteFilePath( const std::string& relativePath )
{
    QDir dir = QDir::currentPath();
#if __APPLE__
    // On OS X the executable is inside .app, so this gets a path outside it.
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#endif
    return dir.absoluteFilePath( relativePath.c_str() ).toStdString();
}

void ScreenPointToRay( int screenX, int screenY, float screenWidth, float screenHeight, GameObject& camera, Vec3& outRayOrigin, Vec3& outRayTarget )
{
    const float aspect = screenHeight / screenWidth;
    const float halfWidth = screenWidth * 0.5f;
    const float halfHeight = screenHeight * 0.5f;
    const float fov = camera.GetComponent< CameraComponent >()->GetFovDegrees() * (3.1415926535f / 180.0f);

    // Normalizes screen coordinates and scales them to the FOV.
    const float dx = std::tan( fov * 0.5f ) * (screenX / halfWidth - 1.0f) / aspect;
    const float dy = std::tan( fov * 0.5f ) * (screenY / halfHeight - 1.0f);

    Matrix44 view;
    camera.GetComponent< TransformComponent >()->GetLocalRotation().GetMatrix( view );
    Matrix44 translation;
    translation.Translate( -camera.GetComponent< TransformComponent >()->GetLocalPosition() );
    Matrix44::Multiply( translation, view, view );

    Matrix44 invView;
    Matrix44::Invert( view, invView );

    const float farp = camera.GetComponent< CameraComponent >()->GetFar();

    outRayOrigin = camera.GetComponent< TransformComponent >()->GetLocalPosition();
    outRayTarget = -Vec3( -dx * farp, dy * farp, farp );

    Matrix44::TransformPoint( outRayTarget, invView, &outRayTarget );
}

template< typename T >
T Min2( T t1, T t2 )
{
    return t1 < t2 ? t1 : t2;
}

template< typename T >
T Max2( T t1, T t2 )
{
    return t1 > t2 ? t1 : t2;
}

float IntersectRayAABB( const Vec3& origin, const Vec3& target, const Vec3& min, const Vec3& max )
{
    const Vec3 dir = (target - origin).Normalized();

    Vec3 dirfrac;
    dirfrac.x = 1.0f / dir.x;
    dirfrac.y = 1.0f / dir.y;
    dirfrac.z = 1.0f / dir.z;

    const float t1 = (min.x - origin.x) * dirfrac.x;
    const float t2 = (max.x - origin.x) * dirfrac.x;
    const float t3 = (min.y - origin.y) * dirfrac.y;
    const float t4 = (max.y - origin.y) * dirfrac.y;
    const float t5 = (min.z - origin.z) * dirfrac.z;
    const float t6 = (max.z - origin.z) * dirfrac.z;

    const float tmin = Max2< float >( Max2< float >( Min2< float >(t1, t2), Min2< float >(t3, t4)), Min2< float >(t5, t6) );
    const float tmax = Min2< float >( Min2< float >( Max2< float >(t1, t2), Max2< float >(t3, t4)), Max2< float >(t5, t6) );

    // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
    if (tmax < 0)
    {
        return -1.0f;
    }

    // if tmin > tmax, ray doesn't intersect AABB
    if (tmin > tmax)
    {
        return -1.0f;
    }

    return tmin;
}

struct CollisionInfo
{
    GameObject* go = nullptr;
    std::vector< int > subMeshIndices;
    float meshDistance = 0;
};

SceneWidget::GizmoAxis CollidesWithGizmo( GameObject& camera, GameObject& gizmo, int screenX, int screenY, int width, int height, float maxDistance )
{
    Vec3 rayOrigin, rayTarget;
    ScreenPointToRay( screenX, screenY, width, height, camera, rayOrigin, rayTarget );

    auto meshRenderer = gizmo.GetComponent< MeshRendererComponent >();
    auto meshLocalToWorld = gizmo.GetComponent< TransformComponent >() ? gizmo.GetComponent< TransformComponent >()->GetLocalMatrix() : Matrix44::identity;

    Vec3 oMin, oMax;
    Matrix44::TransformPoint( meshRenderer->GetMesh()->GetAABBMin(), meshLocalToWorld, &oMin );
    Matrix44::TransformPoint( meshRenderer->GetMesh()->GetAABBMax(), meshLocalToWorld, &oMax );

    const float meshDistance = IntersectRayAABB( rayOrigin, rayTarget, oMin, oMax );

    if (meshDistance < 0 || meshDistance > maxDistance)
    {
        return SceneWidget::GizmoAxis::None;
    }

    const SceneWidget::GizmoAxis axisOrder[] { SceneWidget::GizmoAxis::Z, SceneWidget::GizmoAxis::X, SceneWidget::GizmoAxis::Y };

    for (int axis = 0; axis < 3; ++axis)
    {
        Vec3 aabbMin, aabbMax;
        Matrix44::TransformPoint( meshRenderer->GetMesh()->GetSubMeshAABBMin( axis ), meshLocalToWorld, &aabbMin );
        Matrix44::TransformPoint( meshRenderer->GetMesh()->GetSubMeshAABBMax( axis ), meshLocalToWorld, &aabbMax );
        const float aabbDistance = IntersectRayAABB( rayOrigin, rayTarget, aabbMin, aabbMax );

        if (-1 < aabbDistance && aabbDistance < maxDistance)
        {
            return axisOrder[ axis ];
        }
    }

    return SceneWidget::GizmoAxis::None;
}

std::vector< CollisionInfo > GetColliders( GameObject& camera, const std::vector< std::shared_ptr< ae3d::GameObject > >& gameObjects,
                                           int screenX, int screenY, int width, int height, float maxDistance )
{
    Vec3 rayOrigin, rayTarget;
    ScreenPointToRay( screenX, screenY, width, height, camera, rayOrigin, rayTarget );
    std::vector< CollisionInfo > outInfo;

    // Collects meshes that collide with the ray.
    for (auto& go : gameObjects)
    {
        auto meshRenderer = go->GetComponent< MeshRendererComponent >();

        if (!meshRenderer || !meshRenderer->GetMesh())
        {
            continue;
        }

        auto meshLocalToWorld = go->GetComponent< TransformComponent >() ? go->GetComponent< TransformComponent >()->GetLocalMatrix() : Matrix44::identity;
        Vec3 oMin, oMax;
        Matrix44::TransformPoint( meshRenderer->GetMesh()->GetAABBMin(), meshLocalToWorld, &oMin );
        Matrix44::TransformPoint( meshRenderer->GetMesh()->GetAABBMax(), meshLocalToWorld, &oMax );

        const float meshDistance = IntersectRayAABB( rayOrigin, rayTarget, oMin, oMax );

        if (0 < meshDistance && meshDistance < maxDistance)
        {
            CollisionInfo collisionInfo;
            collisionInfo.go = go.get();
            collisionInfo.meshDistance = meshDistance;
            // TODO: submeshindices
            outInfo.emplace_back( collisionInfo );
        }
    }

    auto sortFunction = [](const CollisionInfo& info1, const CollisionInfo& info2) { return info1.meshDistance < info2.meshDistance; };
    std::sort( std::begin( outInfo ), std::end( outInfo ), sortFunction );
    return outInfo;
}

void SceneWidget::TransformGizmo::Init( Shader* shader )
{
    translateTex.Load( FileSystem::FileContents( AbsoluteFilePath("glider.png").c_str() ), TextureWrap::Repeat,
                       TextureFilter::Linear, Mipmaps::None, ColorSpace::RGB, 1 );
    translateMesh.Load( FileSystem::FileContents( AbsoluteFilePath( "cursor_translate.ae3d" ).c_str() ) );

    xAxisMaterial.SetShader( shader );
    xAxisMaterial.SetTexture( "textureMap", &translateTex );
    xAxisMaterial.SetVector( "tint", { 1, 0, 0, 1 } );
    xAxisMaterial.SetBackFaceCulling( true );
    xAxisMaterial.SetDepthFunction( ae3d::Material::DepthFunction::NoneWriteOff );

    yAxisMaterial.SetShader( shader );
    yAxisMaterial.SetTexture( "textureMap", &translateTex );
    yAxisMaterial.SetVector( "tint", { 0, 1, 0, 1 } );
    yAxisMaterial.SetBackFaceCulling( true );
    yAxisMaterial.SetDepthFunction( ae3d::Material::DepthFunction::NoneWriteOff );

    zAxisMaterial.SetShader( shader );
    zAxisMaterial.SetTexture( "textureMap", &translateTex );
    zAxisMaterial.SetVector( "tint", { 0, 0, 1, 1 } );
    zAxisMaterial.SetBackFaceCulling( true );
    zAxisMaterial.SetDepthFunction( ae3d::Material::DepthFunction::NoneWriteOff );

    go.AddComponent< MeshRendererComponent >();
    go.GetComponent< MeshRendererComponent >()->SetMesh( &translateMesh );
    go.GetComponent< MeshRendererComponent >()->SetMaterial( &xAxisMaterial, 1 );
    go.GetComponent< MeshRendererComponent >()->SetMaterial( &yAxisMaterial, 2 );
    go.GetComponent< MeshRendererComponent >()->SetMaterial( &zAxisMaterial, 0 );
    go.AddComponent< TransformComponent >();
    go.GetComponent< TransformComponent >()->SetLocalPosition( { 0, 10, -50 } );
}

void SceneWidget::TransformGizmo::SetPosition( const ae3d::Vec3& position )
{
    go.GetComponent< TransformComponent >()->SetLocalPosition( position );
}

SceneWidget::SceneWidget( QWidget* parent ) : QOpenGLWidget( parent )
{
    if (!QGLFormat::hasOpenGL())
    {
        QMessageBox::critical(this,
                        "No OpenGL Support",
                        "Missing OpenGL support! Maybe you need to update your display driver.");
    }

    QSurfaceFormat fmt;
    fmt.setVersion( 4, 1 );
    fmt.setProfile( QSurfaceFormat::CoreProfile );
    fmt.setDepthBufferSize( 24 );
    setFormat( fmt );
    QSurfaceFormat::setDefaultFormat( fmt );
}

void SceneWidget::Init()
{
    System::Assert( mainWindow != nullptr, "mainWindow not set.");

    System::InitGfxDeviceForEditor( width() * devicePixelRatio(), height() * devicePixelRatio() );
    System::LoadBuiltinAssets();

    camera.SetName( "Camera" );
    camera.AddComponent<CameraComponent>();
    camera.GetComponent<CameraComponent>()->SetProjectionType( ae3d::CameraComponent::ProjectionType::Perspective );
    camera.GetComponent<CameraComponent>()->SetProjection( 45, float( width() * devicePixelRatio() ) / (height() * devicePixelRatio()), 1, 400 );
    camera.GetComponent<CameraComponent>()->SetClearColor( Vec3( 0, 0, 0 ) );
    camera.GetComponent<CameraComponent>()->SetLayerMask( ~0x2 );
    camera.AddComponent<TransformComponent>();
    camera.GetComponent<TransformComponent>()->LookAt( { 0, 0, 0 }, { 0, 0, 100 }, { 0, 1, 0 } );

    lightTex.Load( FileSystem::FileContents( AbsoluteFilePath( "light.png" ).c_str() ), TextureWrap::Repeat,
                    TextureFilter::Linear, Mipmaps::None, ColorSpace::RGB, 1 );

    cameraTex.Load( FileSystem::FileContents( AbsoluteFilePath( "camera.png" ).c_str() ), TextureWrap::Repeat,
                    TextureFilter::Linear, Mipmaps::None, ColorSpace::RGB, 1 );

    spriteTex.Load( FileSystem::FileContents( AbsoluteFilePath( "glider.png" ).c_str() ), TextureWrap::Repeat,
                    TextureFilter::Linear, Mipmaps::None, ColorSpace::RGB, 1 );

    cubeMesh.Load( FileSystem::FileContents( AbsoluteFilePath( "textured_cube.ae3d" ).c_str() ) );

    gameObjects.push_back( std::make_shared< GameObject >() );
    gameObjects[ 0 ]->AddComponent< MeshRendererComponent >();
    gameObjects[ 0 ]->GetComponent< MeshRendererComponent >()->SetMesh( &cubeMesh );
    gameObjects[ 0 ]->AddComponent< TransformComponent >();
    gameObjects[ 0 ]->GetComponent< TransformComponent >()->SetLocalPosition( { 0, 0, -20 } );
    gameObjects[ 0 ]->SetName( "Cube" );

    unlitShader.Load( FileSystem::FileContents( AbsoluteFilePath("unlit.vsh").c_str() ),
                      FileSystem::FileContents( AbsoluteFilePath("unlit.fsh").c_str() ), "unlitVert", "unlitFrag",
                      FileSystem::FileContents(""), FileSystem::FileContents(""),
                      FileSystem::FileContents(""), FileSystem::FileContents(""));

    cubeMaterial.SetShader( &unlitShader );
    cubeMaterial.SetTexture( "textureMap", &spriteTex );
    cubeMaterial.SetVector( "tint", { 1, 1, 1, 1 } );
    cubeMaterial.SetBackFaceCulling( true );
    gCubeMaterial = &cubeMaterial;

    gameObjects[ 0 ]->GetComponent< MeshRendererComponent >()->SetMaterial( &cubeMaterial, 0 );

    transformGizmo.Init( &unlitShader );

    AddEditorObjects();
    scene.Add( gameObjects[ 0 ].get() );

    connect( &myTimer, SIGNAL( timeout() ), this, SLOT( UpdateCamera() ) );
    myTimer.start();
    connect(mainWindow, SIGNAL(GameObjectSelected(std::list< ae3d::GameObject* >)),
            this, SLOT(GameObjectSelected(std::list< ae3d::GameObject* >)));

    const ae3d::Vec3 rtDim = { 256, 256, 0 };
    hudCamera.SetName( "HUD Camera" );
    hudCamera.AddComponent<CameraComponent>();
    hudCamera.GetComponent<CameraComponent>()->SetProjection( 0, rtDim.x, 0, rtDim.y, 0, 1 );
    hudCamera.GetComponent<CameraComponent>()->SetProjectionType( CameraComponent::ProjectionType::Orthographic );
    hudCamera.GetComponent<CameraComponent>()->SetClearFlag( CameraComponent::ClearFlag::Depth );
    hudCamera.GetComponent<CameraComponent>()->SetClearColor( Vec3( 0.5f, 0.0f, 0.0f ) );
    hudCamera.GetComponent<CameraComponent>()->SetLayerMask( 0x2 );
    hudCamera.GetComponent<CameraComponent>()->SetRenderOrder( 2 );
    hudCamera.AddComponent<TransformComponent>();

    scene.Add( &hudCamera );

    previewCameraTex.Create2D( rtDim.x, rtDim.y, ae3d::RenderTexture::DataType::UByte, ae3d::TextureWrap::Clamp, ae3d::TextureFilter::Linear );

    previewCamera.AddComponent<CameraComponent>();
    previewCamera.GetComponent<CameraComponent>()->SetProjectionType( CameraComponent::ProjectionType::Perspective );
    previewCamera.GetComponent<CameraComponent>()->SetProjection( 45, float( width() * devicePixelRatio() ) / (height() * devicePixelRatio()), 1, 400 );
    previewCamera.GetComponent<CameraComponent>()->SetClearColor( Vec3( 0, 0, 0 ) );
    previewCamera.GetComponent<CameraComponent>()->SetClearFlag( CameraComponent::ClearFlag::DepthAndColor );
    previewCamera.GetComponent<ae3d::CameraComponent>()->SetTargetTexture( &previewCameraTex );
    previewCamera.AddComponent<TransformComponent>();
    previewCamera.GetComponent<TransformComponent>()->LookAt( { 0, 0, 0 }, { 0, 0, -100 }, { 0, 1, 0 } );
    scene.Add( &previewCamera );

    hud.SetName( "HUD" );
    hud.AddComponent<SpriteRendererComponent>();
    hud.GetComponent<SpriteRendererComponent>()->SetTexture( &previewCameraTex, Vec3( -50, -150, -0.6f ), Vec3( rtDim.x, rtDim.y, 1 ), Vec4( 0.7f, 0.7f, 1, 1 ) );
    hud.SetLayer( 2 );
    scene.Add( &hud );

    //new QShortcut(QKeySequence("Home"), this, SLOT(resetView()));
    //new QShortcut(QKeySequence("Ctrl+Tab"), this, SLOT(togglePreview()));

    emit GameObjectsAddedOrDeleted();
}

void SceneWidget::DrawLightSprites()
{
    const Vec3 cameraPos = camera.GetComponent< TransformComponent >()->GetLocalPosition();

    /*const float distance = (cameraPos - light->GetPosition()).Length();
    const float lerpDistance = 10;
    float opacity = 1;

    if (distance < lerpDistance)
    {
        opacity = distance / lerpDistance;
    }

    const Vec3 screenPoint = editorCamera->ScreenPoint( light->GetPosition() );

    const Vec3 viewDir = editorCamera->ViewDirection();
    const Vec3 lightDir = (light->GetPosition() - cameraPos).Normalized();
    const float viewDotLight = Vec3::Dot( viewDir, lightDir ) ;

    if (viewDotLight <= 0 &&
        screenPoint.x > -lightTex->Width() && screenPoint.y > -lightTex->Height() &&
        screenPoint.x < renderer->Width() && screenPoint.y < renderer->Height())
    {
        const Vec3 color = static_cast<Light*>( light )->GetColor();
        const float size = renderer->Height() / distance;

        renderer->Draw(lightTex, screenPoint.x, screenPoint.y, size, size, Vec4( color.x, color.y, color.z, opacity ) );
    }*/
}

void SceneWidget::RemoveEditorObjects()
{
    scene.Remove( &camera );
    scene.Remove( &previewCamera );
    scene.Remove( &hudCamera );
    scene.Remove( &hud );
}

void SceneWidget::AddEditorObjects()
{
    scene.Add( &camera );
    scene.Add( &previewCamera );
    scene.Add( &hudCamera );
    scene.Add( &hud );
}

void SceneWidget::SetSelectedCameraTargetToPreview()
{
    if (!selectedGameObjects.empty())
    {
        gameObjects[ selectedGameObjects.front() ]->GetComponent< ae3d::CameraComponent >()->SetTargetTexture( &previewCameraTex );
    }
}

void SceneWidget::initializeGL()
{
    Init();
}

void SceneWidget::updateGL()
{
    update();
}

void SceneWidget::paintGL()
{
    scene.Render();
    DrawLightSprites();
}

void SceneWidget::resizeGL( int width, int height )
{
    System::InitGfxDeviceForEditor( width * devicePixelRatio(), height * devicePixelRatio() );
    camera.GetComponent<CameraComponent>()->SetProjection( 45, float( width * devicePixelRatio() ) / (height * devicePixelRatio()), 1, 400 );
}

void SceneWidget::keyPressEvent( QKeyEvent* aEvent )
{
    const bool shift = aEvent->modifiers() & Qt::ShiftModifier;
    const float speed = shift ? 2 : 1;

    if (aEvent->key() == Qt::Key_Escape)
    {
        selectedGameObjects.clear();
        emit GameObjectsAddedOrDeleted();
    }
    else if (mouseMode == MouseMode::Grab)
    {
        if (aEvent->key() == Qt::Key_A)
        {
            cameraMoveDir.x = -speed;
        }
        else if (aEvent->key() == Qt::Key_D)
        {
            cameraMoveDir.x = speed;
        }
        else if (aEvent->key() == Qt::Key_Q)
        {
            cameraMoveDir.y = -speed;
        }
        else if (aEvent->key() == Qt::Key_E)
        {
            cameraMoveDir.y = speed;
        }
        else if (aEvent->key() == Qt::Key_W)
        {
            cameraMoveDir.z = -speed;
        }
        else if (aEvent->key() == Qt::Key_S)
        {
            cameraMoveDir.z = speed;
        }
    }
 }

void SceneWidget::keyReleaseEvent( QKeyEvent* aEvent )
{
    const int macDelete = 16777219;

    if (aEvent->key() == Qt::Key_A || aEvent->key() == Qt::Key_D)
    {
        cameraMoveDir.x = 0;
    }
    else if (aEvent->key() == Qt::Key_Q || aEvent->key() == Qt::Key_E)
    {
        cameraMoveDir.y = 0;
    }
    else if (aEvent->key() == Qt::Key_W || aEvent->key() == Qt::Key_S)
    {
        cameraMoveDir.z = 0;
    }
    else if (aEvent->key() == Qt::Key_Delete || aEvent->key() == macDelete)
    {
        std::vector< GameObject* > pendingRemove;

        for (auto i : selectedGameObjects)
        {
            scene.Remove( gameObjects[ i ].get() );
            pendingRemove.push_back( gameObjects[ i ].get() );
        }

        for (std::size_t i = 0; i < selectedGameObjects.size(); ++i)
        {
            for (std::size_t p = 0; p < pendingRemove.size(); ++p)
            {
                if (!gameObjects.empty() && !pendingRemove.empty() && gameObjects[ i ].get() == pendingRemove[ p ])
                {
                    gameObjects.erase( gameObjects.begin() + i );
                    i = 0;
                    p = 0;
                }
            }
        }

        selectedGameObjects.clear();
        emit GameObjectsAddedOrDeleted();
        std::list< ae3d::GameObject* > emptySelection;
        emit static_cast< MainWindow* >(mainWindow)->GameObjectSelected( emptySelection );
    }
    else if (aEvent->key() == Qt::Key_F)
    {
        if (!selectedGameObjects.empty())
        {
            CenterSelected();
        }
    }
}

void SceneWidget::CenterSelected()
{
    if (selectedGameObjects.empty())
    {
        return;
    }

    camera.GetComponent<TransformComponent>()->LookAt( SelectionAveragePosition() + Vec3( 0, 0, 20 ), SelectionAveragePosition(), Vec3( 0, 1, 0 ) );
}

void SceneWidget::mousePressEvent( QMouseEvent* event )
{
    setFocus();

    if (event->button() == Qt::RightButton && mouseMode != MouseMode::Grab)
    {
        mouseMode = MouseMode::Grab;
        setCursor( Qt::BlankCursor );
        lastMousePosition[ 0 ] = QCursor::pos().x();//event->pos().x();
        lastMousePosition[ 1 ] = QCursor::pos().y();//event->pos().y();
        QApplication::setOverrideCursor( Qt::CrossCursor );
        //std::cout << "button down pos: " << lastMousePosition[ 0 ] << ", " << lastMousePosition[ 1 ] << std::endl;
    }
    else if (event->button() == Qt::MiddleButton)
    {
        mouseMode = MouseMode::Pan;
        lastMousePosition[ 0 ] = QCursor::pos().x();
        lastMousePosition[ 1 ] = QCursor::pos().y();
        //cursor().setShape( Qt::ClosedHandCursor );
        //QApplication::setOverrideCursor( Qt::ClosedHandCursor );
    }
    else if (event->button() == Qt::LeftButton)
    {
        dragAxis = CollidesWithGizmo( camera, transformGizmo.go, event->x(), event->y(), width(), height(), 200 );

        if (dragAxis != GizmoAxis::None)
        {
            std::list< ae3d::GameObject* > selectedObjects;

            for (auto& go : selectedGameObjects)
            {
                selectedObjects.push_back( gameObjects[ go ].get() );
            }

            if (selectedObjects.empty())
            {
                return;
            }

            int i = 0;

            for (auto go : selectedObjects)
            {
                auto transform = go->GetComponent< TransformComponent >();

                if (transform)
                {
                    auto pos = transform->GetLocalPosition();
                    auto rot = transform->GetLocalRotation();
                    auto sca = transform->GetLocalScale();
                    emit TransformModified( i, pos, rot, sca );

                    ++i;
                }
            }
        }
    }
}

void SceneWidget::mouseReleaseEvent( QMouseEvent* event )
{
    QApplication::restoreOverrideCursor();

    if (dragAxis != GizmoAxis::None)
    {
        dragAxis = GizmoAxis::None;
        std::list< ae3d::GameObject* > selectedObjects;

        for (auto& go : selectedGameObjects)
        {
            selectedObjects.push_back( gameObjects[ go ].get() );
        }

        emit static_cast<MainWindow*>(mainWindow)->GameObjectSelected( selectedObjects );
        return;
    }

    if (mouseMode == MouseMode::Grab)
    {
        mouseMode = MouseMode::Normal;
        unsetCursor();
    }
    else if (mouseMode == MouseMode::Pan)
    {
        cameraMoveDir.x = 0;
        cameraMoveDir.y = 0;
        mouseMode = MouseMode::Normal;
        cursor().setShape( Qt::ArrowCursor );
    }

    if (event->button() == Qt::LeftButton)
    {
        const QPoint point = mapFromGlobal( QCursor::pos() );
        auto colliders = GetColliders( camera, gameObjects, point.x(), point.y(), width(), height(), 200 );
        selectedGameObjects.clear();
        std::list< ae3d::GameObject* > selectedObjects;

        if (!colliders.empty())
        {
            selectedObjects.push_back( colliders.front().go );

            for (std::size_t i = 0; i < gameObjects.size(); ++i)
            {
                if (gameObjects[ i ].get() == selectedObjects.back())
                {
                    selectedGameObjects.push_back( int( i ) );
                }
            }
        }

        emit static_cast<MainWindow*>(mainWindow)->GameObjectSelected( selectedObjects );
    }
}

void SceneWidget::UpdateTransformGizmoPosition()
{
    transformGizmo.SetPosition( SelectionAveragePosition() );
}

bool SceneWidget::eventFilter( QObject* /*obj*/, QEvent* event )
{
    if (event->type() == QEvent::MouseMove)
    {
        const QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        float deltaX = (lastMousePosition[ 0 ] - QCursor::pos().x()) * 0.1f;
        float deltaY = (lastMousePosition[ 1 ] - QCursor::pos().y()) * 0.1f;

        if (mouseMode == MouseMode::Grab)
        {
            QPoint globalPos = mapToGlobal(QPoint(mouseEvent->x(), mouseEvent->y()));

            int x = QCursor::pos().x();
            int y = QCursor::pos().y();

            if (globalPos.x() < 5)
            {
                x = desktop.geometry().width() - 10;
                cursor().setPos( desktop.geometry().width() - 10, globalPos.y() );
            }
            else if (globalPos.x() > desktop.geometry().width() - 5)
            {
                x = 10;
                cursor().setPos( 10, globalPos.y() );
            }

            if (globalPos.y() < 5)
            {
                cursor().setPos( globalPos.x(), desktop.geometry().height() - 10 );
            }
            else if (globalPos.y() > desktop.geometry().height() - 10 )
            {
                cursor().setPos( globalPos.x(), 10 );
            }

            deltaX = deltaX > 5 ? 5 : deltaX;
            deltaX = deltaX < -5 ? -5 : deltaX;

            deltaY = deltaY > 5 ? 5 : deltaY;
            deltaY = deltaY < -5 ? -5 : deltaY;

            camera.GetComponent< ae3d::TransformComponent >()->OffsetRotate( Vec3( 0.0f, 1.0f, 0.0f ), deltaX );
            camera.GetComponent< ae3d::TransformComponent >()->OffsetRotate( Vec3( 1.0f, 0.0f, 0.0f ), deltaY );

            lastMousePosition[ 0 ] = x;
            lastMousePosition[ 1 ] = y;
            return true;
        }
        else if (mouseMode == MouseMode::Pan)
        {
            cameraMoveDir.x = deltaX * 0.1f;
            cameraMoveDir.y = -deltaY * 0.1f;

            camera.GetComponent< ae3d::TransformComponent >()->MoveRight( cameraMoveDir.x );
            camera.GetComponent< ae3d::TransformComponent >()->MoveUp( cameraMoveDir.y );

            cameraMoveDir.x = cameraMoveDir.y = 0;
            lastMousePosition[ 0 ] = QCursor::pos().x();
            lastMousePosition[ 1 ] = QCursor::pos().y();
            return true;
        }
        else if (dragAxis == GizmoAxis::X || dragAxis == GizmoAxis::Y || dragAxis == GizmoAxis::Z)
        {
            const Vec3 direction = camera.GetComponent<TransformComponent>()->GetViewDirection();

            for (auto goIndex : selectedGameObjects)
            {
                const Vec3 axisMask { dragAxis == GizmoAxis::X ? 1.0f : 0.0f, dragAxis == GizmoAxis::Y ? 1.0f : 0.0f, dragAxis == GizmoAxis::Z ? 1.0f : 0.0f };
                GameObject* go = gameObjects[ goIndex ].get();
                const Vec3 oldPosition = go->GetComponent< TransformComponent >()->GetLocalPosition();

                float xOffset = -deltaX;

                if (xOffset > 1)
                {
                    xOffset = 1;
                }
                if (xOffset < -1)
                {
                    xOffset = -1;
                }

                float zOffset = -xOffset;

                float yOffset = deltaY;

                if (yOffset > 1)
                {
                    yOffset = 1;
                }
                if (yOffset < -1)
                {
                    yOffset = -1;
                }

                const Vec3 newPosition = oldPosition + Vec3( xOffset * direction.z, yOffset, zOffset * direction.x ) * axisMask;
                go->GetComponent< TransformComponent >()->SetLocalPosition( newPosition );
                transformGizmo.SetPosition( newPosition );
                lastMousePosition[ 0 ] = QCursor::pos().x();
                lastMousePosition[ 1 ] = QCursor::pos().y();
            }
        }
        else if (!selectedGameObjects.empty())
        {
            const GizmoAxis axis = CollidesWithGizmo( camera, transformGizmo.go, mouseEvent->x(), mouseEvent->y(), width(), height(), 200 );

            transformGizmo.xAxisMaterial.SetVector( "tint", axis == GizmoAxis::X ? Vec4( 1, 1, 1, 1 ) : Vec4( 1, 0, 0, 1 ) );
            transformGizmo.yAxisMaterial.SetVector( "tint", axis == GizmoAxis::Y ? Vec4( 1, 1, 1, 1 ) : Vec4( 0, 1, 0, 1 ) );
            transformGizmo.zAxisMaterial.SetVector( "tint", axis == GizmoAxis::Z ? Vec4( 1, 1, 1, 1 ) : Vec4( 0, 0, 1, 1 ) );
        }
    }
    else if (event->type() == QEvent::Quit)
    {
        QApplication::quit();
    }

    return false;
}

void SceneWidget::wheelEvent( QWheelEvent* event )
{
    const float dir = event->angleDelta().y() < 0 ? -1 : 1;
    const float speed = dir;
    camera.GetComponent< ae3d::TransformComponent >()->MoveForward( speed );
}

void SceneWidget::UpdateCamera()
{
    const float speed = 0.2f;
    camera.GetComponent< ae3d::TransformComponent >()->MoveRight( cameraMoveDir.x * speed );
    camera.GetComponent< ae3d::TransformComponent >()->MoveUp( cameraMoveDir.y * speed );
    camera.GetComponent< ae3d::TransformComponent >()->MoveForward( cameraMoveDir.z * speed );

    updateGL();
}

void SceneWidget::GameObjectSelected( std::list< GameObject* > gameObjects )
{
    if (gameObjects.empty())
    {
        hud.SetLayer( 0x4 );
        scene.Remove( &transformGizmo.go );
        return;
    }

    scene.Add( &transformGizmo.go );
    transformGizmo.SetPosition( SelectionAveragePosition() );

    auto cameraTransform = gameObjects.front()->GetComponent< TransformComponent >();

    if (gameObjects.front()->GetComponent< CameraComponent >() && cameraTransform)
    {
        hud.SetLayer( 2 );

        Matrix44 eyeView;
        cameraTransform->GetLocalRotation().GetMatrix( eyeView );
        Matrix44 translation;
        translation.Translate( -cameraTransform->GetLocalPosition() );
        Matrix44::Multiply( translation, eyeView, eyeView );

        const Vec3 eyeViewDir = Vec3( eyeView.m[2], eyeView.m[6], eyeView.m[10] ).Normalized();

        previewCamera.GetComponent< TransformComponent >()->LookAt( gameObjects.front()->GetComponent< TransformComponent >()->GetLocalPosition(),
                                                                    eyeViewDir,
                                                                    Vec3( 0, 1, 0 ) );
    }
    else
    {
        hud.SetLayer( 0x4 );
    }
}

ae3d::Vec3 SceneWidget::SelectionAveragePosition()
{
    Vec3 avgPosition;

    for (auto goIndex : selectedGameObjects)
    {
        auto go = gameObjects[ goIndex ];
        auto transform = go->GetComponent< ae3d::TransformComponent >();

        if (transform)
        {
            avgPosition += transform->GetLocalPosition();
        }
    }

    avgPosition /= selectedGameObjects.size();

    return avgPosition;
}
ae3d::GameObject* SceneWidget::CreateGameObject()
{
    gameObjects.push_back( std::make_shared<ae3d::GameObject>() );
    gameObjects.back()->SetName( "Game Object" );
    gameObjects.back()->AddComponent< ae3d::TransformComponent >();
    scene.Add( gameObjects.back().get() );
    selectedGameObjects.clear();
    selectedGameObjects.push_back( (int)gameObjects.size() - 1 );
    return gameObjects.back().get();
}

void SceneWidget::RemoveGameObject( int index )
{
    if (index < (int)gameObjects.size())
    {
        scene.Remove( gameObjects[ index ].get() );
        gameObjects.erase( gameObjects.begin() + index );
    }
}

void SceneWidget::RemoveGameObject( GameObject* gameObject )
{
    int index = 0;

    for (std::size_t i = 0; i < gameObjects.size(); ++i)
    {
        if (gameObjects[ i ].get() == gameObject)
        {
            index = (int)i;
        }
    }

    scene.Remove( gameObject );
    gameObjects.erase( gameObjects.begin() + index );
}

void SceneWidget::LoadSceneFromFile( const char* path )
{
    std::vector< ae3d::GameObject > gos;
    std::map< std::string, class Texture2D* > texture2Ds;
    std::map< std::string, class Material* > materials;
    std::vector< class Mesh* > meshes;
    Scene::DeserializeResult result = scene.Deserialize( ae3d::FileSystem::FileContents( path ), gos, texture2Ds, materials, meshes );

    if (result == Scene::DeserializeResult::ParseError)
    {
        QMessageBox::critical( this,
                        "Scene Parse Error", "There was an error parsing the scene. More info in console." );
    }

    gameObjects.clear();

    for (auto& go : gos)
    {
        gameObjects.push_back( std::make_shared< ae3d::GameObject >() );
        *gameObjects.back() = go;
        scene.Add( gameObjects.back().get() );
    }

    emit GameObjectsAddedOrDeleted();
}
