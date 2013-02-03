#ifndef MESH_H
#define MESH_H

#include <GLUT/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <utility>
#include <boost/unordered_map.hpp>

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

typedef glm::mat3 mat3 ;
typedef glm::mat4 mat4 ; 
typedef glm::vec3 vec3 ; 
typedef glm::vec4 vec4 ;


using namespace std;

/********* Data Structures for Winged Edges ***********/

struct vertex {
	GLfloat position[3]; 
	GLfloat normal[3]; //normal
	GLfloat padding[2];
};

struct face {
    GLuint face[3];
};

struct half_edge {
    const vertex *v;
    face *f;
    half_edge *prev, *next, *sym; //clockwise ordering
};

/********* Comprehensive Mesh definition **********/

class Mesh {
  boost::unordered_map< pair<int, int>, half_edge* > existing_edges;
  vector<half_edge*> edges;
  GLuint arrayBuffer;
  vertex* verts;
  vertex* bufferVerts;
  GLuint elementArrayBuffer;
  GLuint* bufferInds;
  int numIndices;
  public:
    /** const vector vertices (type vertex), const vector faces (type vec3) */
    Mesh(const vector<vertex>& vertices, const vector<vec3>& faces);
    bool populate_symmetric_edge(half_edge* e, int v0, int v1);
    pair<int, int> get_vertex_key(int v0, int v1);
    vector<half_edge*> get_neighboring_edges(half_edge* he);
    vector<vertex*> get_neighboring_vertices(half_edge* he);
	void draw();
};

#endif //MESH_H
