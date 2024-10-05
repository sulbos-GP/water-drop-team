#include "screencasts.h"
#pragma warning(disable:4996)
/* �ؽ�ó�� ���� ��Ŵ*/
static void reverse(void* x,const int n)
{
   int k;
   char* ch = (char*)x;
   for (k=0;k<n/2;k++)
   {
      char tmp = ch[k];
      ch[k] = ch[n-1-k];
      ch[n-1-k] = tmp;
   }
}


unsigned int loadTexBMP(char* file)
{
  unsigned int   texture;    // �ؽ�ó �̸�
  FILE*          f;          // ���� ������ ����
  unsigned short magic;      /* Image magic */
  unsigned int   dx,dy,size; /* Image dimensions */
  unsigned short nbp,bpp;    /* Planes and bits per pixel */
  unsigned char* image;      /* Image data */
  unsigned int   k;          /* Counter */

  f = fopen(file,"rb");//������ ��
  if (!f) exit(1);
  /*  �̹��� ���� üũ */
  if (fread(&magic,2,1,f)!=1) exit(1);
  if (magic!=0x4D42 && magic!=0x424D) exit(1);
  /*  ���� ������ ��ġ���� ������ ���� */
  if (fseek(f,16,SEEK_CUR) || fread(&dx ,4,1,f)!=1 || fread(&dy ,4,1,f)!=1 ||
      fread(&nbp,2,1,f)!=1 || fread(&bpp,2,1,f)!=1 || fread(&k,4,1,f)!=1)
	  exit(1);
  /*  ���������� �о���� */
  if (magic==0x424D) {
    reverse(&dx,4);
    reverse(&dy,4);
    reverse(&nbp,2);
    reverse(&bpp,2);
    reverse(&k,4);
  }
  /* �̹��� ���� ���� �˻� */
  if (dx<1 || dx>65536) exit(1);
  if (dy<1 || dy>65536) exit(1);
  if (nbp!=1)  exit(1);
  if (bpp!=24) exit(1);
  if (k!=0)    exit(1);


  /*  �̹��� �޸� �Ҵ� */
  size = 3*dx*dy;
  image = (unsigned char*) malloc(size);
  if (!image) exit(1);
  /*  ���� ��ġ���� �̹����� ���� */
  if (fseek(f,20,SEEK_CUR) || fread(image,size,1,f)!=1) 
	  exit(1);
  fclose(f);
  /* ������ ���� BGR�÷��� RGB�÷��� ���� */
  for (k=0;k<size;k+=3) {
    unsigned char temp = image[k];
    image[k]   = image[k+2];
    image[k+2] = temp;
  }

  /*  �˻� */
  int err = glGetError();
  if (err) {
	  fprintf(stderr, "ERROR: %s [%s]\n", gluErrorString(err), "loadTexBMP");
	  exit(1);
  }// ���� üũ ���
  /*  Generate 2D texture */
  glGenTextures(1,&texture);
  glBindTexture(GL_TEXTURE_2D,texture);
  /* �̹��� ���� */
  glTexImage2D(GL_TEXTURE_2D,0,3,dx,dy,0,GL_RGB,GL_UNSIGNED_BYTE,image);
  if (glGetError()) exit(1);
  /*  �̹��� ũ�Ⱑ ��ġ���� ������ ��ҽ�Ŵ */
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

  /* �̹��� �޸� ���� */
  free(image);
  /*  �ؽ�ó �̸� ��ȯ */
  return texture;
}
