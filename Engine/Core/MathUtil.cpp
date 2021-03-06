#include "Vec3.hpp"

using namespace ae3d;

namespace MathUtil
{
    void GetMinMax( const Vec3* aPoints, int count, Vec3& outMin, Vec3& outMax )
    {
        outMin = aPoints[ 0 ];
        outMax = aPoints[ 0 ];

        for (int i = 1, s = count; i < s; ++i)
        {
            const Vec3& point = aPoints[ i ];

            if (point.x < outMin.x)
            {
                outMin.x = point.x;
            }

            if (point.y < outMin.y)
            {
                outMin.y = point.y;
            }

            if (point.z < outMin.z)
            {
                outMin.z = point.z;
            }

            if (point.x > outMax.x)
            {
                outMax.x = point.x;
            }

            if (point.y > outMax.y)
            {
                outMax.y = point.y;
            }

            if (point.z > outMax.z)
            {
                outMax.z = point.z;
            }
        }
    }

    void GetCorners( const Vec3& min, const Vec3& max, Vec3 outCorners[ 8 ] )
    {
        outCorners[ 0 ] = Vec3( min.x, min.y, min.z );
        outCorners[ 1 ] = Vec3( max.x, min.y, min.z );
        outCorners[ 2 ] = Vec3( min.x, max.y, min.z );
        outCorners[ 3 ] = Vec3( min.x, min.y, max.z );
        outCorners[ 4 ] = Vec3( max.x, max.y, min.z );
        outCorners[ 5 ] = Vec3( min.x, max.y, max.z );
        outCorners[ 6 ] = Vec3( max.x, max.y, max.z );
        outCorners[ 7 ] = Vec3( max.x, min.y, max.z );
    }

    float Floor( float f )
    {
        return std::floor( f );
    }

    bool IsNaN( float f )
    {
        return f != f;
    }

    bool IsFinite( float f )
    {
        return std::isfinite( f );
    }

    bool IsPowerOfTwo( unsigned i )
    {
        return ((i & (i - 1)) == 0);
    }

    int Min( int x, int y )
    {
        return x < y ? x : y;
    }

    int Max( int x, int y )
    {
        return x > y ? x : y;
    }

    int GetMipmapCount( int width, int height )
    {
        return 1 + static_cast< int >(std::floor( std::log2( Max( width, height ) ) ));
    }
}
