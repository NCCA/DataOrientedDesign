#include "Emitter.h"
#include <QElapsedTimer>
#include <ngl/Logger.h>
#include <ngl/ShaderLib.h>

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
	ngl::Vec3 p(0,0,0);
	// create the VAO and stuff data
	m_vao->setData(1*sizeof(ngl::Vec3),p.m_x);
	m_vao->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(ngl::Vec3),0);
	m_vao->setNumIndices(1);
	m_vao->unbind();

	m_wind=_wind;
	for (int i=0; i< _numParticles; ++i)
	{
		m_particles.push_back(Particle(_pos,m_wind,this,m_vao));
	}
	m_numParticles=_numParticles;





	log->logMessage("Finished filling vector took %d milliseconds\n",timer.elapsed());

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
		m_particles[i].update();
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

	for(int i=0; i<m_numParticles; ++i)
	{
		m_particles[i].draw();
	}
	m_vao->unbind();

	log->logMessage("Finished draw took %d milliseconds\n",timer.elapsed());

}
