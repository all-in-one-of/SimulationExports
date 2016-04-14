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
/// @brief ctor
/// @param _pos the position of the emitter
/// @param _numParticles the number of particles to create
Emitter::Emitter(ngl::Vec3 _pos, int _numParticles, ngl::Vec3 *_wind )
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
	m_particles = new Particle[_numParticles];
	m_glparticles = new GLParticle[_numParticles];
	m_vao=ngl::VertexArrayObject::createVOA(GL_POINTS);
  float pointOnCircleX= cos(ngl::radians(m_time))*4.0;
  float pointOnCircleZ= sin(ngl::radians(m_time))*4.0;
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
log->logMessage("Finished filling array took %d milliseconds\n",timer.elapsed());

}


Emitter::~Emitter()
{
	delete [] m_glparticles;
	delete [] m_particles;
	m_vao->removeVOA();
}

/// @brief a method to update each of the particles contained in the system
void Emitter::update()
{
	QElapsedTimer timer;
	timer.start();
	ngl::Logger *log = ngl::Logger::instance();
	log->logMessage("Starting emitter update\n");

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

  log->logMessage("Finished update array took %d milliseconds\n",timer.elapsed());
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

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;


	ngl::Mat4 vp=m_cam->getVPMatrix();

  shader->setUniform("MVP",_rot*vp);

	m_vao->bind();
	m_vao->draw();
	m_vao->unbind();

	log->logMessage("Finished draw took %d milliseconds\n",timer.elapsed());
  if(m_export==true)
  {
     timer.restart();
     exportRib();
     log->logMessage("Rib Export took %d milliseconds\n",timer.elapsed());
  }
}

void Emitter::exportRib()
{
  // static frame number for exporting
  static int frame=0;
  std::string fname =boost::str(boost::format("ribs/particle.%04d.rib") %frame );
  // create an ngl::RibExport this can work either as a one shot
  // exporter or we can get it to output by setting frame numbers
  // see the source code to see how it works.
  ngl::RibExport rib(fname);
  // open the rib stream to write (basically a file)
  rib.open();
  // as it is a file we can get a reference to it to write too
  std::fstream &ribStream=rib.getStream();
  // this is out file name
  std::string data=boost::str(boost::format("particle.%04d.tiff") % frame);
  // note the escaped quotes \" if needed we could use new C++ 11
  // raw string literals instead
  ribStream<<"Display \""<<data<<"\" \"file\" \"rgba\"\n";
  // export the display format again this is user selected
  ribStream<<"Format 1024 720 1\n";
  // projection not we use the ngl::Camera fov
  ribStream<<"Projection \"perspective\" \"uniform float fov\" ["<<m_cam->getFOV()<<"]\n";
  // call world begin (see Rib export as it covers most of rib)
  rib.WorldBegin();
  // camera will write itself out into the rib steam in the correct
  // format, this metho is in ngl::Camera basically it does
  // Scale 1 1 -1
  // ConcatTransform [ view matrix .OpenGL[] ]
  m_cam->writeRib(rib);
  // Now we are dumping the data from our particle system as Points
  // I'm setting the data as Points "vertex point P" [ x y z .....]
  ribStream<<"Points \"vertex point P\" [";
  for(int i=0; i<m_numParticles; ++i)
  {
    ribStream<<m_particles[i].m_px<<" "<<m_particles[i].m_py<<" "<<m_particles[i].m_pz<<" ";
  }
  ribStream<<"]\n";
  // now dumping the widths (all the same but needed)
  ribStream<<"\"varying float width\" [";
  for(int i=0; i<m_numParticles; ++i)
  {
    ribStream<<"0.01";
  }
  ribStream<<"]\n";
  // finally end the world

  rib.WorldEnd();
  // close the rib stream
  rib.close();
  // next frame
  ++frame;
}
