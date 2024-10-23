#pragma once
#ifndef DASPOLYHEDRON_H
#define DASPOLYHEDRON_H

#include <vector>
#include <map>

#include "SBIFileReader.hpp"

#include "ADNVectorMath.hpp"
#include "ADNAuxiliary.hpp"

#undef foreach
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>


/* Half-edge implementation */
class DASVertex;
class DASEdge;
class DASPolygon;

using EdgeBps = std::map<DASEdge*, int>;

class SB_EXPORT DASHalfEdge {

public:

	DASHalfEdge() = default;
	/**
	 * Half-edges don't need custom destructor because in Half-edge
	 * representation they don't make sense by themselves
	 */
	~DASHalfEdge() = default;
	DASHalfEdge(const DASHalfEdge& other);
	DASHalfEdge& operator=(const DASHalfEdge& other);

	DASHalfEdge* next_{ nullptr };
	DASHalfEdge* prev_{ nullptr };
	DASHalfEdge* pair_{ nullptr };

	DASVertex* source_{ nullptr };
	DASPolygon* left_{ nullptr };
	DASEdge* edge_{ nullptr };

	// we need an id_ for copying
	unsigned int id_{ 0 };

};

class SB_EXPORT DASVertex {

public:

	DASVertex() = default;
	/**
	 * Topologically vertex has to be removed from the polyhedron.
	 * Hence default destructor.
	 */
	~DASVertex() = default;
	DASVertex(const DASVertex& w);
	DASVertex(DASVertex* w);

	DASVertex& operator=(const DASVertex& other);

	/* Members */
	DASHalfEdge* halfEdge_{ nullptr };
	int id_{ 0 };
	SBPosition3 position_;

	/* Getters */
	std::vector<double> GetVectorCoordinates() const; // always returns picometers 10^-12
	SBPosition3 GetSBPosition() const;

	/* Setters */
	void SetCoordinates(const SBPosition3& coordinates);

};

class SB_EXPORT DASEdge {

public:

	DASEdge() = default;
	~DASEdge();
	DASEdge(const DASEdge& other);

	DASEdge& operator=(const DASEdge& other);

	DASHalfEdge* halfEdge_{ nullptr };
	int id_{ 0 };

};

class SB_EXPORT DASPolygon {

public:

	DASPolygon() = default;
	DASPolygon(const DASPolygon& p);
	~DASPolygon();

	DASPolygon& operator=(const DASPolygon& p);

	SBPosition3 GetCenter() const;

	int id_{ 0 };
	DASHalfEdge* halfEdge_{ nullptr };

};

using Edges = std::vector<DASEdge*>;
using Faces = std::vector<DASPolygon*>;
using Vertices = std::map<int, DASVertex*>;

class SB_EXPORT DASPolyhedron {

public:

	/* Constructor */
	DASPolyhedron() = default;
	DASPolyhedron(const DASPolyhedron& p);
	DASPolyhedron(const std::string& filename);

	~DASPolyhedron();

	DASPolyhedron& operator=(const DASPolyhedron& other);

	/* Getters */
	Vertices GetVertices() const;
	Vertices GetOriginalVertices() const;
	Faces GetFaces() const;
	Edges GetEdges() const;
	DASVertex* GetVertexById(int id);
	static DASEdge* GetEdgeByVertices(DASVertex* source, DASVertex* target);
	unsigned int* GetIndices() const;

	/* Setters */
	void SetVertices(Vertices vertices, bool isOriginal);
	void SetFaces(Faces faces);
	void SetEdges(Edges edges);

	/* Methods */
	size_t GetNumVertices() const;
	size_t GetNumFaces() const;
	void BuildPolyhedron(const std::map<int, SBPosition3>& vertices, const std::map<int, std::vector<int>>& faces);
	static bool isPLYFile(const std::string& filename);
	void LoadFromPLYFile(const std::string& filename);
	void AddVertex(int id, const SBPosition3& coords);
	void AddFace(int id, const std::vector<int>& vertices);
	std::pair<DASEdge*, double> MinimumEdgeLength();
	std::pair<DASEdge*, double> MaximumEdgeLength();
	static double CalculateEdgeLength(DASEdge* edge);
	static int GetVertexDegree(DASVertex* v);
	static bool IsInFace(DASVertex* v, DASPolygon* f);
	DASHalfEdge* GetHalfEdge(unsigned int id);
	// static ANTPolygon* GetSharedFace(ANTVertex* v, ANTVertex* w);
	static DASHalfEdge* GetHalfEdge(DASVertex* v, DASVertex* w);
	void Scale(double scalingFactor);
	void Center(const SBPosition3& center);
	SBPosition3 GetCenter() const;

protected:

	int FetchNumber(const std::string& st, const std::string& tok) const;
	template <typename T> std::vector<T> SplitString(std::string s, std::string type, T dummy);

private:

	Vertices vertices_;					///< Map ids with Vertex objects
	
	Faces faces_;						///< Map ids with Face objects
	
	Edges edges_;						///< List with the edges
	
	Vertices originalVertices_;			///< Store original value for scaling
	
	unsigned int* indices_{ nullptr };	///< store index list of faces

};

#endif
