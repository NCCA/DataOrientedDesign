#include "Emitter.h"
#include <ngl/Random.h>
#include <ngl/Transformation.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/Logger.h>
#include <QElapsedTimer>
#include <ngl/NGLStream.h>
/// @brief ctor
/// @param _pos the position of the emitter
/// @param _numParticles the number of particles to create
Emitter::Emitter(ngl::Vec3 _pos, int _numParticles, ngl::Vec3 *_wind )
{


	OpenCL::printCLInfo();
	m_cl = new OpenCL("kernel/updateparticle.cl");
	m_cl->createKernel("updateparticle");
	m_time=0.0;

	m_input = clCreateBuffer(m_cl->getContext(),  CL_MEM_READ_WRITE,  sizeof(Particle) * _numParticles, NULL, NULL);
	m_output = clCreateBuffer(m_cl->getContext(), CL_MEM_WRITE_ONLY, sizeof(GLParticle) * _numParticles, NULL, NULL);
	if (!m_input || !m_output)
	{
			std::cerr<<"Error: Failed to allocate device memory!\n";
			exit(EXIT_FAILURE);
	}

  // Get the maximum work group size for executing the kernel on the device
  //
  int err;
  err = clGetKernelWorkGroupInfo(m_cl->getKernel(), m_cl->getID(), CL_KERNEL_WORK_GROUP_SIZE, sizeof(m_workgroupsize), &m_workgroupsize, NULL);
  std::cout<<"work group size is "<<m_workgroupsize<<"\n";
  if (err != CL_SUCCESS)
  {
      std::cerr<<"Error: Failed to retrieve kernel work group info "<<err<<"\n";
      exit(EXIT_FAILURE);
  }


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

	for (int i=0; i< _numParticles; ++i)
	{		

		g.px=p.m_px=m_pos.m_x;
		g.py=p.m_py=m_pos.m_y;
		g.pz=p.m_pz=m_pos.m_z;
		p.m_dx=end.m_x+rand->randomNumber(2)+0.5;
		p.m_dy=end.m_y+rand->randomPositiveNumber(10)+0.5;
		p.m_dz=end.m_z+rand->randomNumber(2)+0.5;

		p.m_currentLife=0.0;
//		p.m_maxLife=rand->randomNumber(3)+0.5;
//		p.m_gravity=-9;//4.65;


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
	clReleaseMemObject(m_input);
	clReleaseMemObject(m_output);

	m_vao->removeVOA();
	delete m_cl;
}

/// @brief a method to update each of the particles contained in the system
void Emitter::update()
{
	QElapsedTimer timer;
	timer.start();
	ngl::Logger *log = ngl::Logger::instance();
	log->setColour(ngl::GREEN);
	log->logMessage("Starting emitter update\n");


	int err;
	err = clEnqueueWriteBuffer(m_cl->getCommands(), m_input, CL_TRUE, 0, sizeof(float) * m_numParticles, m_particles, 0, NULL, NULL);
	if (err != CL_SUCCESS)
	{
			std::cerr<<"Error: Failed to write to source array!\n";
			exit(EXIT_FAILURE);
	}

  // Set the arguments to our compute kernel
  //
  typedef struct Vec3
  {
    float m_x;
    float m_y;
    float m_z;
  }Vec3;

  Vec3 wind;
  wind.m_x=m_wind->m_x;
  wind.m_y=m_wind->m_y;
  wind.m_z=m_wind->m_z;
  Vec3 pos;
  pos.m_x=m_pos.m_x;
  pos.m_y=m_pos.m_y;
  pos.m_z=m_pos.m_z;
  float gravity=-9.0f;
  err = 0;
  err  = clSetKernelArg(m_cl->getKernel(), 0, sizeof(cl_mem), &m_input);
  err |= clSetKernelArg(m_cl->getKernel(), 1, sizeof(cl_mem), &m_output);
  err |= clSetKernelArg(m_cl->getKernel(), 2, sizeof(Vec3), &wind);
  err |= clSetKernelArg(m_cl->getKernel(), 3, sizeof(Vec3), &pos);
  err |= clSetKernelArg(m_cl->getKernel(), 4, sizeof(float), &gravity);

  if (err != CL_SUCCESS)
  {
      std::cerr<<"Error: Failed to set kernel arguments! "<< err<<"\n";
      exit(EXIT_FAILURE);
  }



  // Execute the kernel over the entire range of our 1d input data set
  // using the maximum number of work group items for this device
  //
  err = clEnqueueNDRangeKernel(m_cl->getCommands(), m_cl->getKernel(), 1, NULL, &m_numParticles, &m_workgroupsize, 0, NULL, NULL);
  if (err)
  {
      m_cl->printError(err);
      std::cerr<<"Error: Failed to execute kernel!\n";
      exit( EXIT_FAILURE);
  }

  // Wait for the command commands to get serviced before reading back results
  //
  clFinish(m_cl->getCommands());

  // Read back the results from the device to verify the output
  //

  err = clEnqueueReadBuffer( m_cl->getCommands(), m_output, CL_TRUE, 0, sizeof(GLParticle) * m_numParticles, m_glparticles, 0, NULL, NULL );
  if (err != CL_SUCCESS)
  {//
      std::cerr<<"Error: Failed to read output array "<< err<<"\n";
      exit(EXIT_FAILURE);
  }


	m_vao->bind();

	m_vao->updateData(m_numParticles*sizeof(GLParticle),m_glparticles[0].px);

	m_vao->unbind();
	static int rot=0;
	static float time=0.0;
	float pointOnCircleX= cos(ngl::radians(time))*4.0;
	float pointOnCircleZ= sin(ngl::radians(time))*4.0;
	ngl::Vec3 end(pointOnCircleX,2.0,pointOnCircleZ);
	end=end-m_pos;
	//end.normalize();
	time+=m_time;
	//std::cout<<end;
	for(unsigned int i=0; i<m_numParticles; ++i)
	{
		m_particles[i].m_currentLife+=0.02;
		m_particles[i].m_py=m_glparticles[i].py;

		// if we go below the origin re-set
		if(m_particles[i].m_py <= m_pos.m_y-0.01)
		{
			++rot;

			m_particles[i].m_px=m_pos.m_x;
			m_particles[i].m_pz=m_pos.m_y;
			m_particles[i].m_px=m_pos.m_z;

			m_particles[i].m_currentLife=0.0;
			ngl::Random *rand=ngl::Random::instance();
			//m_particles[i].m_dx=cos(ngl::radians(rot))*rand->randomNumber(5)+0.5;
			//m_particles[i].m_dy=rand->randomPositiveNumber(10)+0.5;
			//m_particles[i].m_dz=sin(ngl::radians(rot))*rand->randomNumber(5)+0.5;
			m_particles[i].m_dx=end.m_x+rand->randomNumber(2)+0.5;
			m_particles[i].m_dy=end.m_y+rand->randomPositiveNumber(10)+0.5;
			m_particles[i].m_dz=end.m_z+rand->randomNumber(2)+0.5;

		}

	}

	log->logMessage("Finished update array took %d milliseconds\n",timer.elapsed());

}
/// @brief a method to draw all the particles contained in the system
void Emitter::draw(const ngl::Mat4 &_rot)
{
	QElapsedTimer timer;
	timer.start();
	ngl::Logger *log = ngl::Logger::instance();
	log->setColour(ngl::CYAN);
	log->logMessage("Starting emitter draw\n");

  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(getShaderName());

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  static int rot=0;


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

	shader->setShaderParamFromMat4("MVP",_rot*vp);
//	shader->setShaderParamFromMat4("MV",m_cam->getViewMatrix());

	m_vao->bind();
	m_vao->draw();
	m_vao->unbind();

	log->logMessage("Finished draw took %d milliseconds\n",timer.elapsed());

}
