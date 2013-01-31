#ifndef MESH_H
#define MESH_H

#include <GLUT/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <utility>
#include <map>

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

typedef glm::mat3 mat3 ;
typedef glm::mat4 mat4 ; 
typedef glm::vec3 vec3 ; 
typedef glm::vec4 vec4 ;

struct vertex {
	GLfloat position[3]; 
	GLfloat normal[3]; //normal
	GLfloat padding[2];
};

using namespace std;

/********* Data Structures for Winged Edges ***********/

/** forward declaration for winged_vertex and winged_edge */
struct winged_edge;

struct winged_vertex {
    vector<winged_edge*> edges;
    /* reference to vertex struct above -- avoid copying information! */
    const vertex *v;
};

bool operator==(winged_vertex const& wv1, winged_vertex const& wv2);

struct winged_face {
    vector<winged_edge*> edges;
    /* reference to the indices of the included vertices */
    const vec3 *f;
};

struct winged_edge {
    winged_vertex *x_vert, *y_vert;
    winged_face *left_face, *right_face;
    winged_edge *left_pred, *left_succ, *right_pred, *right_succ; //clockwise ordering
};

/********* Comprehensive Mesh definition **********/

class Mesh {
  const vector<vertex>* vertex_reference;
  const vector<vec3>* face_reference;
  map< pair<int, int>, winged_edge > winged_edges;
  vector<winged_vertex> winged_vertices;
  vector<winged_face> winged_faces;
  GLuint arrayBuffer;
  vertex* bufferVerts;
  GLuint elementArrayBuffer;
  GLuint* bufferInds;
  int numIndices;
  public:
    /** const vector vertices (type vertex), const vector faces (type vec3) */
    Mesh(const vector<vertex>& vertices, const vector<vec3>& faces);
    int add_vertex(int face_index, int vertex_index);
    void add_face(int idx, winged_edge* we1, winged_edge* we2, winged_edge* we3);
    pair<int,int> add_edge(int v1, int v2, winged_face* wf, winged_edge* succ, winged_edge* pred);
    void edit_edge_wings(winged_edge* we, winged_face* wf, winged_edge* succ, winged_edge* pred, bool left);
    pair<int, int> make_vertex_pair(int v1, int v2);
    bool run_tests();
    void test_winged_vertices_populated(bool&);
    vertex* to_vertex_list();
	void draw();
};

#endif //MESH_H
