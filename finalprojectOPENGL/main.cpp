
#include "glut.h"
#include <vector>
#include "screencasts.h"
#include "vector.h"


/* 스카이박스 매크로 상수*/
#define SKY_FRONT 0
#define SKY_RIGHT 1
#define SKY_LEFT 2
#define SKY_BACK 3
#define SKY_UP 4
#define SKY_DOWN 5

/* pool 매크로 상수*/
#define NUM_X_OSCILLATORS		300 // x 크기
#define NUM_Z_OSCILLATORS		300 // z 크기
#define NUM_OSCILLATORS			NUM_X_OSCILLATORS*NUM_Z_OSCILLATORS // 배열 크기
#define OSCILLATOR_DISTANCE		0.05 // Vertex 간격
#define OSCILLATOR_WEIGHT       0.0002 // 가중치


bool allLight = false; // 라이트 on/off bool 변수
bool pingpong = false; // input on/off bool 변수
bool poolReset = false; // reset on/off bool 변수
float _zoom = 15.0f; // zoom 변수 값
float _rotate_x = 0.0f; // 회전  x값
float _rotate_y = 0.001f; // 회전 y 값
float _translate_x = 0.0f; // 이동 x 값
float _translate_y = 0.0f; // 이동 y 값
int last_x = 0;
int last_y = 0;
unsigned char _btnStates[3] = { 0 }; //버튼 상태 배열 
/* 반사 큐브 함수 */
void colorcube();
void polygon(int a, int b, int c, int d);

/* 반사 큐브 Vertex 값 2차원 배열*/
GLfloat vertices[][3] =
{
	{-3.0,-3.0,-3.0},
{-1.0,-3.0,-3.0},
{-1.0,-1.0,-3.0},
{-3.0,-1.0,-3.0},
{-3.0,-3.0,-1.0},
{-1.0,-3.0,-1.0},
{-1.0,-1.0,-1.0},
{-3.0,-1.0,-1.0}
};

/* 반사 큐브 폴리곤 지정 값 */
void colorcube() {
	polygon(4, 5, 6, 7);
	polygon(0, 3, 2, 1);
	polygon(1, 2, 6, 5);
	polygon(0, 4, 7, 3);
	polygon(3, 7, 6, 2);
	polygon(0, 1, 5, 4);

}
void polygon(int a, int b, int c, int d) {

	glBegin(GL_TRIANGLES);
	{
		glVertex3fv(vertices[a]);
		glVertex3fv(vertices[b]);
		glVertex3fv(vertices[c]);
	}
	glEnd();
	glBegin(GL_TRIANGLES);
	{
		glVertex3fv(vertices[a]);
		glVertex3fv(vertices[c]);
		glVertex3fv(vertices[d]);
	}
	glEnd();

}

/* Pool Struct*/
typedef struct _Pool
{
	GLfloat x, y, z;
	GLfloat nx, ny, nz;  //normal vector
	GLfloat UpSpeed;
	GLfloat newY;
	bool bIsExciter;
	//only in use, if bIsExciter is true:
	float ExciterAmplitude;
	float ExciterFrequency;
}pool;

/* Lighting Array*/
GLfloat LightAmbient[] = { 0.2f, 0.6f, 1.0f, 0.5f };
GLfloat LightDiffuse[] = { 0.2f, 0.6f, 1.0f, 0.5f };
GLfloat LightPosition[] = { 1.0f, 1.0f, -0.5f, 0.0f };
GLfloat Lightspecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };


pool * Oscillators; // 구조체 
int NumOscillators;  //Vertex Array Size
std::vector <GLuint> IndexVect;  //we first put the indices into this vector, then copy them to the array below 배열에 추가하기 전에 넣는 동적 배열
GLuint * Indices; // 메모리 할당 포인터
int NumIndices;   //size of the index array 배열 Index Size

float g_timePassedSinceStart = 0.0f;  //note: this need not be the real time  주기적 시간체크 (실시간 아님)
bool  g_bExcitersInUse = true; // Input Check

GLfloat GetF3dVectorLength(vector3d * v) // 벡터의 길이 반환
{
	return (GLfloat)(sqrt(v->x*v->x + v->y*v->y + v->z*v->z));
}
vector3d CrossProduct(vector3d * u, vector3d * v) // 벡터의 외적 반환
{
	vector3d resVector;
	resVector.x = u->y*v->z - u->z*v->y;
	resVector.y = u->z*v->x - u->x*v->z;
	resVector.z = u->x*v->y - u->y*v->x;
	return resVector;
}
vector3d Normalize3dVector(vector3d v) // 벡터의 정규화 (방향벡터 )
{
	vector3d res;
	float l = GetF3dVectorLength(&v);
	if (l == 0.0f) return F3dVector(0.0f, 0.0f, 0.0f);
	res.x = v.x / l;
	res.y = v.y / l;
	res.z = v.z / l;
	return res;
}
vector3d operator+ (vector3d v, vector3d u) // 벡터연산 '+'
{
	vector3d res;
	res.x = v.x + u.x;
	res.y = v.y + u.y;
	res.z = v.z + u.z;
	return res;
}
vector3d operator- (vector3d v, vector3d u) // 벡터연산 '-'
{
	vector3d res;
	res.x = v.x - u.x;
	res.y = v.y - u.y;
	res.z = v.z - u.z;
	return res;
}


void CreatePool() // Pool 객체 생성
{
	NumOscillators = NUM_OSCILLATORS; // Vertex Array 의 크기 대입
	Oscillators = new pool[NUM_OSCILLATORS]; // Pool 의 x*z 크기 만큼 객체 배열 생성
	IndexVect.clear();  //to be sure it is empty //  객체를 생성하기 전에 입력하는 벡터 컨테이너를 비움.
	for (int xc = 0; xc < NUM_X_OSCILLATORS; xc++) { // x좌표의 크기만큼 반복
		for (int zc = 0; zc < NUM_Z_OSCILLATORS; zc++) // z좌표의 크기만큼 반복
		{
			Oscillators[xc + zc * NUM_X_OSCILLATORS].x = OSCILLATOR_DISTANCE * float(xc); // 0.05 씩 증가
			Oscillators[xc + zc * NUM_X_OSCILLATORS].y = 0.0f; // y 좌표 0 고정
			Oscillators[xc + zc * NUM_X_OSCILLATORS].z = OSCILLATOR_DISTANCE * float(zc); // 0.05 씩 증가

			Oscillators[xc + zc * NUM_X_OSCILLATORS].nx = 0.0f; // 노멀 벡터 x =0
			Oscillators[xc + zc * NUM_X_OSCILLATORS].ny = 1.0f; // 노멀	벡터 y =1
			Oscillators[xc + zc * NUM_X_OSCILLATORS].nz = 0.0f; // 노멀 벡터 z=0

			Oscillators[xc + zc * NUM_X_OSCILLATORS].UpSpeed = 0; //요동 치는 스피드
			Oscillators[xc + zc * NUM_X_OSCILLATORS].bIsExciter = false; // Input 정점 아님

			 // Triangle 모델로 생성
			if ((xc < NUM_X_OSCILLATORS - 1) && (zc < NUM_Z_OSCILLATORS - 1))
			{
				IndexVect.push_back(xc + zc * NUM_X_OSCILLATORS); // 1
				IndexVect.push_back((xc + 1) + zc * NUM_X_OSCILLATORS); // 2
				IndexVect.push_back((xc + 1) + (zc + 1)*NUM_X_OSCILLATORS); //4

				IndexVect.push_back(xc + zc * NUM_X_OSCILLATORS); //1
				IndexVect.push_back((xc + 1) + (zc + 1)*NUM_X_OSCILLATORS); //4
				IndexVect.push_back(xc + (zc + 1)*NUM_X_OSCILLATORS); //3
			}
		}
	}
	 //indices 포인터에 indexVect에 담았던 크기만큼 할당 (Oscillators 배열의 크기보다 작음)
	Indices = new GLuint[IndexVect.size()]; 
	for (int i = 0; i < IndexVect.size(); i++)
	{
		Indices[i] = IndexVect[i];
	}
	/* Input 정점 설정*/
	Oscillators[100 + 30 * NUM_X_OSCILLATORS].bIsExciter = true;
	Oscillators[100 + 30 * NUM_X_OSCILLATORS].ExciterAmplitude = 0.5f;
	Oscillators[100 + 30 * NUM_X_OSCILLATORS].ExciterFrequency = 50.0f;
	Oscillators[30 + 80 * NUM_X_OSCILLATORS].bIsExciter = true;
	Oscillators[30 + 80 * NUM_X_OSCILLATORS].ExciterAmplitude = 0.5f; // 진폭
	Oscillators[30 + 80 * NUM_X_OSCILLATORS].ExciterFrequency = 50.0f; // 진동수
	NumIndices = IndexVect.size(); //Vertex 크기
	IndexVect.clear();  // 메모리 비움
}


void UpdateScene(bool bEndIsFree, float deltaTime, float time)
{
	for (int xc = 0; xc < NUM_X_OSCILLATORS; xc++) //x 좌표 크기만큼 반복
	{
		for (int zc = 0; zc < NUM_Z_OSCILLATORS; zc++) // y좌표 크기만큼 반복
		{
			int ArrayPos = xc + zc * NUM_X_OSCILLATORS; // ArrayPos에 현재 Index값 대입
			
			if ((Oscillators[ArrayPos].bIsExciter) && g_bExcitersInUse) //if 현재 인덱스의 bIsExciter이 true(input 지점)이거나g_bExcitersInUse true(input 가능 상태)라면
			{
				Oscillators[ArrayPos].newY = Oscillators[ArrayPos].ExciterAmplitude*sin(time*Oscillators[ArrayPos].ExciterFrequency); 
				//현재 index의 newy값을 새로 대입(sin그래프를 (현재 실시간으로 계산하여) 진폭만큼 증폭시켜 대입))
			}
		
			if ((xc == 0) || (xc == NUM_X_OSCILLATORS - 1) || (zc == 0) || (zc == NUM_Z_OSCILLATORS - 1)) //엣지는 다른곳 에서 계산
				;
			else
			{
				//주변 오실레이터의 평균 y 거리 값
				float AvgDifference = Oscillators[ArrayPos - 1].y	//왼쪽 이웃
					+ Oscillators[ArrayPos + 1].y				//오른쪽 이웃
					+ Oscillators[ArrayPos - NUM_X_OSCILLATORS].y  //위쪽 이웃
					+ Oscillators[ArrayPos + NUM_X_OSCILLATORS].y  //아래쪽 이웃
					- 4 * Oscillators[ArrayPos].y;				//	 현재 y위치 값을 4번 뺀다. 평균치를 위해서 x4해서 뺌
				Oscillators[ArrayPos].UpSpeed += AvgDifference * deltaTime / OSCILLATOR_WEIGHT; //속도에 평균값*시간에대한 값을 가중치로 나눔

				Oscillators[ArrayPos].newY += Oscillators[ArrayPos].UpSpeed*deltaTime; // 파장 표현 계산한 값을 새로운 y값에 더함 ,주위 파장을 구현
			}
		}
	}

	// 화면에 표현 하기위해서 카피
	for (int xc = 0; xc < NUM_X_OSCILLATORS; xc++)
	{
		for (int zc = 0; zc < NUM_Z_OSCILLATORS; zc++)
		{
			Oscillators[xc + zc * NUM_X_OSCILLATORS].y = Oscillators[xc + zc * NUM_X_OSCILLATORS].newY;
		}
	}
	 // 빛 반사 벡터 부분 , 조명에 필요
	for (int xc = 0; xc < NUM_X_OSCILLATORS; xc++)
	{
		for (int zc = 0; zc < NUM_Z_OSCILLATORS; zc++)
		{
			vector3d u, v, p1, p2;	// u,v 방향 벡터 p1,p2는 임시

			if (xc > 0) p1 = F3dVector(Oscillators[xc - 1 + zc * NUM_X_OSCILLATORS].x, // 점점 왼쪽으로 벡터가 이동
				Oscillators[xc - 1 + zc * NUM_X_OSCILLATORS].y,
				Oscillators[xc - 1 + zc * NUM_X_OSCILLATORS].z);
			else // (xc==0) 이라면 (엣지)
				p1 = F3dVector(Oscillators[xc + zc * NUM_X_OSCILLATORS].x, // 벡터가 현재위치 유지
					Oscillators[xc + zc * NUM_X_OSCILLATORS].y,
					Oscillators[xc + zc * NUM_X_OSCILLATORS].z);
			if (xc < NUM_X_OSCILLATORS - 1) // 점점 벡터가 오른쪽으로 이동
				p2 = F3dVector(Oscillators[xc + 1 + zc * NUM_X_OSCILLATORS].x,
					Oscillators[xc + 1 + zc * NUM_X_OSCILLATORS].y,
					Oscillators[xc + 1 + zc * NUM_X_OSCILLATORS].z);
			else // (xc가 맨끝에 도달) 이라면 (오른쪽 엣지)
				p2 = F3dVector(Oscillators[xc + zc * NUM_X_OSCILLATORS].x, //벡터가 현재위치
					Oscillators[xc + zc * NUM_X_OSCILLATORS].y,
					Oscillators[xc + zc * NUM_X_OSCILLATORS].z);
			u = p2 - p1;  // 벡터가 왼쪽 이웃에서 오른쪽 이웃으로 이동 파장이 reflection됨 
			if (zc > 0) p1 = F3dVector(Oscillators[xc + (zc - 1)*NUM_X_OSCILLATORS].x, //점점 위쪽으로 벡터가 이동
				Oscillators[xc + (zc - 1)*NUM_X_OSCILLATORS].y,
				Oscillators[xc + (zc - 1)*NUM_X_OSCILLATORS].z);
			else// (zc가 위쪽에 도달) 이라면 (위쪽 엣지)
				p1 = F3dVector(Oscillators[xc + zc * NUM_X_OSCILLATORS].x,
					Oscillators[xc + zc * NUM_X_OSCILLATORS].y,
					Oscillators[xc + zc * NUM_X_OSCILLATORS].z);
			if (zc < NUM_Z_OSCILLATORS - 1)// 점점 벡터가 아래로 이동
				p2 = F3dVector(Oscillators[xc + (zc + 1)*NUM_X_OSCILLATORS].x,
					Oscillators[xc + (zc + 1)*NUM_X_OSCILLATORS].y,
					Oscillators[xc + (zc + 1)*NUM_X_OSCILLATORS].z);
			else// (zc가 아래쪽에 도달) 이라면 (아래쪽 엣지)
				p2 = F3dVector(Oscillators[xc + zc * NUM_X_OSCILLATORS].x,
					Oscillators[xc + zc * NUM_X_OSCILLATORS].y,
					Oscillators[xc + zc * NUM_X_OSCILLATORS].z);
			v = p2 - p1; // 벡터가 위쪽 이웃에서 아래쪽 이웃으로 이동 파장이 reflection 됨
			//calculat the normal:
			vector3d normal = Normalize3dVector(CrossProduct(&u, &v)); //노말 벡터 계산

			//assign the normal:
			Oscillators[xc + zc * NUM_X_OSCILLATORS].nx = normal.x; //노말 벡터x
			Oscillators[xc + zc * NUM_X_OSCILLATORS].ny = normal.y; //노말 벡터y
			Oscillators[xc + zc * NUM_X_OSCILLATORS].nz = normal.z; //노말 벡터z
		}
	}

}
void DrawScene(void)
{
	
	glEnable(GL_LIGHT1); //라이트 1 on
	glDrawElements(GL_TRIANGLES, //삼각형모드로 그림
		NumIndices,  // index길이
		GL_UNSIGNED_INT, // 인덱스 타입
		Indices);; // 인덱스 배열
	glDisable(GL_LIGHT1); //라이트 1 off
}
/* 표면 값을 리셋 시킴*/
void Reset() {
	for (int xc = 0; xc < NUM_X_OSCILLATORS; xc++)
		for (int zc = 0; zc < NUM_Z_OSCILLATORS; zc++)
		{
			Oscillators[xc + zc * NUM_X_OSCILLATORS].y = 0.0f;
			Oscillators[xc + zc * NUM_X_OSCILLATORS].UpSpeed = 0.0f;
			Oscillators[xc + zc * NUM_X_OSCILLATORS].nx = 0.0f;
			Oscillators[xc + zc * NUM_X_OSCILLATORS].ny = 0.0f;
			Oscillators[xc + zc * NUM_X_OSCILLATORS].nz = 0.0f;
		}
}
void Idle(void)
{
	float dtime = 0.004f;  //실시간은 아니지만 시간계산
	g_timePassedSinceStart += dtime; // 시간을 더함

	if (pingpong)
	{
		g_bExcitersInUse = false;  //인풋 중지
	}
	else {
		g_bExcitersInUse = true; // 인풋 가능
	}
	if (poolReset) { // 리셋시킴
		Reset();
		poolReset = !poolReset;
	}
	UpdateScene(false, dtime, g_timePassedSinceStart); // 표면 업데이트

	glutPostRedisplay();
}
void init() {
	glEnable(GL_DEPTH_TEST); //자동으로  z값에 저장
}
void changeSize(int w, int h) {
	if (h == 0)
		h = 1;

	float ratio = w * 1.0 / h;

	glMatrixMode(GL_PROJECTION);


	glLoadIdentity();


	glViewport(0, 0, w, h);


	gluPerspective(45.0f,  (float)w/h, 0.1f, 100.0f);


	glMatrixMode(GL_MODELVIEW);
}

void bench(void) {
	glPushMatrix();
	glColor4f(1, 1, 1,0.1);
	colorcube();
	glPopMatrix();
}
void renderScene() {
	glClearStencil(0); //0으로 버퍼를 초기화
	glClearDepth(1.0f); //1로 버퍼 초기화
	

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
		GL_STENCIL_BUFFER_BIT);

	// 위치 값 초기화
	glLoadIdentity();
	// 카메라 위치 설정
	gluLookAt(0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, -_zoom);
	glTranslatef(_translate_x, _translate_y, 0.0f);
	glRotatef(_rotate_x, 0, 1, 0);//x축 회전
	glRotatef(_rotate_y, 1, 0, 0);//y축 회전

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // 화면에 보여질 모습이 저장되는 버퍼에 아무것도 그리지 않도록 설정 (stencil 버퍼에 그림을 그리고 color buffer에 그림을 그리는것이 아니기 때문)
	//disable the color mask
	glDepthMask(GL_FALSE);//깊이 버퍼 비활성화

	glEnable(GL_STENCIL_TEST); // stencil 기능을 킴

	glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);  // stencil 버퍼에 값을 쓸때 어떠한 경우에 쓰고 어떠한 값을 쓸지 설정하는 함수, 0과1로 이루어진 비트플랜임. 0이면 통과실패  항상 통과하도록 설정
	//버퍼를 정의할 때 일일이 각각의 화소 위치에 대해서 1과 0을 세팅하는 것이 아니고 내가 원하는 위치에 물체를 그려넣음으로써 그 부분을 1로 채워지게 만드는것. 
	//원래 스텐실 버퍼는 0으로 가득 차있는데 내가 보여주고 싶은 부분만 그림을 그려넣는것. 그부분만 1로 모두 바뀌는것임. 두번째 매개변수로 스텐실 버퍼내용은 이값과 비교됨 
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE); // 첫번째: 스텐실 실패 두번째: 스탠실 성공,뎁스 테스트 실패 세번째: 스텐실 테스트,뎁스 테스트 성공 스텐실 값을 stencilfunc함수에서 지정한 레페런스 값으로 설정.

	bench(); //set the data plane to be replaced 교체할 데이터 평면 설정 

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);  // 컬러 마스크 활성화
	glDepthMask(GL_TRUE);  // 뎁스 버퍼 활성화

	glStencilFunc(GL_EQUAL, 1, 0xFFFFFFFF); //1과 같으면 통과 아닐시 폐기  0xFFFFFFFF는 어떠한 비트도 마스크도 하지 않겠다는 의미(defalut값임)
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);  // 스텐실 버퍼 내의 값 그대로 유지

	glDisable(GL_DEPTH_TEST); 
	glPushMatrix();
	glScalef(-1.0f, -1.0f, -1.0f); //그리는 화면 반전 시킴(반사)


	drawScene(); //push pop 사이에 반사될 객체를 그림.(진짜로 그리는것이아니라 반사되어 보여질 모습을 그림)
	glPopMatrix();
	glEnable(GL_DEPTH_TEST); 

	glDisable(GL_STENCIL_TEST);//스텐실 버퍼 비활성화

	glEnable(GL_BLEND);// 반투명화 버퍼
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 내가 지금 그리고자 하는것은 SRC ALPHA 에 의해 칠해지고 dst(이미 그려져있는것) 은 DST RGBA는 (1-src Alpha )로 칠할것이다.

	bench(); //(진짜 객체를 그림)

	glDisable(GL_BLEND); //반투명 비활성화



	drawScene(); //큐브맵 그림

	if (allLight) {
		glEnable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
	}
	else {
		glDisable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glMaterialf(GL_LIGHT2, GL_SHININESS, 0);
	}

	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);// 양면 모두 광원이 적용됨
	//이경우 glmaterialfv()로 양면의 물체 특성을 정의 해야함.

	DrawScene();
	glFlush();			//랜더링 끝,메모리 비우기
	glutSwapBuffers();
}

void GL_Motion(int x, int y) {
	int diff_x = x - last_x;
	int diff_y = y - last_y;

	last_x = x;
	last_y = y;

	if (_btnStates[2]) {
		_zoom -= (float)0.05f*diff_x;
	}
	else if (_btnStates[0]) {
		_rotate_x += (float)0.5f*diff_x;
		_rotate_y += (float)0.5f*diff_y;
	}
	else if (_btnStates[1]) {
		_translate_x += (float)0.05f*diff_x;
		_translate_y -= (float)0.05f*diff_y;
	}
	glutPostRedisplay();
}
void GL_Mouse(int button, int state, int x, int y) {
	last_x = x;
	last_y = y;
	switch (button) {
	case GLUT_LEFT_BUTTON:
		_btnStates[0] = ((GLUT_DOWN == state) ? 1 : 0);
		break;
	case GLUT_MIDDLE_BUTTON:
		_btnStates[1] = ((GLUT_DOWN == state) ? 1 : 0);
		break;
	case GLUT_RIGHT_BUTTON:
		_btnStates[2] = ((GLUT_DOWN == state) ? 1 : 0);
		break;
	default:
		break;
	}
	glutPostRedisplay();
}
void GL_Keyboard(unsigned char key, int x, int y) {
	switch (key)
	{
	case 27:	//ESC
		exit(0);
		break;
	case 'a':
		allLight = !allLight;
		std::cout << "GL_LIGHTING : " << std::boolalpha<<allLight << std::endl;
		break;
	case 's':
		pingpong = !pingpong;
		std::cout << "Input PingPong : " << std::boolalpha <<!pingpong << std::endl;
		break;
	case 'd':
		poolReset = !poolReset;
		std::cout << "Pool Reset " <<  std::endl;
		break;
	}
}

void initSkybox(void)
{
	/*
	  SKY_FRONT 0
	  SKY_RIGHT 1
	  SKY_LEFT 2
	  SKY_BACK 3
	  SKY_UP 4
	  SKY_DOWN 5
	 */
	skybox[SKY_FRONT] = loadTexBMP("front.bmp");
	skybox[SKY_RIGHT] = loadTexBMP("right.bmp");
	skybox[SKY_LEFT] = loadTexBMP("left.bmp");
	skybox[SKY_BACK] = loadTexBMP("back.bmp");
	skybox[SKY_UP] = loadTexBMP("top.bmp");
	skybox[SKY_DOWN] = loadTexBMP("bottom.bmp");
}
int main(int argc, char **argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL | GLUT_DEPTH);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL");

	init();
	initSkybox();

	CreatePool();


	glVertexPointer(3,   // 정점 좌표 데이터를 담고있는 메모리 위치를 설정. 정점당 좌표 개수,좌표의 데이터 종류,좌표의 메모리 간격, 배열 을 넘김.
		GL_FLOAT,
		sizeof(pool),
		Oscillators);
	glNormalPointer(GL_FLOAT,   //노멀 벡터 데이터가 포함되어있는 배열을 넘김.
		sizeof(pool),
		&Oscillators[0].nx);  

	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);


	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);// 앞뒤 둘다 색칠
	glEnable(GL_DEPTH_TEST);//z값을 depth buffer 에 저장하고 실패하면 z값 폐기


	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient); // 주변광
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse); // 분산광
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition); // 광원 위치
	glLightfv(GL_LIGHT1, GL_SPECULAR, Lightspecular); // 반사광의 반사 정도


	glEnable(GL_LIGHTING); // 라이팅 on

	glFrontFace(GL_CCW);  
	//반 시계 방향으로 그려진 것이 전면
	glShadeModel(GL_SMOOTH); // 모델을 부드럽게

	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(Idle);
	glutMouseFunc(GL_Mouse);
	glutMotionFunc(GL_Motion);
	glutKeyboardFunc(GL_Keyboard);

	glutMainLoop();

	return 1;
}




