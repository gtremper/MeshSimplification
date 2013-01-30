/*************************************************************************/
/*   This is a mesh viewer written by Graham Tremper and Gabe Fierro  */
/*************************************************************************/

#include <iostream>
#include <sstream>
#include <fstream>
#include <stack>
#include <map>
#include <vector>
#include <string>
#include <GLUT/glut.h>
#include "shaders.h"
#include "Transform.h"

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

using namespace std;

const int MAXLIGHTS = 10;
const double WALKSPEED = 0.15;
const float SENSITIVITY = 0.3;
const vec3 UP = vec3(0.0,1.0,0.0);
const vec3 LEFT = vec3(-1.0,0.0,0.0);
const vec3 CENTER = vec3(0.0,0.0,0.0);

double transX;
double transY;
double transZ;

float pitch;
float yaw;

vec3 eye;

float width, height;  
GLuint vertexshader, fragmentshader, shaderprogram ; // shaders
float fovy;    // field of view
vec4 light_position[MAXLIGHTS]; //current position of the 10 lights
vec4 light_specular[MAXLIGHTS]; //color of lights
int numLights;
int lastx, lasty ; // For mouse motion

bool useWire;
bool useFlat; // Toggle light shading on and off

/* Forward Declaration */
void loadObjects(char*);
void draw();
void parseOFF(char*);


/* Variables to set uniform params for lighting fragment shader */
GLuint isWire;
GLuint isFlat;
GLuint numLightsShader;

GLuint ambient ; 
GLuint diffuse ; 
GLuint specular ; 
GLuint shininess ;
GLuint emission ;

GLuint lightPosn;
GLuint lightColor;

/* Uses the Projection matrices (technically deprecated) to set perspective 
   We could also do this in a more modern fashion with glm.	*/ 
void reshape(int w, int h){
	glMatrixMode(GL_PROJECTION);
	
	width = w;
	height = h;
	// zNear=0.1, zFar=99
	mat4 mv = glm::perspectiveFov(fovy, width, height, 0.1f, 99.0f);
	
	glLoadMatrixf(&mv[0][0]); 
	glViewport(0, 0, width, height);
}


void printHelp() {
  std::cout << "\npress 'h' to print this message again.\n"
			<< "press 'f' to toggle wireframe.\n"
			<< "press 'p' to toggle flat shading.\n"
			<< "use 'wasd' to move object.\n"
			<< "press ESC to quit.\n";
	
}

/* Mouse Functions */

void mouseClick(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		lastx = x;
		lasty = y;
	}
}

void mouse(int x, int y) {
	
	int diffx=x-lastx; 
    int diffy=y-lasty; 
    lastx=x; //set lastx to the current x position
    lasty=y; //set lasty to the current y position
	
    yaw += diffx*SENSITIVITY; 
    pitch += diffy*SENSITIVITY;
	
	if (yaw > 360) yaw -= 360.0f;
	if (yaw < 0) yaw += 360.0f;
	if (pitch > 180) pitch -= 360.0f;
	if (pitch < -180) pitch += 360.0f;
}


/* Keyboard options */
void keyboard(unsigned char key, int x, int y) {
	switch(key) {
	case 'w':
		transZ += WALKSPEED;
		break;
	case 'a':
		transX += WALKSPEED;
		break;
	case 's':
		transZ -= WALKSPEED;
		break;
	case 'd':
		transX -= WALKSPEED;
		break;
	case 'q':
		transY -= WALKSPEED;
		break;
	case 'e':
		transY += WALKSPEED;
		break;
	case 'h':
		printHelp();
		break;
	case 27:  // Escape to quit
		exit(0);
		break; 
	case 'p':
		useFlat = !useFlat;
		glUniform1i(isFlat, useFlat);
		std::cout << "Flat shading is now set to" << (useFlat ? " true " : " false ") << "\n";
		break; 
	case 'f': //wireframe mode for maze
		useWire = !useWire;
		glUniform1i(isWire, useWire);
		if (useWire){
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		} else {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
		std::cout << "Wireframe is now set to" << (useWire ? " true " : " false ") << "\n";
		break;
	}
	glutPostRedisplay();
}


/* Default values so the program doesn't crash with empty input */
void def() {
	numLights = 1;
	fovy = 60;
	width = 600;
	height = 400;
}

void init() {
	eye = vec3(0,0,-10);
	transX = 0.0;
	transY = 0.0;
	transZ = 0.0;
	useWire = false;
	useFlat = false;

	glEnable(GL_DEPTH_TEST);

	vertexshader = initshaders(GL_VERTEX_SHADER, "shaders/light.vert.glsl");
	fragmentshader = initshaders(GL_FRAGMENT_SHADER, "shaders/light.frag.glsl");
	shaderprogram = initprogram(vertexshader, fragmentshader);
	isWire = glGetUniformLocation(shaderprogram,"wire");
	isFlat = glGetUniformLocation(shaderprogram,"flat");
	numLightsShader = glGetUniformLocation(shaderprogram,"numLights");
	
	ambient = glGetUniformLocation(shaderprogram,"ambient");
	diffuse = glGetUniformLocation(shaderprogram,"diffuse");
	specular = glGetUniformLocation(shaderprogram,"specular");
	shininess = glGetUniformLocation(shaderprogram,"shininess");
	emission = glGetUniformLocation(shaderprogram,"emission");
	
	lightPosn = glGetUniformLocation(shaderprogram,"lightPosn");
	lightColor = glGetUniformLocation(shaderprogram,"lightColor");
	
	glUniform1i(isFlat, useFlat);
	glUniform1i(isWire, useWire) ;
	glUniform1i(numLightsShader, numLights);
	
	
	
	light_specular[0] = vec4(.6,.3,0,1);
	light_position[0] = vec4(0,10,-10,1);
	glUniform4fv(lightColor, MAXLIGHTS, (GLfloat*)&light_specular[0]);
	glUniform4fv(lightPosn, MAXLIGHTS, (GLfloat*)&light_position[0]);	
	
	
	vec4 amb(.2,.2,.2,1);
	vec4 diff(.2,.2,.2,1);
	vec4 spec(1,1,1,1);
	vec4 emiss(0,0,0,1); 
	
	glUniform4fv(ambient,1,(GLfloat*)&amb);
	glUniform4fv(diffuse,1,(GLfloat*)&diff);
	glUniform4fv(specular,1,(GLfloat*)&spec);
	glUniform4fv(emission,1,(GLfloat*)&emiss);
	glUniform1f(shininess,20);
	
}

/* main display */
void display() {
	if (useWire){
		glClearColor(0,0,0,0);
	} else {
		glClearColor(135/225.0, 206/255.0, 250/255.0, 0);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	
	
	mat4 mv = glm::lookAt(eye, CENTER, UP);
	vec3 trans(transX,transY,transZ);
	mv = glm::translate(mv,trans);
	
	vec4 light[MAXLIGHTS];
	for (int i=0; i<numLights; i++){
		light[i] = mv * light_position[i];
	}
	glUniform4fv(lightPosn, MAXLIGHTS, (GLfloat*)&light[0]);
	
	mv = glm::rotate(mv,yaw,UP);
	mv = glm::rotate(mv,pitch,LEFT);
	
	glLoadMatrixf(&mv[0][0]); 
	
	draw();
	
	glutSwapBuffers();
}

int main(int argc, char* argv[]) {
	def();
	if (argc != 2) {
		std::cerr << "You need a mesh file as an argument";
		exit(1);
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("Mesh Viewer");
	parseOFF(argv[1]);
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutReshapeWindow(width,height);
	glutIdleFunc(display);
	glutMotionFunc(mouse);
	glutMouseFunc(mouseClick) ;
	printHelp();
	glutMainLoop();
	return 0;
}
