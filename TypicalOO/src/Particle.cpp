#include "Particle.h"
#include <ngl/Camera.h>
#include <ngl/Random.h>
#include <ngl/Transformation.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
#include "Emitter.h"
/// @brief ctor
/// @param _pos the start position of the particle
Particle::Particle(ngl::Vec3 _pos, ngl::Vec3 *_wind,  Emitter *_emitter , ngl::VertexArrayObject *vao  )
{
	m_pos=_pos;
	m_origin=_pos;
	m_wind=_wind;
	m_vao=vao;
  ngl::Random *rand=ngl::Random::instance();
	m_dir.m_x=rand->randomNumber(5)+0.5;
	m_dir.m_y=rand->randomPositiveNumber(10)+0.5;
	m_dir.m_z=rand->randomNumber(5)+0.5;
	m_currentLife=0.0;
	m_gravity=-9;//4.65;
  m_emitter=_emitter;

}
/// @brief a method to update the particle position
void Particle::update()
{

	m_currentLife+=0.05;
	// use projectile motion equation to calculate the new position
	// x(t)=Ix+Vxt
	// y(t)=Iy+Vxt-1/2gt^2
	// z(t)=Iz+Vzt
	m_pos.m_x=m_origin.m_x+(m_wind->m_x*m_dir.m_x*m_currentLife);
	m_pos.m_y= m_origin.m_y+(m_wind->m_y*m_dir.m_y*m_currentLife)+m_gravity*(m_currentLife*m_currentLife);
	m_pos.m_z=m_origin.m_z+(m_wind->m_z*m_dir.m_z*m_currentLife);

	// if we go below the origin re-set
	if(m_pos.m_y <= m_origin.m_y-0.01)
	{
		m_pos=m_origin;
		m_currentLife=0.0;
		ngl::Random *rand=ngl::Random::instance();
		m_dir.m_x=rand->randomNumber(5)+0.5;
		m_dir.m_y=rand->randomPositiveNumber(10)+0.5;
		m_dir.m_z=rand->randomNumber(5)+0.5;

	}
}
/// @brief a method to draw the particle
void Particle::draw()
{
  // get the VBO instance and draw the built in teapot
  ngl::Mat4 pos;
  pos.translate(m_pos.m_x,m_pos.m_y,m_pos.m_z);
  ngl::Mat4 MVP;
  MVP=pos*m_emitter->getCam()->getVPMatrix() ;
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  shader->setRegisteredUniform("MVP",MVP);
  m_vao->draw();

}
