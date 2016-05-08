#ifndef VAOTRIANGLE_H
#define VAOTRIANGLE_H

#include <ngl/Vec3.h>
#include <vector>
#include <memory>
#include <ngl/VertexArrayObject.h>


class VaoTriangle
{
public:
    //in case of a sphere we check against the following
    int m_id;
    std::unique_ptr<ngl::VertexArrayObject> m_vao;
    ngl::Vec3 m_center;
    ngl::Vec3 m_velocity;

    ngl::Vec3 m_verts[3];
    std::vector <ngl::Vec3> normals;

public:
    VaoTriangle(int _id, ngl::Vec3 _center, ngl::Vec3 _velocity, const ngl::Vec3 _verts[3]);
};

#endif // VAOTRIANGLE_H
