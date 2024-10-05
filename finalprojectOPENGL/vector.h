#include "glut.h"		

typedef struct _Vector3D 
{
	GLfloat x, y, z;
}vector3d;
struct SF2dVector
{
	GLfloat x,y;
};

vector3d F3dVector ( GLfloat x, GLfloat y, GLfloat z );
vector3d Add3dVectors (vector3d * u, vector3d * v);
void Add3dVectorToVector (vector3d * Dst, vector3d * V2);
