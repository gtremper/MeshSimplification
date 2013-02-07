#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "mesh.h"

using namespace std;

// COLORS: sky: (135/225.0, 206/255.0, 250/255)

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

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

Mesh* parseOFF(char* filename){
	ifstream myfile(filename, ifstream::in);
	if(!myfile.is_open()){
		cout << "Unable to open file " << filename << endl;
		exit(1);
	}
	
	vector<vertex> vertices; // vectors
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
	
	for (int i=0; i<numVerts; i+=1){
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
		vertex v = vertex(x,y,z);
		vertices.push_back(v);
	}
	
	for (int i=0; i<numFaces; i+=1){
		int vid0,vid1,vid2,junk;
		getline(myfile, line);
		stringstream ln(line);
		ln >> junk >> vid0 >> vid1 >> vid2;
		vec3 f(vid0,vid1,vid2);
		faces.push_back(f);
		vec3 v0 = vertices[vid0].position;
		vec3 v1 = vertices[vid1].position;
		vec3 v2 = vertices[vid2].position;
		vec3 norm = glm::cross(v1-v0,v2-v0);
		vertices[vid0].normal += norm;
		vertices[vid1].normal += norm;
		vertices[vid2].normal += norm;
		
		/** Calculate Quadratic error matrix **/
		norm = glm::normalize(norm);
		vec4 p = vec4(norm,-glm::dot(norm,v0));
		
		int index = 0;
		for (int y=0; y<4; y+=1){
			for (int x=y; x < 4; x+=1){
				float Qvalue = p[x]*p[y];
				vertices[vid0].Q[index] += Qvalue;
				vertices[vid1].Q[index] += Qvalue;
				vertices[vid2].Q[index] += Qvalue;
				index += 1;
			}
		}
	}
	myfile.close();
	
	/*** Center model around origin  ***/
	
	float middlex = (maxx+minx)/2.0;
	float middley = (maxy+miny)/2.0;
	float middlez = (maxz+minz)/2.0;
	vec3 makeMiddle(middlex,middley,middlez);
	float ratio = 8.0/max(max(maxx-minx, maxy-miny), maxz-minz);
	
	for (int i=0; i<numVerts; i+=1) {
		vertices[i].position -= makeMiddle;
		vertices[i].position *= ratio;
		
		vertices[i].normal = glm::normalize(vertices[i].normal);
	}
	
    return new Mesh(vertices, faces);
}

