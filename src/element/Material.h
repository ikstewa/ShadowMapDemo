#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include <string>
#include "../util/vec3.h"


using namespace::std;

/**
 * Object to represent an OpenGL material.
 * Currently just color.
 * Maybe extend to support textures.
 */
class Material
{
   public:

      string id;

      unsigned int m_texID;

      float m_ambientIntensity;
      Vec3f m_diffuseColor;
      Vec3f m_emissiveColor;
      float m_shininess;
      Vec3f m_specularColor;

      // do i need??
      float m_transparency;

      Material() : m_ambientIntensity(0.0),
                   m_diffuseColor(Vec3f()),
                   m_emissiveColor(Vec3f()),
                   m_shininess(0.0),
                   m_specularColor(Vec3f()),
                   m_transparency(0.0),
                   m_texID(0)
      {}

      void attach();

};

#endif
