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

void parseConfig(char* filename){
	ifstream file(filename, ifstream::in);
	if(!file.is_open()){
		cout << "Unable to open file " << filename << endl;
		exit(1);
	}
	
}

void parseOFF(char* filename){
	
	ifstream myfile(filename, ifstream::in);
	if(!myfile.is_open()){
		cout << "Unable to open file " << filename << endl;
		exit(1);
	}
	
	vector<vec3> verticies; // vectors
	vector<vec3> faces; // faces
	int numVerts, numFaces;
	string line;
	
	
	getline(myfile, line); //skip first line
	getline(myfile, line);
	stringstream firstln(line);
	firstln >> numVerts >> numFaces;
	
	vertex verts[numVerts];
	for (int i=0; i<numVerts; i+=1){
		float x,y,z;
		getline(myfile, line);
		stringstream ln(line);
		ln >> x >> y >> z;
		vec3 v(x,y,z);
		verticies.push_back(v);
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
	
	GLuint inds[numFaces*3];
	size = numFaces*3;
	for(int i=0; i<numFaces; i+=1){
		int ind0 = faces[i][0];
		int ind1 = faces[i][1];
		int ind2 = faces[i][2];
		inds[3*i] = ind0;
		inds[3*i+1] = ind1;
		inds[3*i+2] = ind2;
		
		vec3 v0 = verticies[ind0];
		vec3 v1 = verticies[ind1];
		vec3 v2 = verticies[ind2];
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
    vector<vertex> tmp_verts = vector<vertex>(verts, verts+numVerts);
    Mesh m = Mesh( tmp_verts, faces );
	
	glGenBuffers(1, &meshArrayBuffer);
	glGenBuffers(1, &meshElementArrayBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshArrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshElementArrayBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), inds, GL_STATIC_DRAW);
}

/* Draw object number "obj" */
void draw(){
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}
