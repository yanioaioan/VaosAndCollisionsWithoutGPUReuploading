#include "Collisionstructure.h"

CollisionStructure::CollisionStructure(int _id, ngl::Vec3 _center, float _radius, ngl::Vec3 _velocity)
{
    m_id = _id;
    m_center =_center;
    m_radius =_radius;
    m_velocity = _velocity;
}

CollisionStructure::~CollisionStructure(){}
