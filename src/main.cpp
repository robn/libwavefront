#include <iostream>
#include <string>
#include <sstream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <wavefront.h>

int frameCount = 0;
int lastTime = 0;
int currentWidth = 0;
int currentHeight = 0;
double deltaTime = 0;
float rotation = 0;

std::auto_ptr<Wavefront::Model> modelData;
std::auto_ptr<Wavefront::Animation> runAnimation;
std::auto_ptr<Wavefront::Animation> shieldAnimation;
std::auto_ptr<Wavefront::AnimatedModel> model;

void idle()
{
  int nowTime = glutGet(GLUT_ELAPSED_TIME);
  int diffTime = nowTime - lastTime;
  lastTime = nowTime;
  deltaTime = (float)diffTime / 1000.0f;

  model->update(50 * deltaTime);
  rotation += -25 * deltaTime;
  glutPostRedisplay();
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();
  glPushMatrix();
  glTranslatef(0, 0, -10);
  glRotatef(rotation, 0, 1, 0);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  //modelData->draw();
  model->draw();
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_2D);
  glPopMatrix();

  glutSwapBuffers();
  frameCount++;
}

void resize(int width, int height)
{
  currentWidth = width;
  currentHeight = height;
  glViewport(0, 0, currentWidth, currentHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, (GLfloat)width / (GLfloat)height, 0.1, 100.0);
  glMatrixMode(GL_MODELVIEW);
}

void timer(int tick)
{
  std::stringstream ss;

  ss << "Animated Wavefront Example -"
     << " FPS: " << frameCount * 4
     << " Width: " << currentWidth
     << " Height: " << currentHeight;

  glutSetWindowTitle(ss.str().c_str());

  frameCount = 0;
  glutTimerFunc(250, timer, 0);
}

void safe_main(int argc, char* argv[])
{
  int result = 0;

  glutInit(&argc, argv);
 
  //glutInitContextVersion(4, 0);
  //glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
  //glutInitContextProfile(GLUT_CORE_PROFILE);

  glutSetOption(
    GLUT_ACTION_ON_WINDOW_CLOSE,
    GLUT_ACTION_GLUTMAINLOOP_RETURNS
  );
 
  glutInitWindowSize(800, 600);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

  result = glutCreateWindow("Shader Test");

  if(result < 1)
  {
    std::cout << "Error: Could not create a new rendering window" << std::endl;
    throw std::exception();
  }

  result = glewInit();

  if(result != GLEW_OK)
  {
    std::cout << "Error: " << glewGetErrorString(result) << std::endl;
    throw std::exception();
  }

  std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

  glutReshapeFunc(resize);
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutTimerFunc(0, timer, 0);

  glClearColor(0.0f, 0.0f, 0.3f, 0.0f);

  modelData.reset(new Wavefront::Model("curuthers/curuthers.obj"));
  model.reset(new Wavefront::AnimatedModel(modelData.get()));
  shieldAnimation.reset(new Wavefront::Animation("curuthers/shield.anm"));
  runAnimation.reset(new Wavefront::Animation("curuthers/run.anm"));
  runAnimation->interpolate(2, true);
  model->addAnimation(runAnimation.get());
  model->addAnimation(shieldAnimation.get());

  glutMainLoop();
}

int main(int argc, char* argv[])
{
  try
  {
    safe_main(argc, argv);

    return 0;
  }
  catch(std::exception& e)
  {
    std::cout << "Exception: " << e.what() << std::endl;

    return 1;
  }
}
