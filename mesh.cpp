#include "mesh.h"
using namespace std;

Mesh::Mesh (const vector<vertex>& vertices, const vector<vec3>& faces) {

  int numIndices = vertices.size();
  int numFaces = faces.size();

  for (unsigned int i=0; i < faces.size(); i+=1) {
    half_edge* e0 = new half_edge;
    half_edge* e1 = new half_edge;
    half_edge* e2 = new half_edge;

    e0->prev = e2;
    e0->next = e1;
    e1->prev = e0;
    e1->next = e2;
    e2->prev = e1;
    e2->next = e0;

    vec3 current_face = faces[i];

    populate_symmetric_edge(e0, current_face[0], current_face[1]);
    populate_symmetric_edge(e1, current_face[1], current_face[2]);
    populate_symmetric_edge(e2, current_face[2], current_face[0]);

    edges.push_back(e0);
    edges.push_back(e1);
    edges.push_back(e2);
  }

}

/** Given a half_edge [e] and its two defining vertices [v0,v1], we check if
 * the pair [v0,v1] already exists in the existing_edges map, where v0 < v1.
 * If it doesn't exist, we set the value in the map to be our current edge*,
 * else we set [e]'s symmetric edge to be the value in the map and the map's
 * symmetric edge to be [e]. 
 * => Returns true if the value was found in the map.
 */
bool
Mesh::populate_symmetric_edge(half_edge* e, int v0, int v1) {
  pair<int, int> key = get_vertex_key(v0, v1);
  boost::unordered_map< pair<int, int>, half_edge* >::iterator it = 
        existing_edges.find(key);
  bool res = true;
  if (it != existing_edges.end() ) { // exists
    e->sym = it->second;
    it->second = e;
  } else {
    existing_edges[key] = e;
    res = false;
  }
  return res;
}

pair<int, int>
Mesh::get_vertex_key(int v0, int v1) {
   if (v0 < v1) 
     return pair<int, int> (v0, v1);
   else
     return pair<int, int> (v1, v0);
}

void
Mesh::draw() {
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}
