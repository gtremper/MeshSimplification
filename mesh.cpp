#include "mesh.h"

using namespace std;

/********* Data Structures for Winged Edges ***********/

/** forward declaration for winged_vertex and winged_edge */
struct winged_edge;

struct winged_vertex {
    vector<winged_edge*> edges;
    /* reference to vertex struct above -- avoid copying information! */
    const vertex *v;
};

bool operator==(winged_vertex const& wv1, winged_vertex const& wv2) {
    return wv1.v == wv1.v;
}

struct winged_face {
    vector<winged_edge*> edges;
    /* reference to the indices of the included vertices */
    const vec3 *f;
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
    void add_face(int idx, winged_edge* we1, winged_edge* we2, winged_edge* we3);
    vertex* to_vertex_list();
};

/** We take in two vectors, one representing the vertices of the mesh, the
 * other representing the faces of the mesh (in terms of the vertex indices).
 * For each of the winged faces and vertices, we want to keep track of which
 * edges they are associated with. For each winged edge, we want to track what
 * is component faces and vertices are as well as its neighboring edges.
 *
 * Proper construction of the winged_edges is as follows: Iterate through all
 * of the faces. For each face f, create the relevant winged_vertices,
 * winged_edges, and winged_face and add them to the appropriate lists. For each
 * edge created by this face, we set the edge vertices, the appropriate edge face,
 * and appropriate edge successor/predecessors for the edges connected to the
 * current one.
 */
Mesh::Mesh (const vector<vertex>& vertices, const vector<vec3>& faces) {

  vertex_reference = &vertices;
  face_reference = &faces;

  for (unsigned int i=0; i < faces.size(); i+=1) {
    /** forward declaration of edges */
    winged_edge we1, we2, we3;

    /** create winged_face of current face */
    add_face(i, &we1, &we2, &we3);
    winged_face wf = winged_faces[i];

    /** create winged_vertices for current face */
    winged_vertex wv1 = winged_vertices[add_vertex(i,0)];
    winged_vertex wv2 = winged_vertices[add_vertex(i,1)];
    winged_vertex wv3 = winged_vertices[add_vertex(i,2)];

    /** create winged_edges for current face */
    //we1.x_vert = &wv1;
    //we1.y_vert = &wv3;
    //we1.b_face = &wf; // or a_face? need method to figure out
    //we1.b_succ = &we2;
    //we1.b_pred = &we3;

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

void
Mesh::add_face(int idx, winged_edge* we1, winged_edge* we2, winged_edge* we3) {
    winged_face wf;
    wf.edges.push_back(we1);
    wf.edges.push_back(we2);
    wf.edges.push_back(we3);
    winged_faces[idx] = wf;
}
