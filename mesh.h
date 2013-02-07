#ifndef MESH_H
#define MESH_H

#include <GLUT/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <utility>
#include <boost/unordered_map.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

typedef glm::mat3 mat3 ;
typedef glm::mat4 mat4 ; 
typedef glm::vec3 vec3 ; 
typedef glm::vec4 vec4 ;


using namespace std;

/********* Data Structures for Winged Edges ***********/

struct vertex {
	vec3 position; 
	vec3 normal;
	GLfloat padding[2];
	vertex(float,float,float);
	vertex(vertex*);
	vertex();
};

void get_midpoint(vertex* res, vertex* v1, vertex* v2);

typedef boost::shared_ptr<vertex> vertexPtr;

struct half_edge {
    vertex *v;
    half_edge *prev, *next, *sym; //anti-clockwise ordering
	
	half_edge(vertex*);
	~half_edge();
};

/********* Comprehensive Mesh definition **********/

class Mesh {
  vector<half_edge*> edges;
  GLuint arrayBuffer;
  vertex* bufferVerts;
  GLuint elementArrayBuffer;
  GLuint* bufferInds;
  unsigned int numIndices;
  public:
    boost::unordered_map< pair<int, int>, half_edge* > existing_edges;
    /** const vector vertices (type vertex), const vector faces (type vec3) **/
    Mesh(vector<vertex>& vertices, vector<vec3>& faces);
	~Mesh();
    bool populate_symmetric_edge(half_edge* e, int v0, int v1);
    pair<int, int> get_vertex_key(int v0, int v1);
    void get_neighboring_edges(vector<half_edge*> &res, half_edge* he);
    void get_neighboring_vertices(vector<vertex*> &res, half_edge* he);
    void collapse_edge(half_edge *he);
	void update_buffer();
	void draw();
};

#endif //MESH_H
