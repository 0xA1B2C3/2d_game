#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <windows.h>
#include <windowsx.h>
#include <GL/GL.h>
#include <GL/GLU.h>
#include <GL/glut.h>
#include <GL/GLAux.h>
#pragma comment(lib, "glut32")
#pragma comment(lib, "Opengl32")
#pragma comment(lib, "SOIL")
#include "C:\Hacking\opengl\VC-Desktop-Lesson06\SOIL.h"


#define ROT_LEFT 10
#define ROT_RIGHT 20
#define ROT_UP 30
#define ROT_DOWN 40

#define SHFT_LEFT 1
#define SHFT_UP   2
#define SHFT_DOWN 3
#define SHFT_RT   4
#define SHFT_BACK 5
#define SHFT_FORWARD 6

#define DIR_LEFT 1
#define DIR_UP   2
#define DIR_DOWN 3
#define DIR_RT   4
#define DIR_CHECKPT 255

#define OBJ_CUBE 1
#define OBJ_PYRAMID 2
#define OBJ_SQ 3

#define TEX_DIED 0
#define TEX_VOID 1
#define TEX_SPAWN 2
#define TEX_FLAG 3
#define TEX_TITLE 4
#define TEX_SKILLS1 5
#define TEX_SKILLS2 6
#define TEX_CHKPT 7
#define TEX_SKILLS3 8
#define TEX_SKILLS4 9

#define _CRT_SECURE_NO_WARNINGS 1

//Structures
typedef struct obj {
	void *obj;
	unsigned int type;
	bool phys;
} obj;           // [Obj] [Type] [Phys]
typedef struct color {
	double r;
	double g;
	double b;
} color;
typedef struct phys_2d {
	double y_acc;
	double y_vel;
	double x_acc;
	double x_vel;
} phys_2d;
typedef struct camera {
	//Position
	double posx;
	double posy;
	double posz;
	//Rotation
	double rotx;
	double roty;
	double rotz;
	//Vector
	double vecx;
	double vecy;
	double vecz;
} camera;
typedef struct vertex   {
	double x;
	double y;
	double z;
} vertex;     // [X] [Y] [Z]
typedef struct square  {
	color clr;
	GLuint tex;
	vertex btl;
	vertex tpl;
	vertex tpr;
	vertex btr;
	phys_2d phys;
	bool ground;
	bool WillKill;
	bool coll;
} square;     // [Color] [Tex] [BTL] [TPL] [TPR] [BTR] [Phys] [Ground] [WillKill] [Coll]
typedef struct cube  {
	square top;
	square bt;
	square front;
	square back;
	square left;
	square right;
	bool ground;
	double acceleration;
	double velocity;
} cube;
typedef struct triangle {
	vertex top;
	vertex left;
	vertex right;
	bool ground;
	double acceleration;
	double velocity;
} triangle;
typedef struct pyramid {
	triangle front;
	triangle right;
	triangle back;
	triangle left;
	bool ground;
	double acceleration;
	double velocity;
} pyramid;
typedef struct spawns {
	square defaultsp;
	square leftsp;
	square rightsp;
	square upsp;
	square downsp;
} spawns;
typedef struct level{
	obj *obj;
	spawns spwn;

	unsigned int bytes;
	unsigned int current;
	int i;

	struct level *left;
	struct level *right;
	struct level *up;
	struct level *down;

	bool IsCheckPoint;
	unsigned int skills_earned;
} level;       // [Objects] [Bytes Total] [Bytes Currently] [Left Level] [Right Level] [Top Level] [Bottom Level]
typedef struct skills {
	bool super_jump;
	bool climber;
	bool ultimate_climber;
	bool void_immunity;
	bool chkpt_teleport;
} skills;     // [Super Jump] [Climber] [Ultimate Climber] [Void Immunity] [Chkpt Teleport]
typedef struct tex {
	GLuint texture;
	unsigned int type;
} tex;
typedef struct game {
	GLuint global_tex;
	tex *t;

	camera cam; //SORT OF DONE
	square *CurrentPlayer; //DONE
	level **ls; //DONE
	skills skls; //DONE
	level *CurrentLevel;
	level *CheckPoint;
	level **CheckPts;
	bool *found; //DONE
	int num_chkpts; //DONE
	int num_levels; //DONE
	int num_textures; //DONE
} game;
typedef struct link {
	unsigned int left;
	unsigned int right;
	unsigned int down;
	unsigned int up;
} link;
game g1, g2;
game *CG = &g1;
color red, green, blue, white;

//Globals
char msg[1000];
int level_timer;
int global_x = 640, global_y = 480;
int mouse_x = 0, mouse_y = 0, i = 0;
int main_clock;
double global_decrease = 0.001;
double global_alpha = 1.0;
double clearance = 0.0;
double tele_clock = 0;
bool TitleScreen = true;
square *temp;
void SetupReadLevelShift(level *l, unsigned int direction);

//Debug
void Message(LPCWSTR msg) {
	MessageBox(NULL, msg, L"MSG", MB_OK);
}
void add_obj(void *structure, unsigned int type, level *l) {
	//Reallocate if neccessary
	if (l->current + sizeof(obj) >= l->bytes) {
		l->bytes += sizeof(obj) * 10;
		if (!(l->current && l->bytes)) {
			l->obj = (obj *)calloc(sizeof(obj) * 10, sizeof(obj));
		} else {
			l->obj = (obj *)realloc(l->obj, l->bytes);
		}
	}
	//Define and Increment
	l->obj[l->i].obj = structure;
	l->obj[l->i].type = type;
	l->i++; l->current += sizeof(obj);
}

HDC			hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application
bool	keys[256];			// Array Used For The Keyboard Routine
bool	active = TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen = TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default


//Declarations
bool GetCollisionBoundary(square *input, int direction, double amount, double *clearance);
bool GetCollisionSqToSq(square *input, int direction, double amount, double *clearance);
bool GetCollisionSq(square *input, int direction, double amount, double *clearance);
double flabs(double input);
double flabs2(double input);
void InitLevel(level *input);
void SetupL1();
void SetupL2();
void ChangeLevel(level *l, unsigned int direction);
void KillPlayer();
void ScaleSquare(square *input, double amount);
void ShiftSq(square *input, int direction, double amount);
void SetTexture(obj *object, unsigned int type, unsigned int texture);

//Debug
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc
//Debug
wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString;
	wString = (wchar_t *)calloc(4096, 1);
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}
void DispAxis(double num) {
	double gravity = -.00000027;
	char *str;
	str = (char *)calloc(500, 1);
	sprintf_s(str, 400, "(Clear: %f SqtoSq: %d)", num, GetCollisionSqToSq(CG->CurrentPlayer, SHFT_LEFT, flabs2(gravity), &clearance));
	Message(convertCharArrayToLPCWSTR(str));
	free(str);
}
void DispSq(square input) {
	char *str;
	str = (char *)calloc(500, 1);
	sprintf_s(str, 400, "TPL_X: %f TPL_Y: %f | TPR_X: %f TPR_Y: %f | BTL_X: %f BTL_Y: %f | BTR_X: %f BTR_Y: %f", input.tpl.x, input.tpl.y, input.tpr.x, input.tpr.y, \
		input.btl.x, input.btl.y, input.btr.x, input.btr.y);
	Message(convertCharArrayToLPCWSTR(str));
	free(str);
}

//Init
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height == 0)										// Prevent A Divide By Zero By
	{
		height = 1;										// Making Height Equal One
	}

	glViewport(0, 0, width, height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}
void DefineColors() {
	red.r = 1.0;
	red.g = 0.0;
	red.b = 0.0;
	green.r = 0.0;
	green.g = 1.0;
	green.b = 0.0;
	blue.r = 0.0;
	blue.g = 0.0;
	blue.b = 1.0;
	white.r = 1.0;
	white.g = 1.0;
	white.b = 1.0;
	return;
}
void InitTextures() {
	//Textures
	CG->t[0].texture = SOIL_load_OGL_texture("died.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	CG->t[1].texture = SOIL_load_OGL_texture("void.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	CG->t[2].texture = SOIL_load_OGL_texture("spawn.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	CG->t[3].texture = SOIL_load_OGL_texture("flag.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	CG->t[4].texture = SOIL_load_OGL_texture("title.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	CG->t[5].texture = SOIL_load_OGL_texture("skill1.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	CG->t[6].texture = SOIL_load_OGL_texture("skill2.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	CG->t[7].texture = SOIL_load_OGL_texture("chkpt.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	CG->t[8].texture = SOIL_load_OGL_texture("skill3.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	// Typical Texture Generation Using Data From The Bitmap
	glBindTexture(GL_TEXTURE_2D, CG->t[0].texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0, 0.0, 0.0, 0.5);				// Black Background
	glClearDepth(1.0);									// Depth Buffer Setup
	glDisable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);					// Set The Blending Function For Translucency
	
	glEnable(GL_BLEND);			// Turn Blending On
	glDisable(GL_DEPTH_TEST);	// Turn Depth Testing Off
	return TRUE;										// Initialization Went OK
}
void InitCamera(camera *cam) {
	//Position
	cam->posx = 0.0;
	cam->posy = 0.0;
	cam->posz = 0.0;
	//Rotation
	cam->rotx = 0.0;
	cam->roty = 0.0;
	cam->rotz = -1.0;
	//Up-vector
	cam->vecx = 0.0;
	cam->vecy = 1.0;
	cam->vecz = 0.0;
	return;
}
void InitSquare(square *input) {
	input->clr = white;
	input->tex = 0;
	input->ground = false;
	input->WillKill = false;
	input->coll = true;
	
	input->phys.y_acc = 0.0;
	input->phys.y_vel = 0.0;
	input->phys.x_acc = 0.0;
	input->phys.x_vel = 0.0;

	input->tpl.x = 0.5;
	input->tpl.y = 1.0;
	input->tpl.z = -6.0;

	input->tpr.x = 2.5;
	input->tpr.y = 1.0;
	input->tpr.z = -6.0;

	input->btr.x = 2.5;
	input->btr.y = -1.0;
	input->btr.z = -6.0;

	input->btl.x = 0.5;
	input->btl.y = -1.0;
	input->btl.z = -6.0;
	return;
}
void InitCube(cube *input) {
	//Top
	input->top.tpr.x = 1.0;
	input->top.tpr.y = 1.0;
	input->top.tpr.z = -1.0;

	input->top.tpl.x = -1.0;
	input->top.tpl.y = 1.0;
	input->top.tpl.z = -1.0;

	input->top.btl.x = -1.0;
	input->top.btl.y = 1.0;
	input->top.btl.z = 1.0;

	input->top.btr.x = 1.0;
	input->top.btr.y = 1.0;
	input->top.btr.z = 1.0;
	//Bottom
	input->bt.tpr.x = 1.0;
	input->bt.tpr.y = -1.0;
	input->bt.tpr.z = 1.0;

	input->bt.tpl.x = -1.0;
	input->bt.tpl.y = -1.0;
	input->bt.tpl.z = 1.0;

	input->bt.btl.x = -1.0;
	input->bt.btl.y = -1.0;
	input->bt.btl.z = -1.0;

	input->bt.btr.x = 1.0;
	input->bt.btr.y = -1.0;
	input->bt.btr.z = -1.0;
	//Front
	input->front.tpr.x = 1.0;
	input->front.tpr.y = 1.0;
	input->front.tpr.z = 1.0;

	input->front.tpl.x = -1.0;
	input->front.tpl.y = 1.0;
	input->front.tpl.z = 1.0;

	input->front.btl.x = -1.0;
	input->front.btl.y = -1.0;
	input->front.btl.z = 1.0;

	input->front.btr.x = 1.0;
	input->front.btr.y = -1.0;
	input->front.btr.z = 1.0;
	//Back
	input->back.tpr.x = 1.0;
	input->back.tpr.y = -1.0;
	input->back.tpr.z = -1.0;

	input->back.tpl.x = -1.0;
	input->back.tpl.y = -1.0;
	input->back.tpl.z = -1.0;

	input->back.btl.x = -1.0;
	input->back.btl.y = 1.0;
	input->back.btl.z = -1.0;

	input->back.btr.x = 1.0;
	input->back.btr.y = 1.0;
	input->back.btr.z = -1.0;
	//Left
	input->left.tpr.x = -1.0;
	input->left.tpr.y = 1.0;
	input->left.tpr.z = 1.0;

	input->left.tpl.x = -1.0;
	input->left.tpl.y = 1.0;
	input->left.tpl.z = -1.0;

	input->left.btl.x = -1.0;
	input->left.btl.y = -1.0;
	input->left.btl.z = -1.0;

	input->left.btr.x = -1.0;
	input->left.btr.y = -1.0;
	input->left.btr.z = 1.0;
	//Right
	input->right.tpr.x = 1.0;
	input->right.tpr.y = 1.0;
	input->right.tpr.z = -1.0;

	input->right.tpl.x = 1.0;
	input->right.tpl.y = 1.0;
	input->right.tpl.z = 1.0;

	input->right.btl.x = 1.0;
	input->right.btl.y = -1.0;
	input->right.btl.z = 1.0;

	input->right.btr.x = 1.0;
	input->right.btr.y = -1.0;
	input->right.btr.z = -1.0;
}
void InitPyramid(pyramid *input) {
	//Top - Front
	input->front.top.x = 0.0;
	input->front.top.y = 1.0;
	input->front.top.z = 0.0;
	//Left
	input->front.left.x = -1.0;
	input->front.left.y = -1.0;
	input->front.left.z = 1.0;
	//Right
	input->front.right.x = 1.0;
	input->front.right.y = -1.0;
	input->front.right.z = 1.0;
	//Top - Right
	input->right.top.x = 0.0;
	input->right.top.y = 1.0;
	input->right.top.z = 0.0;
	//Left
	input->right.left.x = 1.0;
	input->right.left.y = -1.0;
	input->right.left.z = 1.0;
	//Right
	input->right.right.x = 1.0;
	input->right.right.y = -1.0;
	input->right.right.z = -1.0;
	//Top - Back
	input->back.top.x = 0.0;
	input->back.top.y = 1.0;
	input->back.top.z = 0.0;
	//Left
	input->back.left.x = 1.0;
	input->back.left.y = -1.0;
	input->back.left.z = -1.0;
	//Right
	input->back.right.x = -1.0;
	input->back.right.y = -1.0;
	input->back.right.z = -1.0;
	//Top - Left
	input->left.top.x = 0.0;
	input->left.top.y = 1.0;
	input->left.top.z = 0.0;
	//Left
	input->left.left.x = -1.0;
	input->left.left.y = -1.0;
	input->left.left.z = -1.0;
	//Right
	input->left.right.x = -1.0;
	input->left.right.y = -1.0;
	input->left.right.z = 1.0;
}
void InitGame(game *g) {

}

//Polygon Templates
void TemplatePlayer(square *input) {
	CG->CurrentPlayer->clr = red;
	ScaleSquare(CG->CurrentPlayer, 4.0);
}
void TemplateFlag(square *input) {
	input->coll = false;
	input->tex = CG->t[TEX_FLAG].texture;
	ScaleSquare(input, 1.5);
}
void TemplateSquare(square *input, level *l) {
	InitSquare(input);
	add_obj((void *)input, OBJ_SQ, l);
}
void TemplateVoidFloor(level *l, square *storage) {
	int i;
	for (i = 0; i < 10; i++) {
		InitSquare(&storage[i]);
		add_obj((void *)&storage[i], OBJ_SQ, l);
		SetTexture(&l->obj[l->i-1], OBJ_SQ, TEX_VOID);
		storage[i].WillKill = true;
		ScaleSquare(&storage[i], 4);
		ShiftSq(&storage[i], SHFT_DOWN, 2.2);
		ShiftSq(&storage[i], SHFT_RT, .8);
		ShiftSq(&storage[i], SHFT_LEFT, i);
	}
}

//Rotation
void GetMousePos() {
	if ((mouse_x != global_x / 2) && (mouse_y != global_y / 2) && (mouse_x != 0) && (mouse_y != 0)) {
		if (mouse_x < global_x / 2) {
			if (mouse_y < global_x / 2) {

			}
			else {

			}
		}
		else if (mouse_x > global_x / 2) {
			if (mouse_y < global_x / 2) {

			}
			else {

			}
		}
	}
	return;
}
void ResetMouse() {
	if ((mouse_x != global_x / 2) && (mouse_y != global_y / 2)) {
		SetCursorPos(global_x / 2, global_y / 2);
		mouse_x = global_x / 2;
		mouse_y = global_y / 2;
	}
	return;
}
void MoveCamera(int direction, double amount) {
	switch (direction) {
	case SHFT_RT:
		CG->cam.rotx += amount;
		break;
	case SHFT_LEFT:
		CG->cam.rotx -= amount;
		break;
	case SHFT_UP:
		CG->cam.roty += amount;
		break;
	case SHFT_DOWN:
		CG->cam.roty -= amount;
		break;
	}
	return;
}

//Drawing
void DrawSquare(square input) {
	if (input.tex != 0) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, input.tex);
		glBegin(GL_QUADS);
		glColor3d(input.clr.r, input.clr.g, input.clr.b);
		glTexCoord2d(0.0, 0.0); glVertex3d(input.tpl.x, input.tpl.y, input.tpl.z);
		glTexCoord2d(0.0, 1.0); glVertex3d(input.tpr.x, input.tpr.y, input.tpr.z);
		glTexCoord2d(1.0, 1.0); glVertex3d(input.btr.x, input.btr.y, input.btr.z);
		glTexCoord2d(1.0, 0.0); glVertex3d(input.btl.x, input.btl.y, input.btl.z);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
	else {
		glBegin(GL_QUADS);
		glColor3d(input.clr.r, input.clr.g, input.clr.b);
		glVertex3d(input.tpl.x, input.tpl.y, input.tpl.z);
		glVertex3d(input.tpr.x, input.tpr.y, input.tpr.z);
		glVertex3d(input.btr.x, input.btr.y, input.btr.z);
		glVertex3d(input.btl.x, input.btl.y, input.btl.z);
		glEnd();
	}
	return;
}
void DrawCube(cube input) {
	glBegin(GL_QUADS);
	glColor3d(0.0, 1.0, 0.0);
	glVertex3d(input.top.tpr.x, input.top.tpr.y, input.top.tpr.z);
	glVertex3d(input.top.tpl.x, input.top.tpl.y, input.top.tpl.z);
	glVertex3d(input.top.btl.x, input.top.btl.y, input.top.btl.z);
	glVertex3d(input.top.btr.x, input.top.btr.y, input.top.btr.z);
	// Bottom
	glColor3d(1.0, 0.5, 0.0);
	glVertex3d(input.bt.tpr.x, input.bt.tpr.y, input.bt.tpr.z);
	glVertex3d(input.bt.tpl.x, input.bt.tpl.y, input.bt.tpl.z);
	glVertex3d(input.bt.btl.x, input.bt.btl.y, input.bt.btl.z);
	glVertex3d(input.bt.btr.x, input.bt.btr.y, input.bt.btr.z);
	// Front
	glColor3d(1.0, 0.0, 0.0);
	glVertex3d(input.front.tpr.x, input.front.tpr.y, input.front.tpr.z);
	glVertex3d(input.front.tpl.x, input.front.tpl.y, input.front.tpl.z);
	glVertex3d(input.front.btl.x, input.front.btl.y, input.front.btl.z);
	glVertex3d(input.front.btr.x, input.front.btr.y, input.front.btr.z);
	// back
	glColor3d(1.0, 1.0, 0.0);
	glVertex3d(input.back.tpr.x, input.back.tpr.y, input.back.tpr.z);
	glVertex3d(input.back.tpl.x, input.back.tpl.y, input.back.tpl.z);
	glVertex3d(input.back.btl.x, input.back.btl.y, input.back.btl.z);
	glVertex3d(input.back.btr.x, input.back.btr.y, input.back.btr.z);
	// left
	glColor3d(0.0, 0.0, 1.0);
	glVertex3d(input.left.tpr.x, input.left.tpr.y, input.left.tpr.z);
	glVertex3d(input.left.tpl.x, input.left.tpl.y, input.left.tpl.z);
	glVertex3d(input.left.btl.x, input.left.btl.y, input.left.btl.z);
	glVertex3d(input.left.btr.x, input.left.btr.y, input.left.btr.z);
	// right
	glColor3d(1.0, 0.0, 1.0);
	glVertex3d(input.right.tpr.x, input.right.tpr.y, input.right.tpr.z);
	glVertex3d(input.right.tpl.x, input.right.tpl.y, input.right.tpl.z);
	glVertex3d(input.right.btl.x, input.right.btl.y, input.right.btl.z);
	glVertex3d(input.right.btr.x, input.right.btr.y, input.right.btr.z);
	glEnd();
	return;
}
void DrawPyramid(pyramid input) {
	glBegin(GL_TRIANGLES);								// Start Drawing A Triangle
	glVertex3d(input.front.top.x, input.front.top.y, input.front.top.z);					// Top Of Triangle (Front)
	glVertex3d(input.front.left.x, input.front.left.y, input.front.left.z);					// Left Of Triangle (Front)
	glVertex3d(input.front.right.x, input.front.right.y, input.front.right.z);					// Right Of Triangle (Front)
	glVertex3d(input.right.top.x, input.right.top.y, input.right.top.z);					// Top Of Triangle (Right)
	glVertex3d(input.right.left.x, input.right.left.y, input.right.left.z);					// Left Of Triangle (Right)
	glVertex3d(input.right.right.x, input.right.right.y, input.right.right.z);					// Right Of Triangle (Right)
	glVertex3d(input.back.top.x, input.back.top.y, input.back.top.z);					// Top Of Triangle (Back)
	glVertex3d(input.back.left.x, input.back.left.y, input.back.left.z);					// Left Of Triangle (Back)
	glVertex3d(input.back.right.x, input.back.right.y, input.back.right.z);				// Right Of Triangle (Back)
	glVertex3d(input.left.top.x, input.left.top.y, input.left.top.z);					// Top Of Triangle (Left)
	glVertex3d(input.left.left.x, input.left.left.y, input.left.left.z);				// Left Of Triangle (Left)
	glVertex3d(input.left.right.x, input.left.right.y, input.left.right.z);					// Right Of Triangle (Left)
	glEnd();											// Done Drawing The Pyramid
}
void DrawLevel(level l) {
	int i = 0; square *temp = NULL;
	for (i = 0; i < l.i; i++) {
		if (l.obj[i].type == OBJ_SQ) {
			temp = (square *)l.obj[i].obj;
			DrawSquare(*temp);
		}
	}
	return;
}

//Scale
void ScaleSquare(square *input, double amount) {
	if (amount > 0) {
		input->tpl.x /= amount;
		input->tpl.y /= amount;
		input->tpr.x /= amount;
		input->tpr.y /= amount;
		input->btr.x /= amount;
		input->btr.y /= amount;
		input->btl.x /= amount;
		input->btl.y /= amount;
	} 
	else if (amount < 0) {
		input->tpl.x *= amount;
		input->tpl.y *= amount;
		input->tpr.x *= amount;
		input->tpr.y *= amount;
		input->btr.x *= amount;
		input->btr.y *= amount;
		input->btl.x *= amount;
		input->btl.y *= amount;
	}

	return;
}

//Collisions
bool GetCollisionSqToSq(square *input, int direction, double amount, double *clearance) { 
	square *temp;
	if (direction == SHFT_LEFT) {
		for (i = 1; i < CG->CurrentLevel->i; i++) {
			temp = (square *)CG->CurrentLevel->obj[i].obj;
			if (temp->coll) {
				if ((input->btl.x - amount <= temp->btr.x) && (input->btl.x - amount >= temp->btl.x)) { //X 
					if (input->btl.y >= temp->tpl.y) {
						//Message(L"On top");
						return true;
					}
					if ((input->btl.y >= temp->btr.y) && (input->btl.y <= temp->tpr.y)) { //Y Possibility 1
						//Message(L"Left Pos 1");
						*clearance = input->btl.x - temp->btr.x;
						//DispAxis(*clearance);
						if ((temp->WillKill == true) && (!CG->skls.void_immunity))
							KillPlayer();
						if (CG->skls.climber)
							CG->CurrentPlayer->ground = true;
						return false;
					}
					if ((input->tpl.y <= temp->tpr.y) && (input->tpl.y >= temp->btr.y)) { //Y Possibility 2
						//Message(L"Left Pos 2");
						*clearance = input->btl.x - temp->btr.x;
						//DispAxis(*clearance);
						if ((temp->WillKill == true) && (!CG->skls.void_immunity))
							KillPlayer();
						if (CG->skls.climber)
							CG->CurrentPlayer->ground = true;
						return false;
					}
				}
			}
		}
	}
	else if (direction == SHFT_RT) {
		for (i = 1; i < CG->CurrentLevel->i; i++) {
			temp = (square *)CG->CurrentLevel->obj[i].obj;
			if (temp->coll) {
				if ((input->btr.x + amount >= temp->btl.x) && (input->btr.x + amount <= temp->btr.x)) { //X Restrictions
					if (input->btl.y >= temp->tpl.y) {
						//Message(L"On top");
						return true;
					}
					if ((input->btr.y >= temp->btl.y) && (input->btr.y <= temp->tpl.y)) { //Y Possibility 1
						//Message(L"Right Pos 1");
						*clearance = temp->btl.x - input->btr.x;
						//DispAxis(*clearance);
						if ((temp->WillKill == true) && (!CG->skls.void_immunity))
							KillPlayer();
						if (CG->skls.climber)
							CG->CurrentPlayer->ground = true;
						return false;
					}
					if ((input->tpr.y <= temp->tpl.y) && (input->tpr.y >= temp->btl.y)) { //Y Possibility 2
						//Message(L"Right Pos 2");
						*clearance = temp->btl.x - input->btr.x;
						//DispAxis(*clearance);
						if ((temp->WillKill == true) && (!CG->skls.void_immunity))
							KillPlayer();
						if (CG->skls.climber)
							CG->CurrentPlayer->ground = true;
						return false;
					}
				}
			}
		}
	}
	else if (direction == SHFT_DOWN) {
		for (i = 1; i < CG->CurrentLevel->i; i++) {
			temp = (square *)CG->CurrentLevel->obj[i].obj;
			if (temp->coll) {
				if (((input->tpl.x > temp->tpl.x) && (input->tpl.x < temp->tpr.x)) || ((input->tpr.x < temp->tpr.x) && (input->tpr.x > temp->tpl.x))) { //Check X Axis
					if ((input->btl.y - amount <= temp->tpl.y) && (input->tpl.y - amount >= temp->btl.y)) { //Check Y Axis
						input->ground = true; //?
						if (input == CG->CurrentPlayer) { //Physics Errors
							if ((input->btl.y < temp->tpl.y) && (input->btl.y > temp->btl.y) && (input->tpl.y > temp->tpl.y)) { //Y Restrictions
								if ((input->btr.x > temp->btl.x) && (input->btr.x < temp->btr.x)) {
									input->tpl.y += (temp->tpl.y - input->btl.y);
									input->tpr.y += (temp->tpr.y - input->btr.y);

									input->btl.y = temp->tpl.y;
									input->btr.y = temp->tpr.y;
								}
								if ((input->btl.x < temp->btr.x) && (input->btl.x > temp->btl.x)) {
									input->tpl.y += (temp->tpl.y - input->btl.y);
									input->tpr.y += (temp->tpr.y - input->btr.y);

									input->btl.y = temp->tpl.y;
									input->btr.y = temp->tpr.y;
								}
							}

						}
						*clearance = input->btl.y - temp->tpl.y;
						if ((temp->WillKill == true) && (!CG->skls.void_immunity))
							KillPlayer();
						return false;
					}
				}
			}
		}
	}
	else if (direction == SHFT_UP) {
		for (i = 1; i < CG->CurrentLevel->i; i++) {
			temp = (square *)CG->CurrentLevel->obj[i].obj;
			if (temp->coll) {
				if (((input->tpl.x > temp->tpl.x) && (input->tpl.x < temp->tpr.x)) || ((input->tpr.x < temp->tpr.x) && (input->tpr.x > temp->tpl.x))) { //X Axis
					if ((input->tpl.y + amount >= temp->btl.y) && (input->tpl.y + amount <= temp->tpl.y)) { //Y Axis
						*clearance = temp->btl.y - input->tpl.y;
						CG->CurrentPlayer->phys.y_acc = 0.0;
						CG->CurrentPlayer->phys.y_vel = 0.0;
						if ((temp->WillKill == true) && (!CG->skls.void_immunity))
							KillPlayer();
						return false;
					}
				}
			}
		}
	}
	return true;
}
bool GetCollisionBoundary(square *input, int direction, double amount, double *clearance) {
	//Boundary Restrictions
	if (direction == SHFT_LEFT) {
		if (input->tpl.x + amount < -3.30) {
			if (input == CG->CurrentPlayer)
				if ((CG->CurrentLevel->left != NULL) && (clock() / CLOCKS_PER_SEC > level_timer + .5))
					SetupReadLevelShift(CG->CurrentLevel, SHFT_LEFT);
			return false;
		}
	}
	else if (direction == SHFT_RT) {
		if (input->tpr.x + amount > 3.30) {
			if (input == CG->CurrentPlayer)
				if ((CG->CurrentLevel->right != NULL) && (clock() / CLOCKS_PER_SEC > level_timer + .5))
					SetupReadLevelShift(CG->CurrentLevel, SHFT_RT);
			return false;
		}
	}
	else if (direction == SHFT_UP) {
		if (input->tpl.y + amount > 2.47) {
			if (input == CG->CurrentPlayer)
				if ((CG->CurrentLevel->up != NULL) && (clock() / CLOCKS_PER_SEC > level_timer + .5))
					SetupReadLevelShift(CG->CurrentLevel, SHFT_UP);
				CG->CurrentPlayer->phys.x_acc = 0.0;
				CG->CurrentPlayer->phys.x_vel = 0.0;
				CG->CurrentPlayer->phys.y_vel = 0.0;
				CG->CurrentPlayer->phys.y_acc = 0.0;
			return false;
		}
	}
	else if (direction == SHFT_DOWN) {
		if (input->btr.y - amount < -2.47) {
			if (input == CG->CurrentPlayer)
				if ((CG->CurrentLevel->down != NULL) && (clock() / CLOCKS_PER_SEC > level_timer + .5))
					SetupReadLevelShift(CG->CurrentLevel, SHFT_DOWN);
			*clearance = flabs2(input->btr.y) - 2.47;
			input->ground = true;
			return false;
		}
	}
	return true;
}
bool GetCollisionSq(square *input, int direction, double amount, double *clearance) {
	if (GetCollisionBoundary(input, direction, amount, clearance)) {
		if (GetCollisionSqToSq(input, direction, amount, clearance)) {
			return true;
		}
	}
	return false;
}

//Shifting - low level, no collisions
void ShiftSq(square *input, int direction, double amount) {
	if (amount > 1.0)
		printf("Bad");
	switch (direction) {

	case SHFT_LEFT:
		input->btl.x -= amount;
		input->tpl.x -= amount;
		input->tpr.x -= amount;
		input->btr.x -= amount;
		break;
	case SHFT_RT:
		input->btl.x += amount;
		input->tpl.x += amount;
		input->tpr.x += amount;
		input->btr.x += amount;
		break;
	case SHFT_DOWN:
		input->btl.y -= amount;
		input->tpl.y -= amount;
		input->tpr.y -= amount;
		input->btr.y -= amount;
		break;
	case SHFT_UP:
		input->btl.y += amount;
		input->tpl.y += amount;
		input->tpr.y += amount;
		input->btr.y += amount;
		break;
	case SHFT_BACK:
		input->btl.z -= amount;
		input->tpl.z -= amount;
		input->tpr.z -= amount;
		input->btr.z -= amount;
		break;
	case SHFT_FORWARD:
		input->btl.z += amount;
		input->tpl.z += amount;
		input->tpr.z += amount;
		input->btr.z += amount;
		break;
	default:
		break;
	}
	return;
}
void ShiftTri(triangle *input, int direction, double amount) {
	switch (direction) {
	case SHFT_LEFT:
		input->top.x -= amount;
		input->left.x -= amount;
		input->right.x -= amount;
		break;
	case SHFT_RT:
		input->top.x += amount;
		input->left.x += amount;
		input->right.x += amount;
		break;
	case SHFT_UP:
		input->top.y += amount;
		input->left.y += amount;
		input->right.y += amount;
		break;
	case SHFT_DOWN:
		input->top.y -= amount;
		input->left.y -= amount;
		input->right.y -= amount;
		break;
	case SHFT_FORWARD:
		input->top.z += amount;
		input->left.z += amount;
		input->right.z += amount;
		break;
	case SHFT_BACK:
		input->top.z -= amount;
		input->left.z -= amount;
		input->right.z -= amount;
		break;
	}
	return;
}
void ShiftCube(cube *input, int direction, double amount) {
	ShiftSq(&input->top, direction, amount);
	ShiftSq(&input->back, direction, amount);
	ShiftSq(&input->bt, direction, amount);
	ShiftSq(&input->front, direction, amount);
	ShiftSq(&input->left, direction, amount);
	ShiftSq(&input->right, direction, amount);
	return;
}
void ShiftPyramid(pyramid *input, int direction, double amount) {
	ShiftTri(&input->front, direction, amount);
	ShiftTri(&input->back, direction, amount);
	ShiftTri(&input->left, direction, amount);
	ShiftTri(&input->right, direction, amount);
	return;
}
void ShiftWorld(int direction, double amount, level list) {
	for (i = 0; i < list.i; i++) {
		if (list.obj[i].type == OBJ_CUBE) {
			ShiftCube((cube *)list.obj[i].obj, direction, amount);
		}
		else if (list.obj[i].type == OBJ_PYRAMID) {
			ShiftPyramid((pyramid *)list.obj[i].obj, direction, amount);
		}
	}
	return;
}

//Moving - high level, collision blocking
void MoveSq(square *input, int direction, double amount) {
	double clearance = 0.0;
	if (GetCollisionSq(input, direction, amount, &clearance)) {
		if (direction == SHFT_LEFT)
			input->phys.x_vel -= amount;
		if (direction == SHFT_RT)
			input->phys.x_vel += amount;
		if (direction == SHFT_UP)
			input->phys.y_vel += amount;
		if (direction == SHFT_DOWN)
			input->phys.y_vel -= amount;
	}
	else {
		if (GetCollisionSq(CG->CurrentPlayer, direction, clearance, &clearance)) {
			if (direction == SHFT_LEFT)
				input->phys.x_vel -= amount;
			if (direction == SHFT_RT)
				input->phys.x_vel += amount;
			if (direction == SHFT_UP)
				input->phys.y_vel += amount;
			if (direction == SHFT_DOWN)
				input->phys.y_vel -= amount;
		}
	}
}

//Physics
double phys_time = 0.0f, phys_pasttime = 0.0;
double flabs2(double input) {
	if (input < 0.0f)
		return (input /= -1);
	else
		return input;
}
double flabs(double input) {
	return (input /= -1);
}
void UpdatePhysics(level list) {
	double gravity = -.00000027;
	int i = 0;
	for (i = 0; i < list.i; i++) {
		if (list.obj[i].phys) {
			if (list.obj[i].type == OBJ_SQ) {
				square *temp = (square *)list.obj[i].obj;
				
				//If on the ground
				if (temp->ground) {
					temp->phys.y_vel = 0.0;
					temp->phys.y_acc = 0.0;

					if (GetCollisionSq(temp, SHFT_DOWN, flabs2(gravity), &clearance)) {
						temp->ground = false;
					}
					else
						printf("Hi");
				}
				else {
					temp->phys.y_acc += gravity;
					temp->phys.y_vel += temp->phys.y_acc;
				}

				//Vertical Physics
				if (temp->phys.y_vel > 0.0) {
					if (GetCollisionSq(temp, SHFT_UP, temp->phys.y_vel, &clearance)) {
						ShiftSq(temp, SHFT_UP, temp->phys.y_vel);
					}
					else {
						ShiftSq(temp, SHFT_UP, clearance);
					}
				}
				else {
					if (GetCollisionSq(temp, SHFT_DOWN, flabs2(temp->phys.y_vel), &clearance)) {
						ShiftSq(temp, SHFT_DOWN, flabs2(temp->phys.y_vel));
					}
					else if (GetCollisionSq(temp, SHFT_DOWN, flabs2(clearance), &clearance)) {
						ShiftSq(temp, SHFT_DOWN, flabs2(clearance));
					}
				}
				//X-Axis Physics
				if (temp->phys.x_vel > 0.0) {
					if (GetCollisionSq(temp, SHFT_RT, temp->phys.x_vel, &clearance)) {
						if (GetCollisionSqToSq(temp, SHFT_RT, temp->phys.x_vel, &clearance)) {
							ShiftSq(temp, SHFT_RT, temp->phys.x_vel);
						}
					}
				} 
				else if (temp->phys.x_vel < 0) {
					if (GetCollisionSq(temp, SHFT_LEFT, flabs2(temp->phys.x_vel), &clearance)) { //Error - Negative
						if (GetCollisionSqToSq(temp, SHFT_LEFT, flabs2(temp->phys.x_vel), &clearance)) {
							ShiftSq(temp, SHFT_LEFT, flabs2(temp->phys.x_vel));
						}
					}
				}

				//Friction
				if (temp->phys.x_vel != 0) {
					temp->phys.x_vel = 0.0;
				}
			}
		}
	}
	return;
}

//Textures
void SetTexture(obj *object, unsigned int type, unsigned int texture) {
	if (type == OBJ_SQ) {
		square *temp;
		temp = (square *)object->obj;
		temp->tex = CG->t[texture].texture;
	}
	return;
}
void ShowFullTexFade(unsigned int texture, double *alpha, double decrease) {
	if (*alpha > 0.0) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBegin(GL_QUADS);
		glColor4d(1.0, 1.0, 1.0, *alpha);
		glTexCoord2d(0.0, 0.0); glVertex3d(-3.3, 2.47, -6);
		glTexCoord2d(0.0, 1.0); glVertex3d(3.3, 2.47, -6);
		glTexCoord2d(1.0, 1.0); glVertex3d(3.3, -2.47, -6);
		glTexCoord2d(1.0, 0.0); glVertex3d(-3.3, -2.47, -6);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glFlush();
		*alpha -= decrease;
	}
	return;
}
void ShowFullTex(unsigned int texture, double alpha) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glColor4d(1.0, 1.0, 1.0, alpha);
	glTexCoord2d(0.0, 0.0); glVertex3d(-3.3, 2.47, -6);
	glTexCoord2d(0.0, 1.0); glVertex3d(3.3, 2.47, -6);
	glTexCoord2d(1.0, 1.0); glVertex3d(3.3, -2.47, -6);
	glTexCoord2d(1.0, 0.0); glVertex3d(-3.3, -2.47, -6);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glFlush();
	return;
}

//CG->skls
void EnableSuperJump() {
	int i;
	CG->skls.super_jump = true;
	for (i = 0; i < CG->num_textures; i++)
		if (CG->t[i].type == TEX_SKILLS1)
			CG->global_tex = CG->t[i].texture;
	global_alpha = 2.0;
	global_decrease = 0.001;
}
void EnableUltimateWallClimber() {
	int i;
	CG->skls.ultimate_climber = true;
	for (i = 0; i < CG->num_textures; i++)
		if (CG->t[i].type == TEX_SKILLS2)
			CG->global_tex = CG->t[i].texture;
	global_alpha = 2.0;
	global_decrease = 0.001;
}
void EnableVoidImmunity() {
	int i;
	CG->skls.void_immunity = true;
	for (i = 0; i < CG->num_textures; i++)
		if (CG->t[i].type == TEX_SKILLS3)
			CG->global_tex = CG->t[i].texture;
	global_alpha = 2.0;
	global_decrease = 0.001;
}
void EnableTeleport() {
	int i;
	CG->skls.chkpt_teleport = true;
	for (i = 0; i < CG->num_textures; i++)
		if (CG->t[i].type == TEX_SKILLS4)
			CG->global_tex = CG->t[i].texture;
	global_alpha = 2.0;
	global_decrease = 0.001;
}

//Levels
unsigned int RevDir(unsigned int direction) {
	if (direction == DIR_LEFT)
		return DIR_RT;
	if (direction == DIR_RT)
		return DIR_LEFT;
	if (direction == DIR_UP)
		return DIR_DOWN;
	if (direction == DIR_DOWN)
		return DIR_UP;
	return 0;
}
void InitLevel(level *input) {
	input->bytes = 4000;
	input->current = 0;
	input->i = 0;
	input->obj = (obj *)calloc(input->bytes, sizeof(obj));

	input->left = NULL;
	input->right = NULL;
	input->up = NULL;
	input->down = NULL;

	input->IsCheckPoint = false;
	return;
}
void SetupTitle() {
	InitLevel(CG->ls[0]);
}
void SetupL1(unsigned int direction) {
	static square *sqsl1;
	static square sql1;
	CG->found[0] = true;
	//Skills Enabled
	if (!CG->skls.super_jump)
		EnableSuperJump();
	//Objects - player, 3 cubes
	InitLevel(CG->ls[1]);
	CG->CurrentPlayer = &sql1;
	InitSquare(CG->CurrentPlayer);
	//CheckPoint
	CG->CheckPoint = CG->ls[1];
	CG->ls[1]->IsCheckPoint = true;
	//Pointers
	CG->ls[1]->left = CG->ls[2];
	CG->ls[1]->right = CG->ls[3];
	CG->ls[1]->up = NULL;
	CG->ls[1]->down = NULL;
	//Player
	TemplatePlayer(CG->CurrentPlayer);
	//add_obj((void *)CG->CurrentPlayer, OBJ_SQ, CG->ls[1]);
	CG->ls[1]->obj[0].phys = true;
	if ((direction == DIR_LEFT) || (direction == DIR_CHECKPT)) 
		ShiftSq(CG->CurrentPlayer, SHFT_LEFT, 3.0);
	if (direction == DIR_RT) {
		ShiftSq(CG->CurrentPlayer, SHFT_RT, 2.7);
		ShiftSq(CG->CurrentPlayer, SHFT_DOWN, 1.3);
	}
	// - Static Squares -
	sqsl1 = (square *)calloc(4, sizeof(square));
	//Global Registration
	for (i = 0; i < 4; i++) {
		InitSquare(&sqsl1[i]);
		//add_obj((void *)&sqsl1[i], OBJ_SQ, CG->ls[1]);
		CG->ls[1]->obj[i + 1].phys = false;
	}
	//1
	//temp = (square *)CG->ls[1]->obj[1].obj; temp->WillKill = true;
	//SetTexture(&CG->ls[1]->obj[1], OBJ_SQ, TEX_VOID);
	ScaleSquare(&sqsl1[0], 4.0);
	ShiftSq(&sqsl1[0], SHFT_DOWN, 2.2);
	ShiftSq(&sqsl1[0], SHFT_RT, 1);
	//2
	ScaleSquare(&sqsl1[1], 3.0);
	ShiftSq(&sqsl1[1], SHFT_DOWN, 2.1);
	ShiftSq(&sqsl1[1], SHFT_LEFT, 1);
	//3
	ShiftSq(&sqsl1[2], SHFT_DOWN, 1.45);
	ShiftSq(&sqsl1[2], SHFT_LEFT, 4);
	//4
	TemplateFlag(&sqsl1[3]);
	ShiftSq(&sqsl1[3], SHFT_UP, .225);
	ShiftSq(&sqsl1[3], SHFT_LEFT, 3.3);
	//- Timer -
	level_timer = clock() / CLOCKS_PER_SEC;
}
void SetupL2(unsigned int direction) {
	static square *sqsl2;
	static square *floorl2;
	CG->found[2] = true;
	//Skills Found
	if (!CG->skls.void_immunity)
		EnableVoidImmunity();
	//Objects - player, 3 cubes
	InitLevel(CG->ls[2]);
	InitSquare(CG->CurrentPlayer);
	//Pointers
	CG->ls[2]->left = NULL;
	CG->ls[2]->right = CG->ls[1];
	CG->ls[2]->up = NULL;
	CG->ls[2]->down = NULL;
	//Player
	add_obj((void *)CG->CurrentPlayer, OBJ_SQ, CG->ls[2]);
	ScaleSquare(CG->CurrentPlayer, 4.0);
	if (direction == DIR_RT) {
		ShiftSq(CG->CurrentPlayer, SHFT_RT, 2.5);
		ShiftSq(CG->CurrentPlayer, SHFT_UP, .1);
	}
	CG->CurrentPlayer->clr = red;
	CG->ls[2]->obj[0].phys = true;
	//Static Squares
	sqsl2 = (square *)calloc(3, sizeof(square));
	floorl2 = (square *)calloc(10, sizeof(square));
	//1
	TemplateSquare(&sqsl2[0], CG->ls[2]);
	ShiftSq(&sqsl2[0], SHFT_RT, .8);
	ShiftSq(&sqsl2[0], SHFT_DOWN, 1.45);
	CG->ls[2]->obj[1].phys = false;
	//2
	TemplateSquare(&sqsl2[1], CG->ls[2]);
	ShiftSq(&sqsl2[1], SHFT_LEFT, 2);
	ScaleSquare(&sqsl2[1], 4.0);
	CG->ls[2]->obj[2].phys = false;
	//3
	TemplateSquare(&sqsl2[2], CG->ls[2]);
	ShiftSq(&sqsl2[2], SHFT_LEFT, 13);
	ScaleSquare(&sqsl2[2], 4.0);
	CG->ls[2]->obj[2].phys = false;
	//Void Floor
	TemplateVoidFloor(CG->ls[2], floorl2);
	// - Timer -
	level_timer = clock() / CLOCKS_PER_SEC;
}
void SetupL3(unsigned int direction) {
	static square *sqsl3;
	CG->found[3] = true;
	//Objects - player, 3 cubes
	InitLevel(CG->ls[3]);
	InitSquare(CG->CurrentPlayer);
	//Pointers
	CG->ls[3]->left = CG->ls[1];
	CG->ls[3]->right = NULL;
	CG->ls[3]->up = NULL;
	CG->ls[3]->down = NULL;
	//Player
	TemplatePlayer(CG->CurrentPlayer);
	add_obj((void *)CG->CurrentPlayer, OBJ_SQ, CG->ls[3]);
	CG->ls[3]->obj[0].phys = true;
	if (direction == DIR_RT) {
		ShiftSq(CG->CurrentPlayer, SHFT_RT, 2.5);
		ShiftSq(CG->CurrentPlayer, SHFT_UP, .1);
	}
	if (direction == DIR_LEFT) {
		ShiftSq(CG->CurrentPlayer, SHFT_LEFT, 3.2);
		ShiftSq(CG->CurrentPlayer, SHFT_DOWN, 1.5);
	}
	CG->ls[3]->obj[0].phys = true;
	// - Static Squares -
	sqsl3 = (square *)calloc(20, sizeof(square));
	//1
	InitSquare(&sqsl3[0]);
	add_obj((void *)&sqsl3[0], OBJ_SQ, CG->ls[3]);
	ShiftSq(&sqsl3[0], SHFT_RT, .8);
	ShiftSq(&sqsl3[0], SHFT_DOWN, 1.45);
	CG->ls[3]->obj[1].phys = false;
	//2
	InitSquare(&sqsl3[1]);
	add_obj((void *)&sqsl3[1], OBJ_SQ, CG->ls[3]);
	ShiftSq(&sqsl3[1], SHFT_LEFT, 2);
	CG->ls[3]->obj[2].phys = false;
	// - Timer -
	level_timer = clock() / CLOCKS_PER_SEC;
}
void SetupLevel(level *l, unsigned int direction) {
	if (l == CG->ls[0])
		SetupTitle();
	if (l == CG->ls[1])
		SetupL1(direction);
	if (l == CG->ls[2])
		SetupL2(direction);
	if (l == CG->ls[3])
		SetupL3(direction);
	return;
}
/*void ChangeLevel(level *l, unsigned int direction) {
	free(l->obj);
	l->obj = NULL;

	if (direction == SHFT_LEFT)
		CG->CurrentLevel = l->left;
	if (direction == SHFT_RT)
		CG->CurrentLevel = l->right;
	if (direction == SHFT_UP)
		CG->CurrentLevel = l->up;
	if (direction == SHFT_DOWN)
		CG->CurrentLevel = l->down;

	SetupReadLevelShift(CG->CurrentLevel, RevDir(direction));
	return;
}*/
void SnapToLevel(level *current, level *target, unsigned int direction) {
	free(current->obj);
	current->obj = NULL;
	CG->CurrentLevel = target;
	SetupLevel(target, direction);
	return;
}
int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();									// Reset The Current Modelview Matrix
	gluLookAt(CG->cam.posx, CG->cam.posy, CG->cam.posz, CG->cam.rotx, CG->cam.roty, CG->cam.rotz, CG->cam.vecx, CG->cam.vecy, CG->cam.vecz);
	DrawLevel(*CG->CurrentLevel);
	ShowFullTexFade(CG->global_tex, &global_alpha, global_decrease);

	if (TitleScreen) {
		ShowFullTex(CG->t[0].texture, 1.0);
	}

	glFlush();
	return TRUE;
}

//Restart
void KillPlayer() {
	int i;
	for (i = 0; i < CG->num_textures; i++)
		if (CG->t[i].type == TEX_DIED)
			CG->global_tex = CG->t[i].texture;
	global_alpha = 1.0;
	global_decrease = 0.001;

	CG->CurrentPlayer->phys.x_acc = 0.0;
	CG->CurrentPlayer->phys.x_vel = 0.0;
	CG->CurrentPlayer->phys.y_vel = 0.0;
	CG->CurrentPlayer->phys.y_acc = 0.0;

	level_timer = clock() + 1.5;

	SnapToLevel(CG->CurrentLevel, CG->CheckPoint, DIR_CHECKPT);
	return;
}
void SetCheckPoint() {
	int i;
	if (CG->CurrentLevel->IsCheckPoint) {
		CG->CheckPoint = CG->CurrentLevel;
		for (i = 0; i < CG->num_textures; i++)
			if (CG->t[i].type == TEX_TITLE)
				CG->global_tex = CG->t[i].texture;
		global_alpha = 2.0;
		global_decrease = 0.001;
	}
	return;
}
void RandTeleport() {
	int choice;
	srand(time(NULL)*time(NULL));
	choice = rand() % CG->num_chkpts;
	SnapToLevel(CG->CurrentLevel, CG->CheckPts[choice], DIR_CHECKPT);
}

//Game Reading From File
void RemoveWhiteSpace(char *str) {
	unsigned int i, i2 = 0;
	for (i = 0; i < strlen(str); i++) {
		if ((str[i] != ' ') && (str[i] != '\n') && (str[i] != '\t')) {
			str[i2] = str[i];
			i2++;
		}
	}
}
int fileSize(char *filename) {
	int size;
	FILE *fp;
	fopen_s(&fp, filename, "rb");
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	rewind(fp);
	fclose(fp);
	return size;
}
void eof_extract(char *str) {
	int i, count = 0;
	for (i = 0; str[i] != '!'; i++) count++;
	str[count] = 0;
}
// [X] [Y] [Z]
void ReadVertex(char *ptr, vertex *out) {
	char *str;
	ptr = strtok(NULL, ",");
	out->x = strtod(ptr, &str);
	ptr = strtok(NULL, ",");
	out->y = strtod(ptr, &str);
	ptr = strtok(NULL, ",");
	out->z = strtod(ptr, &str);
}
// [Color RGB] , [Texture] , [4 Vertices] , [Physics] , [Ground] , [WillKill] , [HasCollisions] !
char *ReadSq(char *str, square *out) {
	static char *ptr = NULL; 
	char *temp = NULL;
	temp = (char *)calloc(strlen(str), 1);
	
	ptr = strtok(str, ",");
	out->clr.r = strtod(ptr, &str);
	ptr = strtok(NULL, ",");
	out->clr.g = strtod(ptr, &str);
	ptr = strtok(NULL, ",");
	out->clr.b = strtod(ptr, &str);

	ptr = strtok(NULL, ",");
	out->tex = (GLuint)strtol(ptr, &str, 10);

	ReadVertex(ptr, &out->btl);
	ReadVertex(ptr, &out->btr);
	ReadVertex(ptr, &out->tpl);
	ReadVertex(ptr, &out->tpr);

	ptr = strtok(NULL, ",");
	if (!strcmp(ptr, "DEFAULT")) {
		out->phys.x_acc = 0.0;
		out->phys.x_vel = 0.0;
		out->phys.y_acc = 0.0;
		out->phys.y_vel = 0.0;
	}

	ptr = strtok(NULL, ",");
	if (ptr[0] == '1') out->ground = true; else out->ground = false;

	ptr = strtok(NULL, ",");
	if (ptr[0] == '1') out->WillKill = true; else out->WillKill = false;

	ptr = strtok(NULL, ",");
	if (ptr[0] == '1') out->coll = true; else out->coll = false;
	return (ptr+2);
}
char *ReadSpawns(char *str, spawns *out) {
	char *temp;
	str = strtok(str, "#");
	if (strcmp(str, "NULL")) {
		str = ReadSq(str, &out->defaultsp);
		str = strtok(str, "#");
	}
	else {
		str = strtok(NULL, "#");
		out->defaultsp.clr.r = 99999;
	}
	if (strcmp(str, "NULL")) {
		str = ReadSq(str, &out->leftsp);
		str = strtok(str, "#");
	}
	else {
		str = strtok(NULL, "#");
		out->leftsp.clr.r = 99999;
	}
	if (strcmp(str, "NULL")) {
		str = ReadSq(str, &out->rightsp);
		str = strtok(str, "#");
	}
	else {
		str = strtok(NULL, "#");
		out->rightsp.clr.r = 99999;
	}
	if (strcmp(str, "NULL")) {
		str = ReadSq(str, &out->upsp);
		str = strtok(str, "#");
	}
	else {
		str = strtok(NULL, "#");
		out->upsp.clr.r = 99999;
	}
	if (strcmp(str, "NULL")) {
		str = ReadSq(str, &out->downsp);
		return (str + 1);
	}
	else {
		out->downsp.clr.r = 99999;
		return (str + 6);
	}
}
// [IsCheckPoint] % [Skill Earned] % [Left Ptr] % [Right Ptr] % [Bottom Ptr] % [Top Ptr] % [Num of Objs] % [Spawns (5)] % [Squares / Phys...] !
char *ReadLevel(char *str, level *out, game *in, link *lk, int iter) {
	static char *ptr = NULL; char *temp2; int num_objs; 
	static char *ret = NULL;

	//add_obj((void *)&in->CurrentPlayer, OBJ_SQ, out);

	ptr = strtok(str, "%");
	out->IsCheckPoint = (bool)strtol(ptr, &temp2, 10);
	
	ptr = strtok(NULL, "%");
	if (strcmp(ptr, "NULL"))
		out->skills_earned = (unsigned int)strtoul(ptr, &temp2, 10);
	else
		out->skills_earned = 99999;

	ptr = strtok(NULL, "%");
	if (!strcmp(ptr, "NULL"))
		lk->left = 99999;
	else
		lk->left = (unsigned int)strtoul(ptr, &temp2, 10);
	ptr = strtok(NULL, "%");
	if (!strcmp(ptr, "NULL"))
		lk->right = 99999;
	else
		lk->right = (unsigned int)strtoul(ptr, &temp2, 10);
	ptr = strtok(NULL, "%");
	if (!strcmp(ptr, "NULL"))
		lk->up = 99999;
	else
		lk->up = (unsigned int)strtoul(ptr, &temp2, 10);
	ptr = strtok(NULL, "%");
	if (!strcmp(ptr, "NULL"))
		lk->down = 99999;
	else
		lk->down = (unsigned int)strtoul(ptr, &temp2, 10);

	out->i = 0;
	out->bytes = 4000;
	out->current = 0;

	ptr = strtok(NULL, "%");
	num_objs = (int)strtol(ptr, &temp2, 10);
	static square *squares;
	squares = (square *)calloc(num_objs, sizeof(square));
	out->obj = (obj *)calloc(num_objs, sizeof(obj));

	i = 0;
	ptr = strtok(NULL, "%");
	if (strcmp(ptr, "NULL")) {
		ptr = ReadSpawns(ptr, &out->spwn);
		ptr = strtok(ptr, "%");
	}
	else
		ptr = strtok(NULL, "%");

	while (ptr) {
		if (!strcmp(ptr, "NULL")) {
			ret = ptr + 5;
			break;
		}
		ptr = ReadSq(ptr, &squares[i]);
		add_obj((void *)&squares[i], OBJ_SQ, out);
		ptr = strtok(ptr, "%");
		out->obj[i].phys = (bool)strtol(ptr, &temp2, 10);
		ret = ptr;
		ptr = strtok(NULL, "%");
		i++;
	}
	return (ret + iter + 1);
} 
bool ReadGame(char *folder_name, game *g) {
	FILE *fp; 
	char *filename; 
	char *content; 
	char *content_copy[8];
	char *temp_content;
	char *temp = NULL;
	char *ptr = NULL;
	char main[] = "/main.game";
	int i;
	link *links;

	filename = calloc(strlen(folder_name) + 11, 1);
	sprintf(filename, "%s%s", folder_name, main);
	content = (char *)calloc(fileSize(filename), 1);

	fp = fopen(filename, "r");
	fread(content, 1, fileSize(filename), fp);
	free(filename);
	fclose(fp);
	RemoveWhiteSpace(content);

	fprintf(stderr, "%s", content);
	for (i = 0; i < 8; i++) {
		content_copy[i] = (char *)calloc(strlen(content), 1);
		strcpy(content_copy[i], content);
	}

	if (strstr(content, "FOUND:")) {
		temp_content = (char *)(strstr(content, "FOUND:") + 6);
		int counter = 0;
		for (i = 0; temp_content[i] != '!'; i++) counter++;
		g->found = calloc(counter, sizeof(bool));
		for (i = 0; i < counter; i++) {
			if (temp_content[i] == '1')
				g->found[i] = true;
			if (temp_content[i] == '0')
				g->found[i] = false;
		}
		g->num_levels = counter;
	}
	if (strstr(content, "NUM_CHKPTS:")) {
		temp_content = (char *)(strstr(content, "NUM_CHKPTS:") + 11);
		eof_extract(temp_content);
		g->num_chkpts = (int) strtol(temp_content, &ptr, 10);
	}
	if (strstr(content_copy[0], "NUM_TEXTURES:")) {
		temp_content = (char *)(strstr(content_copy[0], "NUM_TEXTURES:") + 13);
		eof_extract(temp_content);

		g->num_textures = (int)strtol(temp_content, &temp, 10);
		g->t = (tex *)calloc(g->num_textures, sizeof(tex));
	}
	if (strstr(content_copy[1], "VAR_TEXTURES:")) {
		temp_content = (char *)(strstr(content_copy[1], "VAR_TEXTURES:") + 13);
		eof_extract(temp_content);
		int counter = 0;
		ptr = strtok(temp_content, ",");
		while (ptr) {
			temp = (char *)calloc(strlen(folder_name) + strlen(ptr) + 2, 1);
			sprintf(temp, "%s/%s", folder_name, ptr);
			g->t[counter].texture = SOIL_load_OGL_texture(temp, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
			ptr = strtok(NULL, ",");
			free(temp);
			temp = NULL;
			counter++;
		}
	}
	if (strstr(content_copy[2], "CAMERA:")) {
		temp_content = (char *)(strstr(content_copy[2], "CAMERA:") + 7);
		eof_extract(temp_content);

		if (!strcmp(temp_content, "DEFAULT")) {
			InitCamera(&g->cam);
		}
	}
	if (strstr(content_copy[3], "SKILLS:")) {
		temp_content = (char *)(strstr(content_copy[3], "SKILLS:") + 7);
		eof_extract(temp_content);

		if (!strcmp(temp_content, "DEFAULT")) {
			g->skls.chkpt_teleport = false;
			g->skls.climber = false;
			g->skls.super_jump = false;
			g->skls.ultimate_climber = false;
			g->skls.void_immunity = false;
		}
	}
	if (strstr(content_copy[4], "LEVELS:")) {
		temp_content = (char *)(strstr(content_copy[4], "LEVELS:") + 7);
		eof_extract(temp_content);

		static level *levs;
		levs = (level *)calloc(g->num_levels, sizeof(level));
		g->ls = (level **)calloc(g->num_levels, sizeof(level *));
		links = (link *)calloc(g->num_levels, sizeof(link));
		for (i = 0; i < g->num_levels; i++) g->ls[i] = &levs[i];

		ptr = strtok(temp_content, "$");
		i = 0;
		while (ptr) {
			ptr = ReadLevel(ptr, g->ls[i], g, &links[i], i);
			i++;
			ptr = strtok(ptr, "$");
		}
		for (i = 0; i < g->num_levels; i++) {
			if (links[i].left == 99999)
				g->ls[i]->left = NULL;
			else
				g->ls[i]->left = g->ls[links[i].left];
			if (links[i].right == 99999)
				g->ls[i]->right = NULL;
			else
				g->ls[i]->right = g->ls[links[i].right];
			if (links[i].up == 99999)
				g->ls[i]->up = NULL;
			else
				g->ls[i]->up = g->ls[links[i].up];
			if (links[i].down == 99999)
				g->ls[i]->down = NULL;
			else
				g->ls[i]->down = g->ls[links[i].down];
		}
	}
	if (strstr(content_copy[5], "CHECKPTS:")) {
		temp_content = (char *)(strstr(content_copy[5], "CHECKPTS:") + 9);
		eof_extract(temp_content);

		g->CheckPts = (level **)calloc(g->num_chkpts, sizeof(level *));

		ptr = strtok(temp_content, ",");
		i = 0;
		while (ptr) {
			g->CheckPts[i] = g->ls[(int)strtol(ptr, &temp, 10)];
			i++;
			ptr = strtok(NULL, ",");
		}
	}
	if (strstr(content_copy[6], "CURRENT_CHECKPT:")) {
		temp_content = (char *)(strstr(content_copy[6], "CURRENT_CHECKPT:") + 16);
		eof_extract(temp_content);

		g->CheckPoint = g->ls[(int)strtol(temp_content, &temp, 10)];
	}
	if (strstr(content_copy[7], "PTEXTURES:")) {
		temp_content = (char *)(strstr(content_copy[6], "PTEXTURES:") + 10);
		eof_extract(temp_content);

		ptr = strtok(temp_content, ",");
		i = 0;
		while (ptr) {
			if (!strcmp(ptr, "t"))
				g->t[i].type = TEX_TITLE;
			if (!strcmp(ptr, "s1"))
				g->t[i].type = TEX_SKILLS1;
			ptr = strtok(NULL, ",");
			i++;
		}
	}
	
	g->global_tex = 0;

	return true;
}
void SetupReadLevel(unsigned int lev, unsigned int dir) {
	CG->CurrentLevel = CG->ls[lev];
	
	if (!TitleScreen) {
		CG->CurrentPlayer = CG->ls[lev]->obj[0].obj;
		
		if ((dir == DIR_LEFT) && (CG->CurrentLevel->spwn.leftsp.clr.r != 99999))
			*CG->CurrentPlayer = CG->CurrentLevel->spwn.leftsp;
		else if ((dir == DIR_RT) && (CG->CurrentLevel->spwn.rightsp.clr.r != 99999))
			*CG->CurrentPlayer = CG->CurrentLevel->spwn.rightsp;
		else if ((dir == DIR_UP) && (CG->CurrentLevel->spwn.upsp.clr.r != 99999))
			*CG->CurrentPlayer = CG->CurrentLevel->spwn.upsp;
		else if ((dir == DIR_DOWN) && (CG->CurrentLevel->spwn.downsp.clr.r != 99999))
			*CG->CurrentPlayer = CG->CurrentLevel->spwn.downsp;
		else if (CG->CurrentLevel->spwn.defaultsp.clr.r != 99999)
			*CG->CurrentPlayer = CG->CurrentLevel->spwn.defaultsp;
	}
	
	switch (CG->ls[lev]->skills_earned) {
	case (0) :
		if (!(CG->skls.super_jump))
			EnableSuperJump();
		break;
	case (1) :
		if (!CG->skls.void_immunity)
			EnableVoidImmunity();
		break;
	case (2) :
		if (!CG->skls.chkpt_teleport)
			EnableTeleport();
		break;
	case (3) :
		break;
	case (4) :
		if (!CG->skls.ultimate_climber)
			EnableUltimateWallClimber();
		break;
	}
}
void SetupReadLevelShift(level *l, unsigned int direction) {
	int i; level *target = NULL;
	if (direction == SHFT_LEFT)
		target = l->left;
	if (direction == SHFT_RT)
		target = l->right;
	if (direction == SHFT_UP)
		target = l->up;
	if (direction == SHFT_DOWN)
		target = l->down;

	for (i = 0; i < CG->num_levels; i++) {
		if (target == CG->ls[i])
			SetupReadLevel(i, RevDir(direction));
	}
}

//Window
GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL, 0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL, L"Release Of DC And RC Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL, L"Release Rendering Context Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL, L"Release Device Context Failed.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL, L"Could Not Release hWnd.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass(L"OpenGL", hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL, L"Could Not Unregister Class.", L"SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// Set hInstance To NULL
	}
}
BOOL CreateGLWindow(LPCWSTR title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc = (WNDPROC)WndProc;					// WndProc Handles Messages
	wc.cbClsExtra = 0;									// No Extra Window Data
	wc.cbWndExtra = 0;									// No Extra Window Data
	wc.hInstance = hInstance;							// Set The Instance
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground = NULL;									// No Background Required For GL
	wc.lpszMenuName = NULL;									// We Don't Want A Menu
	wc.lpszClassName = L"OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL, L"Failed To Register The Window Class.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight = height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel = bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL, L"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", L"NeHe GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL, L"Program Will Now Close.", L"ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle = WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		L"OpenGL",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Window Creation Error.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		16,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC = GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Create A GL Device Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Find A Suitable PixelFormat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Set The PixelFormat.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Create A GL Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Can't Activate The GL Rendering Context.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, L"Initialization Failed.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

//Main
LRESULT CALLBACK WndProc(HWND	hWnd,			// Handle For This Window
	UINT	uMsg,			// Message For This Window
	WPARAM	wParam,			// Additional Message Information
	LPARAM	lParam)			// Additional Message Information
{

	switch (uMsg)									// Check For Windows Messages	
	{
	case WM_ACTIVATE:							// Watch For Window Activate Message
	{
		// LoWord Can Be WA_INACTIVE, WA_ACTIVE, WA_CLICKACTIVE,
		// The High-Order Word Specifies The Minimized State Of The Window Being Activated Or Deactivated.
		// A NonZero Value Indicates The Window Is Minimized.
		if ((LOWORD(wParam) != WA_INACTIVE) && !((BOOL)HIWORD(wParam)))
			active = TRUE;						// Program Is Active
		else
			active = FALSE;						// Program Is No Longer Active

		return 0;								// Return To The Message Loop
	}

	case WM_SYSCOMMAND:							// Intercept System Commands
	{
		switch (wParam)							// Check System Calls
		{
		case SC_SCREENSAVE:					// Screensaver Trying To Start?
		case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
			return 0;							// Prevent From Happening
		}
		break;									// Exit
	}

	case WM_CLOSE:								// Did We Receive A Close Message?
	{
		PostQuitMessage(0);						// Send A Quit Message
		return 0;								// Jump Back
	}

	case WM_KEYDOWN:							// Is A Key Being Held Down?
	{
		keys[wParam] = TRUE;					// If So, Mark It As TRUE
		if (!TitleScreen) {
			//Jumping
			if (wParam == 'W') {
				if (!(lParam & (1 << 30))) {
					if ((GetCollisionSq(CG->CurrentPlayer, SHFT_UP, .0035, &clearance)) && (CG->CurrentPlayer->ground == true)) {
						CG->CurrentPlayer->ground = false;
						if (!CG->skls.super_jump)
							CG->CurrentPlayer->phys.y_acc += .00005;
						else
							CG->CurrentPlayer->phys.y_acc += .00006;
					}
				}
			}
			if (wParam == 'R') {
				if (!(lParam & (1 << 30))) {
					KillPlayer();
				}
			}
			if (wParam == 'E') {
				if (!(lParam & (1 << 30))) {
					SetCheckPoint();
				}
			}
			if (wParam == 'T') {
				if (!(lParam & (1 << 30))) {
					if (tele_clock < clock() / CLOCKS_PER_SEC) {
						RandTeleport();
						tele_clock = (clock() / CLOCKS_PER_SEC) + 60;
					}
				}
			}
			return 0;								// Jump Back
		}
		else {
			if (wParam == 'Y') {
				if (!(lParam & (1 << 30))) {
					TitleScreen = false;
					SetupReadLevel(1, DIR_CHECKPT);
					//WAS SetupLevel
					//SetupReadLevel(CG->ls[1]);
				}
			}
		}
	}

	case WM_KEYUP:								// Has A Key Been Released?
	{
		if (keys[wParam] == keys['P']) {
			//DispAxis(player.tpr);
		}
		keys[wParam] = FALSE;					// If So, Mark It As FALSE
		return 0;								// Jump Back
	}

	case WM_SIZE:								// Resize The OpenGL Window
	{
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
		global_x = LOWORD(lParam);
		global_y = HIWORD(lParam);
		return 0;								// Jump Back
	}
	case WM_MOUSEMOVE:
	{
		mouse_x = GET_X_LPARAM(lParam);
		mouse_y = GET_Y_LPARAM(lParam);
		return 0;
	}

	}


	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE	hInstance,			// Instance
	HINSTANCE	hPrevInstance,		// Previous Instance
	LPSTR		lpCmdLine,			// Command Line Parameters
	int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	BOOL	done = FALSE;								// Bool Variable To Exit Loop
	DefineColors();
	//Setup Game

	// Ask The User Which Screen Mode They Prefer
	fullscreen = false;

	// Create Our OpenGL Window
	if (!CreateGLWindow(L"NeHe's Solid Object Tutorial", global_x, global_y, 16, fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	//InitTextures();
	ReadGame("g2", &g2);
	CG = &g2;
	SetupReadLevel(0, DIR_CHECKPT);

	while (!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message == WM_QUIT)				// Have We Received A Quit Message?
			{
				done = TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			if (main_clock + .5 < clock()) {
				if (!TitleScreen) {
					// Gravity
					UpdatePhysics(*CG->CurrentLevel);
					// Test for keypresses

					if (GetAsyncKeyState('A')) {
						MoveSq(CG->CurrentPlayer, SHFT_LEFT, .0035);
					}
					if (GetAsyncKeyState('S')) {
						MoveSq(CG->CurrentPlayer, SHFT_DOWN, .0035);
					}
					if (GetAsyncKeyState('D')) {
						MoveSq(CG->CurrentPlayer, SHFT_RT, .0035);
					}
					//Rotate Camera
					if (GetAsyncKeyState('J')) {
						MoveCamera(SHFT_LEFT, .0025);
					}
					if (GetAsyncKeyState('L')) {
						MoveCamera(SHFT_RT, .0025);
					}
					if (GetAsyncKeyState('I')) {
						MoveCamera(SHFT_UP, .0025);
					}
					if (GetAsyncKeyState('K')) {
						MoveCamera(SHFT_DOWN, .0025);
					}
					main_clock = clock();
				}
			}
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
			{
				done = TRUE;							// ESC or DrawGLScene Signalled A Quit
			}
			else									// Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
			}

			if (keys[VK_F1])						// Is F1 Being Pressed?
			{
				keys[VK_F1] = FALSE;					// If So Make Key FALSE
				KillGLWindow();						// Kill Our Current Window
				fullscreen = !fullscreen;				// Toggle Fullscreen / Windowed Mode
				// Recreate Our OpenGL Window
				if (!CreateGLWindow(L"NeHe's Solid Object Tutorial", global_x, global_y, 16, fullscreen))
				{
					return 0;						// Quit If Window Was Not Created
				}
			}
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program
}
