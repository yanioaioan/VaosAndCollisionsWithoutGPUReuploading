#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Transformation.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Quaternion.h>
#include <ngl/NGLStream.h>


//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=0.01;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=1;

NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0;
  m_spinYFace=0;
  setTitle("Multiple Vaos");
}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";

  m_vao->removeVOA();
  m_vao2->removeVOA();
}

void NGLScene::resizeGL(int _w, int _h)
{
  // set the viewport for openGL
  glViewport(0,0,_w,_h);
  // now set the camera size values as the screen size has changed
  update();
}


void NGLScene::initializeGL()
{
  // we need to initialise the NGL lib which will load all of the OpenGL functions, this must
  // be done once we have a valid GL context but before we call any GL commands. If we dont do
  // this everything will crash
  ngl::NGLInit::instance();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0,1,20);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);

  m_cam= new ngl::Camera(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam->setShape(45,(float)720.0/576.0,0.001,150);

  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // load a frag and vert shaders

  // we are creating a shader called Phong
  shader->createShaderProgram("Phong");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("PhongVertex",ngl::ShaderType::VERTEX);
  shader->attachShader("PhongFragment",ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource("PhongVertex","shaders/PhongVertex.glsl");
  shader->loadShaderSource("PhongFragment","shaders/PhongFragment.glsl");
  // compile the shaders
  shader->compileShader("PhongVertex");
  shader->compileShader("PhongFragment");
  // add them to the program
  shader->attachShaderToProgram("Phong","PhongVertex");
  shader->attachShaderToProgram("Phong","PhongFragment");

  // now we have associated this data we can link the shader
  shader->linkProgramObject("Phong");
  // and make it active ready to load values
  (*shader)["Phong"]->use();
  // the shader will use the currently active material and light0 so set them
  ngl::Material m(ngl::STDMAT::GOLD);
  // load our material values to the shader into the structure material (see Vertex shader)
  m.loadToShader("material");
  shader->setShaderParam3f("viewerPos",m_cam->getEye().m_x,m_cam->getEye().m_y,m_cam->getEye().m_z);
  // now create our light this is done after the camera so we can pass the
  // transpose of the projection matrix to the light to do correct eye space
  // transformations
  ngl::Mat4 iv=m_cam->getViewMatrix();
  iv.transpose();
  iv=iv.inverse();
  ngl::Light l(ngl::Vec3(0,1,0),ngl::Colour(1,1,1,1),ngl::Colour(1,1,1,1),ngl::LightModes::POINTLIGHT);
  l.setTransform(iv);
  // load these values to the shader as well
  l.loadToShader("light");



  //sphere creation
  ngl::VAOPrimitives::instance()->createSphere("sphere",1,40);

  //add a couple of spheres
  for(int i=0;i<2;i++)
  {

     float velocitymultiplier;
     if( (i%2)==0 )
     {
         velocitymultiplier=1;
     }
     else
     {
         velocitymultiplier=-1;
     }
     container.push_back(CollisionStructure(i,ngl::Vec3(i*5,0,0),1, ngl::Vec3(velocitymultiplier/10,0,0)));
  }



  // Triangle creation , add a couple of triangles

  verts[0]=ngl::Vec3(-1.0f, -1.0f, 1.0f);//fbl 0
  verts[1]=ngl::Vec3(1.0f, -1.0f, 1.0f);//fbr 1
  verts[2]=ngl::Vec3(1.0f, -1.0f, -1.0f);//bbr 5
  //create triangle
  triangles.push_back(VaoTriangle(0, ngl::Vec3(-4,0,0), ngl::Vec3(0.1,0,0), verts));//verts is passed by copy

  //set 2nd triangle somewhere else
  verts[0]=ngl::Vec3(-3.0f, -1.0f, 1.0f);//fbl 0
  verts[1]=ngl::Vec3(-1.0f, -1.0f, 1.0f);//fbr 1
  verts[2]=ngl::Vec3(-1.0f, -1.0f, -1.0f);//bbr 5
  triangles.push_back(VaoTriangle(1, ngl::Vec3(4,0,0), ngl::Vec3(-0.1,0,0), verts));//verts is passed by copy



//  buildVAO();

//  verts[0]=ngl::Vec3(-3.0f, -1.0f, 1.0f);//fbl 0
//  verts[1]=ngl::Vec3(-1.0f, -1.0f, 1.0f);//fbr 1
//  verts[2]=ngl::Vec3(-1.0f, -1.0f, -1.0f);//bbr 5

//  buildVAO2();

  glViewport(0,0,width(),height());

  startTimer(2);
}

void NGLScene::buildVAO()
{

     //1st face normals-bottom
     ngl::Vec3 n=ngl::calcNormal(verts[1],verts[0],verts[2]);
     normals.push_back(n);
     normals.push_back(n);
     normals.push_back(n);

     std::cout<<"sizeof(verts) "<<sizeof(verts)<<" sizeof(ngl::Vec3) "<<sizeof(ngl::Vec3)<<"\n";
     // create a vao as a series of GL_TRIANGLES
     m_vao.reset(ngl::VertexArrayObject::createVOA(GL_TRIANGLES));
     m_vao->bind();

     // in this case we are going to set our data as the vertices above

       m_vao->setData(sizeof(verts),verts[0].m_x);
       // now we set the attribute pointer to be 0 (as this matches vertIn in our shader)

       m_vao->setVertexAttributePointer(0,3,GL_FLOAT,0,0);

       m_vao->setData(normals.size()*sizeof(ngl::Vec3),normals[0].m_x);
       // now we set the attribute pointer to be 2 (as this matches normal in our shader)

       m_vao->setVertexAttributePointer(2,3,GL_FLOAT,0,0);

       m_vao->setNumIndices(sizeof(verts)/sizeof(ngl::Vec3));

    // now unbind
     m_vao->unbind();
}

void NGLScene::buildVAO2()
{
     std::vector <ngl::Vec3> normals;

     //1st face normals-bottom
     ngl::Vec3 n=ngl::calcNormal(verts[1],verts[0],verts[2]);
     normals.push_back(n);
     normals.push_back(n);
     normals.push_back(n);

     std::cout<<"sizeof(verts) "<<sizeof(verts)<<" sizeof(ngl::Vec3) "<<sizeof(ngl::Vec3)<<"\n";
     // create a vao as a series of GL_TRIANGLES
     m_vao2.reset(ngl::VertexArrayObject::createVOA(GL_TRIANGLES));
     m_vao2->bind();

     // in this case we are going to set our data as the vertices above

       m_vao2->setData(sizeof(verts),verts[0].m_x);
       // now we set the attribute pointer to be 0 (as this matches vertIn in our shader)

       m_vao2->setVertexAttributePointer(0,3,GL_FLOAT,0,0);

       m_vao2->setData(normals.size()*sizeof(ngl::Vec3),normals[0].m_x);
       // now we set the attribute pointer to be 2 (as this matches normal in our shader)

       m_vao2->setVertexAttributePointer(2,3,GL_FLOAT,0,0);

       m_vao2->setNumIndices(sizeof(verts)/sizeof(ngl::Vec3));

    // now unbind
     m_vao2->unbind();

}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Rotation based on the mouse position for our global transform

  // Rotation based on the mouse position for our global
  // transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_spinXFace);
  rotY.rotateY(m_spinYFace);
  // multiply the rotations
  m_mouseGlobalTX=rotY*rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;


  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["Phong"]->use();
  ngl::Material m(ngl::STDMAT::PEWTER);
  ngl::Material m2(ngl::STDMAT::BRONZE);


  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;

  //draw triangles
    for(auto &s:triangles)
    {
      m_transform.setPosition(s.m_center);
      M=m_transform.getMatrix()*m_mouseGlobalTX;
      MV=  M*m_cam->getViewMatrix();
      MVP= M*m_cam->getVPMatrix();
      normalMatrix=MV;
      normalMatrix.inverse();
      shader->setShaderParamFromMat4("MV",MV);
      shader->setShaderParamFromMat4("MVP",MVP);
      shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
      shader->setShaderParamFromMat4("M",M);

      if (s.m_id==0)
      {
          // load our material values to the shader into the structure material (see Vertex shader)
          m.loadToShader("material");
      }
      else
      {
          // load our material values to the shader into the structure material (see Vertex shader)
          m2.loadToShader("material");
      }
      //ngl::VAOPrimitives::instance()->draw("sphere");

      s.m_vao->bind();
      s.m_vao->draw();
      s.m_vao->unbind();

    }




  //draw spheres
//  for(auto &s:container)
//  {
//    m_transform.setPosition(s.m_center);
//    M=m_transform.getMatrix()*m_mouseGlobalTX;
//    MV=  M*m_cam->getViewMatrix();
//    MVP= M*m_cam->getVPMatrix();
//    normalMatrix=MV;
//    normalMatrix.inverse();
//    shader->setShaderParamFromMat4("MV",MV);
//    shader->setShaderParamFromMat4("MVP",MVP);
//    shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
//    shader->setShaderParamFromMat4("M",M);

//    if (s.m_id==0)
//    {
//        // load our material values to the shader into the structure material (see Vertex shader)
//        m.loadToShader("material");
//    }
//    else
//    {
//        // load our material values to the shader into the structure material (see Vertex shader)
//        m2.loadToShader("material");
//    }
//    //ngl::VAOPrimitives::instance()->draw("sphere");
//  }





//  m_transform.reset();
//  {
//      //    load our material values to the shader into the structure material (see Vertex shader)
//      m.set(ngl::STDMAT::CHROME);
//      m.loadToShader("material");

//      M=m_transform.getMatrix()*m_mouseGlobalTX;
//      MV=  M*m_cam->getViewMatrix();
//      MVP= M*m_cam->getVPMatrix();
//      normalMatrix=MV;
//      normalMatrix.inverse();
//      shader->setShaderParamFromMat4("MV",MV);
//      shader->setShaderParamFromMat4("MVP",MVP);
//      shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
//      shader->setShaderParamFromMat4("M",M);
//      m_vao->bind();
//      m_vao->draw();
//      m_vao->unbind();

//  }

//  m_transform.reset();
//  {
//      //    load our material values to the shader into the structure material (see Vertex shader)
//      m.set(ngl::STDMAT::BRONZE);
//      m.loadToShader("material");

//      //physics update
//      testMOdelToWorldVector.m_x+=0.000001;//(possible vertex position update)
//      m_transform.setPosition(testMOdelToWorldVector);

//      //physics transformation
//      ngl::Mat4 physicsModelToWorlsTransformationMatrix=m_transform.getMatrix();

//      //now transform all vertices of the model based on the physicsModelToWorlsTransformationMatrix
//      testMOdelToWorldVector=testMOdelToWorldVector*physicsModelToWorlsTransformationMatrix;

//      //now use the testMOdelToWorldVector to do collision detection & response in worldSpace
//      if (testMOdelToWorldVector.m_x>2)
//          testMOdelToWorldVector.m_x=0;



//      //m_transform.setPosition(testMOdelToWorldVector);
//      //std::cout<<testMOdelToWorldVector.m_x<<std::endl;

//      //now that collision detection is resolved update the VAO with the physics matrix (that is just for drawing)

//      m_transform.setMatrix(physicsModelToWorlsTransformationMatrix);
//      M= m_transform.getMatrix()*m_mouseGlobalTX;
//      MV=  M*m_cam->getViewMatrix();
//      MVP= M*m_cam->getVPMatrix();
//      normalMatrix=MV;
//      normalMatrix.inverse();
//      shader->setShaderParamFromMat4("MV",MV);
//      shader->setShaderParamFromMat4("MVP",MVP);
//      shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
//      shader->setShaderParamFromMat4("M",M);
//      m_vao2->bind();
//      m_vao2->draw();
//      m_vao2->unbind();
//   }

}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent (QMouseEvent * _event)
{
  // note the method buttons() is the button state when event was called
  // this is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx=_event->x()-m_origX;
    int diffy=_event->y()-m_origY;
    m_spinXFace += (float) 0.5f * diffy;
    m_spinYFace += (float) 0.5f * diffx;
    m_origX = _event->x();
    m_origY = _event->y();
    update();

  }
        // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = (int)(_event->x() - m_origXPos);
    int diffY = (int)(_event->y() - m_origYPos);
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();

   }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent ( QMouseEvent * _event)
{
  // this method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if(_event->button() == Qt::LeftButton)
  {
    m_origX = _event->x();
    m_origY = _event->y();
    m_rotate =true;
  }
  // right mouse translate mode
  else if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translate=true;
  }

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent ( QMouseEvent * _event )
{
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotate=false;
  }
        // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate=false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

    // check the diff of the wheel position (0 means no change)
    if(_event->delta() > 0)
    {
        m_modelPos.m_z+=ZOOM;
    }
    else if(_event->delta() <0 )
    {
        m_modelPos.m_z-=ZOOM;
    }
    update();
}
//----------------------------------------------------------------------------------------------------------------------

ngl::Vec3 testMOdelToWorldVector(0,0,0);

static double flow;
void NGLScene::timerEvent( QTimerEvent *_event )
{
//    //update spheres based on physics & check for collisions
//    for(auto &t:container)
//    {
//        //collision check
//        for(auto &s:container)
//        {
//            //when not itself
//            if(s.m_id!=t.m_id)
//            {
//                if( abs((t.m_center-s.m_center).length()) < t.m_radius )//sphere-sphere collision detected
//                {
//                    //reverse both velocities
//                    t.m_velocity=-t.m_velocity;
//                    s.m_velocity=-s.m_velocity;
//                }
//            }
//        }

//        //left right wall collision
//        if (t.m_center.m_x>6 || t.m_center.m_x<-6)
//        {
//            //reverse velocity too
//            t.m_velocity=-t.m_velocity;
//        }

//        t.m_center+=t.m_velocity;
//    }






    //update triangles (their vertices) based on physics & check for collisions
    for(auto &t:triangles)
    {
        //that's the update we intend to do
        //physics update
        testMOdelToWorldVector=t.m_velocity;//(possible vertex position update)
        //so setup a matrix based on it
        ngl::Transformation tmpTrans;
        tmpTrans.setPosition(testMOdelToWorldVector);


        //physics transformation
        ngl::Mat4 physicsModelToWorlsTransformationMatrix=tmpTrans.getMatrix();//tmpTrans is the previous force applied to the object in a way

        //now transform all vertices of the model based on the physicsModelToWorlsTransformationMatrix
//                  testMOdelToWorldVector=testMOdelToWorldVector*physicsModelToWorlsTransformationMatrix;

        //update all vao vertices , but I won't upload them back to GPU as long as it's a uniform transformation based on a matrix
        //just going to use the new v so as to perform collision detection at the new world coordinates
        for(auto &v:t.m_verts)
        {
            ngl::Vec4 tmp(v);

            //Transform all the vertices of the vao with from Model to World space (multiplying with physicsModelToWorlsTransformationMatrix) to perform collision detection
            tmp=tmp*physicsModelToWorlsTransformationMatrix;
            v= tmp.toVec3();
//            std::cout<<v<<std::endl;

        }

        //collision check
        for(auto &s:triangles)
        {
            //when not itself
            if(s.m_id!=t.m_id)
            {
//                if( abs((t.m_center-s.m_center).length()) < t.m_radius )//sphere-sphere collision detected
//                {
//                    //reverse both velocities
//                    t.m_velocity=-t.m_velocity;
//                    s.m_velocity=-s.m_velocity;
//                }

                //check Triangle-Triangle collision based on new transformed vertices
                for(auto &v:s.m_verts)//if any of this triangle's vertices satisfies the following condition reverse the triangle's velocity
                {
                    std::cout<<v<<std::endl;

                    if (v.m_x >4 || v.m_x <-4)
                    {
                        //reverse velocity too
                        s.m_velocity=-s.m_velocity;
                        break;
                    }
                }



                  //tmpTrans.setPosition(testMOdelToWorldVector);
                  //std::cout<<testMOdelToWorldVector.m_x<<std::endl;

                  //now that collision detection is resolved update the VAO with the physics matrix (that is just for drawing)
//                  m_transform.setMatrix(physicsModelToWorlsTransformationMatrix);

            }
        }

        //left right wall collision
//        if (t.m_center.m_x>6 || t.m_center.m_x<-6)
//        {
//            //reverse velocity too
//            t.m_velocity=-t.m_velocity;
//        }

        t.m_center+=t.m_velocity;
    }







//    //update 1st vbo vertices and normals and reload it to GPU
//    m_vao->bind();
//    //update vertices
//    verts[0].m_x+=sin(flow+=0.005)/100;
//    verts[1].m_y+=cos(flow+=0.002)/100;
//    verts[2].m_z+=cos(flow+=0.003)/100;
////    std::cout<<verts[0].m_x<<std::endl;

//    m_vao->updateData(sizeof(verts), verts[0].m_x );

//    //update normals
//    ngl::Vec3 n=ngl::calcNormal(verts[1],verts[2],verts[0]);
//    normals[0]=n;normals[1]=n;normals[2]=n;

//    m_vao->updateIndexedData(normals.size()*sizeof(ngl::Vec3), normals[0].m_x );

//    //update 2nd vbo vertices and normals and reload it to GPU


   update();
}

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // turn on wirframe rendering
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  default : break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    update();
}
