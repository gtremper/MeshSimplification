#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "mesh.h"

using namespace std;

// COLORS: sky: (135/225.0, 206/255.0, 250/255)

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

GLuint meshArrayBuffer;
GLuint meshElementArrayBuffer;
int size;

extern vec3 eye;
extern vec3 lookat;
extern float fovy;
extern int numLights;
extern GLuint ambient ; 
extern GLuint diffuse ; 
extern GLuint specular ; 
extern GLuint shininess ;
extern GLuint emission ;
extern vec4 emis;
extern vec4 light_specular[];
extern vec4 light_position[];
extern int MAXLIGHTS;

void parseConfig(const char* filename){
	ifstream file(filename, ifstream::in);
	if(!file.is_open()){
		cout << "Unable to open file " << filename << endl;
		exit(1);
	}
	string linetxt;
	while(file.good()){
		getline(file,linetxt);
		string cmd;
		stringstream line(linetxt);
		line >> cmd;
		if (cmd[0] == '#') {
			continue;
		} else if (cmd == "") {
			continue;
		} else if (cmd == "camera"){
			float x,y,z,lx,ly,lz,fov;
			line >> x >> y >> z >> lx >> ly >> lz >> fov;
			eye = vec3(x,y,z);
			lookat = vec3(lx,ly,lz);
			fovy = fov;
		} else if (cmd=="light"){
			if(numLights > 9) {
				return;
			} else {
				float x,y,z,w,r,g,b,a;
				line >> x >> y >> z >> w >> r >> g >> b >> a;
				light_position[numLights] = vec4(x,y,z,w);
				light_specular[numLights] = vec4(r,g,b,a);
				numLights++;
			}
		} else if(cmd == "ambient") {
			float r,g,b,a;
			line >> r >> g >> b >> a;
			GLfloat vec[] = {r,g,b,a};
			glUniform4fv(ambient,1,vec);
		} else if(cmd == "diffuse") {
			float r,g,b,a;
			line >> r >> g >> b >> a;
			GLfloat vec[] = {r,g,b,a};
			glUniform4fv(diffuse,1,vec);
		} else if(cmd == "specular") {
			float r,g,b,a;
			line >> r >> g >> b >> a;
			GLfloat vec[] = {r,g,b,a};
			glUniform4fv(specular,1,vec);
		} else if(cmd == "emission") {
			float r,g,b,a;
			line >> r >> g >> b >> a;
			GLfloat vec[] = {r,g,b,a};
			emis = vec4(r,g,b,a);
			glUniform4fv(emission,1,vec);
		} else if(cmd == "shininess") {
			float shine;
			line >> shine;
			glUniform1f(shininess,shine);
		}	
	}
}

void parseOFF(char* filename){
	
	ifstream myfile(filename, ifstream::in);
	if(!myfile.is_open()){
		cout << "Unable to open file " << filename << endl;
		exit(1);
	}
	
	vector<vec3> vertices; // vectors
	vector<vec3> faces; // faces
	int numVerts, numFaces;
	string line;
	
	
	getline(myfile, line); //skip first line
	getline(myfile, line);
	stringstream firstln(line);
	firstln >> numVerts >> numFaces;
	
	vertices.reserve(numVerts);
	faces.reserve(numFaces);
	
	vertex* verts = new vertex[numVerts];
	for (int i=0; i<numVerts; i+=1){
		float x,y,z;
		getline(myfile, line);
		stringstream ln(line);
		ln >> x >> y >> z;
		vec3 v(x,y,z);
		vertices.push_back(v);
        verts[i] = (vertex){ v[0], v[1], v[2], 0.0, 0.0, 0.0, 0.0,0.0};
	}
	
	for (int i=0; i<numFaces; i+=1){
		int v1,v2,v3,junk;
		getline(myfile, line);
		stringstream ln(line);
		ln >> junk >> v1 >> v2 >> v3;
		vec3 f(v1,v2,v3);
		faces.push_back(f);
		}
	
	GLuint* inds = new GLuint[numFaces*3];
	size = numFaces*3;
	for(int i=0; i<numFaces; i+=1){
		int ind0 = faces[i][0];
		int ind1 = faces[i][1];
		int ind2 = faces[i][2];
		inds[3*i] = ind0;
		inds[3*i+1] = ind1;
		inds[3*i+2] = ind2;
		
		vec3 v0 = vertices[ind0];
		vec3 v1 = vertices[ind1];
		vec3 v2 = vertices[ind2];
		vec3 norm = glm::cross(v1-v0,v2-v0);
		verts[ind0].normal[0] += norm[0];
		verts[ind0].normal[1] += norm[1];
		verts[ind0].normal[2] += norm[2];
		verts[ind1].normal[0] += norm[0];
		verts[ind1].normal[1] += norm[1];
		verts[ind1].normal[2] += norm[2];
		verts[ind2].normal[0] += norm[0];
		verts[ind2].normal[1] += norm[1];
		verts[ind2].normal[2] += norm[2];
	}
	for (int i=0; i<numVerts; i+=1){
		vec3 normal(verts[i].normal[0],verts[i].normal[1],verts[i].normal[2]);
		normal = glm::normalize(normal);
		verts[i].normal[0] = normal[0];
		verts[i].normal[1] = normal[1];
		verts[i].normal[2] = normal[2];
	}
	
    Mesh m = Mesh(vertices, faces);
	
	glGenBuffers(1, &meshArrayBuffer);
	glGenBuffers(1, &meshElementArrayBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*numVerts, verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshElementArrayBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*3*numFaces, inds, GL_STATIC_DRAW);
	
	delete [] inds;
	delete [] verts;
}

Mesh* parseOFFmesh(char* filename){
	ifstream myfile(filename, ifstream::in);
	if(!myfile.is_open()){
		cout << "Unable to open file " << filename << endl;
		exit(1);
	}
	
	vector<vec3> vertices; // vectors
	vector<vec3> faces; // faces
	int numVerts, numFaces;
	string line;
	
	getline(myfile, line); //skip first line
	getline(myfile, line);
	stringstream firstln(line);
	firstln >> numVerts >> numFaces;
	
	vertices.reserve(numVerts);
	faces.reserve(numFaces);
	
	float minx = 999999;
	float maxx = -999999;
	float miny = 999999;
	float maxy = -999999;
	float minz = 999999;
	float maxz = -999999;
	
	for (unsigned int i=0; i<numVerts; i+=1){
		float x,y,z;
		getline(myfile, line);
		stringstream ln(line);
		ln >> x >> y >> z;
		minx = min(x,minx);
		maxx = max(x,maxx);
		miny = min(y,miny);
		maxy = max(y,maxy);
		minz = min(z,minz);
		maxz = max(z,maxz);
		vec3 v(x,y,z);
		vertices.push_back(v);
	}
	
	for (unsigned int i=0; i<numFaces; i+=1){
		int v1,v2,v3,junk;
		getline(myfile, line);
		stringstream ln(line);
		ln >> junk >> v1 >> v2 >> v3;
		vec3 f(v1,v2,v3);
		faces.push_back(f);
	}
	myfile.close();
	
	/*** Center model around origin  ***/
	
	myfile.close();
	vec3 moveMiddle((maxx+minx)/2.0, (maxy+miny)/2.0, (maxz+minz)/2.0);
	float ratio = 2.0/max(max(maxx-minx, maxy-miny), maxz-minz);
	
	for (int i=0; i<numVerts; i+=1) {
		vertices[i] = (vertices[i] - moveMiddle) * ratio;
	}
	
    return new Mesh(vertices, faces);
}

/* Draw object number "obj" */
void draw(){
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}
