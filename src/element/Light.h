//-----------------------------------------------------------------------------
// Interface for an OpenGL light object
//-----------------------------------------------------------------------------



#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "../util/vec3.h"
#include <GL/glut.h>
#include <math.h>

#define PI 3.14159265


struct Light
{

public:
   // Ambient
   Vec3f m_ambient;
   // Diffuse
   Vec3f m_diffuse;
   // Specular
   Vec3f m_specular;
   // Position
   Vec3f m_position;
   // Constant Attenuation
   float m_constantAttenuation;
   // Linear Attenuation
   float m_linearAttenuation;
   // Quadratic Attenuation
   float m_quadraticAttenuation;
   // Spot Cutoff (spot angle degrees)
   float m_spotCutoff;
   // Spot Direction
   Vec3f m_spotDirection;

   bool m_enabled;


   Light(Vec3f p, Vec3f d = Vec3f(), Vec3f a = Vec3f(), Vec3f s = Vec3f(),
         float ca = 1.0, float la = 0.0, float qa = 0.0,
         Vec3f spotDir = Vec3f(),
         float innerCutoff = 180.0, float outerCutoff = 180.0) :
      m_enabled(true),
      m_position(p),
      m_diffuse(d),
      m_ambient(a),
      m_specular(s),
      m_constantAttenuation(ca),
      m_linearAttenuation(la),
      m_quadraticAttenuation(qa),
      m_spotDirection(spotDir),
      m_spotCutoff(innerCutoff)
      //setOuterCutoff(outerCutoff)
      //m_spotOuterCutoff(180.0)
   { setOuterCutoff(outerCutoff); }



   // toggles the light
   void toggleLight() {m_enabled = !m_enabled;};

   void applyLight(GLenum lightNum)
   {
      GLfloat lightPos[] = {m_position.x, m_position.y, m_position.z, 1.0f};
      GLfloat Al[] = {m_ambient.x, m_ambient.y, m_ambient.z, 1.0f};
      GLfloat Dl[] = {m_diffuse.x, m_diffuse.y, m_diffuse.z, 1.0f};
      GLfloat Sl[] = {m_specular.x, m_specular.y, m_specular.z, 1.0f};

      glLightfv( lightNum, GL_POSITION, lightPos );
      glLightfv( lightNum, GL_AMBIENT, Al );
      glLightfv( lightNum, GL_DIFFUSE, Dl );
      glLightfv( lightNum, GL_SPECULAR, Sl );
      glLightf( lightNum, GL_CONSTANT_ATTENUATION, m_constantAttenuation );
      glLightf( lightNum, GL_LINEAR_ATTENUATION, m_linearAttenuation );
      glLightf( lightNum, GL_QUADRATIC_ATTENUATION, m_quadraticAttenuation );

      glLightf( lightNum, GL_SPOT_CUTOFF, m_spotCutoff);
      // TODO: set the outer cutoff
   }

   // Sets the angle for the spot light outer cutoff
   void setOuterCutoff(float angle) { m_spotOuterCutoff = cos(angle*PI/180.0); }

   protected:

   // Outer cone angle cos of angle
   float m_spotOuterCutoff;

};

#endif
