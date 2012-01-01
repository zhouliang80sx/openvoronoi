/*  
 *  Copyright 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com)
 *  
 *  This file is part of OpenVoronoi.
 *
 *  OpenVoronoi is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenVoronoi is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenVoronoi.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef HEDI_H
#define HEDI_H

#include <vector>
#include <list>

#include <boost/graph/adjacency_list.hpp>
#include <boost/foreach.hpp> 
#include <boost/iterator/iterator_facade.hpp>

// bundled BGL properties, see: http://www.boost.org/doc/libs/1_44_0/libs/graph/doc/bundles.html

// dcel notes from http://www.holmes3d.net/graphics/dcel/

// vertex (boost::out_edges)
//  -leaving pointer to HalfEdge that has this vertex as origin
//   if many HalfEdges have this vertex as origin, choose one arbitrarily

// HalfEdge
//  - origin pointer to vertex (boost::source)
//  - face to the left of halfedge
//  - twin pointer to HalfEdge (on the right of this edge)
//  - next pointer to HalfEdge
//     this edge starts from h->twin->origin and ends at next vertex in h->face
//     traveling ccw around boundary
//     (allows face traverse, follow h->next until we arrive back at h)

// Face
//  - edge pointer to HalfEdge
//    this edge has this Face object as face
//    half-edge can be any one on the boundary of face
// special "infinite face", face on "outside" of boundary
// may or may not store edge pointer


/// HEDIGraph is a A half-edge diagram class.
/// Templated on Vertex/Edge/Face property classes which allow
/// attaching information to vertices/edges/faces that is 
/// required for a particular algorithm.
/// 
/// Inherits from boost::adjacency_list
/// minor additions allow storing face-properties.
///
/// the hedi namespace contains functions for manipulating HEDIGraphs
///
/// For a general description of the half-edge data structure see e.g.:
///  - http://www.holmes3d.net/graphics/dcel/
///  - http://openmesh.org/index.php?id=228


namespace hedi  { 


template <class TOutEdgeList, 
          class TVertexList,
          class TDirected, 
          class TVertexProperties,
          class TEdgeProperties,
          class TFaceProperties,
          class TGraphProperties,
          class TEdgeList 
          >
class HEDIGraph {
public:
    typedef unsigned int Face; 
    typedef typename boost::adjacency_list< TOutEdgeList,            
                                            TVertexList,            
                                            TDirected,   
                                            TVertexProperties,             
                                            TEdgeProperties,                
                                            TGraphProperties,
                                            TEdgeList
                                            > BGLGraph;
    typedef typename boost::graph_traits< BGLGraph >::edge_descriptor   Edge;
    typedef typename boost::graph_traits< BGLGraph >::vertex_descriptor Vertex;
    typedef typename boost::graph_traits< BGLGraph >::vertex_iterator   VertexItr;
    typedef typename boost::graph_traits< BGLGraph >::out_edge_iterator OutEdgeItr;
    typedef typename boost::graph_traits< BGLGraph >::edge_iterator     EdgeItr; 
    
    typedef std::vector<Vertex> VertexVector;
    typedef std::vector<Face>   FaceVector;
    typedef std::vector<Edge>   EdgeVector;  

    inline TFaceProperties& operator[](Face f)  { return faces[f];  }
    inline const TFaceProperties& operator[](Face f) const  { return faces[f]; } 
    
    inline TEdgeProperties& operator[](Edge e)  { return g[e];  }
    inline const TEdgeProperties& operator[](Edge e) const  { return g[e];  }
    
    inline TVertexProperties& operator[](Vertex v)  { return g[v];  }
    inline const TVertexProperties& operator[](Vertex v) const  { return g[v];  }
    
//DATA
    std::vector< TFaceProperties > faces;
    BGLGraph g;

    virtual ~HEDIGraph(){}
    
/// add a blank vertex and return its descriptor
Vertex add_vertex() { 
    return boost::add_vertex( g );
}


/// add a vertex with given properties, return vertex descriptor
Vertex add_vertex(const TVertexProperties& prop) {
    return boost::add_vertex( prop, g );
}

/// add an edge between vertices v1-v2
Edge add_edge(Vertex v1, Vertex v2) {
    Edge e;
    bool b;
    boost::tie( e , b ) = boost::add_edge( v1, v2, g);
    return e;
}

std::pair<Edge,Edge> add_twin_edges(Vertex v1, Vertex v2) {
    Edge e1,e2;
    bool b;
    boost::tie( e1 , b ) = boost::add_edge( v1, v2, g);
    boost::tie( e2 , b ) = boost::add_edge( v2, v1, g);
    twin_edges(e1,e2);
    return std::make_pair(e1,e2);
}

/// add an edge with given properties between vertices v1-v2
Edge add_edge( Vertex v1, Vertex  v2, const TEdgeProperties& prop ) {
    Edge e;
    bool b;
    boost::tie( e , b ) = boost::add_edge( v1, v2, prop, g);
    return e;
}

/// make e1 the twin of e2 (and vice versa)
void twin_edges( Edge e1, Edge e2 ) {
    if (target(e1) != source(e2)) {
        std::cout << " error target(e1)= " << g[target(e1)].index << " != " << g[source(e2)].index << " = source(e2) \n";
        std::cout << "target(e1) = " << target(e1) << "\n";
        std::cout << "source(e2) = " << source(e2) << "\n";
    }
    assert( target(e1) == source(e2) );
    assert( source(e1) == target(e2) );
    
    g[e1].twin = e2;
    g[e2].twin = e1;
}

/// add a face 
Face add_face() {
    TFaceProperties f_prop;
    faces.push_back( f_prop); 
    Face index = faces.size()-1;
    faces[index].idx = index;
    return index;    
}

/// add a face 
Face add_face(const TFaceProperties& prop) {
    faces.push_back( prop ); 
    Face index = faces.size()-1;
    faces[index].idx = index;
    return index;    
}

/// return the target vertex of the given edge
Vertex target(const Edge e ) const { 
    return boost::target( e, g );
}

/// return the source vertex of the given edge
Vertex source(const Edge e ) const { 
    return boost::source( e, g ); 
}

/// return all vertices in a vector of vertex descriptors
VertexVector vertices()  {
    VertexVector vv;
    VertexItr it_begin, it_end, itr;
    boost::tie( it_begin, it_end ) = boost::vertices( g );
    for ( itr=it_begin ; itr != it_end ; ++itr ) {
        vv.push_back( *itr );
    }
    return vv;
}

/// return all vertices adjecent to given vertex
VertexVector adjacent_vertices(  Vertex v) {
    VertexVector vv;
    BOOST_FOREACH( Edge edge, out_edges( v ) ) {
        vv.push_back( target( edge ) );
    }
    return vv;
}

/// return all vertices of given face
VertexVector face_vertices(Face face_idx) const {
    VertexVector verts;
    Edge startedge = faces[face_idx].edge; // the edge where we start
    Vertex start_target = boost::target( startedge, g ); 
    verts.push_back(start_target);
    Edge current = g[startedge].next;
    int count=0;
    EdgeVector f_edges; // for debug.
    f_edges.push_back(current);
    do {
        Vertex current_target = boost::target( current, g ); 
        //assert( current_target != start_target );
        verts.push_back(current_target);
        f_edges.push_back(current);
        assert( g[current].face == g[ g[current].next ].face );
        current = g[current].next;
        
        if (count >= 3000000 ) {
            std::cout << " ERROR too many vertices on face! count=" << count << "\n";
            std::cout << " verts.size() = " << verts.size();
            std::cout << " edges.size()=" << f_edges.size() <<"\n";
            for (unsigned int n=0;n<verts.size()-10;n++) {
                std::cout << n << "   : " << g[ verts[n] ].index << "\n"; 
            }
        }
        assert( count < 3000000 ); // stop at some max limit
        count++;
    } while ( current != startedge );
    return verts;
}

/// return edges of face f

EdgeVector face_edges( Face f) {
    Edge start_edge = faces[f].edge;
    Edge current_edge = start_edge;
    EdgeVector out;
    std::cout << " edges on face " << f << " :\n ";
    do {
        Vertex src = source(current_edge);
        Vertex trg = target(current_edge);
           std::cout << out.size() << " " << g[src].index << "[" << g[src].type <<"]";
           std::cout << " - " << g[trg].index << "[" << g[trg].type <<"]" <<"\n ";
        out.push_back(current_edge);
        current_edge = g[current_edge].next;
    } while( current_edge != start_edge );
    return out;
}



/// return degree of given vertex
unsigned int degree( Vertex v)  { 
    return boost::degree( v, g); 
}

/// return number of vertices in graph
unsigned int num_vertices() const { 
    return boost::num_vertices( g ); 
}

unsigned int num_edges() const { 
    return boost::num_edges( g ); 
}

unsigned int num_edges(Face f) { 
    EdgeVector ev = face_edges(f);
    return ev.size(); 
}

/// return out_edges of given vertex
EdgeVector out_edges( Vertex v) { 
    EdgeVector ev;
    OutEdgeItr it, it_end;
    boost::tie( it, it_end ) = boost::out_edges( v, g );
    for ( ; it != it_end ; ++it ) {
        ev.push_back(*it);
    }
    return ev;
}

std::pair<OutEdgeItr, OutEdgeItr> out_edge_itr( Vertex v ) {
    return boost::out_edges( v, g );
}

/// return all edges
EdgeVector edges() {
    EdgeVector ev;
    EdgeItr it, it_end;
    boost::tie( it, it_end ) = boost::edges( g );
    for ( ; it != it_end ; ++it ) {
        ev.push_back(*it);
    }
    return ev;
}
        
        



/// return the previous edge. traverses all edges in face until previous found.
Edge previous_edge( Edge e ) {
    Edge previous = g[e].next;
    while ( g[previous].next != e ) {
        previous = g[previous].next;
    }
    return previous;
}


/// return true if v1-v2 edge exists
bool has_edge( Vertex v1, Vertex v2) {
    typedef typename std::pair<Edge, bool> EdgeBool;
    EdgeBool result = boost::edge(v1, v2, g );
    return result.second;
}

/// return v1-v2 edge descriptor
/*
template <class BGLGraph>
typename boost::graph_traits< BGLGraph >::edge_descriptor edge( 
           typename boost::graph_traits< BGLGraph >::vertex_descriptor v1, 
           typename boost::graph_traits< BGLGraph >::vertex_descriptor v2, 
           BGLGraph& g ) {
    typedef typename boost::graph_traits< BGLGraph >::edge_descriptor HEEdge;
    typedef typename std::pair<HEEdge, bool> EdgeBool;
    EdgeBool result = boost::edge(v1, v2, g );
    return result.first;
}*/

/// return adjacent faces to the given vertex
FaceVector adjacent_faces( Vertex q ) {
    std::set<Face> face_set;
    OutEdgeItr itr, itr_end;
    boost::tie( itr, itr_end) = boost::out_edges(q, g);
    for ( ; itr!=itr_end ; ++itr ) {
        face_set.insert( g[*itr].face );
    }
    //assert( face_set.size() == 3); // degree of q is three, so has three faces
    FaceVector fv;
    BOOST_FOREACH(unsigned int m, face_set) {
        fv.push_back(m);
    }
    return fv;
}

/// return number of faces in graph
unsigned int num_faces() const { 
    return faces.size(); 
}

/// return number of edges in graph
/*
unsigned int num_edges() const { 
    return boost::num_edges( g ); 
}*/

/// inserts given vertex into edge e, and into the twin edge e_twin
void insert_vertex_in_edge(Vertex v, Edge e ) {
    // the vertex v is in the middle of edge e
    //                    face
    //                    e1   e2
    // previous-> source  -> v -> target -> next
    //            tw_trg  <- v <- tw_src <- tw_previous
    //                    te2  te1
    //                    twin_face
    
    Edge twin = g[e].twin;
    Vertex src = boost::source( e , g );
    Vertex trg = boost::target( e , g);
    Vertex twin_source = boost::source( twin , g);
    Vertex twin_target = boost::target( twin , g );
    assert( src == twin_target );    
    assert( trg == twin_source );
    
    Face face = g[e].face;
    Face twin_face = g[twin].face;
    Edge previous = previous_edge(e);
    assert( g[previous].face == g[e].face );
    Edge twin_previous = previous_edge(twin);
    assert( g[twin_previous].face == g[twin].face );
    
    Edge e1 = add_edge( src, v ); // these replace e
    Edge e2 = add_edge( v, trg );
    
    // preserve the left/right face link
    g[e1].face = face;
    g[e2].face = face;
    // next-pointers
    g[previous].next = e1;
    g[e1].next = e2;
    g[e2].next = g[e].next;
    
    Edge te1 = add_edge( twin_source, v  ); // these replace twin
    Edge te2 = add_edge( v, twin_target  );
    
    g[te1].face = twin_face;
    g[te2].face = twin_face;
    
    g[twin_previous].next = te1;
    g[te1].next = te2;
    g[te2].next = g[twin].next;
    
    // TWINNING (note indices 'cross', see ASCII art above)
    g[e1].twin = te2;
    g[te2].twin = e1;
    g[e2].twin = te1;
    g[te1].twin = e2;
    
    // update the faces (required here?)
    faces[face].edge = e1;
    faces[twin_face].edge = te1;
    
    // finally, remove the old edge
    boost::remove_edge( e   , g);
    boost::remove_edge( twin, g);
}


/// inserts given vertex into edge e
/*
template <class BGLGraph>
void insert_vertex_in_half_edge(typename boost::graph_traits< BGLGraph >::vertex_descriptor  v, 
                           typename boost::graph_traits< BGLGraph >::edge_descriptor e, 
                           BGLGraph& g) {
    typedef typename boost::graph_traits< BGLGraph >::edge_descriptor    HEEdge;
    typedef typename boost::graph_traits< BGLGraph >::vertex_descriptor  HEVertex;
    // the vertex v is in the middle of edge e
    //                    face
    //                    e1   e2
    // previous-> source  -> v -> target -> next
    
    HEVertex source = boost::source( e , g );
    HEVertex target = boost::target( e , g);
    unsigned int face = g[e].face;
    HEEdge previous = previous_edge(e, g);
    assert( g[previous].face == g[e].face );
    HEEdge e1 = add_edge( source, v , g);
    HEEdge e2 = add_edge( v, target , g);
    // preserve the left/right face link
    g[e1].face = face;
    g[e2].face = face;
    // next-pointers
    g[previous].next = e1;
    g[e1].next = e2;
    g[e2].next = g[e].next;
    // update the faces (required here?)
    g.faces[face].edge = e1;
    // finally, remove the old edge
    boost::remove_edge( e   , g);
    // NOTE: twinning is not done here, since the twin edge is not split...
}

*/
        /// check that all edges belong to the correct face, TODO: template this to make it a useful check
        /*
        bool checkFaces() {
            BOOST_FOREACH(FaceProps f, faces) {
                BOOST_FOREACH( HEEdge e, face_edges(f.idx)) {
                    if ( g[e].face != f.idx )
                        return false;
                }
            }
            return true;
        }*/

/// delete a vertex
void delete_vertex(Vertex v) { 
    clear_vertex(v);
    remove_vertex(v); 
}

/// clear given vertex. this removes all edges connecting to the vertex.
void clear_vertex( Vertex v) { 
    boost::clear_vertex( v, g ); 
}
/// remove given vertex
void remove_vertex( Vertex v) { 
    boost::remove_vertex( v , g );
}

/// remove given vertex
void remove_edge( Edge e) { 
    boost::remove_edge( e , g );
}

/// remove given vertex
void remove_edge( Vertex v1, Vertex v2) { 
    assert( has_edge(v1,v2) );
    typedef typename std::pair<Edge, bool> EdgeBool;
    EdgeBool result = boost::edge(v1, v2, g );    
    boost::remove_edge( result.first , g );
}

// see http://www.boost.org/doc/libs/1_48_0/libs/iterator/doc/iterator_facade.htm
//template<class Graph>
class edge_iterator : public boost::iterator_facade<
               edge_iterator ,
               Edge,
               boost::forward_traversal_tag> 
{
public:
    edge_iterator(): m_edge(Edge()), m_inc(false)  { }
    explicit edge_iterator(BGLGraph& g, Edge e): m_edge(e), m_g(g), m_inc(false)  {}
protected:
    friend class boost::iterator_core_access;
    void increment() { 
        m_edge = ( m_g[m_edge].next ); 
        if(!m_inc) m_inc = true;
    } 
    bool equal( edge_iterator const& other) const {
        return (((m_edge) == (other.m_edge)) && m_inc);
    }
    Edge& dereference() const { 
        Edge cpy(m_edge);
        Edge&  e = cpy; 
        return e; 
    } 
    
    Edge m_edge;
    BGLGraph& m_g;
    bool m_inc;
};

// EXPERIMENTAL! seems to work in debug-mode but not with a Release build!
std::pair<edge_iterator , edge_iterator  > face_edges_itr(Face f) {
    edge_iterator  itr1( g, faces[f].edge );
    edge_iterator  itr2( g, faces[f].edge );
    return std::make_pair(itr1,itr2); ;
}


void set_next(Edge e1, Edge e2) {
    if (target(e1) != source(e2) ){
        std::cout << " ERROR target(e1) = " << g[target(e1)].index << " source(e2)= " << g[source(e2)].index << "\n"; 
    }
    assert( target(e1) == source(e2) );
    g[e1].next = e2;
}

// form a face from the edge-list:
// e1->e2->...->e1
// for all edges, set edge.face=f, and edge.k=k
void set_next_cycle( std::list<Edge> list, Face f, double k) {
    typename std::list<Edge>::iterator begin,it,nxt,end;
    it= list.begin();
    begin = it;
    faces[f].edge = *it;
    end= list.end();
    for( ; it!=end ; it++ ) {
        nxt = it;
        nxt++;
        if ( nxt != end )
            set_next(*it,*nxt);
        else
            set_next(*it,*begin);
            
        g[*it].face = f;
        g[*it].k = k;
    }
}

void set_next_chain( std::list<Edge> list, Face f, double k) {
    typename std::list<Edge>::iterator it,nxt,end;
    it= list.begin();
    faces[f].edge = *it;
    //set_next_chain(list);
    //begin = it;    
    end= list.end();
    for( ; it!=end ; it++ ) {
        nxt = it;
        nxt++;
        if ( nxt != end )
            set_next(*it,*nxt);
            
        g[*it].face = f;
        g[*it].k = k;
    }
}

void set_next_chain( std::list<Edge> list ) {
    typename std::list<Edge>::iterator it,nxt,end;
    it= list.begin();
    //begin = it;
    end= list.end();
    for( ; it!=end ; it++ ) {
        nxt = it;
        nxt++;
        if ( nxt != end )
            set_next(*it,*nxt);
    }
}

}; // end HEDIGraph class definition




} // end hedi namespace

#endif
// end halfedgediagram.hpp
