#include "screencasts.h"

/* ��ī�̹ڽ��� �׸��� �Լ� ȣ��*/
void drawScene()
{
  drawSkybox(35);
}

/* �ҷ��� �ؽ��ĸ� ��ī�̹ڽ��� ����.*/
void drawSkybox(double D)
{
  glColor3f(1,1,1);
  glEnable(GL_TEXTURE_2D);

  /* ���� 4�� */
  glBindTexture(GL_TEXTURE_2D,skybox[SKY_RIGHT]);
  glBegin(GL_QUADS);
  glTexCoord2f(0,0); glVertex3f(-D,-D,-D);
  glTexCoord2f(1,0); glVertex3f(+D,-D,-D);
  glTexCoord2f(1,1); glVertex3f(+D,+D,-D);
  glTexCoord2f(0,1); glVertex3f(-D,+D,-D);
  glEnd();
  glBindTexture(GL_TEXTURE_2D,skybox[SKY_FRONT]);
  glBegin(GL_QUADS);
  glTexCoord2f(0,0); glVertex3f(+D,-D,-D);
  glTexCoord2f(1,0); glVertex3f(+D,-D,+D);
  glTexCoord2f(1,1); glVertex3f(+D,+D,+D);
  glTexCoord2f(0,1); glVertex3f(+D,+D,-D);
  glEnd();
  glBindTexture(GL_TEXTURE_2D,skybox[SKY_LEFT]);
  glBegin(GL_QUADS);
  glTexCoord2f(0,0); glVertex3f(+D,-D,+D);
  glTexCoord2f(1,0); glVertex3f(-D,-D,+D);
  glTexCoord2f(1,1); glVertex3f(-D,+D,+D);
  glTexCoord2f(0,1); glVertex3f(+D,+D,+D);
  glEnd();
  glBindTexture(GL_TEXTURE_2D,skybox[SKY_BACK]);
  glBegin(GL_QUADS);
  glTexCoord2f(0,0); glVertex3f(-D,-D,+D);
  glTexCoord2f(1,0); glVertex3f(-D,-D,-D);
  glTexCoord2f(1,1); glVertex3f(-D,+D,-D);
  glTexCoord2f(0,1); glVertex3f(-D,+D,+D);
  glEnd();

  /* ��,�Ʒ� */
  glBindTexture(GL_TEXTURE_2D,skybox[SKY_UP]);
  glBegin(GL_QUADS);
  glTexCoord2f(0,0); glVertex3f(-D,+D,-D);
  glTexCoord2f(1,0); glVertex3f(+D,+D,-D);
  glTexCoord2f(1,1); glVertex3f(+D,+D,+D);
  glTexCoord2f(0,1); glVertex3f(-D,+D,+D);
  glEnd();
  glBindTexture(GL_TEXTURE_2D,skybox[SKY_DOWN]);
  glBegin(GL_QUADS);
  glTexCoord2f(1,1); glVertex3f(+D,-D,-D);
  glTexCoord2f(0,1); glVertex3f(-D,-D,-D);
  glTexCoord2f(0,0); glVertex3f(-D,-D,+D);
  glTexCoord2f(1,0); glVertex3f(+D,-D,+D);
  glEnd();

  glDisable(GL_TEXTURE_2D);
}
