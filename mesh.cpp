#include "mesh.h"
using namespace std;

const float THRESHOLD = 100;

Mesh::Mesh(vector<vertex>& vertices, vector<vec3>& faces) {

	unsigned int numFaces = faces.size();
	numIndices = numFaces*3;
	max_lod = -1;
  
	existing_edges = boost::unordered_map< pair<int, int>, half_edge* >();
    pq_contains = boost::unordered_map< edge_data*, bool >();
	level_of_detail = 0;
	
	for (int i=0; i<vertices.size(); i+=1){
		verts.push_back(vertices[i]);
	}
	
	vector<edge_data*> edatas;

  for (unsigned int i=0; i < numFaces; i+=1) {
	int v0 = faces[i][0];
	int v1 = faces[i][1];
	int v2 = faces[i][2];
	
	half_edge* e0 = new half_edge();
	half_edge* e1 = new half_edge();
	half_edge* e2 = new half_edge();
	
	e0->v = v0;
	e1->v = v1;
	e2->v = v2;

	/** populate next/prev edges for each edge in this face */
	e0->prev = e2;
	e0->next = e1;
	e1->prev = e0;
	e1->next = e2;
	e2->prev = e1;
	e2->next = e0;
	
	/** populate the symmetric edge for the given edge */
	populate_symmetric_edge(e0, v0, v1, edatas);
	populate_symmetric_edge(e1, v1, v2, edatas);
	populate_symmetric_edge(e2, v2, v0, edatas);

	/** add edges to vector */
	e0->index = edges.size();
	edges.push_back(e0);
	e1->index = edges.size();
	edges.push_back(e1);
	e2->index = edges.size();
	edges.push_back(e2);
  }
	
	for(int i=0; i<edatas.size(); i+=1){
		edge_data* d = edatas[i];
		d->calculate_quad_error(verts);
		d->pq_handle = pq.push(d);
        //pq_contains[d] = true;
	}

  glGenBuffers(1, &arrayBuffer);
  glGenBuffers(1, &elementArrayBuffer);
  update_buffer();
}

Mesh::~Mesh(){
	vector<half_edge*>::iterator it;
	for (it=edges.begin(); it!=edges.end(); ++it) {
		delete *it;
	}
	while(!pq.empty()){
		delete pq.top();
		pq.pop();
	}
}

void
Mesh::print_half_edge(half_edge* he) {
  cout << "******************" << endl;
  cout << "Data for half_edge" << endl;
  cout << "Position: \nsrc vertex: " << he->v << endl;
  cout << verts[he->v].position[0];
  cout << ", " << verts[he->v].position[1];
  cout << ", " << verts[he->v].position[2] << endl;
  cout << "Next edge: " << he->next->v << endl;
  cout << verts[he->next->v].position[0];
  cout << ", " << verts[he->next->v].position[1];
  cout << ", " << verts[he->next->v].position[2] << endl;
  cout << "Prev edge: " << he->prev->v << endl;
  cout << verts[he->prev->v].position[0];
  cout << ", " << verts[he->prev->v].position[1];
  cout << ", " << verts[he->prev->v].position[2] << endl;
}

/** Half Edge functions **/

void
edge_data::calculate_quad_error(vector<vertex>& verts) {
	float Q1[10];
	float Q2[10];
	
	memcpy(Q1, verts[edge->v].Q, sizeof(Q1));
	memcpy(Q2, verts[edge->next->v].Q, sizeof(Q2));
	
	for (int i=0; i<10; i+=1) {
		Q1[i] += Q2[i];
	}
	/** Hard coded solution to minimum quadric error **/
	float a = Q1[0];
	float b = Q1[1];
	float c = Q1[2];
	float d = Q1[3];
	float e = Q1[4];
	float f = Q1[5];
	float g = Q1[6];
	float h = Q1[7];
	float i = Q1[8];
	
	float det = a*e*h - a*f*f - b*b*h + 2*b*c*f - c*c*e;
	
	/* Check if one of the ends of the edge is on the edge of the mesh */
	half_edge* curEdge = edge;
	bool firstEdge = false;
	do {
		if (!curEdge->prev->sym){
			firstEdge = true;
			break;
		}
		curEdge = curEdge->prev->sym;
	} while(curEdge != edge);
	
	bool secondEdge = false;
	if(edge->sym){
		curEdge = edge->sym;
		do {
			if (!curEdge->prev->sym){
				secondEdge = true;
				break;
			}
			curEdge = curEdge->prev->sym;
		} while(curEdge != edge->sym);
	}
	
	float x,y,z;
	if (!edge->sym) {
		merge_point = (verts[edge->v].position + verts[edge->next->v].position)/2.0f;
		x = merge_point[0];
		y = merge_point[1];
		z = merge_point[2];
	} else if (firstEdge) {
		merge_point = verts[edge->v].position;
		x = merge_point[0];
		y = merge_point[1];
		z = merge_point[2];
	} else if (secondEdge) {
		merge_point = verts[edge->next->v].position;
		x = merge_point[0];
		y = merge_point[1];
		z = merge_point[2];
	} else if (det<0.01) {
		merge_point = (verts[edge->v].position + verts[edge->next->v].position)/2.0f;
		x = merge_point[0];
		y = merge_point[1];
		z = merge_point[2];
	} else {
		x = d*f*f - c*g*f - b*i*f - d*e*h + b*g*h + c*e*i;
		x /= det;
		y = g*c*c - d*f*c - b*i*c + b*d*h - a*g*h + a*f*i;
		y /= det;
		z = i*b*b - d*f*b - c*g*b + c*d*e + a*f*g - a*e*i;
		z /= det;
		merge_point = vec3(x,y,z);
	}
	merge_cost = a*x*x + 2*b*x*y + 2*c*x*z + 2*d*x + e*y*y
						+ 2*f*y*z + 2*g*y + h*z*z + 2*i*z + Q1[9];
	merge_cost = abs(merge_cost);
	merge_cost += (rand()%1000)/100000000.0f;
}

bool
edge_compare::operator() (const edge_data* he1, const edge_data* he2) const
{
	return he1->merge_cost > he2->merge_cost;
}

bool
operator==(const glm::vec3 &vecA, const glm::vec3 &vecB){ 
	const double epsilion = 0.0001;
	return fabs(vecA[0] -vecB[0]) < epsilion   
			&& fabs(vecA[1] -vecB[1]) < epsilion   
			&& fabs(vecA[2] -vecB[2]) < epsilion;
}

/** Vertex functions **/

vertex::vertex(){};

vertex::vertex(float x, float y, float z) {
	position = vec3(x,y,z);
	normal = vec3(0.0f,0.0f,0.0f);
	for (int i=0; i<10; i+=1){
		Q[i] = 0;
	}
}

vertex::vertex(vertex* v) {
	memcpy(&position, v, sizeof(vertex));
}

vertex_data vertex::data() {
	vertex_data vdata;
	memcpy(&vdata, &position, sizeof(vertex_data));
	return vdata;
}


/** Given a half_edge [e] and its two defining vertices [v0,v1], we check if
 * the pair [v0,v1] already exists in the existing_edges map, where v0 < v1.
 * If it doesn't exist, we set the value in the map to be our current edge*,
 * else we set [e]'s symmetric edge to be the value in the map and the map's
 * symmetric edge to be [e]. Also puts edge in priority queue and calculates
 * the removal priority
 * => Returns true if the value was found in the map.
 */
void
Mesh::populate_symmetric_edge(half_edge* e, int v0, int v1, vector<edge_data*>& edatas) {
	pair<int, int> key = get_vertex_key(v0, v1);
	boost::unordered_map< pair<int, int>, half_edge* >::iterator it = 
		existing_edges.find(key);
	if (it != existing_edges.end() ) { // exists
		e->sym = it->second;
		e->sym->sym = e;
		e->data = e->sym->data;
	} else {
		existing_edges[key] = e;
		e->sym = NULL; 
		edge_data* d = new edge_data();
		d->edge = e;
		e->data = d;
		edatas.push_back(d);
	}
}

pair<int, int>
Mesh::get_vertex_key(int v0, int v1) {
   if (v0 < v1) 
	 return pair<int, int> (v0, v1);
   else
	 return pair<int, int> (v1, v0);
}

void
Mesh::get_src_edges(vector<half_edge*> &res, half_edge* he) {
	if (he->sym == NULL && he->prev->sym == NULL) return;
	half_edge* loop;
	if (he->sym == NULL) {
		loop = he->prev->sym;
		while (loop && loop != he->prev) {
			res.push_back(loop);
			loop = loop->prev->sym;
		}
	} else if (he->prev->sym == NULL) {
		loop = he->sym;
		while (loop) {
			res.push_back(loop->next);
			loop = loop->next->sym;
		}
	} else {
		loop = he->prev->sym;
		while (loop != he && loop) {
			res.push_back(loop);
			loop = loop->prev->sym;
		}
        if (loop) return; //else...
        loop = he->sym;
        while (loop && loop != he) {
            res.push_back(loop->next);
            loop = loop->next->sym;
        }
	}
}

void
Mesh::get_dst_edges(vector<half_edge*> &res, half_edge* he) {
	if (he->sym == NULL && he->next->sym == NULL) return;
	half_edge* loop;
	if (he->sym == NULL) {
		loop = he->next->sym;
		while (loop && loop != he) {
			res.push_back(loop->next);
			loop = loop->next->sym;
		}
	} else if (he->next->sym == NULL) {
		loop = he->sym;
		while (loop && loop != he->next) {
			res.push_back(loop);
			loop = loop->prev->sym;
		}
	} else {
		loop = he->sym;
		while (loop != he->next && loop) {
			res.push_back(loop);
			loop = loop->prev->sym;
		}
        if (loop) return; //else...
        loop = he->next->sym;
        while (loop && loop != he->next) {
            res.push_back(loop->next);
            loop = loop->next->sym;
        }
	}
}

/** given a half_edge pointer he, return a vector of pointers
 * to all the neighboring edges */
//TODO: deal with null edge->data
void
Mesh::get_neighboring_edges(vector<half_edge*> &res, half_edge* he) {
	bool done = false;
	half_edge* loop;

	get_src_edges(res, he);
	get_dst_edges(res, he);
}

void
Mesh::calculate_new_vertex(vertex& midpoint, edge_collapse& ec, edge_data* edata, half_edge* he, half_edge* hesym) {

	float Q1[10];
	memcpy(Q1, verts[he->v].Q, sizeof(Q1));
	for (int j=0; j<10; j+=1) {
		Q1[j] += verts[he->next->v].Q[j];
	}
	
    if (edata->merge_point == verts[he->v].position){
    	midpoint = verts[he->v];
    	ec.collapseVert = he->v;
    	memcpy(verts[he->v].Q, Q1, sizeof(Q1));
    } else if (edata->merge_point == verts[he->next->v].position){
    	midpoint = verts[he->next->v];
    	ec.collapseVert= he->next->v;
    	memcpy(verts[he->next->v].Q, Q1, sizeof(Q1));
    } else {
		memcpy(midpoint.Q, Q1, sizeof(Q1));
		midpoint.position = edata->merge_point;
		midpoint.normal = glm::normalize(verts[he->v].normal + verts[he->next->v].normal);
		verts.push_back(midpoint);
		ec.collapseVert = verts.size()-1;
	}
	
	ec.removed.push_back(he->prev);
	ec.removed.push_back(he->next);
	ec.removed.push_back(he);
	edges[he->prev->index] = NULL;
	edges[he->next->index] = NULL;
	edges[he->index] = NULL;
	if (hesym){
		edges[hesym->prev->index] = NULL;
		edges[hesym->next->index] = NULL;
		edges[hesym->index] = NULL;
		ec.removed.push_back(hesym->prev);
		ec.removed.push_back(hesym->next);
		ec.removed.push_back(hesym);
	}
}

void
Mesh::update_edge_pointers(half_edge* he, half_edge* hesym) {	
	half_edge *first, *second;
	edge_data *newdata;
	
	pq.erase(he->prev->data->pq_handle);
	delete he->prev->data;
	newdata = he->next->data;
	
	first = he->next->sym;
	second = he->prev->sym;
	if (first) {
		first->sym = second;
		first->data = newdata;
		newdata->edge = first;
	}
	if (second) {
		second->sym = first;
		second->data = newdata;
		newdata->edge = second;
	}
	
	if (hesym){
		pq.erase(hesym->prev->data->pq_handle);
		delete hesym->prev->data;
		newdata = hesym->next->data;

		first = hesym->next->sym;
		second = hesym->prev->sym;
		if (first) {
			first->sym = second;
			first->data = newdata;
			newdata->edge = first;
		}
		if (second) {
			second->sym = first;
			second->data = newdata;
			newdata->edge = second;
		}
	}
}

void
Mesh::update_src_neighbors(half_edge* he, vector<half_edge*>& src_neighbors, edge_collapse& ec) {
    for (unsigned int i = 0; i < src_neighbors.size(); i++) {
          if (he->sym) {
              if (src_neighbors[i] == he->sym ||
                  src_neighbors[i] == he->sym->prev ||
                  src_neighbors[i] == he->sym->next)
                continue;
          }
          if (src_neighbors[i] == he ||
              src_neighbors[i] == he->next ||
              src_neighbors[i] == he->prev)
            continue;
          src_neighbors[i]->v = ec.collapseVert; // set vertex to midpoint
          src_neighbors[i]->data->calculate_quad_error(verts);
          pq.update(src_neighbors[i]->data->pq_handle);
          ec.fromV1.push_back(src_neighbors[i]);
    }
}

void
Mesh::update_dst_neighbors(half_edge* he, vector<half_edge*>& dst_neighbors, edge_collapse& ec) {
    for (unsigned int i = 0; i < dst_neighbors.size(); i++) {
          if (he->sym) {
              if (dst_neighbors[i] == he->sym ||
                  dst_neighbors[i] == he->sym->prev ||
                  dst_neighbors[i] == he->sym->next)
                continue;
          }
          if (dst_neighbors[i] == he ||
              dst_neighbors[i] == he->next ||
              dst_neighbors[i] == he->prev)
            continue;
          dst_neighbors[i]->v = ec.collapseVert; // set vertex to midpoint
          dst_neighbors[i]->data->calculate_quad_error(verts);
          pq.update(dst_neighbors[i]->data->pq_handle);
          ec.fromV2.push_back(dst_neighbors[i]);
    }
}

void
Mesh::remove_fins(half_edge* he, edge_collapse& ec) {
	int counter = 0;
	while(true) {
		if (!he->next->sym || !he->next->sym->prev->sym){
			return;
		}
		
		if (he->next->sym == he->prev) {
			cout << "DERP" << endl;
			return;
		}
		
		if (he->next->sym->prev->sym->prev != he->prev->sym) {
			return; //no fin
		}
		
		cout << "REMOVING FINS " << counter <<endl;
		counter += 1;
		
		/* Removed fins */
		ec.removed.push_back(he->next->sym);
		ec.removed.push_back(he->next->sym->next);
		ec.removed.push_back(he->next->sym->prev);
		ec.removed.push_back(he->prev->sym);
		ec.removed.push_back(he->prev->sym->next);
		ec.removed.push_back(he->prev->sym->prev);
		edges[he->next->sym->index] = NULL;
		edges[he->next->sym->next->index] = NULL;
		edges[he->next->sym->prev->index] = NULL;
		edges[he->prev->sym->index] = NULL;
		edges[he->prev->sym->next->index] = NULL;
		edges[he->prev->sym->prev->index] = NULL;
		
		/* remove associated edge_datas */
		pq.erase(he->next->sym->next->data->pq_handle);
		delete he->next->sym->next->data;
		pq.erase(he->prev->sym->prev->data->pq_handle);
		delete he->prev->sym->prev->data;
		pq.erase(he->next->sym->prev->data->pq_handle);
		delete he->next->sym->prev->data;
		
		he->next->data->edge = he->next;
		he->prev->data->edge = he->prev;
		
		/* Only first loop */
		if (counter == 1) {
			ec.newVerts.push_back(he->prev->v);
			ec.changedVerts.push_back(he->prev);
		}
		he->prev->v = he->prev->sym->prev->v;
		
		/* updata new edge data */
		half_edge* first = he->next->sym->next;
		half_edge* second = he->prev->sym->prev;
		
		he->next->sym = first->sym;
		if (first->sym){
			first->sym->sym = he->next;
			first->sym->data = he->next->data;
		}
		
		he->prev->sym = second->sym;
		if (second->sym){
			second->sym->sym = he->prev;
			second->sym->data = he->prev->data;
		}
	}
}

/** Collapses half_edge* [he] and sets the surrounding edges to point to
 * a new vertex v_m that is the midpoint of [he]'s two defining vertices.
 * I know I'm probably forgetting to set some edges, but here are my
 * initial thoughts:
 */

void
Mesh::collapse_edge() {
	edge_data *edata = pq.top();
	if (edata->merge_cost > THRESHOLD || pq.size()<5){
		return;
	}
	pq.pop();
	
    //pq_contains[edata] = false;
	level_of_detail += 1;
	half_edge* he = edata->edge;
	half_edge* hesym = he->sym;	
	edge_collapse ec; //store edge collapse information
	ec.V1 = he->v;
    ec.V2 = he->next->v;
	
	remove_fins(he,ec);
	if (hesym) remove_fins(hesym,ec);

    vector<half_edge*> src_neighbors;
    get_src_edges(src_neighbors, he);
    vector<half_edge*> dst_neighbors;
    get_dst_edges(dst_neighbors, he);

	/* Calculate new vertex position **/
	vertex midpoint = vertex();
	calculate_new_vertex(midpoint, ec, edata, he, hesym);
    update_edge_pointers(he, hesym);
    update_src_neighbors(he, src_neighbors, ec);
    update_dst_neighbors(he, dst_neighbors, ec);
	
	collapse_list.push_back(ec);
	
	/** Delete removed items **/
	delete edata;
}

void
Mesh::upLevelOfDetail(const int num) {
	for (int t=0; t<num; ++t) {
		if (level_of_detail == 0) {
			return;
		}
		
		level_of_detail -= 1;
		edge_collapse ec = collapse_list[level_of_detail];
		
		for (int i=0; i<ec.removed.size(); i+=1) {
			edges[ec.removed[i]->index] = ec.removed[i];
		}
		
		for (int i=0; i<ec.fromV1.size(); i+=1) {
			ec.fromV1[i]->v = ec.V1;
		}
		
		for (int i=0; i<ec.fromV2.size(); i+=1) {
			ec.fromV2[i]->v = ec.V2;
		}
		
		for (int i=0; i<ec.newVerts.size(); i+=1) {
			int vert = ec.changedVerts[i]->v;
			ec.changedVerts[i]->v = ec.newVerts[i];
			ec.newVerts[i] = vert;
		}
	}
}

void
Mesh::downLevelOfDetail(const int num) {
	for(int t=0; t<num; ++t) {
		if (level_of_detail == collapse_list.size()){
			collapse_edge();
			continue;
		}
		edge_collapse ec = collapse_list[level_of_detail];
		
		for (int i=0; i<ec.removed.size(); i+=1) {
			edges[ec.removed[i]->index] = NULL;
		}
		
		for (int i=0; i<ec.fromV1.size(); i+=1) {
			ec.fromV1[i]->v = ec.collapseVert;
		}
		
		for (int i=0; i<ec.fromV2.size(); i+=1) {
			ec.fromV2[i]->v = ec.collapseVert;
		}
		
		for (int i=0; i<ec.newVerts.size(); i+=1) {
			int vert = ec.changedVerts[i]->v;
			ec.changedVerts[i]->v = ec.newVerts[i];
			ec.newVerts[i] = vert;
		}
		
		level_of_detail += 1;
	}
}


void
Mesh::update_buffer() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
	
	vector<GLuint> elements;
	vector<half_edge*>::iterator it;
	for (it=edges.begin(); it!=edges.end(); ++it) {
		if (*it) {
			elements.push_back((*it)->v);
		}
	}
	
	cout << "Triangles: " << elements.size()/3 << endl;
	numIndices = elements.size();
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*elements.size(), &elements[0], GL_STATIC_DRAW);
	if (max_lod < level_of_detail){
		cout << "UPLOAD VERTS" << endl;
		max_lod = level_of_detail;
		vector<vertex_data> vertdata;
		for (int i=0; i<verts.size(); i+=1){
			vertdata.push_back(verts[i].data());
		}
		glBindBuffer(GL_ARRAY_BUFFER, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data)*vertdata.size(), &vertdata[0], GL_STATIC_DRAW);
	}
}

void
Mesh::draw() {
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
}
