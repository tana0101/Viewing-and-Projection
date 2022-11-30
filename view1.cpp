#include <bits/stdc++.h>
#include <windows.h>
#include <GL/glut.h>
#define  PI   acos(-1)

using namespace std;

// 地板建構大小
#define SIZE  50
#define FLOOR_NUMBER_OF_GRID 10

// 音檔
#define jump_sound_path "jump.wav"
#define walk_sound_path "walk.wav"
#define finger_sound_path "finger.wav"

// 模式
#define NORMAL_MODE 0   // 預設視角
#define ROBOT_MODE 1    // 跟隨機器人視角
#define WALK_MODE 0     // 走路
#define RUN_MODE 1      // 跑步
#define RWAVE_MODE 0    // 右手揮臂
#define LWAVE_MODE 1    // 左手揮臂
#define WAVE_MODE 2     // 雙手揮臂

// 投影模式
#define PERSPECTIVE_MODE 1
#define PARALLEL_MODE 2
int projectionMode = PARALLEL_MODE;

#define ENABLE 1
#define DISABLE 0
int mode_viewVolume = DISABLE;

#define Step  0.5

// 運動參數
float step = 0.5; // 行走速度
float swing = 10; // 揮臂速度
float reduce_swing = 0; // 減小揮臂幅度

// 動作角度參數(控制揮臂與踢腿的方向)
int dir = 1;
int finger_dir = 1;

// 重點關節(旋轉角度,對X軸旋轉,對Y軸旋轉,對Z軸旋轉)
float relbow_joint_angle[4] = { 0, 0, 0, 1 }; // 右手手肘
float lelbow_joint_angle[4] = { 0, 0, 0, 1 }; // 左手手肘
float rhand_joint_angle[4] = { 0,0,0,1 }; // 右手肩膀
float lhand_joint_angle[4] = { 0,0,0,1 }; // 左手肩膀

// 其他關節的旋轉角度
float rfoot_joint_angle = -90, lfoot_joint_angle = -90; // 腳的旋轉角度
float rfinger_joint_angle = 45, lfinger_joint_angle = -45; // 手指的旋轉角度
float rknee_joint_angle = 0, lknee_joint_angle = 0; // 膝蓋的旋轉角度

// 視角參數
int mode_orient = NORMAL_MODE; // 視角模式
double fovy = 90;
double zNear = 1.5, zFar = 50;

/*-----Define a unit box--------*/
/* Vertices of the box */
float  points[][3] = {{-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5}, 
                      {0.5, 0.5, -0.5}, {-0.5, 0.5, -0.5}, 
                      {-0.5, -0.5, 0.5}, {0.5, -0.5, 0.5},
                      {0.5, 0.5, 0.5}, {-0.5, 0.5, 0.5}};
/* face of box, each face composing of 4 vertices */
int    face[][4] = {{0, 3, 2, 1}, {0, 1, 5, 4}, {1, 2, 6, 5}, 
                    {4, 5, 6, 7}, {2, 3, 7, 6}, {0, 4, 7, 3}};
float  colors[7][3] = { {0.5,0.5,0.5}, {0.7,0.7,0.7}, {0.7,0.5,0.5},
                     {0.5,0.5,0.5}, {0.5,0.7,0.5}, {0.5,0.5,0.7},
                     {1,0.0,0.0} };
/* indices of the box faces */
int    cube[6] = {0, 1, 2, 3, 4, 5};

/*-Declare GLU quadric objects, sphere, cylinder, and disk --*/
GLUquadricObj  *sphere=NULL, *cylind=NULL, *disk;


/*-Declare car position, orientation--*/
float  self_ang=-90.0, glob_ang=0.0;
float  position[3]={8.0, 3.0, 0.0};

/*-----Define window size----*/
int width=1000, height=1000;


/*-----Translation and rotations of eye coordinate system---*/
float   eyeDx=0.0, eyeDy=0.0, eyeDz=0.0;
float   eyeAngx=0.0, eyeAngy=0.0, eyeAngz=0.0;
double  Eye[3]={0.0, 0.0, 30.0}, Focus[3]={0.0, 0.0, 0.0}, 
        Vup[3]={0.0, 1.0, 0.0};

float   u[3][3]={{1.0,0.0,0.0}, {0.0,1.0,0.0}, {0.0,0.0,1.0}};
float   eye[3];
float   cv, sv; /* cos(5.0) and sin(5.0) */

/*-----Drawing stye
 0:4-windows, 1:x direction, 2:y direction, 3:z-dirtection, 4:perspective
 */
int style = 0;

void draw_scene(void);
void draw_rectangle(double x, double y, double z);
float norm2(float v[]);

/*----------------------------------------------------------
 * Procedure to initialize the working environment.
 */
void  myinit()
{
  glClearColor(0.0, 0.0, 0.0, 1.0);      /*set the background color BLACK */
                   /*Clear the Depth & Color Buffers */
  glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  /*---Create quadratic objects---*/
  if(sphere==NULL){
    sphere = gluNewQuadric();
    gluQuadricDrawStyle(sphere, GLU_FILL);
    gluQuadricNormals(sphere, GLU_SMOOTH);
  }
  if(cylind==NULL){
    cylind = gluNewQuadric();
    gluQuadricDrawStyle(cylind, GLU_FILL);
    gluQuadricNormals(cylind, GLU_SMOOTH);
  }
  if(disk==NULL){
    disk = gluNewQuadric();
    gluQuadricDrawStyle(disk, GLU_FILL);
    gluQuadricNormals(disk, GLU_SMOOTH);
  }

  /*---- Compute cos(5.0) and sin(5.0) ----*/
  cv = cos(5.0*PI/180.0);
  sv = sin(5.0*PI/180.0);
  /*---- Copy eye position ---*/
  eye[0] = Eye[0];
  eye[1] = Eye[1];
  eye[2] = Eye[2];
}


/*--------------------------------------------------------
 * Procedure to draw a 1x1x1 cube. The cube is within
 * (-0.5,-0.5,-0.5) ~ (0.5,0.5,0.5)
 */
void draw_cube()
{
  int    i;

  for(i=0;i<6;i++){
    glColor3fv(colors[i]);  /* Set color */
    glBegin(GL_POLYGON);  /* Draw the face */
      glVertex3fv(points[face[i][0]]);
      glVertex3fv(points[face[i][1]]);
      glVertex3fv(points[face[i][2]]);
      glVertex3fv(points[face[i][3]]);
    glEnd();
  }
}


/*---------------------------------------------------------
 * Procedure to draw the floor.
 */
void draw_floor()
{
  int  i, j;

  for(i=0;i<10;i++)
    for(j=0;j<10;j++){
      if((i+j)%2==0) glColor3f(0.8, 0.8, 0.8);
      else glColor3f(0.2, 0.2, 0.4);
      glBegin(GL_POLYGON);
        glVertex3f((i-5.0)*10.0, -2.2, (j-5.0)*10.0);
        glVertex3f((i-5.0)*10.0, -2.2, (j-4.0)*10.0);
        glVertex3f((i-4.0)*10.0, -2.2, (j-4.0)*10.0);
        glVertex3f((i-4.0)*10.0, -2.2, (j-5.0)*10.0);
      glEnd();
    }
}

/*-------------------------------------------------------
 * Procedure to draw three axes and the orign
 */
void draw_axes()
{

  /*----Draw a white sphere to represent the original-----*/
  glColor3f(0.9, 0.9, 0.9);

  gluSphere(sphere, 2.0,   /* radius=2.0 */
	    12,            /* composing of 12 slices*/
	    12);           /* composing of 8 stacks */

  /*----Draw three axes in colors, yellow, meginta, and cyan--*/
  /* Draw Z axis  */
  glColor3f(0.0, 0.95, 0.95);
  gluCylinder(cylind, 0.5, 0.5, /* radius of top and bottom circle */
	      10.0,              /* height of the cylinder */
              12,               /* use 12-side polygon approximating circle*/
	      3);               /* Divide it into 3 sections */
  
  /* Draw Y axis */
  glPushMatrix();
  glRotatef(-90.0, 1.0, 0.0, 0.0);  /*Rotate about x by -90', z becomes y */
  glColor3f(0.95, 0.0, 0.95);
  gluCylinder(cylind, 0.5, 0.5, /* radius of top and bottom circle */
	      10.0,             /* height of the cylinder */
              12,               /* use 12-side polygon approximating circle*/
	      3);               /* Divide it into 3 sections */
  glPopMatrix();

  /* Draw X axis */
  glColor3f(0.95, 0.95, 0.0);
  glPushMatrix();
  glRotatef(90.0, 0.0, 1.0, 0.0);  /*Rotate about y  by 90', x becomes z */
  gluCylinder(cylind, 0.5, 0.5,   /* radius of top and bottom circle */
	      10.0,             /* height of the cylinder */
              12,               /* use 12-side polygon approximating circle*/
	      3);               /* Divide it into 3 sections */
  glPopMatrix();
  /*-- Restore the original modelview matrix --*/
  glPopMatrix();
}

/*-------------------------------------------------------
 * Make viewing matrix
 */
void make_view(int x)
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  switch(x){
  case 4:       /* Perspective */

    /* In this sample program, eye position and Xe, Ye, Ze are computed
       by ourselves. Therefore, use them directly; no trabsform is
       applied upon eye coordinate system
       */
    gluLookAt(eye[0], eye[1], eye[2], 
	      eye[0]-u[2][0], eye[1]-u[2][1], eye[2]-u[2][2],
	      u[1][0], u[1][1], u[1][2]);
    break;

  case 1:       /* X direction parallel viewing */
    gluLookAt(30.0 - eyeDz, 0.0, 0.0, 0.0 - eyeDz, 0.0, 0.0, 0.0, 1.0, 0.0);
    break;
  case 2:       /* Y direction parallel viewing */
    gluLookAt(0.0, 30.0 - eyeDz, 0.0, 0.0, 0.0 - eyeDz, 0.0, 1.0, 0.0, 0.0);
    break;
  case 3:
    gluLookAt(0.0, 0.0, 30.0 - eyeDz, 0.0, 0.0, 0.0 - eyeDz, 0.0, 1.0, 0.0);
    break;
  }
}

/*------------------------------------------------------
 * Procedure to make projection matrix
 */
void make_projection(int x)
{ // perspective projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if(x == PERSPECTIVE_MODE){
    gluPerspective(fovy, (double) width/ (double) height, zNear , zFar);
  }else if (x == PARALLEL_MODE) { // parallel projections
      if(width>height)
	glOrtho(-60.0, 60.0, -60.0*(float)height/(float)width, 
		60.0*(float)height/(float)width, 
		-0.0, 100.0);
      else
	glOrtho(-60.0*(float)width/(float)height, 
		60.0*(float)width/(float)height, -60.0, 60.0, 
		-0.0, 100.0);
  }
  glMatrixMode(GL_MODELVIEW);
}

struct rotate_node {
    GLfloat x, y, z;
    rotate_node() {}
    rotate_node(GLfloat _x, GLfloat _y, GLfloat _z) {
        x = _x; y = _y; z = _z;
    }
};
// 矩陣旋轉
rotate_node rotate(GLfloat x, GLfloat y, GLfloat z) {
    // y
    GLfloat x1 = x * cos(eyeAngy * PI / 180.0) + z * sin(eyeAngy * PI / 180.0);
    GLfloat y1 = y;
    GLfloat z1 = -x * sin(eyeAngy * PI / 180.0) + z * cos(eyeAngy * PI / 180.0);

    // x
    GLfloat x2 = x1;
    GLfloat y2 = y1 * cos(eyeAngx * PI / 180.0) - z1 * sin(eyeAngx * PI / 180.0);
    GLfloat z2 = y1 * sin(eyeAngx * PI / 180.0) + z1 * cos(eyeAngx * PI / 180.0);

    // z
    GLfloat x3 = x2 * cos(eyeAngz * PI / 180.0) - y2 * sin(eyeAngz * PI / 180.0);
    GLfloat y3 = x2 * sin(eyeAngz * PI / 180.0) + y2 * cos(eyeAngz * PI / 180.0);
    GLfloat z3 = z2;
    
    // result
    return rotate_node(x3, y3, z3);
    // 參考自：https://openhome.cc/Gossip/ComputerGraphics/Rotate3Dimension.htm
}

/*---------------------------------------------------------
 * Procedure to draw view volume, eye position, focus ,...
 * for perspective projection
 */
void draw_view()
{
  int    i;

  glMatrixMode(GL_MODELVIEW);

  /*----Draw Eye position-----*/
  glPushMatrix();
  glTranslatef(eye[0], eye[1], eye[2]);
  glColor3f(0.0, 1.0, 0.0);
  glutWireSphere(1.0, 10, 10);
  glPopMatrix();

  /*----Draw eye coord. axes -----*/
  glColor3f(1.0, 1.0, 0.0);           /* Draw Xe */
  glBegin(GL_LINES);
    glVertex3f(eye[0], eye[1], eye[2]);
    glVertex3f(eye[0]+20.0*u[0][0], eye[1]+20.0*u[0][1], eye[2]+20.0*u[0][2]);
  glEnd();

  glColor3f(1.0, 0.0, 1.0);          /* Draw Ye */
  glBegin(GL_LINES);
    glVertex3f(eye[0], eye[1], eye[2]);
    glVertex3f(eye[0]+20.0*u[1][0], eye[1]+20.0*u[1][1], eye[2]+20.0*u[1][2]);
  glEnd();

  glColor3f(0.0, 1.0, 1.0);          /* Draw Ze */
  glBegin(GL_LINES);
    glVertex3f(eye[0], eye[1], eye[2]);
    glVertex3f(eye[0]+20.0*u[2][0], eye[1]+20.0*u[2][1], eye[2]+20.0*u[2][2]);
  glEnd();

  
  /*-----draw eye projection (viewVolume)-----*/
  
  if (mode_viewVolume == ENABLE) {

      // asp = w/h
      GLfloat nearH = 2 * tan(fovy / 2 * PI / 180.0) * zNear;
      GLfloat nearW = nearH * ((double)width / (double)height);
      GLfloat farH = 2 * tan(fovy / 2 * PI / 180.0) * zFar;
      GLfloat farW = farH * ((double)width / (double)height);
      rotate_node node;

      // far line
      glColor3f(1.0, 1.0, 1.0);
      glBegin(GL_LINES);
      glVertex3f(eye[0], eye[1], eye[2]);
      node = rotate(eye[0] + farW / 2, eye[1] + farH / 2, eye[2] - zFar);
      glVertex3f(node.x, node.y, node.z);
      glEnd();

      glBegin(GL_LINES);
      glVertex3f(eye[0], eye[1], eye[2]);
      node = rotate(eye[0] - farW / 2, eye[1] + farH / 2, eye[2] - zFar);
      glVertex3f(node.x, node.y, node.z);
      glEnd();

      glBegin(GL_LINES);
      glVertex3f(eye[0], eye[1], eye[2]);
      node = rotate(eye[0] + farW / 2, eye[1] - farH / 2, eye[2] - zFar);
      glVertex3f(node.x, node.y, node.z);
      glEnd();

      glBegin(GL_LINES);
      glVertex3f(eye[0], eye[1], eye[2]);
      node = rotate(eye[0] - farW / 2, eye[1] - farH / 2, eye[2] - zFar);
      glVertex3f(node.x, node.y, node.z);
      glEnd();

      // far square
      double farWidth = 5;
      double nearWidth = 3;
      glEnable(GL_LINE_STIPPLE);

      glColor3f(1.0, 1.0, 1.0);
      glLineWidth(farWidth);
      glLineStipple(1, 0x3f3f);
      glBegin(GL_LINES);
      node = rotate(eye[0] + farW / 2, eye[1] + farH / 2, eye[2] - zFar);
      glVertex3f(node.x, node.y, node.z);
      node = rotate(eye[0] + farW / 2, eye[1] - farH / 2, eye[2] - zFar);
      glVertex3f(node.x, node.y, node.z);
      glEnd();

      glColor3f(1.0, 1.0, 1.0);
      glLineWidth(farWidth);
      glLineStipple(1, 0x3f3f);
      glBegin(GL_LINES);
      node = rotate(eye[0] + farW / 2, eye[1] - farH / 2, eye[2] - zFar);
      glVertex3f(node.x, node.y, node.z);
      node = rotate(eye[0] - farW / 2, eye[1] - farH / 2, eye[2] - zFar);
      glVertex3f(node.x, node.y, node.z);
      glEnd();

      glColor3f(1.0, 1.0, 1.0);
      glLineWidth(farWidth);
      glLineStipple(1, 0x3f3f);
      glBegin(GL_LINES);
      node = rotate(eye[0] - farW / 2, eye[1] - farH / 2, eye[2] - zFar);
      glVertex3f(node.x, node.y, node.z);
      node = rotate(eye[0] - farW / 2, eye[1] + farH / 2, eye[2] - zFar);
      glVertex3f(node.x, node.y, node.z);
      glEnd();

      glColor3f(1.0, 1.0, 1.0);
      glLineWidth(farWidth);
      glLineStipple(1, 0x3f3f);
      glBegin(GL_LINES);
      node = rotate(eye[0] - farW / 2, eye[1] + farH / 2, eye[2] - zFar);
      glVertex3f(node.x, node.y, node.z);
      node = rotate(eye[0] + farW / 2, eye[1] + farH / 2, eye[2] - zFar);
      glVertex3f(node.x, node.y, node.z);
      glEnd();

      /*
      //near square
      glColor3f(1.0, 0.0, 0.0);
      glLineWidth(nearWidth);
      glLineStipple(1, 0x3f3f);
        glBegin(GL_LINES);
        node = rotate(eye[0] + nearW / 2, eye[1] + nearH / 2, eye[2] - zNear);
        glVertex3f(node.x, node.y, node.z);
        node = rotate(eye[0] + nearW / 2, eye[1] - nearH / 2, eye[2] - zNear);
        glVertex3f(node.x, node.y, node.z);
      glEnd();

      glColor3f(1.0, 0.0, 0.0);
      glLineWidth(nearWidth);
      glLineStipple(1, 0x3f3f);
      glBegin(GL_LINES);
        node = rotate(eye[0] + nearW / 2, eye[1] - nearH / 2, eye[2] - zNear);
        glVertex3f(node.x, node.y, node.z);
        node = rotate(eye[0] - nearW / 2, eye[1] - nearH / 2, eye[2] - zNear);
        glVertex3f(node.x, node.y, node.z);
      glEnd();

      glColor3f(1.0, 0.0, 0.0);
      glLineWidth(nearWidth);
      glLineStipple(1, 0x3f3f);
      glBegin(GL_LINES);
        node = rotate(eye[0] - nearW / 2, eye[1] - nearH / 2, eye[2] - zNear);
        glVertex3f(node.x, node.y, node.z);
        node = rotate(eye[0] - nearW / 2, eye[1] + nearH / 2, eye[2] - zNear);
        glVertex3f(node.x, node.y, node.z);
      glEnd();

      glColor3f(1.0, 0.0, 0.0);
      glLineWidth(nearWidth);
      glLineStipple(1, 0x3f3f);
      glBegin(GL_LINES);
        node = rotate(eye[0] - nearW / 2, eye[1] + nearH / 2, eye[2] - zNear);
        glVertex3f(node.x, node.y, node.z);
        node = rotate(eye[0] + nearW / 2, eye[1] + nearH / 2, eye[2] - zNear);
        glVertex3f(node.x, node.y, node.z);
      glEnd();
      */
      glDisable(GL_LINE_STIPPLE);
  }
}

void draw_cube(double x, double y, double z) {
    glPushMatrix();
    glScalef(x, y, z);
    draw_cube();
    glPopMatrix();
}

void draw_object() {
    glPushMatrix();
    glTranslatef(10, -0.5, 10);
    glColor3fv(colors[1]);
    draw_cube(8, 3, 8);
    glTranslatef(0, 3, 0);
    glColor3fv(colors[2]);
    draw_cube(4, 3, 4);
    glTranslatef(0, 3, 0);
    glColor3fv(colors[3]);
    draw_cube(2, 3, 2);
    glPopMatrix();
}

// draw_robot
void draw_rectangle(double x, double y, double z) {
    glPushMatrix();
    glTranslatef(x / 2, 0, 0);
    glScalef(x, y, z);
    draw_cube();
    glPopMatrix();
}

void draw_finger(float finger_joint_angle) {
    glPushMatrix();
    glRotatef(finger_joint_angle, 0, 1, 0);
    glColor3fv(colors[6]);
    draw_rectangle(0.8, 0.3, 0.3);
    glPopMatrix();
}

void draw_hand(float* hand_joint_angle, float* elbow_joint_angle) {
    glColor3fv(colors[6]);
    glRotatef(-90, 0, 0, 1); // 預設讓手放下

    glPushMatrix();
    glRotatef(hand_joint_angle[0], hand_joint_angle[1], hand_joint_angle[2], hand_joint_angle[3]);
    glutSolidSphere(0.5, 20.0, 20.0); //肩膀
    draw_rectangle(1.5, 0.7, 0.7); //上臂


    glTranslatef(1.5, 0, 0);
    glPushMatrix();
    glColor3fv(colors[6]);
    glRotatef(90, 0, 0, 1); // 預設讓手肘微彎
    glRotatef(elbow_joint_angle[0], elbow_joint_angle[1], elbow_joint_angle[2], elbow_joint_angle[3]);
    glutSolidSphere(0.5, 20.0, 20.0); //手肘

    draw_rectangle(1.5, 0.7, 0.7); //下臂

    glTranslatef(1.5, 0, 0);
    glColor3fv(colors[6]);
    glutSolidSphere(0.3, 20.0, 20.0); //手腕
    // 左手指
    draw_finger(lfinger_joint_angle);
    // 右手指
    draw_finger(rfinger_joint_angle);

    glPopMatrix();
    glPopMatrix();
}

void draw_foot(float foot_joint_angle, float knee_joint_angle) {
    glPushMatrix();
    glRotatef(foot_joint_angle, 0, 0, 1);
    draw_rectangle(1.5, 0.7, 0.7); //大腿
    glTranslatef(1.5, 0, 0);
    glColor3fv(colors[6]);
    glutSolidSphere(0.5, 20.0, 20.0); //膝蓋
    glRotatef(knee_joint_angle, 0, 0, 1);
    draw_rectangle(1.5, 0.7, 0.7); //小腿
    glPopMatrix();
}

/*--------------------------------------------------------
 * Display callback procedure to draw the scene
 */
void display()
{
  /*Clear previous frame and the depth buffer */
  glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
  switch(style){
  case 0:
    make_view(4);
    make_projection(PERSPECTIVE_MODE);
    glViewport(width/2, 0, width/2, height/2);
    draw_scene();

    make_view(1);
    make_projection(projectionMode);
    glViewport(0, height/2, width/2, height/2);
    draw_scene();
    make_view(1);
    draw_view();

    make_view(2);
    glViewport(width/2, height/2, width/2, height/2);
    draw_scene();
    make_view(2);
    draw_view();

    make_view(3);
    glViewport(0, 0, width/2, height/2);
    draw_scene();
    make_view(3);
    draw_view();
    break;

  case 1:
      make_view(1);
      make_projection(projectionMode);
      glViewport(0, 0, width, height);
      draw_scene();
      make_view(1);
      draw_view();
      break;

  case 2:
      make_view(2);
      make_projection(projectionMode);
      glViewport(0, 0, width, height);
      draw_scene();
      make_view(2);
      draw_view();
      break;

  case 3:
      make_view(3);
      make_projection(projectionMode);
      glViewport(0, 0, width, height);
      draw_scene();
      make_view(3);
      draw_view();
      break;

  case 4:
    make_view(4);
    glViewport(0, 0, width, height);
    make_projection(PERSPECTIVE_MODE);
    draw_scene();
    break;
  }
  /*-------Swap the back buffer to the front --------*/
  glutSwapBuffers();
  return;

}

/*-------------------------------------------------------
 * This procedure draw the car
 */
void draw_scene()
{

  draw_floor();
  //draw_object();
  draw_axes();

  glTranslatef(position[0], position[1], position[2]);

  glRotatef(self_ang, 0.0, 1.0, 0.0);

  glPushMatrix();              /* Save M1 coord. sys */
  glScalef(2.0, 4.0, 3.0);    /* Scale up the axes */
  draw_cube();
  glPopMatrix();               /* Get M1 back */

  // 頭
  glPushMatrix();
  glTranslatef(0.0, 3.0, 0.0);
  glutSolidSphere(1.0, 20.0, 20.0);
  glPopMatrix();

  // 左手
  glPushMatrix();
  glTranslatef(0.0, 1.5, -1.8);
  draw_hand(lhand_joint_angle, lelbow_joint_angle);
  glPopMatrix();

  // 右手
  glPushMatrix();
  glTranslatef(0.0, 1.5, 1.8);
  draw_hand(rhand_joint_angle, relbow_joint_angle);
  glPopMatrix();

  // 左腳
  glPushMatrix();
  glTranslatef(0.0, -2.0, -0.75);
  draw_foot(lfoot_joint_angle, lknee_joint_angle);
  glPopMatrix();

  // 右腳
  glPushMatrix();
  glTranslatef(0.0, -2.0, 0.75);
  draw_foot(rfoot_joint_angle, rknee_joint_angle);
  glPopMatrix();

}


/*--------------------------------------------------
 * Reshape callback function which defines the size
 * of the main window when a reshape event occurrs.
 */
void my_reshape(int w, int h)
{

  width = w;
  height = h;

  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(90.0, (double) w/ (double) h, 1.5, 50.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

// sound

void walk_sound() {
    PlaySound(TEXT(walk_sound_path), NULL, SND_ASYNC | SND_FILENAME | SND_NOSTOP);
}

void jump_sound() {
    PlaySound(TEXT(jump_sound_path), NULL, SND_ASYNC | SND_FILENAME);
}

void finger_sound() {
    PlaySound(TEXT(finger_sound_path), NULL, SND_ASYNC | SND_FILENAME | SND_NOSTOP);
}

// motion

void reset() {

    //motion
    dir = 1, finger_dir = 1;

    // hand
    rhand_joint_angle[0] = 0, rhand_joint_angle[1] = 0, rhand_joint_angle[2] = 0, rhand_joint_angle[3] = 1;
    lhand_joint_angle[0] = 0, lhand_joint_angle[1] = 0, lhand_joint_angle[2] = 0, lhand_joint_angle[3] = 1;
    relbow_joint_angle[0] = 0, relbow_joint_angle[1] = 0, relbow_joint_angle[2] = 0, relbow_joint_angle[3] = 1;
    lelbow_joint_angle[0] = 0, lelbow_joint_angle[1] = 0, lelbow_joint_angle[2] = 0, lelbow_joint_angle[3] = 1;
    rfinger_joint_angle = 45, lfinger_joint_angle = -45;

    //foot
    rfoot_joint_angle = -90, lfoot_joint_angle = -90;
    rknee_joint_angle = 0, lknee_joint_angle = 0;

    // y_axis
    position[1] = 3.0;
}

void reset_pos() {
    self_ang = -90.0, glob_ang = 0.0;
    position[0] = 8.0, position[1] = 3.0, position[2] = 0.0;
}

void squat() {
    reset();
    rknee_joint_angle = -90;
    lknee_joint_angle = -90;
    rfoot_joint_angle = 20;
    lfoot_joint_angle = 20;
    position[1] -= 1.5 * sin(20);
    display();
    Sleep(200);
    reset();
}

void jump() {
    jump_sound();
    for (int i = 0; i < 5; i++) {
        position[1] += 0.5;
        display();
        Sleep(20);
    }
    for (int i = 0; i < 5; i++) {
        position[1] -= 0.5;
        display();
        Sleep(20);
    }
}

void wave(int mode) {
    reset();
    if (mode == RWAVE_MODE) {
        for (int i = 0; i < 5; i++) {
            rhand_joint_angle[0] += 18;
            display();
            Sleep(20);
        }
        relbow_joint_angle[1] = 0, relbow_joint_angle[2] = 1, relbow_joint_angle[3] = 0;
        for (int i = 0; i < 5; i++) {
            relbow_joint_angle[0] += 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 10; i++) {
            relbow_joint_angle[0] -= 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 10; i++) {
            relbow_joint_angle[0] += 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 10; i++) {
            relbow_joint_angle[0] -= 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 5; i++) {
            relbow_joint_angle[0] += 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 5; i++) {
            rhand_joint_angle[0] -= 18;
            display();
            Sleep(20);
        }
    }
    else if (mode == LWAVE_MODE) {
        for (int i = 0; i < 5; i++) {
            lhand_joint_angle[0] += 18;
            display();
            Sleep(20);
        }
        lelbow_joint_angle[1] = 0, lelbow_joint_angle[2] = 1, lelbow_joint_angle[3] = 0;
        for (int i = 0; i < 5; i++) {
            lelbow_joint_angle[0] += 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 10; i++) {
            lelbow_joint_angle[0] -= 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 10; i++) {
            lelbow_joint_angle[0] += 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 10; i++) {
            lelbow_joint_angle[0] -= 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 5; i++) {
            lelbow_joint_angle[0] += 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 5; i++) {
            lhand_joint_angle[0] -= 18;
            display();
            Sleep(20);
        }
    }
    else if (mode == WAVE_MODE) {
        for (int i = 0; i < 5; i++) {
            lhand_joint_angle[0] += 18;
            rhand_joint_angle[0] += 18;
            display();
            Sleep(20);
        }
        relbow_joint_angle[1] = 0, relbow_joint_angle[2] = 1, relbow_joint_angle[3] = 0;
        lelbow_joint_angle[1] = 0, lelbow_joint_angle[2] = 1, lelbow_joint_angle[3] = 0;
        for (int i = 0; i < 5; i++) {
            lelbow_joint_angle[0] += 9;
            relbow_joint_angle[0] -= 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 10; i++) {
            lelbow_joint_angle[0] -= 9;
            relbow_joint_angle[0] += 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 10; i++) {
            lelbow_joint_angle[0] += 9;
            relbow_joint_angle[0] -= 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 10; i++) {
            lelbow_joint_angle[0] -= 9;
            relbow_joint_angle[0] += 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 5; i++) {
            lelbow_joint_angle[0] += 9;
            relbow_joint_angle[0] -= 9;
            display();
            Sleep(20);
        }
        for (int i = 0; i < 5; i++) {
            lhand_joint_angle[0] -= 18;
            rhand_joint_angle[0] -= 18;
            display();
            Sleep(20);
        }
    }
}

void dance() {
    rhand_joint_angle[1] = 0, rhand_joint_angle[2] = 1, rhand_joint_angle[3] = 0;
    lhand_joint_angle[1] = 0, lhand_joint_angle[2] = 1, lhand_joint_angle[3] = 0;
    relbow_joint_angle[1] = 0, relbow_joint_angle[2] = 1, relbow_joint_angle[3] = 0;
    lelbow_joint_angle[1] = 0, lelbow_joint_angle[2] = 1, lelbow_joint_angle[3] = 0;
    for (int i = 0; i < 5; i++) {
        rhand_joint_angle[0] -= 18;
        lhand_joint_angle[0] += 18;
        display();
        Sleep(50);
    }
    for (int i = 0; i < 5; i++) {
        relbow_joint_angle[0] += 18;
        lelbow_joint_angle[0] -= 18;
        display();
        Sleep(50);
    }
    for (int i = 0; i < 5; i++) {
        rhand_joint_angle[0] -= 18;
        lhand_joint_angle[0] += 18;
        display();
        Sleep(50);
    }
    for (int i = 0; i < 5; i++) {
        relbow_joint_angle[0] += 18;
        lelbow_joint_angle[0] -= 18;
        display();
        Sleep(50);
    }
    for (int i = 0; i < 5; i++) {
        relbow_joint_angle[0] -= 18;
        lelbow_joint_angle[0] += 18;
        display();
        Sleep(50);
    }
    for (int i = 0; i < 5; i++) {
        rhand_joint_angle[0] += 18;
        lhand_joint_angle[0] -= 18;
        display();
        Sleep(50);
    }
    for (int i = 0; i < 5; i++) {
        relbow_joint_angle[0] -= 18;
        lelbow_joint_angle[0] += 18;
        display();
        Sleep(50);
    }
    for (int i = 0; i < 5; i++) {
        rhand_joint_angle[0] += 18;
        lhand_joint_angle[0] -= 18;
        display();
        Sleep(50);
    }
    reset();
    squat();
    reset();
}

void look_on_me() { // 讓視角觀看機器人
    u[2][0] = eye[0] - position[0];
    u[2][1] = eye[1] - position[1];
    u[2][2] = eye[2] - position[2];
}

void set_moving(int mode_move) { // 設定動作
    if (mode_move == WALK_MODE) {
        step = 0.1; // 行走速度
        swing = 5; // 揮臂速度
        reduce_swing = 30; // 減小揮臂幅度
    }
    else if (mode_move == RUN_MODE) {
        step = 0.5;
        swing = 10;
        reduce_swing = 0;
    }
}

void move(int mode_move) {
    // 設定動作
    set_moving(mode_move);
    // 隱形牆
    float next_step_sin = step * sin(self_ang * PI / 180.0);
    float next_step_cos = step * cos(self_ang * PI / 180.0);
    if (-40.0 < position[0] + next_step_cos && position[0] + next_step_cos < 40)position[0] += next_step_cos;
    if (-40.0 < position[2] - next_step_sin && position[2] - next_step_sin < 40)position[2] -= next_step_sin;
    // 行走揮臂
    if (rfoot_joint_angle > (0 - reduce_swing)) reset(); // 轉換動作時的修正
    if (rfoot_joint_angle == (0 - reduce_swing) || rfoot_joint_angle == (-180 + reduce_swing)) dir *= -1;
    rfoot_joint_angle += (swing * dir);
    lfoot_joint_angle += (swing * -1 * dir);
    rhand_joint_angle[0] += (swing * -1 * dir);
    lhand_joint_angle[0] += (swing * dir);
    // 走路音效
    if (rfoot_joint_angle == -90 || lfoot_joint_angle == -90) {
        walk_sound();
    }
}

void back(int mode_move) {
    // 設定動作
    set_moving(mode_move);
    // 隱形牆
    float next_step_sin = step * sin(self_ang * PI / 180.0);
    float next_step_cos = step * cos(self_ang * PI / 180.0);
    if (-40.0 < position[0] - next_step_cos && position[0] - next_step_cos < 40)position[0] -= next_step_cos;
    if (-40.0 < position[2] + next_step_sin && position[2] + next_step_sin < 40)position[2] += next_step_sin;
    // 行走揮臂
    if (rfoot_joint_angle > (0 - reduce_swing)) reset(); // 轉換動作時的修正
    if (rfoot_joint_angle == (0 - reduce_swing) || rfoot_joint_angle == (-180 + reduce_swing)) dir *= -1;
    rfoot_joint_angle -= (swing * dir);
    lfoot_joint_angle -= (swing * -1 * dir);
    rhand_joint_angle[0] -= (swing * -1 * dir);
    lhand_joint_angle[0] -= (swing * dir);
    // 走路音效
    if (rfoot_joint_angle == -90 || lfoot_joint_angle == -90) {
        walk_sound();
    }
}

/*--------------------------------------------------
 * Keyboard callback func. When a 'q' key is pressed,
 * the program is aborted.
 */
int prev_key; // 上一次按鍵
void my_quit(unsigned char key, int ix, int iy)
{
  int    i;
  float  x[3], y[3], z[3];
  cout << int(key) << "\n";
  /*--- Control the car. ----*/
  if (int(key) == 27) exit(0); //ESC
  if (key == 'w') {
      if (mode_orient == ROBOT_MODE) {
          look_on_me(); // 讓視角觀看機器人
      }
      move(RUN_MODE);
  }
  else if (int(key) == 87) { // shift + w 走路
      if (mode_orient == ROBOT_MODE) {
          look_on_me(); // 讓視角觀看機器人
      }
      if (prev_key != 87 && prev_key != 83) reset();
      move(WALK_MODE);
  }
  else if (key == 's') {
      if (mode_orient == ROBOT_MODE) {
          look_on_me();
      }
      back(RUN_MODE);
  }
  else if (key == 83) { // shift + s 走路
      if (mode_orient == ROBOT_MODE) {
          look_on_me();
      }
      if (prev_key != 87 && prev_key != 83) reset();
      back(WALK_MODE);
  }
  else if (key == 'a') {                        /* make a left turn */
      self_ang += 10.0;
      if (self_ang > 360.0) self_ang -= 360.0;
  }
  else if (key == 'd') {                        /* make a right turn */
      self_ang += -10.0;
      if (self_ang < 0.0) self_ang += 360.0;
  }
  else if (key == 'r') {
      reset(); // 一下r動作復位
      if (prev_key == int('r')) { // 兩下r位置復位
          reset_pos();
      }
  }
  /*------ action ------*/
  else if (key == 't') {
      if (rfinger_joint_angle == 0 || rfinger_joint_angle == 45) finger_dir *= -1;
      rfinger_joint_angle += (5 * finger_dir);
      lfinger_joint_angle += (5 * -1 * finger_dir);
      // 手指音效
      if (rfinger_joint_angle == 45) {
          finger_sound();
      }
  }
  else if (int(key) == 32) { // space
      jump();
  }
  else if (int(key) == int('`')) {
      style = 0;
  }
  else if (key == '1') {
      style = 1;
  }
  else if (key == '2') {
      style = 2;
  }
  else if (key == '3') {
      style = 3;
  }
  else if (key == '4') {
      style = 4;
  }
  else if (key == '5') { // 揮左手
      wave(LWAVE_MODE);
  }
  else  if (key == '6') { // 揮右手
      wave(RWAVE_MODE);
  }
  else if (key == '7') {  // 揮雙手
      wave(WAVE_MODE);
  }
  else if (key == '8') { // 跳舞
      dance();
  }
  else if (key == '9') { // squat
      squat();
  }
  else if (key == '0') {
      squat();
      jump();
  }
  else if (key == '=') { // 跟隨機器人
      if (mode_orient == ROBOT_MODE) { // 改為預設視角
          mode_orient = NORMAL_MODE;
      }
      else if(mode_orient == NORMAL_MODE) {
          look_on_me();
          mode_orient = ROBOT_MODE;
      }
  }
  else if (int(key) == 9) { // viewVolume
      if (mode_viewVolume == ENABLE) { // 改為預設視角
          mode_viewVolume = DISABLE;
      }
      else if (mode_viewVolume == DISABLE){
          mode_viewVolume = ENABLE;
      }
  }
/*------transform the EYE coordinate system ------*/
  else if(key=='z'){
    eyeDy += 0.5;       /* move up */
    for(i=0;i<3;i++) eye[i] -= 0.5*u[1][i];
  }
  else if (key == 'x') {
      eyeDy += -0.5;       /* move down */
      for (i = 0; i < 3; i++) eye[i] += 0.5 * u[1][i];
  }
  else if (int(key) == 23) {    // ctrl + w
      eyeDz += 0.5;        /* move in */
      for (int i = 0; i < 3; i++) eye[i] -= 0.5 * u[2][i];
  }
  else if (int(key) == 19) {    // ctrl + s
      eyeDz += -0.5;       /* zoom out */
      for (int i = 0; i < 3; i++) eye[i] += 0.5 * u[2][i];
  }
  else if (int(key) == 4) {     // ctrl + d
    eyeDx += -0.5;       /* move left */
    for(i=0;i<3;i++) eye[i] += 0.5*u[0][i];
  }else if(int(key)==1){        // ctrl + a
    eyeDx += 0.5;        /* move right */
    for(i=0;i<3;i++) eye[i] -= 0.5*u[0][i];
  }else if(key=='i'){             /* pitching */
    eyeAngx += 5.0;
    if(eyeAngx > 360.0) eyeAngx -= 360.0;
    y[0] = u[1][0]*cv - u[2][0]*sv;
    y[1] = u[1][1]*cv - u[2][1]*sv;
    y[2] = u[1][2]*cv - u[2][2]*sv;

    z[0] = u[2][0]*cv + u[1][0]*sv;
    z[1] = u[2][1]*cv + u[1][1]*sv;
    z[2] = u[2][2]*cv + u[1][2]*sv;

    for(i=0;i<3;i++){
      u[1][i] = y[i];
      u[2][i] = z[i];
    }
  }else if(key=='k'){          
    eyeAngx += -5.0;
    if(eyeAngx<0.0) eyeAngx += 360.0;
    y[0] = u[1][0]*cv + u[2][0]*sv;
    y[1] = u[1][1]*cv + u[2][1]*sv;
    y[2] = u[1][2]*cv + u[2][2]*sv;

    z[0] = u[2][0]*cv - u[1][0]*sv;
    z[1] = u[2][1]*cv - u[1][1]*sv;
    z[2] = u[2][2]*cv - u[1][2]*sv;

    for(i=0;i<3;i++){
      u[1][i] = y[i];
      u[2][i] = z[i];
    }
  }else if(key==','){            /* heading */
    eyeAngy += 5.0;
    if(eyeAngy>360.0) eyeAngy -= 360.0;
    for(i=0;i<3;i++){
      x[i] = cv*u[0][i] - sv*u[2][i];
      z[i] = sv*u[0][i] + cv*u[2][i];
    }
    for(i=0;i<3;i++){
      u[0][i] = x[i];
      u[2][i] = z[i];
    }
  }else if(key=='.'){
    eyeAngy += -5.0;
    if(eyeAngy<0.0) eyeAngy += 360.0;
    for(i=0;i<3;i++){
      x[i] = cv*u[0][i] + sv*u[2][i];
      z[i] = -sv*u[0][i] + cv*u[2][i];
    }
    for(i=0;i<3;i++){
      u[0][i] = x[i];
      u[2][i] = z[i];
    }
  }else if(key=='l'){            /* rolling */
    eyeAngz += 5.0;
    if(eyeAngz>360.0) eyeAngz -= 360.0;
    for(i=0;i<3;i++){
      x[i] = cv*u[0][i] - sv*u[1][i];
      y[i] = sv*u[0][i] + cv*u[1][i];
    }
    for(i=0;i<3;i++){
      u[0][i] = x[i];
      u[1][i] = y[i];
    }
  }else if(key=='j'){
    eyeAngz += -5.0;
    if(eyeAngz<0.0) eyeAngz += 360.0;
    for(i=0;i<3;i++){
      x[i] = cv*u[0][i] + sv*u[1][i];
      y[i] = -sv*u[0][i] + cv*u[1][i];
    }
    for(i=0;i<3;i++){
      u[0][i] = x[i];
      u[1][i] = y[i];
    }
  }
  display();
  prev_key = int(key);
}

void mouse(int button, int state, int x, int y)
{
    // Wheel reports as button 3(scroll up) and button 4(scroll down)
    if ((button == 3) || (button == 4)) // It's a wheel event
    {
        // Each wheel event reports like a button click, GLUT_DOWN then GLUT_UP
        if (state == GLUT_UP) return; // Disregard redundant GLUT_UP events
        printf("Scroll %s At %d %d\n", (button == 3) ? "Up" : "Down", x, y);

        // zoom
        if (button == 4 && fovy < 90) { // in
            fovy+=3;
        }
        else if (button == 3 && fovy > 0) { //out
            fovy-=3;
        }
    }
    else {  // normal button event
#define PERSPECTIVE_MODE 1
#define PARALLEL_MODE 2
        if (projectionMode == PARALLEL_MODE) {
            projectionMode = PERSPECTIVE_MODE;
        }else projectionMode = PARALLEL_MODE;
        printf("Button %s At %d %d\n", (state == GLUT_DOWN) ? "Down" : "Up", x, y);
    }
    display();
}


/*---------------------------------------------------
 * Main procedure which defines the graphics environment,
 * the operation style, and the callback func's.
 */
void main(int argc, char **argv)
{
  /*-----Initialize the GLUT environment-------*/
  glutInit(&argc, argv);

  /*-----Depth buffer is used, be careful !!!----*/
  glutInitDisplayMode(GLUT_DOUBLE| GLUT_RGB| GLUT_DEPTH);

  glutInitWindowSize(width, height);
  glutCreateWindow("windmill");

  myinit();      /*---Initialize other state varibales----*/
  
  /*----Associate callback func's whith events------*/
  glutDisplayFunc(display);
  /*  glutIdleFunc(display); */
  glutReshapeFunc(my_reshape);
  glutKeyboardFunc(my_quit);
  glutMouseFunc(mouse);

  glutMainLoop();
}


/*------------------------------------------
 * Procedure to compute norm of vector v[]
 */
float norm2(float v[])
{
  double  sum;
  float   temp;

  sum = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
  temp = (float) sqrt(sum);
  return temp;
}
