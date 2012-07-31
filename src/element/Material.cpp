#include "Material.h"

#include "../texlib/TextureLibrary.h"
#include <GL/glut.h>


void Material::attach()
{
   
   glColor3f(m_diffuseColor.x, 
               m_diffuseColor.y, 
               m_diffuseColor.z);

   GLfloat Am[4] = {0.3f, 0.3f, 0.3f, 1.0f};
   GLfloat Dm[4] = {m_diffuseColor.x, m_diffuseColor.y, m_diffuseColor.z, 1.0f};
   GLfloat Sm[4] = {0.6f, 0.6f, 0.6f, 1.0f};
   float f = 60.0f;
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Am);
   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Dm);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Sm);
   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, f);


   // TODO: attach texture
   if (m_texID)
      TextureLibrary::getInstance()->Activate(m_texID);
   else
      TextureLibrary::getInstance()->Activate(TEXTURE_DISABLE);
}