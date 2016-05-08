#ifndef COLLISIONSTRUCTURE_H
#define COLLISIONSTRUCTURE_H

#include <ngl/Vec3.h>

class CollisionStructure
{    
public:
    //in case of a sphere we check against the following
    int m_id;
    ngl::Vec3 m_center;
    float m_radius;
    ngl::Vec3 m_velocity;

public:
    CollisionStructure(int _id, ngl::Vec3 _center, float _radius, ngl::Vec3 _velocity);
    ~CollisionStructure();
};

#endif // COLLISIONSTRUCTURE_H
