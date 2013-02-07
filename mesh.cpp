#include "mesh.h"
using namespace std;

void
get_midpoint(vertex* res, vertex* v1, vertex* v2) {
  res->position = (v1->position + v2->position) / 2.0f;
  res->normal = (v1->normal + v2->normal) / 2.0f;
}


Mesh::Mesh(vector<vertex>& vertices, vector<vec3>& faces) {

  unsigned int numFaces = faces.size();
  numIndices = numFaces*3;
  
  existing_edges = boost::unordered_map< pair<int, int>, half_edge* >();
  boost::unordered_map<int, vertex*>
	existing_vertices = boost::unordered_map<int, vertex* >();

  for (unsigned int i=0; i < numFaces; i+=1) {
	
	int v0 = faces[i][0];
	int v1 = faces[i][1];
	int v2 = faces[i][2];
	
	/** Create new vertex and edge objects if they don't already exist **/
	vertex *vert0, *vert1, *vert2;
	boost::unordered_map<int, vertex*>::iterator vertIt; 
	
	vertIt = existing_vertices.find(v0);
	if (vertIt == existing_vertices.end()) {
		vert0 = new vertex(&vertices[v0]);
	} else {
		vert0 = vertIt->second;
	}
	
	vertIt = existing_vertices.find(v1);
	if (vertIt == existing_vertices.end()) {
		vert1 = new vertex(&vertices[v1]);
	} else {
		vert1 = vertIt->second;
	}
	
	vertIt = existing_vertices.find(v2);
	if (vertIt == existing_vertices.end()) {
		vert2 = new vertex(&vertices[v2]);
	} else {
		vert2 = vertIt->second;
	}
	
	half_edge* e0 = new half_edge(vert0);
	half_edge* e1 = new half_edge(vert1);
	half_edge* e2 = new half_edge(vert2);

	/** populate next/prev edges for each edge in this face */
	e0->prev = e2;
	e0->next = e1;
	e1->prev = e0;
	e1->next = e2;
	e2->prev = e1;
	e2->next = e0;
	
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

half_edge::half_edge(vertex* vert){
	v = vert;
}

half_edge::~half_edge(){}

vertex::vertex(float x, float y, float z) {
	position = vec3(x,y,z);
	normal = vec3(0.0f,0.0f,0.0f);
}

vertex::vertex(vertex* v) {
	memcpy(&position, v, sizeof(vertex));
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
	half_edge* loop = he->prev->sym;
	/** add one vertex of the half_edge */
	do {
		if (loop == NULL)
			break;
		res.push_back(loop);
		res.push_back(loop->sym);
		loop = loop->prev->sym;
	} while (loop != he);
	/** now add the other */
	loop = he->sym->prev->sym;
	do {
		if (loop == NULL)
			break;
		res.push_back(loop);
		res.push_back(loop->sym);
		loop = loop->prev->sym;
	} while (loop != he->sym);
	
	he->next->sym->sym = he->prev->sym;
	he->prev->sym->sym = he->next->sym;
	he = he->sym;
	he->next->sym->sym = he->prev->sym;
	he->prev->sym->sym = he->next->sym;
}

void
Mesh::get_neighboring_vertices(vector<vertex*> &res, half_edge* he) {
	half_edge* loop = he;
	cout << "NEIGHBOR TEST" <<endl;
	do {
		res.push_back(loop->v);
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
	he = edges[rand() % edges.size()];
	
	
	vertex* midpoint = new vertex();
	get_midpoint(midpoint, he->v, he->sym->v);
	
	vector<half_edge*> neighbors;
	get_neighboring_edges(neighbors, he);
	for (unsigned int i = 0; i < neighbors.size(); i++) {
		if (neighbors[i]->v == he->v || neighbors[i]->v == he->sym->v) {
			neighbors[i]->v = midpoint;
		}
	}
	
	/* Remove edges from faces that disapear */
	edges.erase(remove(edges.begin(), edges.end(), he->prev), edges.end());
	edges.erase(remove(edges.begin(), edges.end(), he->next), edges.end());
	edges.erase(remove(edges.begin(), edges.end(), he), edges.end());
	edges.erase(remove(edges.begin(), edges.end(), he->sym->prev), edges.end());
	edges.erase(remove(edges.begin(), edges.end(), he->sym->next), edges.end());
	edges.erase(remove(edges.begin(), edges.end(), he->sym), edges.end());
	
	delete he->v;
	delete he->sym->v;
	
	delete he->sym->prev;
	delete he->sym->next;
	delete he->sym;
	delete he->prev;
	delete he->next;
	delete he;
	
	numIndices = edges.size();
}

void
Mesh::update_buffer() {
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
	
	vector<GLuint> elements;
	vector<vertex> verts;
	
	int gtime = glutGet(GLUT_ELAPSED_TIME);
	
	boost::unordered_map<vertex*, GLuint> vertMap = boost::unordered_map<vertex*, GLuint>();
	
	cout << "size: " << edges.size() << endl;
	
	GLuint counter = 0;
	for (int i=0; i<edges.size(); i+=1) {
		boost::unordered_map<vertex*, GLuint>::iterator
			mapit = vertMap.find(edges[i]->v);
		if (mapit != vertMap.end() ) { 
			elements.push_back(mapit->second);
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
	
	cout <<"Time to update buffer: " << glutGet(GLUT_ELAPSED_TIME)-gtime << endl;
}

void
Mesh::draw() {
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}
