#include "Emitter.h"
#include <ngl/Random.h>
#include <ngl/Transformation.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/Logger.h>
#include <ngl/ShaderLib.h>
#include <QElapsedTimer>
#include <ngl/Logger.h>

/// @brief ctor
/// @param _pos the position of the emitter
/// @param _numParticles the number of particles to create
Emitter::Emitter(ngl::Vec3 _pos, int _numParticles, ngl::Vec3 *_wind )
{
	ngl::Logger *log = ngl::Logger::instance();
	log->logMessage("Starting emitter ctor\n");
	QElapsedTimer timer;
	timer.start();
	m_vao=ngl::VertexArrayObject::createVOA(GL_POINTS);
	m_vao->bind();
	ngl::Vec3 point(0,0,0);
	// create the VAO and stuff data
	m_vao->setData(1*sizeof(ngl::Vec3),point.m_x);
	m_vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(ngl::Vec3),0);
	m_vao->setNumIndices(1);
	m_vao->unbind();

	m_wind=_wind;
	Particle p;
	ngl::Random *rand=ngl::Random::instance();
	m_pos=_pos;

	for (int i=0; i< _numParticles; ++i)
	{
		p.m_pos=_pos;
		p.m_dir.m_x=rand->randomNumber(5)+0.5;
		p.m_dir.m_y=rand->randomPositiveNumber(10)+0.5;
		p.m_dir.m_z=rand->randomNumber(5)+0.5;
		p.m_currentLife=0.0;
		p.m_gravity=-9;//4.65;

		m_particles.push_back(p);
	}
	m_numParticles=_numParticles;
	log->logMessage("finished emitter ctor\n");

}
/// @brief a method to update each of the particles contained in the system
void Emitter::update()
{
	QElapsedTimer timer;
	timer.start();
	ngl::Logger *log = ngl::Logger::instance();
	log->logMessage("Starting emitter update\n");

	for(int i=0; i<m_numParticles; ++i)
	{
//		m_particles[i].update();
		m_particles[i].m_currentLife+=0.05;
		// use projectile motion equation to calculate the new position
		// x(t)=Ix+Vxt
		// y(t)=Iy+Vxt-1/2gt^2
		// z(t)=Iz+Vzt
		m_particles[i].m_pos.m_x=m_pos.m_x+(m_wind->m_x*m_particles[i].m_dir.m_x*m_particles[i].m_currentLife);
		m_particles[i].m_pos.m_y= m_pos.m_y+(m_wind->m_y*m_particles[i].m_dir.m_y*m_particles[i].m_currentLife)+m_particles[i].m_gravity*(m_particles[i].m_currentLife*m_particles[i].m_currentLife);
		m_particles[i].m_pos.m_z=m_pos.m_z+(m_wind->m_z*m_particles[i].m_dir.m_z*m_particles[i].m_currentLife);

		// if we go below the origin re-set
		if(m_particles[i].m_pos.m_y <= m_pos.m_y-0.01)
		{
			m_particles[i].m_pos=m_pos;
			m_particles[i].m_currentLife=0.0;
			ngl::Random *rand=ngl::Random::instance();
			m_particles[i].m_dir.m_x=rand->randomNumber(5)+0.5;
			m_particles[i].m_dir.m_y=rand->randomPositiveNumber(10)+0.5;
			m_particles[i].m_dir.m_z=rand->randomNumber(5)+0.5;
		}



	}
	log->logMessage("Finished update array took %d milliseconds\n",timer.elapsed());

}
/// @brief a method to draw all the particles contained in the system
void Emitter::draw()
{


	QElapsedTimer timer;
	timer.start();
	ngl::Logger *log = ngl::Logger::instance();
	log->logMessage("Starting emitter draw\n");
	m_vao->bind();
	ngl::ShaderLib *shader=ngl::ShaderLib::instance();
	shader->use("Point");



  ngl::Mat4 MVP;
  // get the VBO instance and draw the built in teapot
  ngl::Mat4 pos;


	for(int i=0; i<m_numParticles; ++i)
	{
		pos.translate(m_particles[i].m_pos.m_x,m_particles[i].m_pos.m_y,m_particles[i].m_pos.m_z);

		MVP=pos*m_cam->getVPMatrix() ;
		shader->setRegisteredUniform("MVP",MVP);
		m_vao->draw();
	}

	m_vao->unbind();
	log->logMessage("Finished draw took %d milliseconds\n",timer.elapsed());

}
