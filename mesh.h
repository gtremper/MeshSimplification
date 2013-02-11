#ifndef MESH_H
#define MESH_H

#include <GLUT/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <utility>
#include <list>
#include <boost/unordered_map.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/heap/priority_queue.hpp>

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

typedef glm::mat3 mat3 ;
typedef glm::mat4 mat4 ; 
typedef glm::vec3 vec3 ; 
typedef glm::vec4 vec4 ;


using namespace std;

/********* Data Structures for Winged Edges ***********/

struct vertex_data {
	vec3 position; 
	vec3 normal;
	GLfloat padding[2];
};

struct vertex {
	vec3 position; 
	vec3 normal;
	GLfloat Q[10];
	vertex(float,float,float);
	vertex(vertex*);
	vertex();
	vertex_data data();
};

void get_midpoint(vertex* res, vertex* v1, vertex* v2);

typedef boost::shared_ptr<vertex> vertexPtr;

struct half_edge;

struct edge_data {
	vec3 merge_point;
	float merge_cost;
	half_edge *edge; //the half edges this data represents
	void calculate_quad_error(vector<vertex>& verts);
};

struct half_edge {
	int v;
    half_edge *prev, *next, *sym; //anti-clockwise ordering
	edge_data* data;
	int index;
	half_edge();
	~half_edge();
};

struct edge_collapse {
	vector<half_edge*> removed;
	vector<half_edge*> fromV1;
	vector<half_edge*> fromV2;
	int V1;
	int V2;
	int collapseVert;
};

struct edge_compare {
    bool operator() (const edge_data* he1, const edge_data* he2) const;
};

/** use .push(item) to add to the heap, use .top() to get the top of the heap,
 * and use .pop() to get rid of the top item of the heap */
typedef boost::heap::priority_queue<edge_data*,
        boost::heap::compare<edge_compare> > Priority_Queue;
/********* Comprehensive Mesh definition **********/

class Mesh {
  GLuint arrayBuffer;
  GLuint elementArrayBuffer;
  unsigned int numIndices;
  vector<edge_collapse> collapse_list;
  int level_of_detail;
  public:
	vector<half_edge*> edges;
	vector<vertex> verts;
    boost::unordered_map< pair<int, int>, half_edge* > existing_edges;
    Priority_Queue pq;
    Mesh(vector<vertex>& vertices, vector<vec3>& faces);
	~Mesh();
    bool populate_symmetric_edge(half_edge*, int, int);
    pair<int, int> get_vertex_key(int,int);
    void get_neighboring_edges(vector<half_edge*>&, half_edge*);
    void get_neighboring_vertices(vector<vertex*>&, half_edge*);
    void collapse_edge();
	void update_buffer();
	void draw();
	void upLevelOfDetail();
	void downLevelOfDetail();
};

#endif //MESH_H
