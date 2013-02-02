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

  winged_vertex we;
  for (unsigned int v=0; v < vertices.size(); v+=1) {
    we.v = &vertices[v];
    winged_vertices.push_back(we);
  }

  for (unsigned int i=0; i < faces.size(); i+=1) {
    /** forward declaration of edges */
    winged_edge we1, we2, we3;
    winged_vertex wv1, wv2, wv3;
    winged_face wf;

    /** create winged_face of current face */
    winged_faces.push_back(wf);
    add_face(i, &we1, &we2, &we3);

    /** create winged_vertices for current face */
    int wv1_index = faces[i][0];
    int wv2_index = faces[i][1];
    int wv3_index = faces[i][2];
    add_vertex(wv1_index, &we1, &we2, &we3);
    add_vertex(wv2_index, &we1, &we2, &we3);
    add_vertex(wv3_index, &we1, &we2, &we3);
    wv1 = winged_vertices[wv1_index];
    wv2 = winged_vertices[wv2_index];
    wv3 = winged_vertices[wv3_index];

    //TODO: check the math on add_edge
    /** create winged_edges for current face */
    pair<int,int> index;
    add_edge(wv1_index, wv2_index, &we1, &wf, &we2, &we3);
    add_edge(wv2_index, wv3_index, &we2, &wf, &we3, &we1);
    add_edge(wv3_index, wv1_index, &we3, &wf, &we1, &we2);
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

void
Mesh::add_vertex(int vertex_index, winged_edge* we1, winged_edge* we2, winged_edge* we3) {
  if (winged_vertices[vertex_index].edges.size() == 3) {
      winged_vertices[vertex_index].edges[0] = we1;
      winged_vertices[vertex_index].edges[1] = we2;
      winged_vertices[vertex_index].edges[2] = we3;
  } else {
      winged_vertices[vertex_index].edges.push_back(we1);
      winged_vertices[vertex_index].edges.push_back(we2);
      winged_vertices[vertex_index].edges.push_back(we3);
  }
}

/** Takes in an index into the faces vector as well as 3 pointers to the edges
 * comprising the relevant face. Constructs a winged_face and inserts it
 * appropriately into the winged_faces vector. Does not return an index like
 * add_vertex, because the index 'idx' is known at the time of calling
 */

void
Mesh::add_face(int idx, winged_edge* we1, winged_edge* we2, winged_edge* we3) {
    if (winged_faces[idx].edges.size() == 3) {
      winged_faces[idx].edges[0] = we1;
      winged_faces[idx].edges[1] = we2;
      winged_faces[idx].edges[2] = we3;
    } else {
      winged_faces[idx].edges.push_back(we1);
      winged_faces[idx].edges.push_back(we2);
      winged_faces[idx].edges.push_back(we3);
    }
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
Mesh::add_edge(int v1, int v2, winged_edge* we, winged_face* wf, winged_edge* succ, winged_edge* pred) {
    pair<int, int> edge_key = make_vertex_pair(v1, v2);
    /** check if edge is already in winged_edges */
    bool exists = winged_edges.find(edge_key) != winged_edges.end();
    if (exists) { // pre-existing winged_edges
        we = &winged_edges[edge_key];
    } else { // edge nonexistant
        we->x_vert = &winged_vertices[v1];
        we->y_vert = &winged_vertices[v2];
    }
    /** populate the struct with either its left or right face,succ,pred */
    edit_edge_wings(we, wf, succ, pred, (v1 < v2));
    /** if edge didn't exist, add it to the map */
    if (!exists)
      winged_edges[edge_key] = (*we);
    return edge_key;
}

void
Mesh::draw() {
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}

/** test the construction of the Mesh */

bool
Mesh::run_tests() {
    bool status = true;
    test_winged_vertices_populated(status);
    test_winged_faces_populated(status);
    test_winged_edges_populated(status);
    return status;
}

void
Mesh::test_winged_vertices_populated(bool& status) {
    status = winged_vertices.size() != 0 ? status : false;
    for (unsigned int i = 0; i < winged_vertices.size(); i += 1) {
        winged_vertex wv = winged_vertices[i];
        status = wv.edges.size() == 3 ? status : false;
        status = wv.v->position != NULL ? status : false;
        status = wv.v->normal != NULL ? status : false;
        status = wv.v->padding != NULL ? status : false;
    }
    if (!status)
      cout << "Mesh failed test_winged_vertices_populated test" << endl;
}

void
Mesh::test_winged_faces_populated(bool& status) {
    status = winged_faces.size() != 0 ? status : false;
    for (unsigned int i = 0; i < winged_faces.size(); i += 1) {
        winged_face wf = winged_faces[i];
        status  = wf.edges.size() == 3 ? status : false;
    }
    if (!status)
      cout << "Mesh failed test_winged_faces_populated" << endl;
}

void
Mesh::test_winged_edges_populated(bool& status) {
    status = winged_edges.size() != 0 ? status : false;
    map< pair<int,int>, winged_edge>::iterator it;
    for (it = winged_edges.begin(); it != winged_edges.end(); it++) {
        winged_edge we = it->second;
        status = we.x_vert->v->position != NULL ? status : false;
        status = we.x_vert->v->normal != NULL ? status : false;
        status = we.x_vert->v->padding != NULL ? status : false;
        //status = we.left_face->edges.size() == 3 ? status : false;
        //status = we.right_face->edges.size() == 3 ? status : false;
        status = we.left_pred != NULL ? status : false;
        status = we.right_pred != NULL ? status : false;
        status = we.left_succ != NULL ? status : false;
        status = we.right_succ != NULL ? status : false;
    }
    if (!status)
      cout << "Mesh failed test_winged_edges_populated" << endl;
}
