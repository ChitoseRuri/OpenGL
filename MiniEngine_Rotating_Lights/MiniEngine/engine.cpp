#define _CRT_SECURE_NO_WARNINGS

// ͷ�ļ� /////////////////////////////////////////////////////////////////////

#include <windows.h>			// ���ɴ��ڡ�VS���Ѱ�װ Windows SDK������VC Express������
#include <stdio.h>				// ��׼�������
#include <vector>				// std����
#include <time.h>				// ʱ��

#include <gl\glut.h>			// GLUT��ʵ��ʹ��freeGLUT��
#include <gl\gl.h>				// OpenGL
#include <gl\glu.h>				// OpenGL utility
//#include <gl\glaux.h>			// OpenGL auxilary

using namespace std;			// ʹ��std�����ռ䣬�򻯱�������


// ȫ�ֲ��� ///////////////////////////////////////////////////////////////////

float _camx = 0.0f;				// �����λ��x
float _camy = 0.0f;				// �����λ��y
float _camz = 2.0f;				// �����λ��z
float _tgtx = 0.0f;				// �����Ŀ��x
float _tgty = 0.0f;				// �����Ŀ��y
float _tgtz = 0.0f;				// �����Ŀ��z

bool _fullscreen = false;		// ȫ����ʾ
bool _wireframe = true;			// �߿���Ⱦ
bool _cull_back_faces = true;	// ȥ�������棬�����Ⱦ�ٶ�
bool _lighting = true;			// ����
bool _smooth_shading = true;	// �⻬��Ⱦ

vector<float> _verts;			// �����������飺x0, y0, z0, x1, y1, z1, ...
vector<unsigned int> _faces;	// �����ζ���������飺v0x, v0y, v0z, v1x, v1y, v1z, ...
vector<float> _vert_normals;	// ���㷨�����顣ÿ�������Ӧһ�����ߡ�nx0, ny0, nz0, nx1, ny1, nz1, ...
vector<float> _face_normals;	// �淨�����顣ÿ�����Ӧһ�����ߡ�n0x, n0y, n0z, n1x, n1y, n1z, ...

float _scale = 1.0f;			// ��������
float _rot_x;					// ��x�����ת�Ƕ�
float _rot_y;					// ��y�����ת�Ƕ�

int _mouse_state = GLUT_UP;		// ��갴ť״̬
int _mouse_button;				// ��갴ť
int _mouse_x, _mouse_y;			// �ϴ�������Ļ����

clock_t _now, _justnow;			// ��ǰʱ�䡢�ղ�ʱ��
clock_t _msec;					// ����ʱ�䣨���룩�������ڼ��㶯���ٶ���ر���

clock_t _fps_time;				// ֡�ٲ�����ʼʱ��
int _fps_interval = 1;			// ֡�ٲ���������룩
int _fps_cnt = 0;				// ֡/��

char _title[128];				// ���ڱ���

const char _help[] = "Mini 3D Engine 1.1\n"
"LI Zheng, lizheng AT gdut.edu.cn\n"
"School of Computers, Guangdong University of Technology\n"
"Licensed under the GNU GPLv3.\n"
"Copyright: 2013 Guangdong University of Technology. All rights reserved.\n\n"
"Usage: MiniEngine [filename].obj\n"
"Note that only a limited version of Wavefront Object File format is supported.\n"
"arrow keys / left mouse button: rotate the model.\n"
"w, s, a, d, q, e: pan the camera\n"
"z, x: scale the model\n"
"r: reset the camera\n"
"h: print this help info\n"
"Esc: exit the program\n"
"F1: full screen / windowed mode\n"
"F2: wireframe / surface mode\n"
"F3: smooth / flat shading\n"
"F4: cull back faces on / off\n"
"F5: lighting on / off\n\n";

// ���� ///////////////////////////////////////////////////////////////////////

// ��ȡWavefront Object File��
// ֻ֧�����޵ĸ�ʽ��
// ��֧�������涥�����������ܴ����ߡ���ͼ����������
// 
// path:     �ļ�·��
// verts:    �������顣
//           ���� x0, y0, z0, x1, y1, z1, x2, y2, z2, ... ÿ��������ֵΪ˳���¼��
//           n������3*n����¼��
// faces:    ������Ķ���������顣
//           ���� f0_0, f0_1, f0_2, f1_0, f1_1, f1_2, f2_0, f2_1, f2_2, ... ÿ�����������Ϊ˳���¼��
//           n����������3*n����¼��
// centerize: ��������ƽ����ԭ�㡣
// normalize: �����С��һ����
bool load_obj_file(const char *path, vector<float> &verts, vector<unsigned int> &faces, bool centerize = false, bool normalize = false){
	FILE *file = fopen(path, "r");
	if (file == NULL)			// �Ҳ����ļ�
		return false;

	// ��С�������ֵ
	float minx, miny, minz, maxx, maxy, maxz;
	minx = miny = minz = maxx = maxy = maxz = 0.0f;

	while (true) {
		char lineHeader[128];
		// ��ȡ��ǰ�е�һ������
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)		// ���ļ���β���˳�ѭ��
			break;

		// �����ĵ�����
		if (strcmp(lineHeader, "v") == 0) {				// ��������
			float vx, vy, vz;
			fscanf(file, "%f %f %f\n", &vx, &vy, &vz);
			verts.push_back(vx);
			verts.push_back(vy);
			verts.push_back(vz);

			// �������꼫ֵ
			minx = min(vx, minx);	maxx = max(vx, maxx);
			miny = min(vy, miny);	maxy = max(vy, maxy);
			minz = min(vz, minz);	maxz = max(vz, maxz);
		}
		else if (strcmp(lineHeader, "f") == 0) {		// ������ <<��֧�������涥�����������ܴ����ߡ���ͼ����������>>
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3];
			int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
			if (matches != 3){
				printf("File format error. Please check the file content.\n");
				return false;
			}
			// ע�⣺obj�ļ��Ķ�����Ŵ�1��ʼ����������Ŵ�0��ʼ��
			faces.push_back(vertexIndex[0] - 1);
			faces.push_back(vertexIndex[1] - 1);
			faces.push_back(vertexIndex[2] - 1);
		}
		else {											// ����������
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}

	// ��������ƽ����ԭ��
	if (centerize) {
		// ���������Χ����������
		float cenx = 0.5f * (maxx + minx);
		float ceny = 0.5f * (maxy + miny);
		float cenz = 0.5f * (maxz + minz);

		// ƽ�ƶ���
		for (unsigned int i = 0; i < verts.size();) {
			verts[i++] -= cenx;
			verts[i++] -= ceny;
			verts[i++] -= cenz;
		}
	}

	// �����С��һ����������ԭ�����ŵ�����ߵ����ֵΪ1��
	if (normalize) {
		// �������ߵ����ֵ�����������ű���
		// Ϊ�ⳤ��ߵ����ֵ̫�ӽ�0��Ҫ���������������0.00001f�Ƚ�
		float scale = 1.0f / max(0.00001f, max(maxx - minx, max(maxy - miny, maxz - minz)));

		// ����ԭ�����Ŷ���λ��
		for (unsigned int i = 0; i < verts.size();) {
			verts[i++] *= scale;
		}
	}

	return true;
}

// GLUT�����Ϣ������ ///////////////////////////////////////////////////////

// ��ʼ��OpenGL�����ɴ��ں���á�
void init()
{
	if (_smooth_shading)
		glShadeModel(GL_SMOOTH);						// ʹ�ù⻬��Ⱦ
	else
		glShadeModel(GL_FLAT);							// ʹ��ƽ̹��Ⱦ
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// ��ɫ��������ֵ��RGBA��
	glClearDepth(1.0f);									// ��Ȼ�������ֵ��[0,1]������1��ʾ��Զ��0��ʾ�����
	glEnable(GL_DEPTH_TEST);							// ʹ����Ȳ���
	glDepthFunc(GL_LEQUAL);								// ������Ȳ��Է�����С�ڣ���ǰ��������ڣ�ͬ���ĳ���ص����ֵ��ȡ��֮��
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// �������͸��ͶӰ��������
	_wireframe
		? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) 	// ��Ⱦģʽ���߿�����
		: glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// ��Ⱦģʽ���������
	glCullFace(GL_BACK);								// �趨ȥ��ģʽ��ȥ��������
	_cull_back_faces
		? glEnable(GL_CULL_FACE)						// ��ȥ��������
		: glDisable(GL_CULL_FACE);						// �ر�ȥ��������

	// ���ù��պͲ��� /////////////////////////////////////
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);	// ������ɫ���ʲ���
	glEnable(GL_COLOR_MATERIAL);								// ʹ����ɫ����
	glColor3f(1.0, 1.0, 1.0);									// ����Ĭ�ϵ�������ɫ����ɫ��
	glEnable(GL_LIGHTING);										// ʹ�ù���

	// ���0�ŵ�
	GLfloat LightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };		// ��������ɫ
	GLfloat LightDiffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };		// ɢ�����ɫ
	GLfloat LightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };		// �߹���ɫ
	GLfloat LightPosition[] = { 1.0f, 1.0f, 1.0f, 0.0f };		// ƽ�й⣨ע����������0����ʾ������
	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);				// ���û�����
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);				// ����ɢ���
	glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);			// ���ø߹�
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);			// ����ƽ�й�
	glEnable(GL_LIGHT0);										// ��0�ŵ�

	// ���� ///////////////////////////////////////////////
	_justnow = _fps_time = clock();								// ��ʼ��ʱ��
}

// ��ʼ������
void init_scene(const char *path) {

	// ���������Ϣ ///////////////////////////////////////
	_verts.clear();
	_faces.clear();
	_vert_normals.clear();
	_face_normals.clear();

	// ��ģ���ļ� ///////////////////////////////////////
	if (!load_obj_file(path, _verts, _faces, true, true)) {
		printf("Error occured when initilizing the scene.");
		return;
	}

	// ���㷨�� ///////////////////////////////////////////

	unsigned int i0, i1, i2;
	float x0, y0, z0, x1, y1, z1, x2, y2, z2;
	float ux, uy, uz, vx, vy, vz;
	float nx, ny, nz;
	float length;

	// ��ʼ�����㷨��
	_vert_normals.assign(_verts.size(), 0.0f);

	// �����淨��
	for (unsigned int i = 0; i < _faces.size() / 3; ++i) {		// �������������Σ�ÿ3������Ϊһ��
		// ȡ�ø������ε�3����������
		i0 = _faces[i * 3] * 3;
		i1 = _faces[i * 3 + 1] * 3;
		i2 = _faces[i * 3 + 2] * 3;

		// ȡ�ø������ε�3�����������
		x0 = _verts[i0];   y0 = _verts[i0 + 1];   z0 = _verts[i0 + 2];
		x1 = _verts[i1];   y1 = _verts[i1 + 1];   z1 = _verts[i1 + 2];
		x2 = _verts[i2];   y2 = _verts[i2 + 1];   z2 = _verts[i2 + 2];

		// ��������������
		ux = x1 - x0;	uy = y1 - y0;	uz = z1 - z0;
		vx = x2 - x0;	vy = y2 - y0;	vz = z2 - z0;

		// ������
		nx = uy * vz - uz * vy;		ny = uz * vx - ux * vz;		nz = ux * vy - uy * vx;

		// �������滯�����淨������Ϊ��λ����
		length = sqrt(nx*nx + ny*ny + nz*nz);						// ���㳤��
		if (length < 0.00001f)
			length = 0.00001f;										// Ҫ���������Է�Ϊ��
		length = (float)(1.0 / length);								// Ԥ�ȼ��㵹��
		nx = nx * length;	ny = ny * length;	nz = nz * length;	// ��������Ϊ1

		// �����淨��
		_face_normals.push_back(nx);	_face_normals.push_back(ny);	_face_normals.push_back(nz);

		// ���淨���ۼӵ����㷨�ߣ�Ȩ��Ϊ1
		_vert_normals[i0] += nx;   _vert_normals[i0 + 1] += ny;   _vert_normals[i0 + 2] += nz;	// �ۼӵ���1�����㷨��
		_vert_normals[i1] += nx;   _vert_normals[i1 + 1] += ny;   _vert_normals[i1 + 2] += nz;	// �ۼӵ���2�����㷨��
		_vert_normals[i2] += nx;   _vert_normals[i2 + 1] += ny;   _vert_normals[i2 + 2] += nz;	// �ۼӵ���3�����㷨��
	}

	// ���㷨�߳������滯�������㷨������Ϊ��λ����
	for (unsigned int i = 0; i < _vert_normals.size() / 3; ++i) {
		// ȡ���ۼӽ��
		nx = _vert_normals[i * 3];
		ny = _vert_normals[i * 3 + 1];
		nz = _vert_normals[i * 3 + 2];

		// �������滯
		length = sqrt(nx*nx + ny*ny + nz*nz);						// ���㳤��
		if (length < 0.00001f)
			length = 0.00001f;										// Ҫ���������Է�Ϊ��
		length = (float)(1.0 / length);								// Ԥ�ȼ��㵹��
		_vert_normals[i * 3] = nx * length;							// ��������Ϊ1
		_vert_normals[i * 3 + 1] = ny * length;
		_vert_normals[i * 3 + 2] = nz * length;
	}
}

// ÿ���������ŵ��á�
// ע�⣺�����ڳ�ʼ������Զ�����һ�Ρ�
// w�������ؼ���Ĵ��ڿ��
// h�������ؼ���Ĵ��ڸ߶�
void reshape(int w, int h)
{
	// ��hҪ����������ͶӰ�ݺ��������Ԥ����Ϊ��
	if (h == 0)
		h = 1;

	// ����viewport
	glViewport(0, 0, w, h);			// ������viewport��ռ����������

	// ����Projection����
	glMatrixMode(GL_PROJECTION);	// ѡ���趨ͶӰ��Projection������
	glLoadIdentity();				// ��֮����Ϊ��λ����
	gluPerspective(45.0f,			// �����ӽ�
		1.0f * w / h,				// �����ݺ��
		0.01f,						// ����ƽ��
		100.0f);					// Զ��ƽ��

	// ����Model-View����M
	glMatrixMode(GL_MODELVIEW);	// ѡ���趨Model-View����
	glLoadIdentity();				// ��֮����Ϊ��λ����M=I
}

void draw_Cube(int cl[])		//���Ƶ�����ɫ������
{
	GLfloat color[7][3] = { { 1.0,1.0,1.0 }, {1.0,0.0,0.0},{0.0,1.0,0.0},{0.0,0.0,1.0},
{1.0,1.0,0.0},{1.0,0.0,1.0},{0,1.0,1.0} };
	glBegin(GL_QUADS);
	glColor3fv(color[cl[0]]);					//��6�������
	glVertex3f(0.14f, -0.14f, -0.14f);
	glVertex3f(0.14f, 0.14f, -0.14f);
	glVertex3f(0.14f, 0.14f, 0.14f);
	glVertex3f(0.14f, -0.14f, 0.14f);

	glColor3fv(color[cl[1]]);
	glVertex3f(-0.14f, -0.14f, 0.14f);
	glVertex3f(-0.14f, 0.14f, 0.14f);
	glVertex3f(-0.14f, 0.14f, -0.14f);
	glVertex3f(-0.14f, -0.14f, -0.14f);

	glColor3fv(color[cl[2]]);
	glVertex3f(-0.14f, 0.14f, 0.14f);
	glVertex3f(-0.14f, -0.14f, 0.14f);
	glVertex3f(0.14f, -0.14f, 0.14f);
	glVertex3f(0.14f, 0.14f, 0.14f);

	glColor3fv(color[cl[3]]);
	glVertex3f(-0.14f, 0.14f, -0.14f);
	glVertex3f(0.14f, 0.14f, -0.14f);
	glVertex3f(0.14f, -0.14f, -0.14f);
	glVertex3f(-0.14f, -0.14f, -0.14f);

	glColor3fv(color[cl[4]]);
	glVertex3f(-0.14f, 0.14f, -0.14f);
	glVertex3f(-0.14f, 0.14f, 0.14f);
	glVertex3f(0.14f, 0.14f, 0.14f);
	glVertex3f(0.14f, 0.14f, -0.14f);

	glColor3fv(color[cl[5]]);
	glVertex3f(-0.14f, -0.14f, -0.14f);
	glVertex3f(0.14f, -0.14f, -0.14f);
	glVertex3f(0.14f, -0.14f, 0.14f);
	glVertex3f(-0.14f, -0.14f, 0.14f);
	glEnd();
	glFlush();
}

int n ,m= 0, n1, n2, n3;				//n��ʹ�õ���ɫ������m�Ƿ�������
clock_t timef = clock();
void draw_scene() 
{
	if (m > 18)							//��ֹm���
	{
		m = 0;
	}
	if (clock() - timef >= 100)			//����ʱ��ı�ÿ�����ɫ����
	{
		m+=9;
		timef = clock();
	}
	n = m;
	glPushMatrix();
	int PlaColor[27][6] = { {0,1,0,2,0,0},{ 0,0,0,2,0,0 },{ 0,0,0,2,0,3 }		//���һ��27������Ĳ�ͬ����ɫ�䷽
	,{ 0,0,0,2,0,0 },{ 0,0,0,2,0,0 },{ 0,0,0,2,0,0 }
	,{ 0,0,0,2,5,0 },{ 0,0,0,2,0,0 },{ 4,0,0,2,0,0 }
	,{ 0,0,0,0,0,0 },{ 0,0,0,0,0,3 },{ 0,0,0,0,0,0 }
	,{ 0,1,0,0,0,0 },{ 0,0,0,0,0,0 },{ 4,0,0,0,0,0 }
	,{ 0,0,0,0,0,0 },{ 0,0,0,0,5,0 },{ 0,0,0,0,0,0 }
	,{ 0,0,6,0,0,3 },{ 0,0,6,0,0,0 },{ 4,0,6,0,0,0 }
	,{ 0,0,6,0,0,0 },{ 0,0,6,0,0,0 },{ 0,0,6,0,0,0 }
	,{ 0,1,6,0,0,0 },{ 0,0,6,0,0,0 },{ 0,0,6,0,5,0 } };

	for (n1 = 0; n1 < 37; n1++)		//37*3*3������
	{
		for (n2 = 0; n2 < 3; n2++)
		{
			for (n3 = 0; n3 < 3; n3++,n++)
			{
				if (n >= 27)				//һ��������Ϊ27������ֹ���
				{
					n = 0;
				}
				draw_Cube(PlaColor[n]);		//����
				glTranslatef(0.28f, 0, 0);	//�ı��´λ���ʱX������
			}
			glTranslatef(-0.84f, 0.28f, 0);//X���������ı�Y��
		}
		glTranslatef(0, -0.84f, 0.28f);//X�ᡢY���������ı�Z��
	}
	glPopMatrix();
 }

// ÿ����Ҫ�ػ�ʱ���á�
// ע�⣺�����ڳ�ʼ�������ź���Զ����á�
void display(void)
{
	// ��ʱ ///////////////////////////////////////////////
	_now = clock();					// ��ǰʱ��
	_msec = _now - _justnow;		// ����ʱ�䣨���룩
	_justnow = _now;				// ���浱ǰʱ��

	// ����֡�� ///////////////////////////////////////////
	_fps_cnt += 1;											// ֡��+1
	// ÿ _fps_interval �����һ��֡��
	if (_now - _fps_time > CLK_TCK * _fps_interval) {		// CLK_TCK=1000
		_fps_time = _now;
		sprintf(_title, "Mini 3D Engine - %dfps", _fps_cnt / _fps_interval);
		glutSetWindowTitle(_title);
		_fps_cnt = 0;
	}

	// ��������ɫ���桢��Ȼ��棩 /////////////////////////
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ����Model-View����M ////////////////////////////////
	glLoadIdentity();				// ��֮����Ϊ��λ����M=I
	gluLookAt(						// �����������ʹM=C
		_camx, _camy, _camz,		// �������λ��
		_tgtx, _tgty, _tgtz,		// �۲�Ŀ���λ��
		0.0f, 1.0f, 0.0f);			// ������ġ��ϡ���

	// ���Ƴ��� ///////////////////////////////////////////

	if (_rot_x > 90.0f)					// ������תֵ�Ի�����õĹ۲�Ч��
		_rot_x = 90.0f;
	else if (_rot_x < -90.0f)
		_rot_x = -90.0f;

	glPushMatrix();						// ��ǰModel-View����M=C����ջ

	glTranslatef(0.0f, 0.0f, 0.0f);		// �޸�Model-View����ʹM=C*T
	glRotatef(_rot_x, 1.0, 0.0, 0.0);	// �޸�Model-View����ʹM=C*T*Rx
	glRotatef(_rot_y, 0.0, 1.0, 0.0);	// �޸�Model-View����ʹM=C*T*Rx*Ry
	glScalef(_scale, _scale, _scale);	// �޸�Model-View����ʹM=C*T*Rx*Ry*S

	draw_scene();

	glPopMatrix();					// Model-View����M��ջ��ʹM=C

	// �л������� /////////////////////////////////////////
	glutSwapBuffers();
}

// ������ͨ�ַ���Ϣ������
void keyboard(unsigned char key, int x, int y)
{
	key = tolower(key);
	switch (key) {
	case 27:						// ESC: �˳�����
		exit(0);
		break;

	case 'z':						// z: ����Ŵ�
		_scale += 0.05f;
		break;

	case 'x':						// x: ������С
		_scale -= 0.05f;
		break;

	case 'w':						// w: �����ƽ��
		_camy -= 0.05f;
		_tgty -= 0.05f;
		break;

	case 's':						// s: �����ƽ��
		_camy += 0.05f;
		_tgty += 0.05f;
		break;

	case 'a':						// a: �����ƽ��
		_camx += 0.05f;
		_tgtx += 0.05f;
		break;

	case 'd':						// d: �����ƽ��
		_camx -= 0.05f;
		_tgtx -= 0.05f;
		break;

	case 'q':						// q: �����ƽ��
		_camz -= 0.05f;
		_tgtz -= 0.05f;
		break;

	case 'e':						// e: �����ƽ��
		_camz += 0.05f;
		_tgtz += 0.05f;
		break;

	case 'r':						// r: �������λ
		_camx = _camy = 0.0f;
		_camz = 3.0f;
		_tgtx = _tgty = _tgtz = 0.0f;
		break;

	case 'h':						// h: ������Ϣ
		printf(_help);
		break;

	default:
		break;
	}
}

// ���������ַ���Ϣ������
void arrow_keys(int a_keys, int x, int y)
{
	switch (a_keys) {
	case GLUT_KEY_UP:				// ARROW UP
		_rot_x += 1.0f;
		break;

	case GLUT_KEY_DOWN:				// ARROW DOWN
		_rot_x -= 1.0f;
		break;

	case GLUT_KEY_RIGHT:			// ARROW RIGHT
		_rot_y += 1.0f;
		break;

	case GLUT_KEY_LEFT:				// ARROW LEFT
		_rot_y -= 1.0f;
		break;

	case GLUT_KEY_F1:				// F1: ȫ����ʾ
		_fullscreen = !_fullscreen;
		if (_fullscreen)
			glutFullScreen();				// ȫ����ʾ
		else {
			glutReshapeWindow(800, 600);	// �������سߴ�
			glutPositionWindow(50, 50);		// ����λ��
		}
		break;

	case GLUT_KEY_F2:				// F2: �߿���Ⱦ/����Ⱦ
		_wireframe = !_wireframe;
		_wireframe ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;

	case GLUT_KEY_F3:				// F3: �⻬��Ⱦ/ƽ̹��Ⱦ
		_smooth_shading = !_smooth_shading;
		_smooth_shading ? glShadeModel(GL_SMOOTH) : glShadeModel(GL_FLAT);
		break;

	case GLUT_KEY_F4:				// F4: �޳�������
		_cull_back_faces = !_cull_back_faces;
		_cull_back_faces ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
		break;

	case GLUT_KEY_F5:				// F5: ��/�رչ���
		_lighting = !_lighting;
		_lighting ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
		break;

	default:
		break;
	}
}

// ��갴����Ϣ������
void mouse_buttons(int button, int state, int x, int y) {
	switch (button) {
	case  GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {		// LEFT BUTTON DOWN
			_mouse_state = GLUT_DOWN;
			_mouse_button = button;
			_mouse_x = x;
			_mouse_y = y;
		}
		else {						// LEFT BUTTON UP
			_mouse_state = GLUT_UP;
		}
		break;

	case  GLUT_MIDDLE_BUTTON:
		break;

	case  GLUT_RIGHT_BUTTON:
		break;

	default:
		break;
	}
}

// ����ƶ���Ϣ������
void mouse_moving(int x, int y) {
	if (_mouse_state == GLUT_DOWN) {
		int dx, dy;
		switch (_mouse_button) {
		case  GLUT_LEFT_BUTTON:
			// ��������ƶ���
			dx = x - _mouse_x;
			dy = y - _mouse_y;

			// �޸�������򳡾�
			_rot_x += dy;
			_rot_y += dx;

			// ������굱ǰλ�ã���������
			_mouse_x = x;
			_mouse_y = y;
			break;

		case  GLUT_MIDDLE_BUTTON:
			break;

		case  GLUT_RIGHT_BUTTON:
			break;

		default:
			break;
		}
	}
}

// ������
// argc��������Ŀ
// argv�������ַ���
int main(int argc, char* argv[])
{
	//if (argc < 2)
	//{
	//	printf("Usage: MiniEngine [filename].obj\n");
	//	printf("Example: MiniEngine models\\bunny.obj\n");
	//	return 1;
	//}

	// ��ʼ��GLUT
	glutInit(&argc, argv);					// ��ʼ��GLUT
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);	// ��ʾģʽ����Ȳ��ԡ�˫���塢RGBA��ɫģ��
	glutInitWindowPosition(50, 50);			// ����λ��
	glutInitWindowSize(800, 600);			// �������سߴ�
	glutCreateWindow("Mini 3D Engine");		// ���ڱ���
	init();									// ��ʼ����GLUTĬ��ֵ֮��ģ�OpenGL����
	//init_scene(argv[1]);					// ��ʼ�����������ļ�����ģ��
	if (_fullscreen)
		glutFullScreen();					// ȫ����ʾ

	// ���ø�����Ϣ������
	glutDisplayFunc(display);				// ���ڻ��ƴ�����
	glutReshapeFunc(reshape);				// �������Ŵ�����
	glutKeyboardFunc(keyboard);				// ������ͨ�ַ���Ϣ������
	glutSpecialFunc(arrow_keys);			// ���������ַ���Ϣ������
	glutMouseFunc(mouse_buttons);			// ��갴����Ϣ������
	glutMotionFunc(mouse_moving);			// ����ƶ���Ϣ������
	glutIdleFunc(display);					// ���д�����

	// ��ӡ������Ϣ
	printf(_help);

	// ������Ϣѭ����ֱ���رմ���
	glutMainLoop();

	return 0;
}