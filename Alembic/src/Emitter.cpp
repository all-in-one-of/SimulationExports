#include "Emitter.h"
#include <ngl/Random.h>
#include <ngl/Transformation.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/Logger.h>
#include <ngl/NGLStream.h>
#include <QElapsedTimer>
#include <ngl/RibExport.h>
#include <boost/format.hpp>
#include <array>

namespace AbcG = Alembic::AbcGeom;
using namespace AbcG;
using Alembic::AbcCoreAbstract::chrono_t;
using Alembic::AbcCoreAbstract::index_t;
/// @brief ctor
/// @param _pos the position of the emitter
/// @param _numParticles the number of particles to create
Emitter::Emitter(ngl::Vec3 _pos, int _numParticles, ngl::Vec3 *_wind )
{
	m_wind=_wind;
	Particle p;
	GLParticle g;
	ngl::Random *rand=ngl::Random::instance();
//	ngl::Logger *log = ngl::Logger::instance();
//	log->logMessage("Starting emitter ctor\n");
	QElapsedTimer timer;
	timer.start();
	m_pos=_pos;
  m_particles.reset(  new Particle[_numParticles]);
  m_glparticles.reset( new GLParticle[_numParticles]);
	m_vao=ngl::VertexArrayObject::createVOA(GL_POINTS);
  float pointOnCircleX= cosf(ngl::radians(m_time))*4.0f;
  float pointOnCircleZ= sinf(ngl::radians(m_time))*4.0f;
  ngl::Vec3 end(pointOnCircleX,2.0,pointOnCircleZ);
  end=end-m_pos;

	#pragma omp parallel for ordered schedule(dynamic)
	for (int i=0; i< _numParticles; ++i)
	{		

    g.px=p.m_px=m_pos.m_x;
    g.py=p.m_py=m_pos.m_y;
    g.pz=p.m_pz=m_pos.m_z;
    p.m_dx=end.m_x+rand->randomNumber(2)+0.5;
    p.m_dy=end.m_y+rand->randomPositiveNumber(10)+0.5;
    p.m_dz=end.m_z+rand->randomNumber(2)+0.5;
    p.m_gravity=-9.0f;
    p.m_currentLife=0.0;


		m_particles[i]=p;
		m_glparticles[i]=g;
	}
	m_numParticles=_numParticles;
	m_vao->bind();
	// create the VAO and stuff data
	m_vao->setData(m_numParticles*sizeof(GLParticle),m_glparticles[0].px);
	m_vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(GLParticle),0);
// uv same as above but starts at 0 and is attrib 1 and only u,v so 2
//m_vao->setVertexAttributePointer(1,3,GL_FLOAT,sizeof(GLParticle),3);
m_vao->setNumIndices(m_numParticles);
m_vao->unbind();
//log->logMessage("Finished filling array took %d milliseconds\n",timer.elapsed());




m_archive.reset(new  AbcG::OArchive(Alembic::AbcCoreOgawa::WriteArchive(),"particlesOut2.abc") );
TimeSampling ts(1.0f/24.0f, 0.0f);

AbcG::OObject topObj( *m_archive.get(), AbcG::kTop );
Alembic::Util::uint32_t tsidx = topObj.getArchive().addTimeSampling(ts);
// Create our object.
//OPoints
partsOut.reset( new OPoints(topObj, "simpleParticles", tsidx) );

}


Emitter::~Emitter()
{
	m_vao->removeVOA();
}

/// @brief a method to update each of the particles contained in the system
void Emitter::update()
{
	QElapsedTimer timer;
	timer.start();
//	ngl::Logger *log = ngl::Logger::instance();
//	log->logMessage("Starting emitter update\n");

	m_vao->bind();
	ngl::Real *glPtr=m_vao->getDataPointer(0);
    unsigned int glIndex=0;
    #pragma omp parallel for
    static int rot=0;
    static float time=0.0;
    float pointOnCircleX= cos(ngl::radians(time))*4.0;
    float pointOnCircleZ= sin(ngl::radians(time))*4.0;
    ngl::Vec3 end(pointOnCircleX,2.0,pointOnCircleZ);
    end=end-m_pos;
    //end.normalize();
    time+=m_time;

	for(int i=0; i<m_numParticles; ++i)
	{
//		m_particles[i].update();
    m_particles[i].m_currentLife+=0.002;
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
//			m_particles[i].m_dx=rand->randomNumber(5)+0.5;
//			m_particles[i].m_dy=rand->randomPositiveNumber(10)+0.5;
//			m_particles[i].m_dz=rand->randomNumber(5)+0.5;
      m_particles[i].m_dx=end.m_x+rand->randomNumber(2)+0.5;
      m_particles[i].m_dy=end.m_y+rand->randomPositiveNumber(10)+0.5;
      m_particles[i].m_dz=end.m_z+rand->randomNumber(2)+0.5;

      glPtr[glIndex]=m_particles[i].m_px;
			glPtr[glIndex+1]=m_particles[i].m_py;
			glPtr[glIndex+2]=m_particles[i].m_pz;

		}
		#pragma omp atomic
		glIndex+=3;

	}
	m_vao->freeDataPointer();

	m_vao->unbind();

 // log->logMessage("Finished update array took %d milliseconds\n",timer.elapsed());
}
/// @brief a method to draw all the particles contained in the system
void Emitter::draw(const ngl::Mat4 &_rot)
{
	QElapsedTimer timer;
	timer.start();
//	ngl::Logger *log = ngl::Logger::instance();
//	log->logMessage("Starting emitter draw\n");

  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(getShaderName());

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;


	ngl::Mat4 vp=m_cam->getVPMatrix();

  shader->setUniform("MVP",_rot*vp);

	m_vao->bind();
	m_vao->draw();
	m_vao->unbind();

//	log->logMessage("Finished draw took %d milliseconds\n",timer.elapsed());
  if(m_export==true)
  {
     timer.restart();
     exportFrame();
    // log->logMessage("Rib Export took %d milliseconds\n",timer.elapsed());
  }
}

void Emitter::exportFrame()
{
  static int frame=0;
  ++frame;
  ngl::Logger::instance()->logError("Create top");

  TimeSampling ts(1.0f/24.0f, 0.0f);
  AbcG::OObject topObj( *m_archive.get(), AbcG::kTop );

  ngl::Logger::instance()->logError("createObj");

  std::cout << "Created Simple Particles" << std::endl;
  // Add attributes
  ngl::Logger::instance()->logError("getSchema");

  std::vector<V3f> positions;
  std::vector<Alembic::Util::uint64_t> id;

  V3f data;

  for(auto i=0; i<m_numParticles; ++i)
  {

    data.x=m_particles[i].m_px;
    data.y=m_particles[i].m_py;
    data.z=m_particles[i].m_pz;
    positions.push_back(data);
    id.push_back(i);
  }
  V3fArraySample pos(positions);
  UInt64ArraySample ids(id);
  OPointsSchema::Sample psamp( pos,ids );

  partsOut->getSchema().set( psamp );
}
