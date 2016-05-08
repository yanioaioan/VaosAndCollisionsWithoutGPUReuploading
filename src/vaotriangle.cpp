#include "vaotriangle.h"
#include <iostream>
#include <ngl/VertexArrayObject.h>
#include <ngl/ShaderLib.h>

VaoTriangle::VaoTriangle(int _id, ngl::Vec3 _center, ngl::Vec3 _velocity , const ngl::Vec3 _verts[3])
{
    m_id = _id;
    m_center =_center;
    m_velocity = _velocity;


    m_verts[0]=_verts[0];//fbl 0
    m_verts[1]=_verts[1];//fbr 1
    m_verts[2]=_verts[2];//bbr 5



    //1st face normals-bottom
    ngl::Vec3 n=ngl::calcNormal(m_verts[1],m_verts[0],m_verts[2]);
    normals.push_back(n);
    normals.push_back(n);
    normals.push_back(n);

    std::cout<<"sizeof(verts) "<<sizeof(m_verts)<<" sizeof(ngl::Vec3) "<<sizeof(ngl::Vec3)<<"\n";
    // create a vao as a series of GL_TRIANGLES
    m_vao.reset(ngl::VertexArrayObject::createVOA(GL_TRIANGLES));
    m_vao->bind();

    // in this case we are going to set our data as the vertices above

      m_vao->setData(sizeof(m_verts),m_verts[0].m_x);
      // now we set the attribute pointer to be 0 (as this matches vertIn in our shader)

      m_vao->setVertexAttributePointer(0,3,GL_FLOAT,0,0);

      m_vao->setData(normals.size()*sizeof(ngl::Vec3),normals[0].m_x);
      // now we set the attribute pointer to be 2 (as this matches normal in our shader)

      m_vao->setVertexAttributePointer(2,3,GL_FLOAT,0,0);

      m_vao->setNumIndices(sizeof(m_verts)/sizeof(ngl::Vec3));

   // now unbind
    m_vao->unbind();

}
