#include "Scene.h"

// TEMP BAD PLACE
/*
float lightRot = 0.0;
GLfloat lightPos[3] = {-200.0f, 10.0f, -80.0f};
GLfloat lightPos2[3] = {200.0f, 10.0f, -100.0f};
GLfloat Al[4] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat Dl[4] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat Sl[4] = {1.0f, 1.0f, 1.0f, 1.0f};

GLfloat att[3] = { 1.0f, 0.0f, 0.00001f }; // constant, linear, quadratic
*/


Scene::~Scene()
{
   for each(Drawable* d in m_dynamicObjects)
   {
      delete d;
   }
   for each(Drawable* d in m_staticObjects)
   {
      delete d;
   }
   for each(Light* l in m_lights)
   {
      delete l;
   }

   if (m_transform)
   {
      delete m_transform;
   }
}

float angle = -0;
float lightRot = 0.0;

void Scene::setupCamera()
{
   //m_cameraLoc.y += 0.01;
   // Setup camera
   //glMatrixMode(GL_PROJECTION);
   //glLoadIdentity();
   glMatrixMode(GL_MODELVIEW);
   gluLookAt(  m_cameraLoc.x, m_cameraLoc.y, m_cameraLoc.z, // camera
               0.0f, 0.0f, 0.0f,    // look at point
               0.0f, 1.0f, 0.0f);   // up vector
   
}

GLenum counterToEnum(unsigned int cnt)
{
   switch(cnt)
   {
   case 0:
      return GL_LIGHT0;
   case 1:
      return GL_LIGHT1;
   case 2:
      return GL_LIGHT2;
   case 3:
      return GL_LIGHT3;
   case 4:
      return GL_LIGHT4;
   case 5:
      return GL_LIGHT5;
   case 6:
      return GL_LIGHT6;
   case 7:
      return GL_LIGHT7;
   default:
      return -1;
   }

}

void Scene::setupLights()
{
   unsigned int lightCount = 0;
   for each(Light* l in m_lights)
   {
      if (l->m_enabled)
         l->applyLight(counterToEnum(lightCount++));
   }

   /*

   lightRot += 0.01;
   lightPos[1] -= 0.01;
   lightPos2[1] -= 0.01;
   glPushMatrix();
   //glRotatef(lightRot, 1.0, 0.0, 0.0);
   
   // Light 0
   glLightfv( GL_LIGHT0, GL_POSITION, lightPos );
   glLightfv( GL_LIGHT0, GL_AMBIENT, Al );
   glLightfv( GL_LIGHT0, GL_DIFFUSE, Dl );
   glLightfv( GL_LIGHT0, GL_SPECULAR, Sl );
   glLightf( GL_LIGHT0, GL_CONSTANT_ATTENUATION, att[0] );
   glLightf( GL_LIGHT0, GL_LINEAR_ATTENUATION, att[1] );
   glLightf( GL_LIGHT0, GL_QUADRATIC_ATTENUATION, att[2] );

   // Light 1
   glLightfv( GL_LIGHT1, GL_POSITION, lightPos2 );
   glLightfv( GL_LIGHT1, GL_AMBIENT, Al );
   glLightfv( GL_LIGHT1, GL_DIFFUSE, Dl );
   glLightfv( GL_LIGHT1, GL_SPECULAR, Sl );
   glLightf( GL_LIGHT1, GL_CONSTANT_ATTENUATION, att[0] );
   glLightf( GL_LIGHT1, GL_LINEAR_ATTENUATION, att[1] );
   glLightf( GL_LIGHT1, GL_QUADRATIC_ATTENUATION, att[2] );
   

   glColor3f(1.0, 0.0, 0.0);
   glPushMatrix();
   glTranslatef(lightPos[0], lightPos[1], lightPos[2]);
   glutSolidCube(1);
   glPopMatrix();
   glPushMatrix();
   glTranslatef(lightPos2[0], lightPos2[1], lightPos2[2]);
   glutSolidCube(1);
   glPopMatrix();

   glPopMatrix();
   */


}

void Scene::draw()
{
   //setupCamera();

   /*
   glPushMatrix();
   //glRotatef(lightRot, 0.0, 1.0, 0.0); lightRot += 0.02;
   setupLights();
   glPopMatrix();
   */

   // TODO: apply transform
   //glRotatef(angle, 1.0, 0.0, 1.0); //angle += 0.02;

   //glTranslatef(-100.0, 0.0, 0.0);

   // enable the shader program
   activeShader->attach();
   GLint enableShadows = glGetUniformLocation(activeShader->shaderProg, "enableShadows");
   glUniform1f(enableShadows, 0.0);

   renderScene();

   glUseProgram(0);
}

void Scene::renderScene()
{
   // Render static elements
   for each (Drawable* d in m_staticObjects)
   {
      d->draw();
   }

   // Render dynamic elements
   for each (Drawable* d in m_dynamicObjects)
   {
      //if (d->id.find("Shape") != std::string::npos)
         d->draw();
   }
}


void Scene::addShape(Shape* shape, bool dynamic)
{
   // dynamic object
   if (dynamic)
   {
      m_dynamicObjects.push_back(shape);
   }
   // static object
   else
   {
      // TODO: Static elements??
   }
}

void Scene::addLight(Light* l)
{
   m_lights.push_back(l);
}

void Scene::printElements()
{
   printf("Scene:\n");
   printf("\tDynamic:\n");
   for each(Shape* s in m_dynamicObjects)
   {
      printf("\t\t%s\n", s->id.c_str() );
   }
   printf("\tStatic:\n");
   for each(Shape* s in m_staticObjects)
   {
      printf("\t\t%s\n", s->id.c_str() );
   }
}
