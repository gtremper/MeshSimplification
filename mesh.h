#ifndef MESH_H
#define MESH_H

#include <vector>
#include <GLUT/glut.h>
#include "globals.h"

using namespace std;

struct vertex {
	GLfloat x,y,z; 
	GLfloat nx,ny,nz; //normal
	GLfloat faces;
	GLfloat padding;
};

/********* Data Structures for Winged Edges ***********/

/** forward declaration for winged_vertex and winged_edge */
struct winged_edge;

struct winged_vertex {
    vector<winged_edge> edges;
    /* reference to vertext struct above -- avoid copying information! */
    vertex *v;
};

struct winged_face {
    vector<winged_edge> edges;
};

struct winged_edge {
    winged_vertex *x_vert, *y_vert;
    winged_face *a_face, *b_face;
    winged_edge *a_pred, *a_succ, *b_pred, *b_succ; //clockwise ordering
};

/********* Comprehensive Mesh definition **********/

class Mesh {
  const vector<vertex>& vertex_reference;
  const vector<vec3>& face_reference;
  vector<winged_edge> winged_edges;
  vector<winged_vertex> winged_vertices;
  vector<winged_face> winged_faces;
  public:
    /** const vector vertices (type vertex), const vector faces (type vec3) */
    Mesh(const vector<vertex>& vertices, const vector<vec3>& faces);
};

#endif
