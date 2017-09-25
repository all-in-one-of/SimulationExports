#include "Emitter.h"
#include <ngl/Random.h>
#include <ngl/Transformation.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleVAO.h>
#include <ngl/Logger.h>
#include <ngl/NGLStream.h>
#include <QElapsedTimer>
#include <boost/format.hpp>
#include <array>
#include "VRayExporter.h"

/// @brief ctor
/// @param _pos the position of the emitter
/// @param _numParticles the number of particles to create
Emitter::Emitter(ngl::Vec3 _pos, unsigned int _numParticles, ngl::Vec3 *_wind )
{
  m_wind=_wind;
  Particle p;
  GLParticle g;
  ngl::Random *rand=ngl::Random::instance();
  ngl::Logger *log = ngl::Logger::instance();
  log->logMessage("Starting emitter ctor\n");
  QElapsedTimer timer;
  timer.start();
  m_pos=_pos;
  m_particles.reset(  new Particle[_numParticles]);
  m_glparticles.reset( new GLParticle[_numParticles]);
  m_vao.reset( ngl::VAOFactory::createVAO(ngl::simpleVAO,GL_POINTS));
  float pointOnCircleX= cosf(ngl::radians(m_time))*4.0f;
  float pointOnCircleZ= sinf(ngl::radians(m_time))*4.0f;
  ngl::Vec3 end(pointOnCircleX,2.0,pointOnCircleZ);
  end=end-m_pos;

  #pragma omp parallel for ordered schedule(dynamic)
  for (unsigned int i=0; i< _numParticles; ++i)
  {
    g.px=p.m_px=m_pos.m_x;
    g.py=p.m_py=m_pos.m_y;
    g.pz=p.m_pz=m_pos.m_z;
    ngl::Colour c=rand->getRandomColour();
    p.m_r=g.pr=c.m_r;
    p.m_g=g.pg=c.m_g;
    p.m_b=g.pb=c.m_b;

    p.m_dx=end.m_x+rand->randomNumber(2)+0.5f;
    p.m_dy=end.m_y+rand->randomPositiveNumber(10)+0.5f;
    p.m_dz=end.m_z+rand->randomNumber(2)+0.5f;
    p.m_gravity=-9.0f;
    p.m_currentLife=0.0f;
    m_particles[i]=p;
    m_glparticles[i]=g;
  }
  m_numParticles=_numParticles;
  m_vao->bind();
  // now copy the data
  m_vao->setData(ngl::SimpleVAO::VertexData(m_numParticles*sizeof(GLParticle),m_glparticles[0].px));
  m_vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(GLParticle),0);
  m_vao->setVertexAttributePointer(1,3,GL_FLOAT,sizeof(GLParticle),3);
  m_vao->setNumIndices(m_numParticles);
  m_vao->unbind();
  log->logMessage("Finished filling array took %d milliseconds\n",timer.elapsed());
  /// @note this demo is based on alembic/lib/Alembic/AbcGeom/Tests/PointsTest.cpp

}


Emitter::~Emitter()
{
}

/// @brief a method to update each of the particles contained in the system
void Emitter::update()
{
  QElapsedTimer timer;
  timer.start();
  ngl::Logger *log = ngl::Logger::instance();
  log->logMessage("Starting emitter update\n");

  m_vao->bind();
  ngl::Real *glPtr=m_vao->mapBuffer();
  unsigned int glIndex=0;
  #pragma omp parallel for
  static int rot=0;
  static float time=0.0;
  float pointOnCircleX= cosf(ngl::radians(time))*4.0f;
  float pointOnCircleZ= sinf(ngl::radians(time))*4.0f;
  ngl::Vec3 end(pointOnCircleX,2.0f,pointOnCircleZ);
  end=end-m_pos;
  //end.normalize();
  time+=m_time;

  for(unsigned int i=0; i<m_numParticles; ++i)
  {
    m_particles[i].m_currentLife+=0.002f;
    // use projectile motion equation to calculate the new position
    // x(t)=Ix+Vxt
    // y(t)=Iy+Vxt-1/2gt^2
    // z(t)=Iz+Vzt

    m_particles[i].m_px=m_pos.m_x+(m_wind->m_x*m_particles[i].m_dx*m_particles[i].m_currentLife);
    m_particles[i].m_py= m_pos.m_y+(m_wind->m_y*m_particles[i].m_dy*m_particles[i].m_currentLife)+m_particles[i].m_gravity*(m_particles[i].m_currentLife*m_particles[i].m_currentLife);
    m_particles[i].m_pz=m_pos.m_z+(m_wind->m_z*m_particles[i].m_dz*m_particles[i].m_currentLife);
    glPtr[glIndex]=m_particles[i].m_px;
    glPtr[glIndex+1]=m_particles[i].m_py;
    glPtr[glIndex+2]=m_particles[i].m_pz;
    // if we go below the origin re-set
    if(m_particles[i].m_py <= m_pos.m_y-0.01f)
    {
      ++rot;
      m_particles[i].m_px=m_pos.m_x;
      m_particles[i].m_pz=m_pos.m_y;
      m_particles[i].m_px=m_pos.m_z;

      m_particles[i].m_currentLife=0.0;
      ngl::Random *rand=ngl::Random::instance();
      m_particles[i].m_dx=end.m_x+rand->randomNumber(2)+0.5f;
      m_particles[i].m_dy=end.m_y+rand->randomPositiveNumber(10)+0.5f;
      m_particles[i].m_dz=end.m_z+rand->randomNumber(2)+0.5f;

      glPtr[glIndex]=m_particles[i].m_px;
      glPtr[glIndex+1]=m_particles[i].m_py;
      glPtr[glIndex+2]=m_particles[i].m_pz;
      ngl::Colour c=rand->getRandomColour();
      glPtr[glIndex+3]=c.m_r;
      glPtr[glIndex+4]=c.m_g;
      glPtr[glIndex+5]=c.m_b;
      m_particles[i].m_r=c.m_r;
      m_particles[i].m_g=c.m_g;
      m_particles[i].m_b=c.m_b;

    }
    #pragma omp atomic
    glIndex+=6;

  }
  m_vao->unmapBuffer();

  m_vao->unbind();

 // log->logMessage("Finished update array took %d milliseconds\n",timer.elapsed());
}
/// @brief a method to draw all the particles contained in the system
void Emitter::draw(const ngl::Mat4 &_rot)
{
  QElapsedTimer timer;
  timer.start();
  ngl::Logger *log = ngl::Logger::instance();
  log->logMessage("Starting emitter draw\n");

  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(getShaderName());

  ngl::Mat4 vp=m_cam->getVPMatrix();

  shader->setUniform("MVP",vp*_rot);

  m_vao->bind();
  m_vao->draw();
  m_vao->unbind();

  log->logMessage("Finished draw took %d milliseconds\n",timer.elapsed());
  if(m_export==true)
  {
     timer.restart();
     exportFrame();
     log->logMessage("Alembic Export took %d milliseconds\n",timer.elapsed());
  }
}

void Emitter::exportFrame()
{
  static int s_frame=0;
  char fname[50];
  std::sprintf(fname,"scenes/particle.%03d.vrscene",s_frame++);
  VRayExporter scene(fname);
  ngl::Transformation t;
  t.setPosition(0,0,-2);
  scene.renderView(m_cam->getViewMatrix());
  scene.setImageSize(1024,720);
  scene.setFOV(ngl::radians(45.0f));
  scene.setBGColour(ngl::Colour(1,1,1));
  scene.includeFile("sceneSetup.vrscene");
  std::vector<ngl::Vec3> positions;
  std::vector<ngl::Colour> colours;
  for(unsigned int i=0; i<m_numParticles; ++i)
  {
    positions.push_back(ngl::Vec3(m_particles[i].m_px,m_particles[i].m_py,m_particles[i].m_pz));
    colours.push_back(ngl::Colour(1,1,0));//m_particles[i].m_r,m_particles[i].m_g,m_particles[i].m_b));
  }
  scene.writeGeomParticle("particles",positions,colours,VRayExporter::PointModes::points,0);
  ngl::Mat4 scale;
 // scale.scale(0.1f,0.1f,0.1f);
  scene.writeNode("ParticleNode","particles","particleBRDF",scale);
  scene.writeObj("Cube","cube.obj");
  t.setPosition(0.0f,-0.1f,0.0f);
  t.setScale(20.0f,0.1f,20.0f);
  t.setRotation(0.0f,0.0f,0.0f);

  scene.writeNode("floor","Cube","Floor_brdf",t.getMatrix());

}
