#include <iostream>
#include <sstream>
#include <fstream>
#include <stack>
#include <vector>
#include <string>
#include <map>
#include <GLUT/glut.h>
#include "Transform.h"
#include "globals.h"

using namespace std;

// COLORS: sky: (135/225.0, 206/255.0, 250/255) forest green: (34/255.0, 139/255.0, 34/255.0)

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))
#define NumberOf(array) (sizeof(array)/sizeof(array[0]))

GLuint meshArrayBuffer;
GLuint meshElementArrayBuffer;
int size;

struct vertex {
	GLfloat x, y, z; //position
	GLfloat nx, ny, nz; //normal
	GLfloat padding[2]; // padding to be 32bit
};

/* Parses a line of input and takes appropriate action */
void parseLine(string l, vector<command> &commands) {
	float arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8;
	stringstream line(l);
	string cmd;
	line >> cmd;
	if(cmd[0] == '#') { // comment
		return;
	} else if(cmd == "") { // blank line
		return;
	} else if(cmd == "size") {
		line >> width >> height;
	} else if(cmd == "camera") {
		line >> arg1 >> arg2 >> arg3 >> arg4 >> arg5 >> fovy;
		//eyeinit = vec3(arg1,arg2,arg3);
		//yawInit = arg4;
		//pitchInit = arg5;
	} else if(cmd == "light") {
		if(numLights > 9) {
			return;
		} else {
			line >> arg1 >> arg2 >> arg3 >> arg4 >> arg5 >> arg6 >> arg7 >> arg8;
			light_position[numLights] = vec4(arg1,arg2,arg3,arg4);
			light_specular[numLights] = vec4(arg5,arg6,arg7,arg8);
			numLights++;
		}
	} else if(cmd == "ambient") {
		line >> arg1 >> arg2 >> arg3 >> arg4; //r g b a
		command com;
		com.op = amb;
		com.args = vec4(arg1,arg2,arg3,arg4);
		commands.push_back(com);
	} else if(cmd == "diffuse") {
		line >> arg1 >> arg2 >> arg3 >> arg4; //r g b a
		command com;
		com.op = diff;
		com.args = vec4(arg1,arg2,arg3,arg4);
		commands.push_back(com);
	} else if(cmd == "specular") {
		line >> arg1 >> arg2 >> arg3 >> arg4; //r g b a
		command com;
		com.op = spec;
		com.args = vec4(arg1,arg2,arg3,arg4);
		commands.push_back(com);
	} else if(cmd == "emission") {
		line >> arg1 >> arg2 >> arg3 >> arg4; //r g b a
		command com;
		com.op = emis;
		com.args = vec4(arg1,arg2,arg3,arg4);
		commands.push_back(com);
	} else if(cmd == "shininess") {
		line >> arg1; //s
		command com;
		com.op = shin;
		com.args = vec4(arg1,0.0,0.0,0.0);
		commands.push_back(com);
	} else if(cmd == "translate") {
		line >> arg1 >> arg2 >> arg3; //x y z
		command com;
		com.op = trans;
		com.args = vec4(arg1,arg2,arg3,0.0);
		commands.push_back(com);
	} else if(cmd == "rotate") {
		line >> arg1 >> arg2 >> arg3 >> arg4; //x y z theta
		command com;
		com.op = rot;
		com.args = vec4(arg1,arg2,arg3,arg4);
		commands.push_back(com);
	} else if(cmd == "scale") {
		line >> arg1 >> arg2 >> arg3; //x y z
		command com;
		com.op = scal;
		com.args = vec4(arg1,arg2,arg3,0.0);
		commands.push_back(com);
	} else if(cmd == "pushTransform") {
		command com;
		com.op = push;
		commands.push_back(com);
	} else if(cmd == "popTransform") {
		command com;
		com.op = pop;
		commands.push_back(com);
	} else {
		cerr << "Command \""<< cmd <<"\" not supported\n";
		exit(1);
	}
}

/* Parse the whole file */
vector<command> parseInput(char* filename) {
	ifstream myfile(filename,ifstream::in);
	string line;
	vector<command> commands;
	if(myfile.is_open()) {
		while(myfile.good()) {
			getline(myfile, line);
			parseLine(line,commands);
		}
	} else { 
		cerr << "Unable to open file " << filename << endl;
		exit(1);
	}
	myfile.close();
	return commands;
}

void parseOFF(char* filename){
	
	ifstream myfile(filename, ifstream::in);
	if(!myfile.is_open()){
		cout << "Unable to open file " << filename << endl;
	}
	
	vector<vec3> verticies; // vectors
	vector<vec3> faces; // faces
	int numVerts, numFaces;
	string line;
	
	
	getline(myfile, line); //skip first line
	getline(myfile, line);
	stringstream firstln(line);
	firstln >> numVerts >> numFaces;
	
	for (int i=0; i<numVerts; i+=1){
		float x,y,z;
		getline(myfile, line);
		stringstream ln(line);
		ln >> x >> y >> z;
		vec3 v(x,y,z);
		verticies.push_back(v);
	}
	
	for (int i=0; i<numFaces; i+=1){
		int v1,v2,v3,junk;
		getline(myfile, line);
		stringstream ln(line);
		ln >> junk >> v1 >> v2 >> v3;
		vec3 f(v1,v2,v3);
		faces.push_back(f);
		}
	
	vertex verts[numVerts];
	GLushort inds[numFaces*3];
	size = numFaces*3;
	for(int i=0; i<numVerts; i++){
		verts[i].x = verticies[i][0];
		verts[i].y = verticies[i][1];
		verts[i].z = verticies[i][2];
	}
	for(int i=0; i<numFaces; i++){
		inds[3*i] = faces[i][0];
		inds[3*i+1] = faces[i][1];
		inds[3*i+2] = faces[i][2];
	}
	
	cout << "Num Verts" << numVerts << endl;
	cout << "Num Faces" << numFaces << endl;
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_INDEX_ARRAY);
	
	glGenBuffers(1, &meshArrayBuffer);
	glGenBuffers(1, &meshElementArrayBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexPointer(3, GL_FLOAT, 32, BUFFER_OFFSET(0));
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshElementArrayBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), inds, GL_STATIC_DRAW);
}

/* Draw object number "obj" */
void draw(){
	glBindBuffer(GL_ARRAY_BUFFER, meshArrayBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshElementArrayBuffer);
	
	glVertexPointer(3, GL_FLOAT, 32, BUFFER_OFFSET(0));
	//glEnableClientState(GL_VERTEX_ARRAY);
	
	//if(normals[obj]){
	//	glNormalPointer(GL_FLOAT, 32, BUFFER_OFFSET(12));
	//	glEnableClientState(GL_NORMAL_ARRAY);
	//}
	//if (obj==0 && wire) {
	//	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	//}
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	//glDisableClientState(GL_NORMAL_ARRAY);
}
