
#include "Transform.h"

Transform::Transform(Vec3f rot, Vec3f trans)
{
   m_rotation = rot;
   m_translation = trans;
}


void Transform::applyTransform()
{
	printf("APPLYING TRANSFORM!\n");
}
