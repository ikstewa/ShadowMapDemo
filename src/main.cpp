/**
 * Main file for the "eclps" demo.
 */
#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <string>
#include "x3d/X3dParser.h"
#include "element/Scene.h"
#include "texlib/TextureLibrary.h"
#include "shadelib/Shader.h"
#include "util/text.h"
#include "settings.h"
#include "util/Matrix.h"
#include "util/ScreenCap.h"


//#define _SCREEN_CAP_ // capture screen
#define CAPTURE_FILENAME "imgs\\test"

// Screen dimensions
int sWidth = 800;
int sHeight = 600;

// offscreen texture width
const int texWidth = 1024;
const int texHeight = 1024;

// shader and FBO
Shader flatShader;
Shader phongShader;
GLuint fbo;
GLuint depthBuffer;
GLuint imgTexture;

//----------------
// Shadows
GLfloat cPM[16];  // camera projection matrix
GLfloat cVM[16];  // camera view
GLfloat lPM[16];  // light proj
GLfloat lVM[16];  // light view
GLuint shadowMap;
GLuint shadowColor;
GLuint shadowFBO;
GLfloat texMat[16];
const float bMatrix[16] = {0.5f, 0.0f, 0.0f, 0.0f,
                           0.0f, 0.5f, 0.0f, 0.0f,
                           0.0f, 0.0f, 0.5f, 0.0f,
                           0.5f, 0.5f, 0.5f, 1.0f};
const int shadowMapSize = 512;
GLfloat light_fov[2] = {500.0, 11150.0};
GLfloat lightAngle = 0.0;
void testRender();



// Scene object
Scene *scene;

const GLfloat bgColor[] = { 0.1, 0.1, 0.1, 0.5 };




void init();
void initFBO();
void initShadows();
void renderTextures(GLuint);
void reshape(int, int);
void display();
void idle();


void init()
{
   glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );
   glutInitWindowSize(sWidth, sHeight);
   glutCreateWindow( "eclps" );

   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   glEnable(GL_DEPTH_TEST);
   glShadeModel(GL_SMOOTH);

   GLenum status = glewInit();
   if (status != GLEW_OK)
      fprintf(stderr, "ERROR: OpenGL 2.0 not supported!\n");
   else
      fprintf(stderr, "OpenGL 2.0 supported!\n");

   // reset the viewport
   reshape(sWidth, sHeight);
}

void initFBO()
{
   // grab the texture lib and disable mipmaps.. silly ATI
   TextureLibrary *texLib = TextureLibrary::getInstance();
   texLib->DisableMipmaps();

   GLint maxBuffers;
   glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &maxBuffers);
   fprintf(stderr, "MAX_COLOR_ATTACHMENTS: %d\n", maxBuffers);

   // setup the FBO
   glGenFramebuffersEXT(1, &fbo);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

   // create the render buffer for depth
   glGenRenderbuffersEXT(1, &depthBuffer);
   glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBuffer);
   glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, texWidth, texHeight);
   // bind the render buffer
   glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthBuffer);

   // Image Texture
   imgTexture = texLib->Load("imgTex");
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texLib->getGLID(imgTexture), 0);

   // define the render targets
   GLenum mrt[] = {GL_COLOR_ATTACHMENT0_EXT};
   glDrawBuffers(1, mrt);

   // check FBO status
   GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
   if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
   {
      fprintf(stderr, "ERROR: initFBO: FBO status not completed!\n");
      exit(1);
   }

   // unbind the FBO

   // Initialize the shaders
   flatShader.addFrag(SHADER_DIR("flat.fs"));
   flatShader.addVert(SHADER_DIR("mrt.vs"));
   flatShader.addFrag(SHADER_DIR("mrt.fs"));
   flatShader.build();
   flatShader.printProgramInfoLog();

   phongShader.addFrag(SHADER_DIR("phong.fs"));
   phongShader.addVert(SHADER_DIR("mrt.vs"));
   phongShader.addFrag(SHADER_DIR("mrt.fs"));
   phongShader.build();
   phongShader.printProgramInfoLog();
}

void generateShadowMatrix()
{
   //------------
   // precalc the matrices
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   // NOTE: Camera projection saved in render Textures
   glLoadIdentity();
   gluPerspective(45, (float)sWidth/(float)sHeight, 1.0, 5000.0);
   // save the camera matrix
   glGetFloatv(GL_MODELVIEW_MATRIX, cPM);

   // Camera setup in scene. Set LAP than grab matrix
   glLoadIdentity();
   scene->setupCamera();
   glGetFloatv(GL_MODELVIEW_MATRIX, cVM);

   glPopMatrix();
}

void initShadows()
{
   // shadow map FBO
   glGenFramebuffersEXT(1, &shadowFBO);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, shadowFBO);

   // shadow map depth texture
   glEnable(GL_TEXTURE_2D);
   glGenTextures(1, &shadowMap);
   glBindTexture(GL_TEXTURE_2D, shadowMap);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
   glTexImage2D(   GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                   shadowMapSize, shadowMapSize, 0,
                   GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
   glFramebufferTexture2DEXT(  GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                               GL_TEXTURE_2D, shadowMap, 0);

   // create the shadow map color texture
   glGenTextures(1, &shadowColor);
   glBindTexture(GL_TEXTURE_2D, shadowColor);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexImage2D(  GL_TEXTURE_2D, 0, GL_RGBA,
                  shadowMapSize, shadowMapSize, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, shadowColor, 0);


   // define the render targets
   /*
   GLenum mrt[] = {GL_COLOR_ATTACHMENT0_EXT};
   glDrawBuffers(1, mrt);
   */

   GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
   if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
   {
      printf("ERROR: FBO status not complete!\n");
      exit(1);
   }


   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
   


   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();

   // Light Projection
   glLoadIdentity();
   gluPerspective(45.0f, 1.0, light_fov[0], light_fov[1]);
   glGetFloatv(GL_MODELVIEW_MATRIX, lPM);

   // NOTE: Light view matrix should be setup on redraw
   glLoadIdentity();
   gluLookAt( 700.0, 500.0f, 0.0f, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
   /*
   gluLookAt(  100.0f,300.0f,300.0f, // camera
               0.0f, 0.0f, 0.0f,    // look at point
               0.0f, 1.0f, 0.0f);   // up vector
   //*/
   //glRotatef(lightAngle, 0.0, 1.0, 0.0); lightAngle += 0.1;
   glGetFloatv(GL_MODELVIEW_MATRIX, lVM);

   glPopMatrix();




   // TODO: NOT HERE!!
   // calculate the texture matrix
   // eye space to light clip space
   Matrix texMatrix = Matrix(bMatrix)*Matrix(lPM)*Matrix(lVM);
   texMatrix.asArray(texMat);

}

void renderShadows()
{
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, shadowFBO);

   generateShadowMatrix();

   //------------
   // Render the terrain shadow map
   glClearDepth(1.0f);
   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);
   glClearColor(0.0, 0.0, 0.0, 0.0);
   //glClearDepth(0.9);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // light projection
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glLoadMatrixf(lPM);

   // light view
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   glLoadMatrixf(lVM);

   glPushAttrib(GL_VIEWPORT_BIT);
   glViewport(0,0, shadowMapSize, shadowMapSize);

   // RENDER
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   glShadeModel(GL_FLAT);
   glColorMask(0,0,0,0);

   glUseProgram(0);
   //scene->setupLights();
   scene->renderScene();

   // restore state
   glShadeModel(GL_SMOOTH);
   glColorMask(1, 1, 1, 1);
   glCullFace(GL_BACK);

   glPopAttrib();

   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();

   glDisable(GL_TEXTURE_2D);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);


}

void renderTextures(GLuint fb)
{

   // setup the projection matrix
   glPushAttrib(GL_VIEWPORT_BIT);
   glViewport(0, 0, texWidth, texHeight);
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   gluPerspective(45, (float)sWidth/(float)sHeight, 1.0, 5000.0);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();




   //-----------------------------
   // Render the Shadows
   renderShadows();

   // bind the FBO for rendering
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
   // render to the FBO
   glClearColor( bgColor[0], bgColor[1], bgColor[2], bgColor[3] );
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   glLoadIdentity();

   // setup shadow textures
   glActiveTexture(GL_TEXTURE1);
   glMatrixMode(GL_TEXTURE);
   glPushMatrix();
   glLoadMatrixf(texMat);
   glMatrixMode(GL_MODELVIEW);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, shadowMap);

   glActiveTexture(GL_TEXTURE0);
   scene->activeShader->attach();

   GLint texColor = glGetUniformLocation(scene->activeShader->shaderProg, "imgTexture");
   GLint shadTex = glGetUniformLocation(scene->activeShader->shaderProg, "shadowMap");
   glUniform1i(texColor, 0);
   glUniform1i(shadTex, 1);
   GLint enableShad = glGetUniformLocation(scene->activeShader->shaderProg, "enableShadows");
   glUniform1f(enableShad, 1.0);



   //--------------------------------------------------------------------------
   // Render the scene
   scene->setupCamera();
   scene->setupLights();
   scene->draw();

   //
   //--------------------------------------------------------------------------

   glMatrixMode(GL_TEXTURE);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glActiveTexture(GL_TEXTURE1);
   glDisable(GL_TEXTURE_2D);
   glActiveTexture(GL_TEXTURE0);
   glDisable(GL_TEXTURE_2D);
   glUniform1f(enableShad, 0.0);

   // disable the shader program
   glUseProgram(0);

   // restore old viewport and projection
   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopAttrib();

   // disable framebuffer
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void reshape(int w, int h)
{
   sWidth = w;
   sHeight = h;

   glViewport( 0, 0, sWidth, sHeight );
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D( -100, 100, -100, 100 );
   //if ( h == 0 )
   //   gluPerspective(45, (float)sWidth, 1.0, 5000.0);
   //else
   //   gluPerspective(45, (float)sWidth/(float)sHeight, 1.0, 5000.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

void idle()
{
   glutPostRedisplay();
}


void testRender()
{
   glDisable(GL_LIGHTING);
   glDisable(GL_BLEND);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   TextureLibrary::getInstance()->Activate(TEXTURE_DISABLE);

   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();

   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();
   

   glViewport(0, 0, sWidth, sHeight);

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, shadowColor);
   //TextureLibrary::getInstance()->Activate(imgTexture);

   glTranslatef(0.0, 0.0, -0.5);
   
   glColor4f(1.0,1.0,1.0,1.0);
   glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f); glVertex2f(-1, -1);
      glTexCoord2f(1.0f, 0.0f); glVertex2f( 1, -1);
      glTexCoord2f(1.0f, 1.0f); glVertex2f( 1,  1);
      glTexCoord2f(0.0f, 1.0f); glVertex2f(-1,  1);
   glEnd();

   glPopMatrix();
   glMatrixMode(GL_PROJECTION);
   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);

   glutSwapBuffers();

}
void display()
{
   renderTextures(fbo);
   glUseProgram(0);

   //testRender();
   //return;

   TextureLibrary *texLib = TextureLibrary::getInstance();

   // Texture Frame color
   glClearColor(0.5, 0.5, 0.5, 0.5);
   glClearDepth(1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glLoadIdentity();

   glColor4f(1.0,1.0,1.0,1.0);

   // draw the screen quad
   texLib->Activate(imgTexture);
   //glEnable(GL_TEXTURE_2D);
   //glBindTexture(GL_TEXTURE_2D, shadowMap);
   glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f); glVertex2f(-99.0, -99.0);
      glTexCoord2f(1.0f, 0.0f); glVertex2f( 99.0, -99.0);
      glTexCoord2f(1.0f, 1.0f); glVertex2f( 99.0,  99.0);
      glTexCoord2f(0.0f, 1.0f); glVertex2f(-99.0,  99.0);
   glEnd();
   texLib->Deactivate(imgTexture);

   glColor3f(1.0, 1.0, 0.1);
   fps();

   glutSwapBuffers();

   // Screen capture
#ifdef _SCREEN_CAP_
   tgaGrabScreenSeries(CAPTURE_FILENAME, 0, 0, sWidth, sHeight);
#endif
}

void keyboard( unsigned char key, int x, int y )
{
   switch (key)
   {
      // ESC
      case 27:
         exit(0);
         break;
      case 'q':
         scene->activeShader = &flatShader;
         break;
      case 'w':
         scene->activeShader = &phongShader;
         break;
      case 'o':
         light_fov[0] += 5.0;
         printf("lightfov: %f\n", light_fov[0]);
         generateShadowMatrix();
         break;
      case 'k':
         light_fov[0] -= 5.0;
         printf("lightfov: %f\n", light_fov[0]);
         generateShadowMatrix();
         break;
      case 'p':
         light_fov[1] += 5.0;
         printf("lightfov: %f\n", light_fov[1]);
         generateShadowMatrix();
         break;
      case 'l':
         light_fov[1] -= 5.0;
         printf("lightfov: %f\n", light_fov[1]);
         generateShadowMatrix();
         break;
      default:
         break;
   }
}

void setupScene()
{
   // Parse the scene
   X3dParser *parser = new X3dParser();
   scene = parser->parseFile(SCENE_FILENAME);
   delete parser;

   if (!scene)
   {
      fprintf(stderr, "ERROR: Failed to load scene!\n");
      exit(1);
   }

   scene->activeShader = &phongShader;

   // lights
   Light* L1 = new Light( Vec3f(200.0, 200.0f, 0.0f), // position
                        Vec3f(1.0f, 1.0f, 1.0f), // diffuse
                        Vec3f(0.0f, 0.0f, 0.0f), // ambient
                        Vec3f(1.0f, 1.0f, 1.0f), // specular
                        1.0f,     // constant att
                        0.0f,     // constant att
                        0.0f,     // constant att
                        Vec3f(0.0, 0.0, 0.0), // spotDir
                        180.0,      // inner cutoff
                        180.0);    // outer cutoff
   /*
   Light* L2 = new Light( Vec3f(200.0f, 10.0f, -100.0f), // position
                        Vec3f(1.0f, 1.0f, 1.0f), // diffuse
                        Vec3f(0.0f, 0.0f, 0.0f), // ambient
                        Vec3f(1.0f, 1.0f, 1.0f), // specular
                        1.0,     // constant att
                        0.0,     // constant att
                        0.00001f,     // constant att
                        Vec3f(0.0, 0.0, 0.0), // spotDir
                        180.0,      // inner cutoff
                        180.0);    // outer cutoff
   */

               

   scene->addLight(L1);
   //scene->addLight(L2);
}




int main(int argc, char* argv[])
{
   // initialize
   glutInit(&argc, argv);
   init();
   // init FBO
   initFBO();
   // initialize shadow buffers
   initShadows();

   // register the callback functions
   glutDisplayFunc( display );
   glutReshapeFunc( reshape );
   glutKeyboardFunc( keyboard );
   glutIdleFunc( idle );

   setupScene();

   glutMainLoop();

   return 0;
}