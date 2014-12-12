#include "Emitter.h"
#include <ngl/Random.h>
#include <ngl/Transformation.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
/// @brief ctor
/// @param _pos the position of the emitter
/// @param _numParticles the number of particles to create
Emitter::Emitter(ngl::Vec3 _pos, int _numParticles, ngl::Vec3 *_wind )
{
	m_wind=_wind;
	Particle p;
	GLParticle g;
	ngl::Random *rand=ngl::Random::instance();
	m_pos=_pos;
	m_particles = new Particle[_numParticles];
	m_glparticles = new GLParticle[_numParticles];
	m_vao=ngl::VertexArrayObject::createVOA(GL_POINTS);
	for (int i=0; i< _numParticles; ++i)
	{		

		g.px=p.m_px=_pos.m_x;
		g.py=p.m_py=_pos.m_y;
		g.pz=p.m_pz=_pos.m_z;
		g.px=p.m_px=rand->randomNumber(5)+0.5;
		g.py=p.m_dy=rand->randomPositiveNumber(10)+0.5;
		g.pz=p.m_dz=rand->randomNumber(5)+0.5;
		p.m_currentLife=0.0;
	//	p.m_gravity=-9;//4.65;


		m_particles[i]=p;
		m_glparticles[i]=g;
	}
	m_numParticles=_numParticles;
	m_vao->bind();
	// create the VAO and stuff data
	m_vao->setData(m_numParticles*sizeof(GLParticle),m_glparticles[0].px);
	m_vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(GLParticle),0);
// uv same as above but starts at 0 and is attrib 1 and only u,v so 2
m_vao->setVertexAttributePointer(1,3,GL_FLOAT,sizeof(GLParticle),3);
m_vao->setNumIndices(m_numParticles);
m_vao->unbind();






}
/// @brief a method to update each of the particles contained in the system
void Emitter::update()
{
	m_vao->bind();
	ngl::Real *glPtr=m_vao->getDataPointer(0);
	unsigned int glIndex=0;
	for(int i=0; i<m_numParticles; ++i)
	{
//		m_particles[i].update();
		m_particles[i].m_currentLife+=0.05;
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
		if(m_particles[i].m_py <= m_pos.m_y-0.01)
		{
			m_particles[i].m_px=m_pos.m_x;
			m_particles[i].m_pz=m_pos.m_y;
			m_particles[i].m_px=m_pos.m_z;

			m_particles[i].m_currentLife=0.0;
			ngl::Random *rand=ngl::Random::instance();
			m_particles[i].m_dx=rand->randomNumber(5)+0.5;
			m_particles[i].m_dy=rand->randomPositiveNumber(10)+0.5;
			m_particles[i].m_dz=rand->randomNumber(5)+0.5;
			glPtr[glIndex]=m_particles[i].m_px;
			glPtr[glIndex+1]=m_particles[i].m_py;
			glPtr[glIndex+2]=m_particles[i].m_pz;

		}
		glIndex+=6;

	}
	m_vao->freeDataPointer();

	m_vao->unbind();
}
/// @brief a method to draw all the particles contained in the system
void Emitter::draw()
{

  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  ngl::Transformation transform;
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(getShaderName());

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;

	ngl::Mat4 vp=m_cam->getVPMatrix();
/*	for(int i=0; i<m_numParticles; ++i)
	{
		transform.setPosition(	m_particles[i].m_px, m_particles[i].m_py,m_particles[i].m_pz);
		M=transform.getMatrix();
		MV=transform.getMatrix()*m_cam->getViewMatrix();
		MVP=M*vp ;
		normalMatrix=MV;
		normalMatrix.inverse();
		shader->setShaderParamFromMat4("MV",MV);
		shader->setShaderParamFromMat4("MVP",MVP);
		shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
		shader->setShaderParamFromMat4("M",M);
	//	prim->draw("sphere");

	}*/

	shader->setShaderParamFromMat4("MVP",vp);
	shader->setShaderParamFromMat4("MV",m_cam->getViewMatrix());

	m_vao->bind();
	m_vao->draw();
	m_vao->unbind();


}
