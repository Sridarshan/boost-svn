\documentclass[11pt]{report}

%\input{defs}
\usepackage{math}
\usepackage{jweb}
\usepackage{lgrind}
\usepackage{times}
\usepackage{fullpage}
\usepackage{graphicx}

\newif\ifpdf
\ifx\pdfoutput\undefined
   \pdffalse
\else
   \pdfoutput=1
   \pdftrue
\fi

\ifpdf
  \usepackage[
              pdftex,
              colorlinks=true, %change to true for the electronic version
              linkcolor=blue,filecolor=blue,pagecolor=blue,urlcolor=blue
              ]{hyperref}
\fi

\ifpdf
  \newcommand{\stlconcept}[1]{\href{http://www.sgi.com/tech/stl/#1.html}{{\small \textsf{#1}}}}
  \newcommand{\bglconcept}[1]{\href{http://www.boost.org/libs/graph/doc/#1.html}{{\small \textsf{#1}}}}
  \newcommand{\pmconcept}[1]{\href{http://www.boost.org/libs/property_map/#1.html}{{\small \textsf{#1}}}}
  \newcommand{\myhyperref}[2]{\hyperref[#1]{#2}}
  \newcommand{\vizfig}[2]{\begin{figure}[htbp]\centerline{\includegraphics*{#1.pdf}}\caption{#2}\label{fig:#1}\end{figure}}
\else
  \newcommand{\myhyperref}[2]{#2}
  \newcommand{\bglconcept}[1]{{\small \textsf{#1}}}
  \newcommand{\pmconcept}[1]{{\small \textsf{#1}}}
  \newcommand{\stlconcept}[1]{{\small \textsf{#1}}}
  \newcommand{\vizfig}[2]{\begin{figure}[htbp]\centerline{\includegraphics*{#1.eps}}\caption{#2}\label{fig:#1}\end{figure}}
\fi

\newcommand{\code}[1]{{\small{\em \textbf{#1}}}}


% jweb -np isomorphism-impl.w; dot -Tps out.dot -o out.eps; dot -Tps in.dot -o in.eps; latex isomorphism-impl.tex; dvips isomorphism-impl.dvi -o isomorphism-impl.ps

\setlength\overfullrule{5pt}
\tolerance=10000
\sloppy
\hfuzz=10pt

\makeindex

\newcommand{\isomorphic}{\cong}

\begin{document}

\title{An Implementation of Isomorphism Testing}
\author{Jeremy G. Siek}

\maketitle

\section{Introduction}

This paper documents the implementation of the \code{isomorphism()}
function of the Boost Graph Library.  The implementation was by Jeremy
Siek with algorithmic improvements and test code from Douglas Gregor.
The \code{isomorphism()} function answers the question, ``are these
two graphs equal?''  By \emph{equal}, we mean the two graphs have the
same structure---the vertices and edges are connected in the same
way. The mathematical name for this kind of equality is
\emph{isomorphic}.

An \emph{isomorphism} is a one-to-one mapping of the vertices in one
graph to the vertices of another graph such that adjacency is
preserved. Another words, given graphs $G_{1} = (V_{1},E_{1})$ and
$G_{2} = (V_{2},E_{2})$, an isomorphism is a function $f$ such that
for all pairs of vertices $a,b$ in $V_{1}$, edge $(a,b)$ is in $E_{1}$
if and only if edge $(f(a),f(b))$ is in $E_{2}$.

Both graphs must be the same size, so let $N = |V_1| = |V_2|$. The
graph $G_1$ is \emph{isomorphic} to $G_2$ if an isomorphism exists
between the two graphs, which we denote by $G_1 \isomorphic G_2$.

In the following discussion we will need to use several notions from
graph theory. The graph $G_s=(V_s,E_s)$ is a \emph{subgraph} of graph
$G=(V,E)$ if $V_s \subseteq V$ and $E_s \subseteq E$.  An
\emph{induced subgraph}, denoted by $G[V_s]$, of a graph $G=(V,E)$
consists of the vertices in $V_s$, which is a subset of $V$, and every
edge $(u,v)$ in $E$ such that both $u$ and $v$ are in $V_s$.  We use
the notation $E[V_s]$ to mean the edges in $G[V_s]$.

In some places we express a function as a set of pairs, so the set $f
= \{ \pair{a_1}{b_1}, \ldots, \pair{a_n}{b_n} \}$
means $f(a_i) = b_i$ for $i=1,\ldots,n$.

\section{Exhaustive Backtracking Search}

The algorithm used by the \code{isomorphism()} function is, at
first approximation, an exhaustive search implemented via
backtracking.  The backtracking algorithm is a recursive function. At
each stage we will try to extend the match that we have found so far.
So suppose that we have already determined that some subgraph of $G_1$
is isomorphic to a subgraph of $G_2$.  We then try to add a vertex to
each subgraph such that the new subgraphs are still isomorphic to one
another. At some point we may hit a dead end---there are no vertices
that can be added to extend the isomorphic subgraphs. We then
backtrack to previous smaller matching subgraphs, and try extending
with a different vertex choice. The process ends by either finding a
complete mapping between $G_1$ and $G_2$ and return true, or by
exhausting all possibilities and returning false.

We are going to consider the vertices of $G_1$ in a specific order
(more about this later), so assume that the vertices of $G_1$ are
labeled $1,\ldots,N$ according to the order that we plan to add them
to the subgraph.  Let $G_1[k]$ denote the subgraph of $G_1$ induced by
the first $k$ vertices, with $G_1[0]$ being an empty graph. At each
stage of the recursion we start with an isomorphism $f_{k-1}$ between
$G_1[k-1]$ and a subgraph of $G_2$, which we denote by $G_2[S]$, so
$G_1[k-1] \isomorphic G_2[S]$. The vertex set $S$ is the subset of
$V_2$ that corresponds via $f_{k-1}$ to the first $k-1$ vertices in
$G_1$. We try to extend the isomorphism by finding a vertex $v \in V_2
- S$ that matches with vertex $k$. If a matching vertex is found, we
have a new isomorphism $f_k$ with $G_1[k] \isomorphic G_2[S \union \{
v \}]$.

\begin{tabbing}
IS\=O\=M\=O\=RPH($k$, $S$, $f_{k-1}$) $\equiv$ \\
\>\textbf{if} ($k = |V_1|+1$) \\
\>\>\textbf{return} true \\
\>\textbf{for} each vertex $v \in V_2 - S$ \\
\>\>\textbf{if} (MATCH($k$, $v$)) \\
\>\>\>$f_k = f_{k-1} \union \pair{k}{v}$ \\
\>\>\>ISOMORPH($k+1$, $S \union \{ v \}$, $f_k$)\\
\>\>\textbf{else}\\
\>\>\>\textbf{return} false \\
\\
ISOMORPH($0$, $G_1$, $\emptyset$, $G_2$)
\end{tabbing}

The basic idea of the match operation is to check whether $G_1[k]$ is
isomorphic to $G_2[S \union \{ v \}]$. We already know that $G_1[k-1]
\isomorphic G_2[S]$ with the mapping $f_{k-1}$, so all we need to do
is verify that the edges in $E_1[k] - E_1[k-1]$ connect vertices that
correspond to the vertices connected by the edges in $E_2[S \union \{
v \}] - E_2[S]$. The edges in $E_1[k] - E_1[k-1]$ are all the
out-edges $(k,j)$ and in-edges $(j,k)$ of $k$ where $j$ is less than
or equal to $k$ according to the ordering.  The edges in $E_2[S \union
\{ v \}] - E_2[S]$ consists of all the out-edges $(v,u)$ and
in-edges $(u,v)$ of $v$ where $u \in S$.

\begin{tabbing}
M\=ATCH($k$, $v$) $\equiv$ \\
\>$out \leftarrow \forall (k,j) \in E_1[k] - E_1[k-1] \Big( (v,f(j)) \in E_2[S \union \{ v \}] - E_2[S] \Big)$ \\
\>$in \leftarrow \forall (j,k) \in E_1[k] - E_1[k-1] \Big( (f(j),v) \in E_2[S \union \{ v \}] - E_2[S] \Big)$ \\
\>\textbf{return} $out \Land in$ 
\end{tabbing}

The problem with the exhaustive backtracking algorithm is that there
are $N!$ possible vertex mappings, and $N!$ gets very large as $N$
increases, so we need to prune the search space. We use the pruning
techniques described in
\cite{deo77:_new_algo_digraph_isomorph,fortin96:_isomorph,reingold77:_combin_algo}
that originated in
\cite{sussenguth65:_isomorphism,unger64:_isomorphism}.

\section{Vertex Invariants}
\label{sec:vertex-invariants}

One way to reduce the search space is through the use of \emph{vertex
invariants}. The idea is to compute a number for each vertex $i(v)$
such that $i(v) = i(v')$ if there exists some isomorphism $f$ where
$f(v) = v'$. Then when we look for a match to some vertex $v$, we only
need to consider those vertices that have the same vertex invariant
number. The number of vertices in a graph with the same vertex
invariant number $i$ is called the \emph{invariant multiplicity} for
$i$.  In this implementation, by default we use the out-degree of the
vertex as the vertex invariant, though the user can also supply there
own invariant function. The ability of the invariant function to prune
the search space varies widely with the type of graph.

As a first check to rule out graphs that have no possibility of
matching, one can create a list of computed vertex invariant numbers
for the vertices in each graph, sort the two lists, and then compare
them.  If the two lists are different then the two graphs are not
isomorphic.  If the two lists are the same then the two graphs may be
isomorphic.

Also, we extend the MATCH operation to use the vertex invariants to
help rule out vertices.

\begin{tabbing}
M\=A\=T\=C\=H-INVAR($k$, $v$) $\equiv$ \\
\>$out \leftarrow \forall (k,j) \in E_1[k] - E_1[k-1] \Big( (v,f(j)) \in E_2[S \union \{ v \}] - E_2[S] \Land i(v) = i(k) \Big)$ \\
\>$in \leftarrow \forall (j,k) \in E_1[k] - E_1[k-1] \Big( (f(j),v) \in E_2[S \union \{ v \}] - E_2[S] \Land i(v) = i(k) \Big)$ \\
\>\textbf{return} $out \Land in$ 
\end{tabbing}

\section{Vertex Order}

A good choice of the labeling for the vertices (which determines the
order in which the subgraph $G_1[k]$ is grown) can also reduce the
search space. In the following we discuss two labeling heuristics.

\subsection{Most Constrained First}

Consider the most constrained vertices first.  That is, examine
lower-degree vertices before higher-degree vertices. This reduces the
search space because it chops off a trunk before the trunk has a
chance to blossom out. We can generalize this to use vertex
invariants. We examine vertices with low invariant multiplicity
before examining vertices with high invariant multiplicity.

\subsection{Adjacent First}

The MATCH operation only considers edges when the other vertex already
has a mapping defined. This means that the MATCH operation can only
weed out vertices that are adjacent to vertices that have already been
matched. Therefore, when choosing the next vertex to examine, it is
desirable to choose one that is adjacent a vertex already in $S_1$.

\subsection{DFS Order, Starting with Lowest Multiplicity}

For this implementation, we combine the above two heuristics in the
following way. To implement the ``adjacent first'' heuristic we apply
DFS to the graph, and use the DFS discovery order as our vertex
order. To comply with the ``most constrained first'' heuristic we
order the roots of our DFS trees by invariant multiplicity.


\section{Implementation}


@d Degree vertex invariant functor
@{
template <typename InDegreeMap, typename Graph>
class degree_vertex_invariant
{
public:
    typedef typename graph_traits<Graph>::vertex_descriptor argument_type;
    typedef typename graph_traits<Graph>::degree_size_type result_type;

    degree_vertex_invariant(const InDegreeMap& in_degree_map, const Graph& g)
        : m_in_degree_map(in_degree_map), m_g(g) { }

    result_type operator()(argument_type v) const {
        return (num_vertices(m_g) + 1) * out_degree(v, m_g)
            + get(m_in_degree_map, v);
    }
    // The largest possible vertex invariant number
    result_type max() const { 
        return num_vertices(m_g) * num_vertices(m_g) + num_vertices(m_g);
    }
private:
    InDegreeMap m_in_degree_map;
    const Graph& m_g;
};
@}


@d Invariant multiplicity comparison functor
@{
struct cmp_multiplicity
{
    cmp_multiplicity(self& algo, size_type* multiplicity)
        : algo(algo), multiplicity(multiplicity) { }
    bool operator()(const vertex1_t& x, const vertex1_t& y) const {
        return multiplicity[algo.invariant1(x)] < multiplicity[algo.invariant1(y)];
    }
    self& algo;
    size_type* multiplicity;
};
@}

ficticiuos edges for the DFS tree roots
Use \code{ordered\_edge} instead of \code{edge1\_t} so that we can create ficticious
edges for the DFS tree roots.

@d Ordered edge class
@{
struct ordered_edge {
    ordered_edge(int s, int t) : source(s), target(t) { }

    bool operator<(const ordered_edge& e) const {
        using namespace std;
        int m1 = max(source, target);
        int m2 = max(e.source, e.target);
        // lexicographical comparison of (m1,source,target) and (m2,e.source,e.target)
        return make_pair(m1, make_pair(source, target)) < make_pair(m2, make_pair(e.source, e.target));
    }
    int source;
    int target;
    int order;
};
@}


@d State used inside the DFS Visitor
@{
struct dfs_order {
    dfs_order(std::vector<vertex1_t>& v, std::vector<ordered_edge>& e) 
	: vertices(v), edges(e) { }
    std::vector<vertex1_t>& vertices;
    std::vector<ordered_edge>& edges;
};
@}

@d DFS visitor to record vertex and edge order
@{
struct record_dfs_order : default_dfs_visitor {
    record_dfs_order(dfs_order& order) : order(order) { }
    void start_vertex(vertex1_t v, const Graph1&) const {
        order.edges.push_back(ordered_edge(-1, v));
    }
    void discover_vertex(vertex1_t v, const Graph1&) const {
        order.vertices.push_back(v);
    }
    void examine_edge(edge1_t e, const Graph1& G1) const {
        order.edges.push_back(ordered_edge(source(e, G1), target(e, G1)));
    }
    dfs_order& order;
};
@}


@d Quick return if the vertex invariants do not match up
@{
{
    std::vector<invar1_value> invar1_array;
    BGL_FORALL_VERTICES_T(v, G1, Graph1)
        invar1_array.push_back(invariant1(v));
    std::sort(invar1_array.begin(), invar1_array.end());

    std::vector<invar2_value> invar2_array;
    BGL_FORALL_VERTICES_T(v, G2, Graph2)
        invar2_array.push_back(invariant2(v));
    std::sort(invar2_array.begin(), invar2_array.end());

    if (!std::equal(invar1_array.begin(), invar1_array.end(), invar2_array.begin())) {
        std::cout << "invariants don't match" << std::endl;
        return false;
    }
}
@}

@d Sort vertices according to invariant multiplicity
@{
std::vector<vertex1_t> V_mult;
BGL_FORALL_VERTICES_T(v, G1, Graph1)
    V_mult.push_back(v);
{
    std::vector<size_type> multiplicity(max_invariant, 0);
    BGL_FORALL_VERTICES_T(v, G1, Graph1)
        ++multiplicity[invariant1(v)];

    std::sort(V_mult.begin(), V_mult.end(), cmp_multiplicity(*this, &multiplicity[0]));

    std::cout << "V_mult=";
    std::copy(V_mult.begin(), V_mult.end(),
              std::ostream_iterator<vertex1_t>(std::cout, " "));
    std::cout << std::endl;
}
@}

@d Order vertices and edges by DFS
@{
{
    dfs_order order(dfs_vertices, ordered_edges);
    std::vector<default_color_type> color_vec(num_vertices(G1));
    record_dfs_order dfs_visitor(order);
    typedef color_traits<default_color_type> Color;
    for (vertex_iter u = V_mult.begin(); u != V_mult.end(); ++u) {
        if (color_vec[*u] == Color::white()) {
            dfs_visitor.start_vertex(*u, G1);
            depth_first_visit(G1, *u, dfs_visitor, &color_vec[0]);
        }
    }
    // Create the dfs_number array
    size_type n = 0;
    dfs_number.resize(num_vertices(G1));
    for (vertex_iter v = dfs_vertices.begin(); v != dfs_vertices.end(); ++v)
        dfs_number[*v] = n++;
    
    // Renumber ordered_edges array according to DFS number
    for (edge_iter e = ordered_edges.begin(); e != ordered_edges.end(); ++e) {
        if (e->source >= 0)
          e->source = dfs_number[e->source];
        e->target = dfs_number[e->target];
    }
}
@}

Reorder the edges so that all edges belonging to $G_1[k]$
appear before any edges not in $G_1[k]$, for $k=1,...,n$.

The order field needs a better name. How about k?

@d Sort edges according to vertex DFS order
@{
{
    std::stable_sort(ordered_edges.begin(), ordered_edges.end());
    // Fill in i->order field
    ordered_edges[0].order = 0;
    for (edge_iter i = next(ordered_edges.begin()); i != ordered_edges.end(); ++i)
        i->order = std::max(prior(i)->source, prior(i)->target);
}
@}


\subsection{Recursive Match Function}




@d Match function
@{
bool match(edge_iter iter)
{
std::cout << "*** entering match" << std::endl;
if (iter != ordered_edges.end()) {
    ordered_edge edge = *iter;
    size_type edge_order_num = edge.order;
    vertex1_t u;
    if (edge.source != -1) // might be a ficticious edge
        u = dfs_vertices[edge.source];
    vertex1_t v = dfs_vertices[edge.target];
    std::cout << "edge: (";
    if (edge.source == -1)
        std::cout << "root";
    else
        std::cout << name_map1[dfs_vertices[edge.source]];
    std::cout << "," << name_map1[dfs_vertices[edge.target]] << ")" << std::endl;
    if (edge.source == -1) { // root node
        @<$v$ is a DFS tree root@>
    } else if (f_assigned[v] == false) {
        @<$v$ is an unmatched vertex, $(u,v)$ is a tree edge@>
    } else {
        @<Check to see if there is an edge in $G_2$ to match $(u,v)$@>
    }
} else 
    return true;
std::cout << "returning false" << std::endl;
return false;
} // match()
@}



@d $v$ is a DFS tree root
@{
std::cout << "** case 1" << std::endl;
// Try all possible mappings
BGL_FORALL_VERTICES_T(y, G2, Graph2) {
    std::cout << "y: " << name_map2[y] << std::endl;
    if (invariant1(v) == invariant2(y) && f_inv_assigned[y] == false) {
        std::cout << "f(" << name_map1[v] << ")=" <<name_map2[y] << std::endl;
        f[v] = y; 
        f_assigned[v] = true;
        f_inv[y] = v; f_inv_assigned[y] = true;
        mc = 0;
        std::cout << "mc = 0" << std::endl;
        if (match(next(iter)))
            return true;
        f_assigned[v] = false;
        f_inv_assigned[y] = false;
    }
    std::cout << "xxx" << std::endl;
}
@}

Growing the subgraph.

@d $v$ is an unmatched vertex, $(u,v)$ is a tree edge
@{
std::cout << "** case 2" << std::endl;
vertex1_t k = dfs_vertices[edge_order_num];
std::cout << "k=" << name_map1[k] << std::endl;
assert(f_assigned[k] == true);
std::cout << "f[k]: " << name_map2[f[k]] << std::endl;

@<Count out-edges of $f(k)$ in $G_2[S]$@>
@<Count in-edges of $f(k)$ in $G_2[S]$@>

std::cout << "mc: " << mc << std::endl;
if (mc != 0) // make sure out/in edges for k and f(k) add up
    return false;
@<Assign $v$ to some vertex in $V_2 - S$@>
@}

@d Count out-edges of $f(k)$ in $G_2[S]$
@{
BGL_FORALL_ADJACENT_T(f[k], w, G2, Graph2) {
    if (f_inv_assigned[w] == true) {
        --mc;
        std::cout << "--mc: " << mc << std::endl;
        std::cout << "(" << name_map2[f[k]] << "," << name_map2[w] << ")\n";
    }
}
@}

@d Count in-edges of $f(k)$ in $G_2[S]$
@{
for (std::size_t ji = 0; ji < edge_order_num; ++ji) {
    vertex1_t j = dfs_vertices[ji];
    BGL_FORALL_ADJACENT_T(f[j], w, G2, Graph2) {
        if (w == f[k]) {
            --mc;
            std::cout << "--mc: " << mc << std::endl;
            std::cout << "(" << name_map2[f[j]] << "," << name_map2[w] << ")\n";
        }
    }
}
@}

@d Assign $v$ to some vertex in $V_2 - S$
@{
BGL_FORALL_ADJACENT_T(f[u], y, G2, Graph2) {
    if (invariant1(v) == invariant2(y) && f_inv_assigned[y] == false) {
        f[v] = y; f_assigned[v] = true;
        std::cout << "f(" << name_map1[v] << ")=" << name_map2[y] << std::endl;;
        f_inv[y] = v; f_inv_assigned[y] = true;
        mc = 1;
        std::cout << "(f(u),y): (" << name_map2[f[u]] << "," << name_map2[y]
                    << ")" << std::endl;
        std::cout << "mc = 1" << std::endl;
        if (match(next(iter)))
            return true;
        f_assigned[v] = false;
        f_inv_assigned[y] = false;
    }
}           
@}



@d Check to see if there is an edge in $G_2$ to match $(u,v)$
@{
std::cout << "** case 3" << std::endl;
bool verify = false;
assert(f_assigned[u] == true);
BGL_FORALL_ADJACENT_T(f[u], y, G2, Graph2) {
    std::cout << "y: " << name_map2[y] << std::endl;
    assert(f_assigned[v] == true);
    if (y == f[v]) {    
        std::cout << "found match, (" << name_map2[f[u]] 
                << "," << name_map2[y] << ")" << std::endl;
        verify = true;
        break;
    }
}
if (verify == true) {
    ++mc; // out or in edge of k
    std::cout << "++mc: " << mc << std::endl;
    if (match(next(iter)))
    return true;
}
@}



@o isomorphism-v2.hpp -d
@{
// Copyright (C) 2001 Jeremy Siek (jsiek@@osl.iu.edu),
//                    Doug Gregor (gregod@@cs.rpi.edu),
//                    Brian Osman (osmanb@@acm.org)
//
// Permission to copy, use, sell and distribute this software is granted
// provided this copyright notice appears in all copies.
// Permission to modify the code and to distribute modified code is granted
// provided this copyright notice appears in all copies, and a notice
// that the code was modified is included with the copyright notice.
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.
#ifndef BOOST_GRAPH_ISOMORPHISM_HPP
#define BOOST_GRAPH_ISOMORPHISM_HPP

#include <iostream>
#include <utility>
#include <vector>
#include <iterator>
#include <algorithm>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>

namespace boost {

namespace detail {
    
template <typename Graph1, typename Graph2, typename IsoMapping,
  typename Invariant1, typename Invariant2,
  typename IndexMap1, typename IndexMap2, 
  typename NameMap1, typename NameMap2>
class isomorphism_algo
{
    typedef isomorphism_algo self;
    typedef typename graph_traits<Graph1>::vertex_descriptor vertex1_t;
    typedef typename graph_traits<Graph2>::vertex_descriptor vertex2_t;
    typedef typename graph_traits<Graph1>::edge_descriptor edge1_t;
    typedef typename graph_traits<Graph1>::vertices_size_type size_type;
    typedef typename Invariant1::result_type invar1_value;
    typedef typename Invariant2::result_type invar2_value;

    // Parameters
    const Graph1& G1;
    const Graph2& G2;
    IsoMapping f;
    Invariant1 invariant1;
    Invariant2 invariant2;
    std::size_t max_invariant;
    IndexMap1 index_map1;
    IndexMap2 index_map2;
    NameMap1 name_map1;
    NameMap2 name_map2;

    @<Ordered edge class@>

    std::vector<vertex1_t> dfs_vertices;
    typedef std::vector<vertex1_t>::iterator vertex_iter;
    std::vector<size_type> dfs_number;
    std::vector<ordered_edge> ordered_edges;
    typedef std::vector<ordered_edge>::iterator edge_iter;

    friend struct cmp_multiplicity;
    @<Invariant multiplicity comparison functor@>
    @<State used inside the DFS Visitor@>
    @<DFS visitor to record vertex and edge order@>
public:

    isomorphism_algo(const Graph1& G1, const Graph2& G2, IsoMapping f,
                     Invariant1 invariant1, Invariant2 invariant2, std::size_t max_invariant,
                     IndexMap1 index_map1, IndexMap2 index_map2,
                     NameMap1 name_map1, NameMap2 name_map2)
        : G1(G1), G2(G2), f(f), invariant1(invariant1), invariant2(invariant2),
          max_invariant(max_invariant),
          index_map1(index_map1), index_map2(index_map2),
          name_map1(name_map1), name_map2(name_map2) { }

    bool test_isomorphism()
    {
        @<Quick return if the vertex invariants do not match up@>
        @<Sort vertices according to invariant multiplicity@>
        @<Order vertices and edges by DFS@>
        @<Sort edges according to vertex DFS order@>
        
        f_assigned.resize(num_vertices(G1));
        f_inv.resize(num_vertices(G1));
        f_inv_assigned.resize(num_vertices(G1));

        return this->match(ordered_edges.begin());
    } // test_isomorphism

private:

    std::vector<vertex1_t> f_inv;
    std::vector<bool> f_assigned;
    std::vector<bool> f_inv_assigned;
    int mc; // #edges incident on k

    @<Match function@>
        
    void print_ordered_edges() {
        std::cout << "ordered edges=";
        for (edge_iter i = ordered_edges.begin(); i != ordered_edges.end(); ++i)
          std::cout << "[" << name_map1[dfs_vertices[i->source]]
                    << "(" << i->source << ")"
                    << "," << name_map1[dfs_vertices[i->target]] << "(" << i->target << ")" 
                    << " : " << i->order << ")";
        std::cout << std::endl;
    }

    void print_dfs_numbers() {
        std::cout << "dfs numbers=";
        std::copy(dfs_number.begin(), dfs_number.end(),
                  std::ostream_iterator<vertex1_t>(std::cout, " "));
        std::cout << std::endl;
    }
};


template <typename Graph, typename InDegreeMap>
void compute_in_degree(const Graph& g, InDegreeMap in_degree_map)
{
    BGL_FORALL_VERTICES_T(v, g, Graph)
        put(in_degree_map, v, 0);

    BGL_FORALL_VERTICES_T(u, g, Graph)
      BGL_FORALL_ADJACENT_T(u, v, g, Graph)
        put(in_degree_map, v, get(in_degree_map, v) + 1);
}

} // namespace detail


@<Degree vertex invariant functor@>

template <typename Graph1, typename Graph2, 
          typename IsoMapping, 
          typename Invariant1, typename Invariant2,
          typename IndexMap1, typename IndexMap2,
          typename NameMap1, typename NameMap2>
bool isomorphism(const Graph1& G1, const Graph2& G2, 
                 IsoMapping f, 
                 Invariant1 invariant1, Invariant2 invariant2, std::size_t max_invariant,
                 IndexMap1 index_map1, IndexMap2 index_map2,
                 NameMap1 name_map1, NameMap2 name_map2)
{
    detail::isomorphism_algo<Graph1, Graph2, IsoMapping, Invariant1, Invariant2, 
        IndexMap1, IndexMap2, NameMap1, NameMap2> 
        algo(G1, G2, f, invariant1, invariant2, max_invariant, 
             index_map1, index_map2, name_map1, name_map2);
    return algo.test_isomorphism();
}

// Verify that the given mapping iso_map from the vertices of g1 to the
// vertices of g2 describes an isomorphism.
// Note: this could be made much faster by specializing based on the graph
// concepts modeled, but since we're verifying an O(n^(lg n)) algorithm,
// O(n^4) won't hurt us.
template<typename Graph1, typename Graph2, typename IsoMap>
inline bool verify_isomorphism(const Graph1& g1, const Graph2& g2, 
                               IsoMap iso_map)
{
  if (num_vertices(g1) != num_vertices(g2) || num_edges(g1) != num_edges(g2))
    return false;
  
  for (typename graph_traits<Graph1>::edge_iterator e1 = edges(g1).first;
       e1 != edges(g1).second; ++e1) {
    bool found_edge = false;
    for (typename graph_traits<Graph2>::edge_iterator e2 = edges(g2).first;
         e2 != edges(g2).second && !found_edge; ++e2) {
      if (source(*e2, g2) == get(iso_map, source(*e1, g1)) &&
          target(*e2, g2) == get(iso_map, target(*e1, g1))) {
        found_edge = true;
      }
    }
    
    if (!found_edge)
      return false;
  }
  
  return true;
}

} // namespace boost

#include <boost/graph/iteration_macros_undef.hpp>

#endif // BOOST_GRAPH_ISOMORPHISM_HPP
@}



\bibliographystyle{abbrv}
\bibliography{ggcl}

\end{document}
% LocalWords:  Isomorphism Siek isomorphism adjacency subgraph subgraphs OM DFS
% LocalWords:  ISOMORPH Invariants invariants typename IsoMapping bool const
% LocalWords:  VertexInvariant VertexIndexMap iterator typedef VertexG Idx num
% LocalWords:  InvarValue struct invar vec iter tmp_matches mult inserter permute ui
% LocalWords:  dfs cmp isomorph VertexIter edge_iter_t IndexMap desc RPH ATCH pre

% LocalWords:  iterators VertexListGraph EdgeListGraph BidirectionalGraph tmp
% LocalWords:  ReadWritePropertyMap VertexListGraphConcept EdgeListGraphConcept
% LocalWords:  BidirectionalGraphConcept ReadWritePropertyMapConcept indices ei
% LocalWords:  IsoMappingValue ReadablePropertyMapConcept namespace InvarFun
% LocalWords:  MultMap vip inline bitset typedefs fj hpp ifndef adaptor params
% LocalWords:  bgl param pmap endif
