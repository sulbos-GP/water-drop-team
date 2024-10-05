#include "vector.h"

vector3d F3dVector ( GLfloat x, GLfloat y, GLfloat z )
{
	vector3d tmp;
	tmp.x = x;
	tmp.y = y;
	tmp.z = z;
	return tmp;
}
vector3d Add3dVectors (vector3d* u, vector3d* v)
{
	vector3d result;
	result.x = u->x + v->x;
	result.y = u->y + v->y;
	result.z = u->z + v->z;
	return result;
}
void Add3dVectorToVector (vector3d * Dst, vector3d * V2)
{
	Dst->x += V2->x;
	Dst->y += V2->y;
	Dst->z += V2->z;
}
