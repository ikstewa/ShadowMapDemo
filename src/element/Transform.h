#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include <string>
#include "../util/vec3.h"


/**
 * Class used to represent a 3d space transform.
 */
class Transform
{
	public:

      // @param rotation, translation
      Transform(Vec3f rot, Vec3f trans);

      std::string id;

		Vec3f m_rotation;
		Vec3f m_translation;



		void applyTransform();

};

#endif
