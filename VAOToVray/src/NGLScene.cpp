#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/Transformation.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include "VRayExporter.h"
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=0.01f;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=0.1f;
constexpr auto VAOName="troll";
NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in that case the GLFrame passed in on construction)
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0.0f;
  m_spinYFace=0.0f;
  setTitle("Qt5 Simple NGL Demo");
}


NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}


void NGLScene::resizeGL(int _w , int _h)
{
  m_project=ngl::perspective(45.0f,static_cast<float>(_w)/_h,0.05f,350.0f);
  m_width=static_cast<int>(_w*devicePixelRatio());
  m_height=static_cast<int>(_h*devicePixelRatio());
}


void NGLScene::initializeGL()
{
  // we must call that first before any other GL commands to load and link the
  // gl commands from the lib, if that is not done program will crash
  ngl::NGLInit::instance();
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // now to load the shader and set the values
  // grab an instance of shader manager
  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // Now we will create a basic Camera
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(20,20,35);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  m_view=ngl::lookAt(from,to,up);
  m_project=ngl::perspective(45,720.0f/576.0f,0.5f,150);

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
  ngl::Vec4 lightPos(-2.0f,5.0f,2.0f,0.0f);
  shader->setUniform("light.position",lightPos);
  shader->setUniform("light.ambient",0.0f,0.0f,0.0f,1.0f);
  shader->setUniform("light.diffuse",1.0f,1.0f,1.0f,1.0f);
  shader->setUniform("light.specular",0.8f,0.8f,0.8f,1.0f);
  // gold like phong material
  shader->setUniform("material.ambient",0.274725f,0.1995f,0.0745f,0.0f);
  shader->setUniform("material.diffuse",0.75164f,0.60648f,0.22648f,0.0f);
  shader->setUniform("material.specular",0.628281f,0.555802f,0.3666065f,0.0f);
  shader->setUniform("material.shininess",51.2f);
  shader->setUniform("viewerPos",from);

  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->createSphere("sphere",0.5f,50);

  prim->createCylinder("cylinder",0.5f,1.4f,40,40);

  prim->createCone("cone",0.5f,1.4f,20,20);

  prim->createDisk("disk",0.8f,120);
  prim->createTorus("torus",0.15f,0.4f,40,40);
  prim->createTrianglePlane("plane",14,14,80,80,ngl::Vec3(0,1,0));

}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=m_mouseGlobalTX*m_transform.getMatrix();
  MV=  m_view*M;
  MVP=  m_project*MV;
  normalMatrix=MV;
  normalMatrix.inverse().transpose();
  shader->setUniform("MV",MV);
  shader->setUniform("MVP",MVP);
  shader->setUniform("normalMatrix",normalMatrix);
  shader->setUniform("M",M);
}
void NGLScene::paintGL()
{
  glViewport(0,0,m_width,m_height);
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0,m_width,m_height);
  drawScene("Phong");
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent (QMouseEvent * _event)
{
  // note the method buttons() is the button state when event was called
  // that is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx=_event->x()-m_origX;
    int diffy=_event->y()-m_origY;
    m_spinXFace += static_cast<int>( 0.5f * diffy);
    m_spinYFace += static_cast<int>( 0.5f * diffx);
    m_origX = _event->x();
    m_origY = _event->y();
    update();

  }
        // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = static_cast<int>(_event->x() - m_origXPos);
    int diffY = static_cast<int>(_event->y() - m_origYPos);
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
  // that method is called when the mouse button is pressed in this case we
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
  // that event is called when the mouse button is released
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

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // that method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quit
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  // turn on wirframe rendering
#ifndef USINGIOS_

  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
#endif
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  case Qt::Key_E : exportFrame(); break;

  default : break;
  }
 update();
}

struct vertData
{
    ngl::Real u;
    ngl::Real v;
    ngl::Real nx;
    ngl::Real ny;
    ngl::Real nz;
    ngl::Real x;
    ngl::Real y;
    ngl::Real z;
};
void NGLScene::exportFrame()
{
  // as we are accessing the GPU we need to make sure
// we have a clean context to access the data
  makeCurrent();


  VRayExporter scene("scenes/test.vrscene");
  ngl::Mat4 MV=m_view*m_mouseGlobalTX;
 // ngl::Mat4 tx;
//  tx.translate(0,2,2);
  ngl::Transformation tx;
  //tx.setPosition(0,2,-10);
  //tx.setRotation(0,0,0);
  scene.renderView(MV);//tx.getMatrix());
  //scene.renderView(tx.getMatrix());
  scene.setImageSize(width(),height());
  scene.setFOV(ngl::radians(45.0f));
  scene.setBGColour(ngl::Vec3(0.4f,0.4f,0.4f));
  scene.includeFile("sceneSetup.vrscene");
  ngl::Transformation t;
  //t.setPosition(0.0f,-1.0f,0.0f);
  //t.setScale(20.0f,0.1f,20.0f);
  //t.setRotation(0.0f,0.0f,0.0f);
  //scene.writeNode("floor","Cube","Green_brdf",t.getMatrix());
 // t.reset();
 // scene.writeVAO("troll",ngl::VAOPrimitives::instance()->getVAOFromName(VAOName));
 // scene.writeNode("TrollNode","troll","Green_brdf",t.getMatrix());
   drawScene("Phong",true,&scene);
}


void NGLScene::drawScene(const std::string &_shader, bool _export, VRayExporter *_exporter)
{
  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)[_shader]->use();
  // clear the screen and depth buffer
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

   // get the VBO instance and draw the built in teapot
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  constexpr auto brdf="Green_brdf";
  m_transform.reset();
  {
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("teapot");
    }
    else
    {
      _exporter->writeVAO("teapot",prim->getVAOFromName("teapot"));
      _exporter->writeNode("Teapot","teapot",brdf,m_transform.getMatrix());
    }
  }

  m_transform.reset();
  {
    m_transform.setPosition(-3,0.0,0.0);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("sphere");
    }
    else
    {
     // _exporter->writeVAO("sphere",prim->getVAOFromName("sphere"));
     // _exporter->writeNode("Sphere","sphere",brdf,m_transform.getMatrix());
    }
  }

  m_transform.reset();
  {
    m_transform.setPosition(3,0.0,0.0);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("cylinder");
    }
    else
    {
      _exporter->writeVAO("cylinder",prim->getVAOFromName("cylinder"));
      _exporter->writeNode("Cylinder","cylinder",brdf,m_transform.getMatrix());
    }

  }

  m_transform.reset();
  {
    m_transform.setPosition(0.0,0.0,3.0);
    loadMatricesToShader();
    prim->draw("cube");
  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(-3.0,0.0,3.0);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("torus");
    }
    else
    {
      _exporter->writeVAO("torus",prim->getVAOFromName("torus"));
      _exporter->writeNode("Torus","torus",brdf,m_transform.getMatrix());
    }

  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(3.0,0.5,3.0);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("icosahedron");
    }
    else
    {
      _exporter->writeVAO("icosahedron",prim->getVAOFromName("icosahedron"));
      _exporter->writeNode("Icosahedron","icosahedron",brdf,m_transform.getMatrix());
    }

  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(0.0,0.0,-3.0);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("cone");
    }
    else
    {
    //  _exporter->writeVAO("cone",prim->getVAOFromName("cone"));
    //  _exporter->writeNode("Cone","cone",brdf,m_transform.getMatrix());
    }

  } // and before a pop


  m_transform.reset();
  {
    m_transform.setPosition(-3.0,0.5,-3.0);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("tetrahedron");
    }
    else
    {
      _exporter->writeVAO("tetrahedron",prim->getVAOFromName("tetrahedron"));
      _exporter->writeNode("Tetrahedron","tetrahedron",brdf,m_transform.getMatrix());
    }

  } // and before a pop


  m_transform.reset();
  {
    m_transform.setPosition(3.0,0.5,-3.0);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("octahedron");
    }
    else
    {
      _exporter->writeVAO("octahedron",prim->getVAOFromName("octahedron"));
      _exporter->writeNode("Octahedron","octahedron",brdf,m_transform.getMatrix());
    }

  } // and before a pop


  m_transform.reset();
  {
    m_transform.setPosition(0.0,0.5,-6.0);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("football");
    }
    else
    {
      _exporter->writeVAO("football",prim->getVAOFromName("football"));
      _exporter->writeNode("Football","football",brdf,m_transform.getMatrix());
    }

  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(-3.0,0.5,-6.0);
    m_transform.setRotation(0,180,0);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("disk");
    }
    else
    {
      _exporter->writeVAO("disk",prim->getVAOFromName("disk"));
      _exporter->writeNode("Disk","disk",brdf,m_transform.getMatrix());
    }

  } // and before a pop


  m_transform.reset();
  {
    m_transform.setPosition(3.0,0.5,-6.0);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("dodecahedron");
    }
    else
    {
      _exporter->writeVAO("dodecahedron",prim->getVAOFromName("dodecahedron"));
      _exporter->writeNode("Dodecahedron","dodecahedron",brdf,m_transform.getMatrix());
    }

  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(1.0,0.35,1.0);
    m_transform.setScale(1.5,1.5,1.5);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("troll");
    }
    else
    {
      _exporter->writeVAO("troll",prim->getVAOFromName("troll"));
      _exporter->writeNode("Troll","troll",brdf,m_transform.getMatrix());
    }

  } // and before a pop

#ifdef ADDLARGEMODELS
  m_transform.reset();
  {
    m_transform.setPosition(-1.0,-0.5,1.0);
    m_transform.setScale(0.1,0.1,0.1);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("dragon");
    }
    else
    {
      _exporter->writeVAO("dragon",prim->getVAOFromName("dragon"));
      _exporter->writeNode("Dragon","dragon",brdf,m_transform.getMatrix());
    }

  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(-2.5,-0.5,1.0);
    m_transform.setScale(0.1,0.1,0.1);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("buddah");
    }
    else
    {
      _exporter->writeVAO("buddah",prim->getVAOFromName("buddah"));
      _exporter->writeNode("Buddah","buddah",brdf,m_transform.getMatrix());
    }

  } // and before a pop

  m_transform.reset();
  {
    m_transform.setPosition(2.5,-0.5,1.0);
    m_transform.setScale(0.1,0.1,0.1);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("bunny");
    }
    else
    {
      _exporter->writeVAO("bunny",prim->getVAOFromName("bunny"));
      _exporter->writeNode("Bunny","bunny",brdf,m_transform.getMatrix());
    }

  } // and before a pop
#endif

  m_transform.reset();
  {
    m_transform.setPosition(0.0,-0.5,0.0);
    if(!_export)
    {
      loadMatricesToShader();
      prim->draw("plane");
    }
    else
    {
      _exporter->writeVAO("plane",prim->getVAOFromName("plane"));
      _exporter->writeNode("Plane","plane",brdf,m_transform.getMatrix());
    }

  } // and before a pop


}
