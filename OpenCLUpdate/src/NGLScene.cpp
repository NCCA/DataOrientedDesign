#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Logger.h>


//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=0.5f;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=1.0f;

NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=0;
  m_spinYFace=0;
  setTitle("Simple Projectiles");
  m_fpsTimer =startTimer(0);
  m_fps=0;
  m_frames=0;
  m_timer.start();
  ngl::Logger *log = ngl::Logger::instance();
  log->logMessage("Testing the logger");

}


NGLScene::~NGLScene()
{
  delete m_emitter;
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
  ngl::Logger *log = ngl::Logger::instance();
  log->close();
}

void NGLScene::resizeGL(int _w, int _h)
{
  // set the viewport for openGL
  glViewport(0,0,_w,_h);
  // now set the camera size values as the screen size has changed
  m_cam->setShape(45,(float)_w/_h,0.05,350);
  update();
}


void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::instance();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0,5,18);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  m_cam= new ngl::Camera(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam->setShape(40,(float)720.0/576.0,0.5,150);
  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // we are creating a shader called Phong
  shader->createShaderProgram("Point");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("PointVertex",ngl::ShaderType::VERTEX);
  shader->attachShader("PointFragment",ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource("PointVertex","shaders/PointVertex.glsl");
  shader->loadShaderSource("PointFragment","shaders/PointFragment.glsl");
  // compile the shaders
  shader->compileShader("PointVertex");
  shader->compileShader("PointFragment");
  // add them to the program
  shader->attachShaderToProgram("Point","PointVertex");
  shader->attachShaderToProgram("Point","PointFragment");
  // now we have associated this data we can link the shader
  shader->linkProgramObject("Point");
  // and make it active ready to load values
  (*shader)["Point"]->use();
  shader->autoRegisterUniforms("Point");


  m_wind=new ngl::Vec3(1,1,1);
  m_numParticles=(1024*1024)*5;
  m_emitter = new Emitter(ngl::Vec3(0,0,0),m_numParticles,m_wind);
  m_emitter->setCam(m_cam);
  m_emitter->setShaderName("Point");

  m_text=new ngl::Text(QFont("Arial",14));
  m_text->setScreenSize(width(),height());
  // as re-size is not explicitly called we need to do this.
  glViewport(0,0,width(),height());
  m_particleTimer=startTimer(20);
  m_text = new ngl::Text(QFont("Arial",14));
  m_text->setScreenSize(width(),height());

}




void NGLScene::paintGL()
{
  // grab an instance of the shader manager
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Rotation based on the mouse position for our global transform
   ngl::Mat4 rotX;
   ngl::Mat4 rotY;
   // create the rotation matrices
   rotX.rotateX(m_spinXFace);
   rotY.rotateY(m_spinYFace);
   // multiply the rotations
   ngl::Mat4 mouseGlobalTX=rotY*rotX;
   // add the translations
   mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
   mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
   mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;



  m_emitter->draw(mouseGlobalTX);
  m_text->setColour(1,1,1);
  QString text=QString("Wind Vector  %1 %2 %3").arg(m_wind->m_x).arg(m_wind->m_y).arg(m_wind->m_z);
  m_text->renderText(10,20,text);
  ++m_frames;
 // glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  m_text->setColour(1,1,0);
  text=QString("%1 Particles at %2fps").arg(m_numParticles).arg(m_fps);
  m_text->renderText(10,40,text);
  //glPointSize(1.0);
  glEnable(GL_PROGRAM_POINT_SIZE);
  // Enable blending
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
 }

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent (QMouseEvent * _event)
{
  // note the method buttons() is the button state when event was called
  // this is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx=_event->x()-m_origX;
    int diffy=_event->y()-m_origY;
    m_spinXFace += (float) 0.5f * diffy;
    m_spinYFace += (float) 0.5f * diffx;
    m_origX = _event->x();
    m_origY = _event->y();
    update();

  }
        // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = (int)(_event->x() - m_origXPos);
    int diffY = (int)(_event->y() - m_origYPos);
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();

   }
}


//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent ( QMouseEvent * _event)
{
  // this method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if(_event->button() == Qt::LeftButton)
  {
    m_origX = _event->x();
    m_origY = _event->y();
    m_rotate =true;
  }
  // right mouse translate mode
  else if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translate=true;
  }

}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent ( QMouseEvent * _event )
{
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotate=false;
  }
        // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate=false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

	// check the diff of the wheel position (0 means no change)
	if(_event->delta() > 0)
	{
		m_modelPos.m_z+=ZOOM;
	}
	else if(_event->delta() <0 )
	{
		m_modelPos.m_z-=ZOOM;
	}
	update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  case Qt::Key_Up : m_wind->m_y+=0.1; break;
  case Qt::Key_Down : m_wind->m_y-=0.1; break;
  case Qt::Key_Left : m_wind->m_x+=0.1; break;
  case Qt::Key_Right : m_wind->m_x-=0.1; break;
  case Qt::Key_A : m_emitter->updatePos(-0.1,0,0); break;
  case Qt::Key_D : m_emitter->updatePos(0.1,0,0); break;
  case Qt::Key_W : m_emitter->updatePos(0,0,-0.1); break;
  case Qt::Key_S : m_emitter->updatePos(0,0,0.1); break;

  case Qt::Key_I : m_wind->m_z+=0.1; break;
  case Qt::Key_O : m_wind->m_z-=0.1; break;
  case Qt::Key_1 : m_emitter->incTime(1.0); break;
  case Qt::Key_2: m_emitter->decTime(1.0); break;

  case Qt::Key_Space : m_wind->set(1,1,1); break;
  default : break;
  }
  // finally update the GLWindow and re-draw
  //if (isExposed())
    update();
}

void NGLScene::timerEvent(QTimerEvent *_event )
{
	if(_event->timerId() ==   m_particleTimer)
	{
		m_emitter->update();
	}
	if(_event->timerId() == m_fpsTimer)
		{
			if( m_timer.elapsed() > 1000.0)
			{
				m_fps=m_frames;
				m_frames=0;
				m_timer.restart();
			}
		 }
			// re-draw GL
	update();
		// re-draw GL
}
