#include "PointLightInspector.hpp"
#include "GameObject.hpp"
#include "PointLightComponent.hpp"
#include "System.hpp"
#include <QLabel>
#include <QBoxLayout>
#include <QCheckBox>

void PointLightInspector::Init( QWidget* mainWindow )
{
    auto componentName = new QLabel( "Point Light" );
    auto shadowCheck = new QCheckBox( "Casts Shadow" );

    auto inspectorLayout = new QBoxLayout( QBoxLayout::TopToBottom );
    inspectorLayout->setContentsMargins( 1, 1, 1, 1 );
    inspectorLayout->addWidget( componentName );
    inspectorLayout->addWidget( shadowCheck );

    root = new QWidget();
    root->setLayout( inspectorLayout );

    connect( shadowCheck, SIGNAL(stateChanged(int)), this, SLOT(ShadowStateChanged(int)) );
    connect( mainWindow, SIGNAL(GameObjectSelected(std::list< ae3d::GameObject* >)),
             this, SLOT(GameObjectSelected(std::list< ae3d::GameObject* >)) );
}

void PointLightInspector::ShadowStateChanged( int enabled )
{
    ae3d::System::Assert( gameObject, "Needs game object" );
    ae3d::System::Assert( gameObject->GetComponent< ae3d::PointLightComponent >() != nullptr, "Needs point light" );

    gameObject->GetComponent< ae3d::PointLightComponent >()->SetCastShadow( enabled != 0, 512 );
}

void PointLightInspector::GameObjectSelected( std::list< ae3d::GameObject* > gameObjects )
{
    if (gameObjects.empty())
    {
        gameObject = nullptr;
    }
    else
    {
        gameObject = gameObjects.front();
    }
}
