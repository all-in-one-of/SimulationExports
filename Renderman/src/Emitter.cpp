#include "Emitter.h"
#include <ngl/Random.h>
#include <ngl/Transformation.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleVAO.h>
#include <ngl/NGLStream.h>
#include <QElapsedTimer>
#include <ngl/RibExport.h>
#include <array>
/// @brief ctor
/// @param _pos the position of the emitter
/// @param _numParticles the number of particles to create
Emitter::Emitter(ngl::Vec3 _pos, unsigned int _numParticles, ngl::Vec3 *_wind )
{
  m_wind=_wind;
  Particle p;
  GLParticle g;
  ngl::Random *rand=ngl::Random::instance();
  ngl::NGLMessage::addMessage("Starting emitter ctor\n");
  QElapsedTimer timer;
  timer.start();
  m_pos=_pos;
  m_particles.reset(  new Particle[_numParticles]);
  m_glparticles.reset( new GLParticle[_numParticles]);
  m_vao=ngl::VAOFactory::createVAO(ngl::simpleVAO,GL_POINTS);

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
    ngl::Vec3 c=rand->getRandomColour3();
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
  // create the VAO and stuff data

  m_vao->bind();
  // now copy the data
  m_vao->setData(ngl::SimpleVAO::VertexData(m_numParticles*sizeof(GLParticle),m_glparticles[0].px));
  m_vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(GLParticle),0);
  m_vao->setVertexAttributePointer(1,3,GL_FLOAT,sizeof(GLParticle),3);
  m_vao->setNumIndices(m_numParticles);
  m_vao->unbind();

 ngl::NGLMessage::addMessage(fmt::format("Finished filling array took %d milliseconds\n",timer.elapsed()));

}


Emitter::~Emitter()
{
}

/// @brief a method to update each of the particles contained in the system
void Emitter::update()
{
  QElapsedTimer timer;
  timer.start();
  ngl::NGLMessage::addMessage("Starting emitter update\n");

  m_vao->bind();
  ngl::Real *glPtr=m_vao->mapBuffer();

  unsigned int glIndex=0;
  static int rot=0;
  static float time=0.0;
  float pointOnCircleX= cosf(ngl::radians(time))*4.0f;
  float pointOnCircleZ= sinf(ngl::radians(time))*4.0f;
  ngl::Vec3 end(pointOnCircleX,2.0f,pointOnCircleZ);
  end=end-m_pos;
  //end.normalize();
  time+=m_time;

  #pragma omp parallel for
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
      ngl::Vec3 c=rand->getRandomColour3();
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

  ngl::NGLMessage::addMessage(fmt::format("Finished update array took %d milliseconds\n",timer.elapsed()));
}
/// @brief a method to draw all the particles contained in the system
void Emitter::draw(const ngl::Mat4 &_view, const ngl::Mat4 &_project,const ngl::Mat4 &_rot)
{
  QElapsedTimer timer;
  timer.start();
  ngl::NGLMessage::addMessage("Starting emitter draw\n");


  ngl::ShaderLib::instance()->setUniform("MVP",_project*_view*_rot);

  m_vao->bind();
  m_vao->draw();
  m_vao->unbind();


  ngl::NGLMessage::addMessage(fmt::format("Finished draw took %d milliseconds\n",timer.elapsed()));
  if(m_export==true)
  {
     timer.restart();
     exportRib(_view);
     ngl::NGLMessage::addMessage(fmt::format("Rib Export took %d milliseconds\n",timer.elapsed()));
  }
}

void Emitter::exportRib(const ngl::Mat4 &_view)
{
  // static frame number for exporting
  static int frame=0;
  std::string fname=fmt::format("ribs/particle.{0:04}.rib",frame );

  // create an ngl::RibExport this can work either as a one shot
  // exporter or we can get it to output by setting frame numbers
  // see the source code to see how it works.
  ngl::RibExport rib(fname);
  // open the rib stream to write (basically a file)
  rib.open();
  // as it is a file we can get a reference to it to write too
  std::fstream &ribStream=rib.getStream();
  // this is out file name
  std::string data=fmt::format("particle.{0:04}.tiff", frame);
  // note the escaped quotes \" if needed we could use new C++ 11
  // raw string literals instead
  ribStream<<"Display \""<<data<<"\" \"file\" \"rgba\"\n";
  // export the display format again this is user selected
  ribStream<<"Format 1024 720 1\n";
  // projection not we use the ngl::Camera fov
  ribStream<<"Projection \"perspective\" \"uniform float fov\" [45.0] \n ";
  // call world begin (see Rib export as it covers most of rib)
  rib.WorldBegin();
  // camera will write itself out into the rib steam in the correct
  // format, this metho is in ngl::Camera basically it does
  // Scale 1 1 -1
  // ConcatTransform [ view matrix .OpenGL[] ]
  //m_cam->writeRib(rib);
  ribStream<<"Scale 1 1 -1\n";
  ribStream<<"ConcatTransform [" ;
  for(auto f : _view.m_openGL)
    ribStream <<f<<' ';
  ribStream <<"] \n";
  // Now we are dumping the data from our particle system as Points
  // I'm setting the data as Points "vertex point P" [ x y z .....]

  ribStream<< R"(
Pattern "colour" "colourShader"
Bxdf "PxrDiffuse" "bxdf" "reference color diffuseColor" ["colourShader:Cout"]
              )";

  ribStream<<"\nPoints \"vertex point P\" [";
  for(unsigned int i=0; i<m_numParticles; ++i)
  {
    ribStream<<m_particles[i].m_px<<" "<<m_particles[i].m_py<<" "<<m_particles[i].m_pz<<" ";
  }
  ribStream<<"]\n";
  // now dumping the widths (all the same but needed)
  ribStream<<"\"varying float width\" [";
  for(unsigned int i=0; i<m_numParticles; ++i)
  {
    ribStream<<"0.05 ";
  }
  ribStream<<"]\n";

  // now dumping the widths (all the same but needed)
  ribStream<<"\"varying color particleColour\" [";
  for(unsigned int i=0; i<m_numParticles; ++i)
  {
    ribStream<<m_particles[i].m_r<<" "<<m_particles[i].m_g<<" "<<m_particles[i].m_b<<" ";
  }
  ribStream<<"]\n";


  // finally end the world

  rib.WorldEnd();
  // close the rib stream
  rib.close();
  // next frame
  ++frame;
}
