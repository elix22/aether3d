#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

namespace ae3d
{
    /**
      GameObject is composed of components that define its behavior.
    */
    class GameObject
    {
    public:
        template< class T > void AddComponent()
        {
            components[GetNextComponentIndex()].handle = T::New();
            components[GetNextComponentIndex()].type = T::Type();
        }

        // \return The first component of type T or null if there is no such component.
        template< class T > T* GetComponent()
        {
            for (auto component : components)
            {
                if (T::Type() == component.type)
                {
                    return T::Get(component);
                }
            }

            return nullptr;
        }

    private:
        struct ComponentEntry
        {
            int type = 0;
            int handle = 0;
        };

        int GetNextComponentIndex();

        static const int MaxComponents = 10; // TODO [2015-03-16] constexpr when VS supports it (VS 2015 should)
        ComponentEntry components[MaxComponents];
    };
}
#endif
