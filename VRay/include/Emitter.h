#ifndef EMITTER_H_
#define EMITTER_H_
#include <vector>
#include <ngl/Vec3.h>
#include <ngl/AbstractVAO.h>
#include <ngl/Mat4.h>
#include <memory>

#pragma pack(push,1)

struct Particle
{
	/// @brief the curent particle position
	GLfloat m_px;
	GLfloat m_py;
	GLfloat m_pz;
	/// @brief the direction vector of the particle
	GLfloat m_dx;
	GLfloat m_dy;
	GLfloat m_dz;
	/// @brief the current life value of the particle
	GLfloat m_currentLife;
	/// @brief gravity
	GLfloat m_gravity;
  GLfloat m_r;
  GLfloat m_g;
  GLfloat m_b;

};
#pragma pack(push,1)
#pragma pack(pop)

typedef struct GLParticle
{
	GLfloat px;
	GLfloat py;
	GLfloat pz;
  GLfloat pr;
  GLfloat pg;
  GLfloat pb;

}GLParticle;

#pragma pack(pop)





class Emitter
{
public :

	/// @brief ctor
	/// @param _pos the position of the emitter
	/// @param _numParticles the number of particles to create
  Emitter( ngl::Vec3 _pos, unsigned int _numParticles, ngl::Vec3 *_wind );
	/// @brief a method to update each of the particles contained in the system
	void update();
	/// @brief a method to draw all the particles contained in the system
  void draw(const ngl::Mat4 &_view,const ngl::Mat4 &_project,const ngl::Mat4 &_rot);
	~Emitter();
  void incTime(float _t){m_time+=_t;}
  void decTime(float _t){m_time-=_t;}
  void toggleExport(){ m_export^=true;}
  private :
	/// @brief the position of the emitter
	ngl::Vec3 m_pos;
	/// @brief the number of particles
  unsigned int m_numParticles;
	/// @brief the container for the particles
  std::unique_ptr<Particle []> m_particles;
  std::unique_ptr<GLParticle []> m_glparticles;
	/// @brief a wind vector
	ngl::Vec3 *m_wind;
  /// @brief the name of the shader to use
  std::string m_shaderName;
  /// @brief a pointer to the camera used for drawing
  std::unique_ptr<ngl::AbstractVAO> m_vao;
  float m_time=0.8f;
  bool m_export=false;
  void exportFrame(const ngl::Mat4 &_view);


};


#endif

