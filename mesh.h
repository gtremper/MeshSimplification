#ifndef MESH_H
#define MESH_H

#include <vector>
#include <GLUT/glut.h>
#include "globals.h"

using namespace std;

struct vertex {
	GLfloat position[3]; 
	GLfloat normal[3]; //normal
	GLfloat padding[2];
};

/********* Data Structures for Winged Edges ***********/

/** forward declaration for winged_vertex and winged_edge */
struct winged_edge;

struct winged_vertex {
    vector<winged_edge> edges;
    /* reference to vertext struct above -- avoid copying information! */
    const vertex *v;
};

bool operator==(winged_vertex const& wv1, winged_vertex const& wv2) {
    return wv1.v == wv1.v;
}

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
  const vector<vertex>* vertex_reference;
  const vector<vec3>* face_reference;
  vector<winged_edge> winged_edges;
  vector<winged_vertex> winged_vertices;
  vector<winged_face> winged_faces;
  public:
    /** const vector vertices (type vertex), const vector faces (type vec3) */
    Mesh(const vector<vertex>& vertices, const vector<vec3>& faces);
    int add_vertex(int face_index, int vertex_index);
    vertex* to_vertex_list();
};

/**
 * We take in two vectors, one representing the vertices of the mesh, the other
 * representing the faces of the mesh (in terms of the vertex indices).
 */
Mesh::Mesh (const vector<vertex>& vertices, const vector<vec3>& faces) {

  for (unsigned int i=0; i < faces.size(); i++) {

  }

}

/** Takes in an index into the faces vector and an index into that face's
 * vector of vertices and constructs a winged_vertex that is inserted into the
 * vector of wigned_vertices at the appropriate index. This ensures that the
 * indices of the original vertices are the same as the winged_vertices.
 * => Returns an index into winged_vertices
 *
 * To create the winged_vertex for the 1st vertex in the 10th face, use
 * : winged_vertex x = winged_vertices[add_vertex(10,0)];
 */
int
Mesh::add_vertex(int face_index, int vertex_index) {
  winged_vertex wv;
  int idx = (*face_reference)[face_index][vertex_index];
  winged_vertices[idx] = wv;
  wv.v = &(*vertex_reference)[idx];
  return idx;
}

#endif
