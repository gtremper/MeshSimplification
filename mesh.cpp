#include "mesh.h"
using namespace std;

/** Custom operators for the winged_* structs */
bool operator==(winged_vertex const& wv1, winged_vertex const& wv2) {
    return wv1.v == wv1.v;
}

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
 *
 * For reference:
 *       wv3
 *        *
 *       /  \
 *      /    \
 * we3 /      \ we2
 *    /   wf   \
 *   /          \
 *  *------------*
 * wv1    we1     wv2
 */

Mesh::Mesh (const vector<vertex>& vertices, const vector<vec3>& faces) {

  vertex_reference = &vertices;
  face_reference = &faces;
  int numIndices = vertices.size();
  int numFaces = faces.size();
  winged_faces.reserve( numFaces );
  winged_vertices.reserve( numIndices );

  for (unsigned int i=0; i < faces.size(); i+=1) {
    /** forward declaration of edges */
    winged_edge we1, we2, we3;

    /** create winged_face of current face */
    add_face(i, &we1, &we2, &we3);
    winged_face wf = winged_faces[i];

    /** create winged_vertices for current face */
    int wv1_index = add_vertex(i,0);
    int wv2_index = add_vertex(i,1);
    int wv3_index = add_vertex(i,2);
    winged_vertex wv1 = winged_vertices[wv1_index];
    winged_vertex wv2 = winged_vertices[wv2_index];
    winged_vertex wv3 = winged_vertices[wv3_index];
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

/** Takes in an index into the faces vector as well as 3 pointers to the edges
 * comprising the relevant face. Constructs a winged_face and inserts it
 * appropriately into the winged_faces vector. Does not return an index like
 * add_vertex, because the index 'idx' is known at the time of calling
 */

void
Mesh::add_face(int idx, winged_edge* we1, winged_edge* we2, winged_edge* we3) {
    winged_face wf;
    wf.edges.push_back(we1);
    wf.edges.push_back(we2);
    wf.edges.push_back(we3);
    winged_faces[idx] = wf;
}

/** given two vertex indices, returns a pair (a,b),
 * where a < b and and a,b are both ints
 */

pair<int, int>
Mesh::make_vertex_pair(int v1, int v2) {
    return (v1 < v2) ? pair<int, int>(v1, v2) : pair<int, int>(v2, v1);
}

/** Because we can determine by the ordering of the vertices which side of an edge
 * our current face is, we pass in the boolean left = vertex_a < vertex_b and
 * populate the appropriate elements of the winged_edge */

void
Mesh::edit_edge_wings(winged_edge* we, winged_face* wf, winged_edge* succ, winged_edge* pred, bool left) {
    if (left) {
        we->left_face = wf;
        we->left_succ = succ;
        we->left_pred = pred;
    } else {
        we->right_face = wf;
        we->right_succ = succ;
        we->right_pred = pred;
    }
}

/** Given two vertex indices v1, v2 as well as a winged_face and two winged_edges,
 * constructs a winged_edge with the appropriate fields populated OR edits a
 * previously created winged_edge to include the other face/edges.
 * Returns the edge_key pair for the edge
 */

pair<int,int>
Mesh::add_edge(int v1, int v2, winged_face* wf, winged_edge* succ, winged_edge* pred) {
    winged_edge we;
    pair<int, int> edge_key = make_vertex_pair(v1, v2);
    /** check if edge is already in winged_edges */
    bool exists = winged_edges.find(edge_key) != winged_edges.end();
    if (exists) { // not in winged_edges
        we.x_vert = &winged_vertices[v1];
        we.y_vert = &winged_vertices[v2];
    } else { // pre-existing edge
        we = winged_edges[edge_key];
    }
    /** populate the struct with either its left or right face,succ,pred */
    edit_edge_wings(&we, wf, succ, pred, (v1 < v2));
    /** if edge didn't exist, add it to the map */
    if (!exists)
      winged_edges[edge_key] = we;
    return edge_key;
}

void
Mesh::draw() {
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}











//hello Gabe
