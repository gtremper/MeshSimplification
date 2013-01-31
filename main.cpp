/*************************************************************************/
/*   This is a mesh viewer written by Graham Tremper and Gabe Fierro  */
/*************************************************************************/

#include <iostream>
#include <GLUT/glut.h>
#include "shaders.h"
#include "mesh.h"

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

using namespace std;

/*** CONSTANTS  ***/
const int MAXLIGHTS = 10;
const double WALKSPEED = 0.15;
const float SENSITIVITY = 0.3;
const vec3 UP = vec3(0.0,1.0,0.0);
const vec3 LEFT = vec3(-1.0,0.0,0.0);
const char * CONFIG = "config.txt";

/***  UPDATE FREQUENTLY  ***/
vec3 trans;
float pitch;
float yaw;
float cameraPitch;
float cameraYaw;
int lastx, lasty; // For mouse motion
int currentLight;
bool useWire;
bool useFlat;
bool moveLight;
bool cameraMode;
bool animate;

/***  SCENE PARAMETERS  ***/
GLuint vertexshader, fragmentshader, shaderprogram ; // shaders
vec4 light_position[MAXLIGHTS]; //current position of the 10 lights
vec4 light_specular[MAXLIGHTS]; //color of lights
vec3 eye; 
vec3 lookat;
float fovy; 
int numLights;

/* Forward Declaration */
void parseConfig(const char*);
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
vec4 emis; //emission data

GLuint lightPosn;
GLuint lightColor;

void reshape(int w, int h){
	glMatrixMode(GL_PROJECTION);
	float width = w;
	float height = h;
	// zNear=0.1, zFar=99
	mat4 mv = glm::perspectiveFov(fovy, width, height, 0.1f, 99.0f);
	glLoadMatrixf(&mv[0][0]); 
	glViewport(0, 0, w, h);
}

void printHelp() {
  std::cout << "\npress 'h' to print this message again.\n"
			<< "press 'f' to toggle wireframe.\n"
			<< "press 'p' to toggle flat shading.\n"
			<< "use 'wasd' to move object.\n"
			<< "use 'c' to move camera.\n"
			<< "use 'm' to animate.\n"
			<< "use '1-9' to move lights.\n"
			<< "press ESC to quit.\n\n";	
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

	if (cameraMode) {
		cameraYaw += diffx*SENSITIVITY; 
	    cameraPitch += diffy*SENSITIVITY;
	} else {
		yaw += diffx*SENSITIVITY; 
	    pitch += diffy*SENSITIVITY;
	}
	
	if (yaw > 360) yaw -= 360.0f;
	if (yaw < 0) yaw += 360.0f;
	if (pitch > 180) pitch -= 360.0f;
	if (pitch < -180) pitch += 360.0f;
}

/* Keyboard options */
void keyboard(unsigned char key, int x, int y) {
	switch(key) {
	case 'w':
		if (moveLight) light_position[currentLight] += vec4(0,0,SENSITIVITY,0);
		else trans += vec3(0,0,WALKSPEED);
		break;
	case 'a':
		if (moveLight) light_position[currentLight] += vec4(SENSITIVITY,0,0,0);
		else trans += vec3(WALKSPEED,0,0);
		break;
	case 's':
		if (moveLight) light_position[currentLight] -= vec4(0,0,SENSITIVITY,0);
		else trans -= vec3(0,0,WALKSPEED);
		break;
	case 'd':
		if (moveLight) light_position[currentLight] -= vec4(SENSITIVITY,0,0,0);
		else trans -= vec3(WALKSPEED,0,0);
		break;
	case 'q':
		if (moveLight) light_position[currentLight] += vec4(0,SENSITIVITY,0,0);
		else trans += vec3(0,WALKSPEED,0);
		break;
	case 'e':
		if (moveLight) light_position[currentLight] -= vec4(0,SENSITIVITY,0,0);
		else trans -= vec3(0,WALKSPEED,0);
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
		cout << "Wireframe is now set to" << (useWire ? " true " : " false ") << "\n";
		break;
	case 'c':
		cameraMode = !cameraMode;
		cameraPitch=0;
		cameraYaw=0;
		cout << "Camera rotation is now set to" << (cameraMode ? " true " : " false ") << "\n";
		break;
	case 'm':
		animate = !animate;
		cout << "Animate is now set to" << (animate ? " true " : " false ") << "\n";
		break;
	case 48:
	case 49:
	case 50:
	case 51:
	case 52:
	case 53:
	case 54:
	case 55:
	case 56:
	case 57:
		int num = key-49;
		if (num == -1) {
			moveLight = false;
			cout << "Controlling Model" << endl;
			break;
		} else if (num >= numLights) {
			moveLight = false;
			cout << "That light does not exist" << endl;
			break;
		}
		moveLight = true;
		currentLight = num;
		cout << "Now controlling light "<<num<<"\n";
		break;
	}
	glutPostRedisplay();
}

void init() {
	/* Default Values */
	eye = vec3(0,0,-10);
	trans = vec3(0,0,0);
	numLights = 0;
	fovy = 60;
	useWire = false;
	useFlat = false;
	moveLight = false;
	cameraMode = false;
	animate = false;

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
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_INDEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, 32, BUFFER_OFFSET(0));
	glNormalPointer(GL_FLOAT, 32, BUFFER_OFFSET(12));
	
	glUniform1i(isFlat, useFlat);
	glUniform1i(isWire, useWire);
	
	parseConfig(CONFIG);
	
	glUniform4fv(lightColor, MAXLIGHTS, (GLfloat*)&light_specular[0]);
	glUniform4fv(lightPosn, MAXLIGHTS, (GLfloat*)&light_position[0]);
	glUniform1i(numLightsShader, numLights);
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
	
	mat4 mv;
	cout << cameraYaw << cameraPitch << endl;
	if (cameraMode) {
		mat4 cameraRot = glm::translate(mat4(1.0f),eye-lookat);
		cameraRot = glm::rotate(cameraRot,cameraYaw,UP);
		cameraRot = glm::rotate(cameraRot,cameraPitch,LEFT);
		cameraRot = glm::translate(cameraRot,lookat-eye);
		glm::lookAt(eye, vec3(cameraRot*vec4(lookat,1.0f)), UP);
	} else {
		mv = glm::lookAt(eye, lookat, UP);
	}
	
	mv = glm::translate(mv,trans);
	
	vec4 light[MAXLIGHTS];
	for (int i=0; i<numLights; i++){
		light[i] = mv * light_position[i];
		if (moveLight) {
			glUniform4fv(emission,1,&light_specular[i][0]);
			mat4 lightTrans = glm::translate(mv,vec3(light_position[i]));
			glLoadMatrixf(&lightTrans[0][0]);
			glutSolidSphere(.4,20,20);
		}
	}
	glUniform4fv(emission,1,&emis[0]);
	glUniform4fv(lightPosn, MAXLIGHTS, (GLfloat*)&light[0]);
	
	
	if (animate) yaw += 1;
	mv = glm::rotate(mv,yaw,UP);
	mv = glm::rotate(mv,pitch,LEFT);
	glLoadMatrixf(&mv[0][0]); 
	
	draw();
	
	glutSwapBuffers();
}

int main(int argc, char* argv[]) {
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
	glutReshapeWindow(600,400);
	glutIdleFunc(display);
	glutMotionFunc(mouse);
	glutMouseFunc(mouseClick) ;
	printHelp();
	glutMainLoop();
	return 0;
}
