#include "mesh.h"
using namespace std;

Mesh::Mesh(vector<vertex*>& vertices, vector<vec3>& faces) {

  unsigned int numFaces = faces.size();
  numIndices = numFaces*3;
  
  existing_edges = boost::unordered_map< pair<int, int>, half_edge* >();

  for (unsigned int i=0; i < numFaces; i+=1) {
    half_edge* e0 = new half_edge();
    half_edge* e1 = new half_edge();
    half_edge* e2 = new half_edge();

    /** populate next/prev edges for each edge in this face */
    e0->prev = e2;
    e0->next = e1;
    e1->prev = e0;
    e1->next = e2;
    e2->prev = e1;
    e2->next = e0;

    /** populate end-vertex for each edge */
	unsigned int v0 = faces[i][0];
	unsigned int v1 = faces[i][1];
	unsigned int v2 = faces[i][2];
	
	e0->v = vertices[v0];
	e1->v = vertices[v1];
	e2->v = vertices[v2];
	
    /** populate the symmetric edge for the given edge */
	populate_symmetric_edge(e0, v0, v1);
	populate_symmetric_edge(e1, v1, v2);
	populate_symmetric_edge(e2, v2, v0);

    /** add edges to vector */
    edges.push_back(e0);
    edges.push_back(e1);
    edges.push_back(e2);
  }

  glGenBuffers(1, &arrayBuffer);
  glGenBuffers(1, &elementArrayBuffer);
  update_buffer();
}

Mesh::~Mesh(){
	for (int i=0; i<edges.size(); i+=1) {
		delete edges[i];
	}
}

half_edge::~half_edge(){
	delete v;
}

vertex::vertex(float x, float y, float z) {
	position[0] = x;
	position[1] = y;
	position[2] = z;
	normal[0] = 0.0;
	normal[1] = 0.0;
	normal[2] = 0.0;
}

vertex::vertex(vertex* v) {
	position[0] = v->position[0];
	position[1] = v->position[1];
	position[2] = v->position[2];
	normal[0] = v->normal[0];
	normal[1] = v->normal[1];
	normal[2] = v->normal[2];
}

vertex::vertex(){};

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
    e->sym->sym = e;
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

/** given a half_edge pointer he, return a vector of pointers
 * to all the neighboring edges */
void
Mesh::get_neighboring_edges(vector<half_edge*> &res, half_edge* he) {
  half_edge* loop = he;
  /** add one vertex of the half_edge */
  do {
    res.push_back(loop);
    loop = loop->next->sym;
  } while (loop != he);
  /** now add the other */
  loop = he->sym;
  half_edge* other = loop;
  do {
    res.push_back(loop);
    loop = loop->next->sym;
  } while (loop != he && loop != other);
}

void
Mesh::get_neighboring_vertices(vector<vertex*> &res, half_edge* he) {
    half_edge* loop = he;
    do {
      res.push_back((vertex*)loop->v);
      loop = loop->next->sym;
    } while (loop != he);
}

/** Collapses half_edge* [he] and sets the surrounding edges to point to
 * a new vertex v_m that is the midpoint of [he]'s two defining vertices.
 * I know I'm probably forgetting to set some edges, but here are my
 * initial thoughts:
 */

void
Mesh::collapse_edge(half_edge* he) {
	//TODO: write method to find midpoint, adjust normals, etc
	vertex* midpoint = new vertex();
	/** set vertices of edge to be the midpoint */
	he->v = midpoint;
	he->sym->v = midpoint;

	//TODO: check what edges actually need to be collapsed
	/** combine collapsed edges */
	he->next = he->prev->sym;
	he->next->sym = he->prev;
	he->sym->next = he->sym->prev->sym;
	he->sym->next->sym = he->sym->prev;
}

void
Mesh::update_buffer() {
	vector<GLuint> elements;
	vector<vertex> verts;
	
	boost::unordered_map<vertex*, GLuint> vertMap = boost::unordered_map<vertex*, GLuint>();
	
	GLuint counter = 0;
	for (int i = 0; i<edges.size(); i+=1) {
		boost::unordered_map<vertex*, GLuint>::iterator 
			it = vertMap.find(edges[i]->v);
		if (it != vertMap.end() ) { 
			elements.push_back(it->second);
		} else {
			vertMap[edges[i]->v] = counter;
			verts.push_back(edges[i]->v);
			elements.push_back(counter);
			counter += 1;
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*verts.size(), &verts[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*elements.size(), &elements[0], GL_STATIC_DRAW);
}

void
Mesh::draw() {
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}
