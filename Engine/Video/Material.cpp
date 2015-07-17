#include "Material.hpp"
#include "Texture2D.hpp"
#include "Shader.hpp"

bool ae3d::Material::IsValidShader() const
{
    return shader && shader->IsValid();
}

void ae3d::Material::Apply()
{
    if (shader == nullptr)
    {
        return;
    }

    shader->Use();

    for (const auto& vec : vec3s)
    {
        shader->SetVector3( vec.first.c_str(), &vec.second.x );
    }

    for (const auto& vec : vec4s)
    {
        shader->SetVector4( vec.first.c_str(), &vec.second.x );
    }

    for (const auto& i : ints)
    {
        shader->SetInt( i.first.c_str(), i.second );
    }

    for (const auto& f : floats)
    {
        shader->SetFloat( f.first.c_str(), f.second );
    }

    int texUnit = 0;
    
    for (const auto& tex2d : tex2ds)
    {
        shader->SetTexture( tex2d.first.c_str(), tex2d.second, texUnit );
        ++texUnit;
    }

    for (const auto& texCube : texCubes)
    {
        shader->SetTexture( texCube.first.c_str(), texCube.second, texUnit );
        ++texUnit;
    }

    for (const auto& mat4 : mat4s)
    {
        shader->SetMatrix( mat4.first.c_str(), mat4.second.m );
    }
}

void ae3d::Material::SetMatrix( const char* name, const Matrix44& matrix )
{
    mat4s[ name ] = matrix;
}

void ae3d::Material::SetShader( Shader* aShader )
{
    shader = aShader;
}

void ae3d::Material::SetTexture( const char* name, Texture2D* texture )
{
    tex2ds[ name ] = texture;
}

void ae3d::Material::SetTexture( const char* name, TextureCube* texture )
{
    texCubes[ name ] = texture;
}

void ae3d::Material::SetInt( const char* name, int value )
{
    ints[ name ] = value;
}

void ae3d::Material::SetFloat( const char* name, float value )
{
    floats[ name ] = value;
}

void ae3d::Material::SetVector( const char* name, const Vec3& vec )
{
    vec3s[ name ] = vec;
}

void ae3d::Material::SetVector( const char* name, const Vec4& vec )
{
    vec4s[ name ] = vec;
}
