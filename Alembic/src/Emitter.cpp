#include "Emitter.h"
#include <ngl/Random.h>
#include <ngl/Transformation.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleVAO.h>
#include <ngl/NGLStream.h>
#include <QElapsedTimer>
#include <ngl/RibExport.h>
#include <boost/format.hpp>
#include <array>

namespace AbcG = Alembic::AbcGeom;
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
  m_vao= ngl::VAOFactory::createVAO(ngl::simpleVAO,GL_POINTS);

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
  /// @note this demo is based on alembic/lib/Alembic/AbcGeom/Tests/PointsTest.cpp

  // create an alembic Geometry output archive called particlesOut.abc
  Alembic::AbcCoreAbstract::MetaData md;
//  CreateArchiveWithInfo(
//                  Alembic::AbcCoreOgawa::WriteArchive(), "archiveInfo.abc",
//                  appWriter, userStr, md );
//  m_archive.reset(new  AbcG::OArchive(Alembic::AbcCoreOgawa::WriteArchive(),"particlesOut.abc",md) );
  namespace Abc = Alembic::Abc;
  using namespace Abc;
  OArchive archive;
  std::string appWriter = "ngl Alembic Export";
  std::string userStr = "Simple Demo of exporting points with alembic";

//  m_archive.reset( CreateArchiveWithInfo(
//                    Alembic::AbcCoreOgawa::WriteArchive(), "archiveInfo.abc",
//                    appWriter, userStr, md )) ;
//
   m_archive=CreateArchiveWithInfo(
                             Alembic::AbcCoreOgawa::WriteArchive(), "particlesOut.abc",
                             appWriter, userStr, md );
  // create time sampling of 24 fps at frame 0 to start
  AbcG::TimeSampling ts(1.0f/24.0f, 0.0f);
  // get the archive top
  //AbcG::OObject topObj( *m_archive.get(), AbcG::kTop );
  AbcG::OObject topObj( m_archive, AbcG::kTop );

  // then add in our time sampling.
  Alembic::Util::uint32_t tsidx = topObj.getArchive().addTimeSampling(ts);
  // this is our particle outputs to write to each frame
  m_partsOut.reset( new AbcG::OPoints(topObj, "simpleParticles", tsidx) );
  // now add a colour property to the alembic file for out points
  AbcG::MetaData mdata;

  AbcG::SetGeometryScope( mdata, AbcG::kVaryingScope );
  AbcG::OPointsSchema &pSchema = m_partsOut->getSchema();
  std::cout<<"Schema "<<pSchema.getNumSamples()<<" "<<pSchema.valid()<<"\n";
  m_rgbOut.reset(new AbcG::OC3fArrayProperty( pSchema, "Cd", tsidx ));
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
  static float time=0.0f;
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
void Emitter::draw(const ngl::Mat4 &_VP, const ngl::Mat4 &_rot)
{
  QElapsedTimer timer;
  timer.start();
  ngl::NGLMessage::addMessage("Starting emitter draw\n");

  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use("Point");
  shader->setUniform("MVP",_VP*_rot);

  m_vao->bind();
  m_vao->draw();
  m_vao->unbind();

  ngl::NGLMessage::addMessage(fmt::format("Finished draw took %d milliseconds\n",timer.elapsed()));
  if(m_export==true)
  {
     timer.restart();
     exportFrame();
     ngl::NGLMessage::addMessage(fmt::format("Alembic Export took %d milliseconds\n",timer.elapsed()));
  }
}

void Emitter::exportFrame()
{
  static int frame=0;
  ++frame;
  // this is the data we are going to store, alembic uses Imath
  // internally so we convert from ngl
  // this is the array of particle positions for the frame
  std::vector<Imath::V3f> positions(m_numParticles);
  // these are the particle id's which are required so use use index no
  std::vector<Alembic::Util::uint64_t> id(m_numParticles);
  // set this to push back into the array
  Imath::V3f data;
  // colour values
  std::vector<Imath::V3f> colours(m_numParticles);

  std::vector<Imath::V3f> velocities(m_numParticles);
  std::vector< Alembic::Util::float32_t > widths;
  AbcG::OFloatGeomParam::Sample widthSamp;
  widthSamp.setScope(Alembic::AbcGeom::kVertexScope);
  widthSamp.setVals(Alembic::Abc::FloatArraySample(widths));

  for(size_t  i=0; i<m_numParticles; ++i)
  {
    positions[i]=Imath::V3f(m_particles[i].m_px,m_particles[i].m_py,m_particles[i].m_pz);
    id[i]=i;
    colours[i]=Imath::V3f(m_particles[i].m_r,m_particles[i].m_g,m_particles[i].m_b);
    velocities[i]=Imath::V3f(m_particles[i].m_r,m_particles[i].m_g,m_particles[i].m_b);
  }
  // create as samples we need to do this else we get a most vexing parse
  // https://en.wikipedia.org/wiki/Most_vexing_parse using below
  // psamp(V3fArraySample( positions),UInt64ArraySample(id))
  AbcG::V3fArraySample pos(positions);
  AbcG::UInt64ArraySample ids(id);
  AbcG::OPointsSchema::Sample psamp( pos,ids,velocities );
  AbcG::OPointsSchema &pSchema = m_partsOut->getSchema();
  std::cout<<"Schema "<<pSchema.getNumSamples()<<" "<<pSchema.valid()<<"\n";

  pSchema.set( psamp );
  AbcG::V3fArraySample colourArray(colours);


  m_rgbOut->set(colourArray);

}
