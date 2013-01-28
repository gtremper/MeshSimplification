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

float yaw;
float pitch;

vec3 eye; 		// The (regularly updated) vector coordinates of the eye location 

bool useLights; // Toggle light shading on and off
bool pictureMode = false; // no movement so you can take picture
float width, height;  
GLuint vertexshader, fragmentshader, shaderprogram ; // shaders
bool flyMode;
float fovy;    // field of view
vec4 light_position[MAXLIGHTS]; //current position of the 10 lights
vec4 light_specular[MAXLIGHTS]; //color of lights
int numLights;
int lastx, lasty ; // For mouse motion

bool wire;

std::vector<command> commands; 

/* Forward Declaration */
std::vector<command> parseInput(char*);
void loadObjects(char*);
void draw();
void parseOFF(char*);


/* Variables to set uniform params for lighting fragment shader */
GLuint islight ;
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
			<< "press 'c' to toggle cartoon shading.\n"
			<< "press 'f' to switch to fly mode.\n"
			<< "press 'l' to toggle lights and textures.\n"
			<< "press 'p' to begin the animation.\n"
			<< "press 'q' to toggle wireframe mode.\n"
			<< "press 't' to toggle textures and leave lights on\n"
			<< "press 'v' to toggle vertex shading.\n"
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
			exit(0) ;
			break ; 
	case 'l':
		useLights = !useLights;
		glUniform1i(islight, useLights) ;
		std::cout << "useLights is now set to" << (useLights ? " true " : " false ") << "\n";
		break; 
	case 'f': //wireframe mode for maze
		wire = !wire;
		std::cout << "wireframe is now set to" << (wire ? " true " : " false ") << "\n";
		break;
	case 'm':
		pictureMode = !pictureMode;
		std::cout << "picture mode is now set to" << (pictureMode ? " true " : " false ") << "\n";
		break;
	}
	glutPostRedisplay();
}


/* Default values so the program doesn't crash with empty input */
void def() {
	numLights = 0;
	fovy = 60;
	width = 600;
	height = 400;
}

void init() {
	eye = vec3(0,0,-10);
	transX = 0.0;
	transY = 0.0;
	transZ = 0.0;
	
	useLights = true;
	flyMode = false;
	lastx = width/2;
	lasty = height/2;

	glEnable(GL_DEPTH_TEST);

	vertexshader = initshaders(GL_VERTEX_SHADER, "shaders/light.vert.glsl");
	fragmentshader = initshaders(GL_FRAGMENT_SHADER, "shaders/light.frag.glsl");
	shaderprogram = initprogram(vertexshader, fragmentshader);
	islight = glGetUniformLocation(shaderprogram,"islight");
	numLightsShader = glGetUniformLocation(shaderprogram,"numLights");
	
	ambient = glGetUniformLocation(shaderprogram,"ambient");
	diffuse = glGetUniformLocation(shaderprogram,"diffuse");
	specular = glGetUniformLocation(shaderprogram,"specular");
	shininess = glGetUniformLocation(shaderprogram,"shininess");
	emission = glGetUniformLocation(shaderprogram,"emission");
	
	lightPosn = glGetUniformLocation(shaderprogram,"lightPosn");
	lightColor = glGetUniformLocation(shaderprogram,"lightColor");
	
	/* Set variables that don't change during simulation */
	glUniform1i(islight, useLights) ;
	glUniform1i(numLightsShader, numLights);
	
	glUniform4fv(lightColor, MAXLIGHTS, (GLfloat*)&light_specular[0]);	
}


/* Draws objects based on list of commands 
void drawObjects(std::vector<command> comms, mat4 mv) {
	std::stack<mat4> matStack;
	matStack.push(mat4(1.0));
	
	std::vector<command>::iterator it;
	for(it = comms.begin(); it<comms.end(); it++){
		
		command com = *it;
		mat4 transf , sc , tr;
		vec3 axis;
		switch(com.op) {
			case amb:
				glUniform4fv(ambient,1,(GLfloat*)&com.args[0]);
				break;
			case diff:
				glUniform4fv(diffuse,1,(GLfloat*)&com.args[0]);
				break;
			case spec:
				glUniform4fv(specular,1,(GLfloat*)&com.args[0]);
				break;
			case emis:
			 	glUniform4fv(emission,1,(GLfloat*)&com.args[0]);
				break;
			case shin:
				glUniform1f(shininess,com.args[0]);
				break;
			case teapot:
				transf = mv*matStack.top();
				glLoadMatrixf(&transf[0][0]);
				glutSolidTeapot(com.args[0]);
				break;
			case sphere:
				transf = mv*matStack.top();
				glLoadMatrixf(&transf[0][0]);
				glutSolidSphere(com.args[0],20,20);
				break;
			case cube:
				transf = mv*matStack.top();
				glLoadMatrixf(&transf[0][0]);
				glutSolidCube(com.args[0]);
				break;
			case trans:
				transf = Transform::translate(com.args[0],com.args[1],com.args[2]);
				transf = glm::transpose(transf);
				matStack.top() = matStack.top()*transf;
				break;
			case rot:
				transf = mat4(Transform::rotate(com.args[3], vec3(com.args)));
				transf = glm::transpose(transf);
				matStack.top() = matStack.top()*transf;
				break;
			case scal:
				transf = Transform::scale(com.args[0],com.args[1],com.args[2]);
				transf = glm::transpose(transf);
				matStack.top() = matStack.top()*transf;
				break;
			case push:
				transf = matStack.top();
				matStack.push(transf);
				break;
			case pop:
				matStack.pop();
				break;
			default:
				transf = mv*matStack.top();
				glLoadMatrixf(&transf[0][0]);
				draw(-com.op-1); // negative so doesnt conflict with enum
				break;
		}	
	}
}
*/

/* main display */
void display() {
	glClearColor(135/225.0, 206/255.0, 250/255.0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	
	
	mat4 mv = glm::lookAt(eye, CENTER, UP);
	
	vec3 trans(transX,transY,transZ);
	mv = glm::translate(mv,trans);
	
	mv = glm::rotate(mv,yaw,UP);
	mv = glm::rotate(mv,pitch,LEFT);
	
	glLoadMatrixf(&mv[0][0]); 
	
	vec4 light[MAXLIGHTS];
	for (int i=0; i<numLights; i++){
		light[i] = mv * light_position[i];
	}
	glUniform4fv(lightPosn, MAXLIGHTS, (GLfloat*)&light[0]);
	//drawObjects(commands,mv);	
	
	//glutSolidTeapot(2.5); //temporarily draw teapot
	
	draw();
	
	glutSwapBuffers();
}

int main(int argc, char* argv[]) {
	def();
	//if (argc != 2) {
	//	std::cerr << "You need a mesh file as an argument";
	//	exit(1);
	//}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("Mesh Viewer");
	//loadObjects(argv[2]);
	//commands = parseInput(argv[1]);
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
