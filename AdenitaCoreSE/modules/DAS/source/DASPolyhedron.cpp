#include "DASPolyhedron.hpp"

#include <algorithm>
#include <cmath>
#include <set>
#include <sstream>
#include <tuple>

namespace {

constexpr double PLY_VERTEX_WELD_TOLERANCE = 1.0e-5;

struct PLYVertexRecord {
	double x{ 0.0 };
	double y{ 0.0 };
	double z{ 0.0 };
};

struct PLYProperty {
	bool isList{ false };
	std::string name;
};

struct PLYHeader {
	int vertexCount{ 0 };
	int faceCount{ 0 };
	unsigned int dataStartLine{ 0 };
	int vertexXProperty{ -1 };
	int vertexYProperty{ -1 };
	int vertexZProperty{ -1 };
	int vertexPropertyCount{ 0 };
	int faceVertexListProperty{ -1 };
	bool isAscii{ false };
	std::vector<PLYProperty> faceProperties;
};

bool ReadPLYHeader(const std::vector<std::string>& lines, PLYHeader& header) {

	if (lines.empty()) return false;

	std::string firstLine = lines[0];
	boost::trim(firstLine);
	if (firstLine != "ply") return false;

	enum class ElementType { None, Vertex, Face, Other };
	ElementType currentElement = ElementType::None;

	for (unsigned int i = 1; i < lines.size(); ++i) {

		std::string line = lines[i];
		boost::trim(line);
		if (line.empty()) continue;

		std::istringstream stream(line);
		std::string token;
		stream >> token;

		if (token == "comment" || token == "obj_info") continue;

		if (token == "format") {

			std::string format;
			stream >> format;
			header.isAscii = (format == "ascii");
			continue;

		}

		if (token == "element") {

			std::string elementName;
			int elementCount = 0;
			stream >> elementName >> elementCount;

			if (elementName == "vertex") {
				currentElement = ElementType::Vertex;
				header.vertexCount = elementCount;
			}
			else if (elementName == "face") {
				currentElement = ElementType::Face;
				header.faceCount = elementCount;
			}
			else {
				currentElement = ElementType::Other;
			}

			continue;

		}

		if (token == "property") {

			std::string propertyType;
			stream >> propertyType;

			if (currentElement == ElementType::Vertex) {

				std::string propertyName;
				if (propertyType == "list") {
					std::string countType;
					std::string itemType;
					stream >> countType >> itemType >> propertyName;
				}
				else {
					stream >> propertyName;
				}

				if (propertyName == "x") header.vertexXProperty = header.vertexPropertyCount;
				else if (propertyName == "y") header.vertexYProperty = header.vertexPropertyCount;
				else if (propertyName == "z") header.vertexZProperty = header.vertexPropertyCount;
				++header.vertexPropertyCount;

			}
			else if (currentElement == ElementType::Face) {

				PLYProperty property;
				if (propertyType == "list") {
					std::string countType;
					std::string itemType;
					stream >> countType >> itemType >> property.name;
					property.isList = true;
				}
				else {
					stream >> property.name;
				}

				if (property.isList &&
					(property.name == "vertex_indices" || property.name == "vertex_index")) {
					header.faceVertexListProperty = static_cast<int>(header.faceProperties.size());
				}

				header.faceProperties.push_back(property);

			}

			continue;

		}

		if (token == "end_header") {
			header.dataStartLine = i + 1;
			break;
		}

	}

	if (!header.isAscii) return false;
	if (header.vertexCount <= 0 || header.faceCount <= 0) return false;
	if (header.dataStartLine == 0) return false;
	if (header.vertexXProperty < 0 || header.vertexYProperty < 0 || header.vertexZProperty < 0) return false;

	if (header.faceVertexListProperty < 0) {
		for (unsigned int i = 0; i < header.faceProperties.size(); ++i) {
			if (header.faceProperties[i].isList) {
				header.faceVertexListProperty = static_cast<int>(i);
				break;
			}
		}
	}

	return header.faceVertexListProperty >= 0;

}

bool ReadPLYVertex(const std::string& line, const PLYHeader& header, PLYVertexRecord& vertex) {

	std::istringstream stream(line);
	std::vector<double> values;
	double value = 0.0;
	while (stream >> value)
		values.push_back(value);

	const int maxCoordinateProperty = std::max(header.vertexXProperty, std::max(header.vertexYProperty, header.vertexZProperty));
	if (static_cast<int>(values.size()) <= maxCoordinateProperty) return false;

	vertex.x = values[header.vertexXProperty];
	vertex.y = values[header.vertexYProperty];
	vertex.z = values[header.vertexZProperty];

	return true;

}

bool ReadPLYFace(const std::string& line, const PLYHeader& header, std::vector<int>& face) {

	std::istringstream stream(line);
	face.clear();

	for (unsigned int i = 0; i < header.faceProperties.size(); ++i) {

		const PLYProperty& property = header.faceProperties[i];
		if (property.isList) {

			int count = 0;
			if (!(stream >> count)) return false;
			if (count < 0) return false;

			std::vector<int> values;
			values.reserve(static_cast<size_t>(count));
			for (int j = 0; j < count; ++j) {
				int value = 0;
				if (!(stream >> value)) return false;
				values.push_back(value);
			}

			if (static_cast<int>(i) == header.faceVertexListProperty) face = values;

		}
		else {

			std::string ignoredValue;
			if (!(stream >> ignoredValue)) return false;

		}

	}

	return face.size() >= 3;

}

std::tuple<long long, long long, long long> GetPLYVertexWeldKey(const PLYVertexRecord& vertex) {

	return std::make_tuple(
		static_cast<long long>(std::llround(vertex.x / PLY_VERTEX_WELD_TOLERANCE)),
		static_cast<long long>(std::llround(vertex.y / PLY_VERTEX_WELD_TOLERANCE)),
		static_cast<long long>(std::llround(vertex.z / PLY_VERTEX_WELD_TOLERANCE)));

}

double DistanceSquared(const PLYVertexRecord& lhs, const PLYVertexRecord& rhs) {

	const double dx = lhs.x - rhs.x;
	const double dy = lhs.y - rhs.y;
	const double dz = lhs.z - rhs.z;
	return dx * dx + dy * dy + dz * dz;

}

SBPosition3 ToSBPosition(const PLYVertexRecord& vertex) {

	SBPosition3 position;
	position[0] = SBQuantity::angstrom(vertex.x);
	position[1] = SBQuantity::angstrom(vertex.y);
	position[2] = SBQuantity::angstrom(vertex.z);
	return position;

}

bool BuildWeldedPLYData(const std::vector<PLYVertexRecord>& rawVertices,
	const std::vector<std::vector<int>>& rawFaces,
	std::map<int, SBPosition3>& vertices,
	std::map<int, std::vector<int>>& faces) {

	vertices.clear();
	faces.clear();

	std::vector<PLYVertexRecord> uniqueVertices;
	std::vector<int> vertexRemap(rawVertices.size(), -1);
	std::map<std::tuple<long long, long long, long long>, std::vector<int>> vertexBins;
	const double toleranceSquared = PLY_VERTEX_WELD_TOLERANCE * PLY_VERTEX_WELD_TOLERANCE;

	for (unsigned int i = 0; i < rawVertices.size(); ++i) {

		const PLYVertexRecord& vertex = rawVertices[i];
		const auto key = GetPLYVertexWeldKey(vertex);

		int duplicateVertexId = -1;
		for (int dx = -1; dx <= 1 && duplicateVertexId < 0; ++dx) {
			for (int dy = -1; dy <= 1 && duplicateVertexId < 0; ++dy) {
				for (int dz = -1; dz <= 1 && duplicateVertexId < 0; ++dz) {

					const auto neighborKey = std::make_tuple(
						std::get<0>(key) + dx,
						std::get<1>(key) + dy,
						std::get<2>(key) + dz);

					const auto binIt = vertexBins.find(neighborKey);
					if (binIt == vertexBins.end()) continue;

					for (int candidateId : binIt->second) {
						if (DistanceSquared(vertex, uniqueVertices[candidateId]) <= toleranceSquared) {
							duplicateVertexId = candidateId;
							break;
						}
					}

				}
			}
		}

		if (duplicateVertexId >= 0) {
			vertexRemap[i] = duplicateVertexId;
			continue;
		}

		const int uniqueVertexId = static_cast<int>(uniqueVertices.size());
		uniqueVertices.push_back(vertex);
		vertices.insert(std::make_pair(uniqueVertexId, ToSBPosition(vertex)));
		vertexBins[key].push_back(uniqueVertexId);
		vertexRemap[i] = uniqueVertexId;

	}

	int faceId = 0;
	for (const std::vector<int>& rawFace : rawFaces) {

		std::vector<int> weldedFace;
		for (int rawVertexId : rawFace) {

			if (rawVertexId < 0 || static_cast<unsigned int>(rawVertexId) >= vertexRemap.size())
				return false;

			const int weldedVertexId = vertexRemap[rawVertexId];
			if (weldedFace.empty() || weldedFace.back() != weldedVertexId)
				weldedFace.push_back(weldedVertexId);

		}

		if (weldedFace.size() > 1 && weldedFace.front() == weldedFace.back())
			weldedFace.pop_back();

		std::set<int> uniqueFaceVertices(weldedFace.begin(), weldedFace.end());
		if (weldedFace.size() < 3 || uniqueFaceVertices.size() != weldedFace.size())
			continue;

		faces.insert(std::make_pair(faceId, weldedFace));
		++faceId;

	}

	return !vertices.empty() && !faces.empty();

}

}


DASHalfEdge::DASHalfEdge(const DASHalfEdge& other) {

	*this = other;

}

DASHalfEdge& DASHalfEdge::operator=(const DASHalfEdge& other) {

	if (this != &other) {
		id_ = other.id_;
	}
	return *this;

}

DASVertex::DASVertex(const DASVertex& w) {

	*this = w;

}

DASVertex& DASVertex::operator=(const DASVertex& other) {

	if (this != &other) {

		id_ = other.id_;
		position_ = other.position_;

	}
	return *this;

}

/* Copy constructor for ANTVertex
*/
DASVertex::DASVertex(DASVertex* w) {

	id_ = w->id_;
	halfEdge_ = w->halfEdge_;
	position_ = w->position_;

}

/* Destructor for ANTEdge
*/
DASEdge::~DASEdge() {

	// Remove all polygons connected to the edge
	DASPolygon* p1 = nullptr;
	if (halfEdge_ != nullptr && halfEdge_->left_ != nullptr) p1 = halfEdge_->left_;
	DASPolygon* p2 = nullptr;
	DASHalfEdge* pair = halfEdge_ != nullptr ? halfEdge_->pair_ : nullptr;
	if (pair != nullptr && pair->left_ != nullptr) p2 = pair->left_;
	if (p1 != nullptr) delete p1;
	if (p2 != nullptr) delete p2;
	if (pair != nullptr) delete pair;
	delete halfEdge_;

}

DASEdge::DASEdge(const DASEdge& other) {

	*this = other;

}

DASEdge& DASEdge::operator=(const DASEdge& other) {

	if (this != &other) {

		// copy vertices only one vertex
		DASVertex* v1 = new DASVertex(other.halfEdge_->source_);
		// create half edges
		DASHalfEdge* he1 = new DASHalfEdge(*other.halfEdge_);
		he1->source_ = v1;
		he1->edge_ = this;
		halfEdge_ = he1;

	}

	return *this;

}

DASPolygon::DASPolygon(const DASPolygon& p) {

	*this = p;

}

/* Destructor for ANTPolygon
*/
DASPolygon::~DASPolygon() {

	if (halfEdge_ == nullptr) return;

	DASHalfEdge* begin = halfEdge_;
	DASHalfEdge* he = begin;
	do {

		he->left_ = nullptr;
		he = he->next_;

	} while (he != nullptr && he != begin);

}

DASPolygon& DASPolygon::operator=(const DASPolygon& other) {

	if (this != &other) {

		// copy all edges and link them
		DASHalfEdge* begin = other.halfEdge_;
		DASHalfEdge* he = begin;
		DASEdge* prev_e = nullptr;
		DASEdge* first_e = nullptr;
		DASEdge* e = nullptr;
		do {

			e = new DASEdge(*(he->edge_));
			e->halfEdge_->left_ = this;
			if (prev_e != nullptr) {
				e->halfEdge_->prev_ = prev_e->halfEdge_;
				prev_e->halfEdge_->next_ = e->halfEdge_;
			}
			else {
				first_e = e;
			}
			prev_e = e;
			he = he->next_;

		} while (he != begin);

		e->halfEdge_->next_ = first_e->halfEdge_;
		first_e->halfEdge_->prev_ = e->halfEdge_;
		halfEdge_ = first_e->halfEdge_;

	}

	return *this;

}

SBPosition3 DASPolygon::GetCenter() const {

	SBPosition3 cm;
	int c = 0;
	DASHalfEdge* begin = halfEdge_;
	DASHalfEdge* he = begin;
	do {

		cm += he->source_->GetSBPosition();
		++c;
		he = he->next_;

	} while (he != begin);
	cm /= c;

	return cm;

}

std::vector<double> DASVertex::GetVectorCoordinates() const {

	std::vector<double> coords = {
	  position_[0].getValue() * 0.01,
	  position_[1].getValue() * 0.01,
	  position_[2].getValue() * 0.01,
	};
	return coords;

}

const SBPosition3& DASVertex::GetSBPosition() const {
	return position_;
}

void DASVertex::SetCoordinates(const SBPosition3& coordinates) {
	position_ = coordinates;
}

DASPolyhedron::DASPolyhedron(const std::string& filename) {
	LoadFromPLYFile(filename);
}

DASPolyhedron::DASPolyhedron(const DASPolyhedron& p) {
	*this = p;
}

/* Destructor */
DASPolyhedron::~DASPolyhedron() {

	Clear();

}

DASPolyhedron& DASPolyhedron::operator=(const DASPolyhedron& p) {

	if (this != &p) {

		Clear();
		// copy new info
		Faces p_faces = p.GetFaces();
		Vertices p_vertices = p.GetVertices();
		std::map<int, SBPosition3> vertices;
		for (auto& p_vertex : p_vertices) {
			vertices.insert(std::make_pair(p_vertex.first, p_vertex.second->GetSBPosition()));
		}

		std::map<int, std::vector<int>> faces;
		int f_id = 0;
		for (auto& p_face : p_faces) {

			std::vector<int> vs;
			DASHalfEdge* begin = p_face->halfEdge_;
			DASHalfEdge* he = begin;
			do {

				vs.push_back(he->source_->id_);
				he = he->next_;

			} while (he != begin);
			faces.insert(std::make_pair(f_id, vs));
			++f_id;

		}

		BuildPolyhedron(vertices, faces);

	}

	return *this;

}

void DASPolyhedron::Clear() {

	delete[] indices_;
	indices_ = nullptr;

	// Delete all Faces - std::vector<ANTPolygon*>;
	for (auto& it : faces_) {
		delete it;
	}

	// Delete all Edges - std::vector<ANTEdge*>;
	for (auto& it : edges_) {
		delete it;
	}

	// Delete vertices
	for (auto& it : vertices_) {
		delete it.second;
	}

	// Delete all Original Vertices - std::map<int, ANTVertex*>;
	for (auto& it : originalVertices_) {
		delete it.second;
	}

	faces_.clear();
	edges_.clear();
	vertices_.clear();
	originalVertices_.clear();

}

Vertices DASPolyhedron::GetVertices() const {
	return vertices_;
}

Vertices DASPolyhedron::GetOriginalVertices() const {
	return originalVertices_;
}

Faces DASPolyhedron::GetFaces() const {
	return faces_;
}

size_t DASPolyhedron::GetNumVertices() const {
	return vertices_.size();
}

size_t DASPolyhedron::GetNumFaces() const {
	return faces_.size();
}

void DASPolyhedron::SetVertices(Vertices vertices, bool isOriginal = false) {

	vertices_ = vertices;
	if (isOriginal) {

		// make a hard copy of vertices
		for (auto& vertex : vertices) {

			DASVertex* v = vertex.second;
			DASVertex* w = new DASVertex(v);
			originalVertices_.insert(std::make_pair(vertex.first, w));

		}

	}

}

void DASPolyhedron::SetFaces(Faces faces) {
	faces_ = faces;
}

void DASPolyhedron::SetEdges(Edges edges) {
	edges_ = edges;
}

void DASPolyhedron::BuildPolyhedron(const std::map<int, SBPosition3>& vertices, const std::map<int, std::vector<int>>& faces) {

	Clear();

	//create indices for faces
	if (faces.size() > 0) {

		const auto& firstFace = faces.at(0);
		const size_t verticesPerFace = firstFace.size();
		indices_ = new unsigned int[verticesPerFace * faces.size()];

		int k = 0;
		for (const auto& face : faces) {

			const auto& faceIndices = face.second;
			if (faceIndices.size() == 3) {

				indices_[verticesPerFace * k + 0] = faceIndices[0];
				indices_[verticesPerFace * k + 1] = faceIndices[1];
				indices_[verticesPerFace * k + 2] = faceIndices[2];

			}
			++k;

		}

	}

	for (const auto& vertex : vertices) {
		AddVertex(vertex.first, vertex.second);
	}

	// store original vertices for scaling and such
	SetVertices(vertices_, true);

	std::map<std::pair<int, int>, DASEdge*> seen_edges;
	unsigned int he_id = 0;
	unsigned int e_id = 0;
	for (const auto& i : faces) {

		DASPolygon* face = new DASPolygon();
		faces_.push_back(face);
		face->id_ = i.first;

		DASHalfEdge* prev = nullptr;
		DASHalfEdge* first = nullptr;
		const auto& v = i.second;

		for (auto j = v.begin(); j != v.end(); ++j) {

			DASVertex* vertex_i = vertices_.at(*j);
			DASHalfEdge* he = new DASHalfEdge();
			he->id_ = he_id;
			++he_id;
			he->source_ = vertex_i;
			face->halfEdge_ = he;
			he->left_ = face;
			vertex_i->halfEdge_ = he;

			// create DASEdge
			auto next_i = std::next(j);
			if (next_i == v.end()) {
				next_i = v.begin();
			}
			DASEdge* edge = nullptr;
			std::pair<int, int> edge_pair = std::make_pair(*j, *next_i);

			if (*j > *next_i) {
				edge_pair = std::make_pair(*next_i, *j);
			}

			if (seen_edges.find(edge_pair) == seen_edges.end()) {

				// we need to create DASEdge;
				edge = new DASEdge();
				edge->halfEdge_ = he;
				edge->id_ = e_id;
				seen_edges.insert(std::make_pair(edge_pair, edge));
				edges_.push_back(edge);
				++e_id;

			}
			else {

				edge = seen_edges.at(edge_pair);
				DASHalfEdge* pair = edge->halfEdge_;
				he->pair_ = pair;
				pair->pair_ = he;

			}

			he->edge_ = edge;

			// add next and prev half edges
			if (prev != nullptr) {
				he->prev_ = prev;
				prev->next_ = he;
			}
			else {
				// save ref to first half-edge
				first = he;
			}

			prev = he;

		}

		// fix first half-edge
		face->halfEdge_ = first;
		if (first)
			first->prev_ = prev;
		if (prev)
			prev->next_ = first;

	}

}

bool DASPolyhedron::isPLYFile(const std::string& filename) {

	// get file lines
	std::vector<std::string> lines;
	SBIFileReader::getFileLines(filename, lines);

	PLYHeader header;
	if (!ReadPLYHeader(lines, header)) return false;

	return true;

}

void DASPolyhedron::LoadFromPLYFile(const std::string& filename) {

	//get file lines
	std::vector<std::string> lines;
	SBIFileReader::getFileLines(filename, lines);

	PLYHeader header;
	if (!ReadPLYHeader(lines, header)) return;

	const unsigned int startVertices = header.dataStartLine;
	const unsigned int startFaces = startVertices + header.vertexCount;
	const unsigned int endFaces = startFaces + header.faceCount;
	if (lines.size() < endFaces) return;

	// Fetch vertices
	std::vector<PLYVertexRecord> rawVertices;
	rawVertices.reserve(static_cast<size_t>(header.vertexCount));
	for (unsigned int i = startVertices; i < startFaces; i++) {

		PLYVertexRecord vertex;
		if (!ReadPLYVertex(lines[i], header, vertex)) return;
		rawVertices.push_back(vertex);

	}

	// Fetch faces
	std::vector<std::vector<int>> rawFaces;
	rawFaces.reserve(static_cast<size_t>(header.faceCount));
	for (unsigned int i = startFaces; i < endFaces; i++) {

		std::vector<int> face;
		if (!ReadPLYFace(lines[i], header, face)) return;
		rawFaces.push_back(face);

	}

	std::map<int, SBPosition3> vertices;
	std::map<int, std::vector<int>> faces;
	if (!BuildWeldedPLYData(rawVertices, rawFaces, vertices, faces)) return;

	BuildPolyhedron(vertices, faces);

}

int DASPolyhedron::FetchNumber(const std::string& st, const std::string& tok) const {

	int num = 0;
	size_t tok_size = tok.length();
	size_t pos = st.find(tok);
	if (pos != std::string::npos) {

		std::string n = st.substr(pos + tok_size);
		num = atoi(n.c_str());

	}

	return num;

}

template <typename T>
std::vector<T> PushToVector(std::vector<T> v, std::string s, std::string type, size_t i, size_t substring_length) {

	std::string sub_s = s.substr(i, substring_length);
	T elem;
	if (type == "double") {
		elem = atof(sub_s.c_str());
	}
	else if (type == "int") {
		elem = atoi(sub_s.c_str());
	}
	v.push_back(elem);

	return v;

}

template <typename T>
std::vector<T> DASPolyhedron::SplitString(std::string s, std::string type, T dummy) {

	// dummy is just a dummy variable to infer the type
	boost::trim(s);
	std::vector<T> v;
	std::string delimiter = " ";
	size_t pos = s.find(delimiter);
	size_t i = 0;
	bool more = true;

	while (more) {

		v = PushToVector(v, s, type, i, pos - i);
		i = ++pos;
		pos = s.find(delimiter, pos);

		if (pos > s.size()) {
			v = PushToVector(v, s, type, i, s.length());
			more = false;
		}

	}

	return v;

}

void DASPolyhedron::AddVertex(int id, const SBPosition3& coords) {

	auto* vertex = new DASVertex();
	vertex->id_ = id;
	vertex->SetCoordinates(coords);
	vertex->halfEdge_ = nullptr;
	vertices_.insert(std::make_pair(id, vertex));

}

void DASPolyhedron::AddFace(int id, const std::vector<int>& vertices) {

	auto* face = new DASPolygon();
	face->id_ = id;
	DASHalfEdge* prev = nullptr;
	DASHalfEdge* first = nullptr;
	for (auto j = vertices.begin(); j != vertices.end(); ++j) {

		DASVertex* vertex_i = GetVertexById(*j);
		// Next vertex
		auto k = std::next(j);
		if (k == vertices.end()) {
			k = vertices.begin();
		}

		auto* he = new DASHalfEdge();
		he->source_ = vertex_i;
		he->left_ = face;
		face->halfEdge_ = he;
		vertex_i->halfEdge_ = he;
		// add next and prev half edges
		if (prev != nullptr) {
			he->prev_ = prev;
			prev->next_ = he;
		}
		else {
			// save ref to first half-edge
			first = he;
		}
		prev = he;

		DASVertex* vertex_k = GetVertexById(*k);
		DASEdge* edge = nullptr;
		bool new_edge = true;
		for (auto& eit : edges_) {

			if (eit == nullptr || eit->halfEdge_ == nullptr || eit->halfEdge_->next_ == nullptr) continue;

			if (eit->halfEdge_->source_ == vertex_i && eit->halfEdge_->next_->source_ == vertex_k) {

				edge = eit;
				he->pair_ = edge->halfEdge_;
				if (edge->halfEdge_ != nullptr) edge->halfEdge_->pair_ = he;
				new_edge = false;
				break;

			}

			if (eit->halfEdge_->source_ == vertex_k && eit->halfEdge_->next_->source_ == vertex_i) {

				edge = eit;
				he->pair_ = edge->halfEdge_;
				if (edge->halfEdge_ != nullptr) edge->halfEdge_->pair_ = he;
				new_edge = false;
				break;

			}

		}

		if (new_edge) {

			edge = new DASEdge();
			edge->halfEdge_ = he;
			edge->id_ = static_cast<int>(edges_.size());
			edges_.push_back(edge);

		}

		he->edge_ = edge;

	}

	// fix first half-edge
	face->halfEdge_ = first;
	if (first != nullptr) first->prev_ = prev;
	if (prev != nullptr) prev->next_ = first;
	faces_.push_back(face);

}

DASVertex* DASPolyhedron::GetVertexById(int id) {

	return vertices_.at(id);

}

std::pair<DASEdge*, double> DASPolyhedron::MinimumEdgeLength() {

	double min_length = 1e13;
	DASEdge* min_edge = nullptr;

	for (auto& edge : edges_) {

		double length = CalculateEdgeLength(edge);
		if (length < min_length) {

			min_length = length;
			min_edge = edge;

		}

	}

	return std::make_pair(min_edge, min_length);

}

std::pair<DASEdge*, double> DASPolyhedron::MaximumEdgeLength() {

	double max_length = 0.0;
	DASEdge* max_edge = nullptr;

	for (auto& edge : edges_) {

		double length = CalculateEdgeLength(edge);
		if (length > max_length) {

			max_length = length;
			max_edge = edge;

		}

	}

	return std::make_pair(max_edge, max_length);

}

double DASPolyhedron::CalculateEdgeLength(DASEdge* edge) {

	if (edge == nullptr) return 0.0;
	if (edge->halfEdge_ == nullptr) return 0.0;
	if (edge->halfEdge_->source_ == nullptr) return 0.0;
	if (edge->halfEdge_->pair_ == nullptr) return 0.0;
	if (edge->halfEdge_->pair_->source_ == nullptr) return 0.0;

	std::vector<double> s_c = edge->halfEdge_->source_->GetVectorCoordinates();
	std::vector<double> t_c = edge->halfEdge_->pair_->source_->GetVectorCoordinates();
	auto s = ADNVectorMath::CreateBoostVector(s_c);
	auto t = ADNVectorMath::CreateBoostVector(t_c);

	return ublas::norm_2(t - s);

}

DASEdge* DASPolyhedron::GetEdgeByVertices(DASVertex* source, DASVertex* target) {

	if (!source) return nullptr;
	if (!target) return nullptr;
	if (!source->halfEdge_) return nullptr;

	DASHalfEdge* he = source->halfEdge_;
	std::set<DASHalfEdge*> visited;
	while (he != nullptr && visited.insert(he).second) {

		DASHalfEdge* pair = he->pair_;
		if (pair != nullptr && pair->source_ == target) return he->edge_;
		if (pair == nullptr || pair->next_ == nullptr) break;
		he = pair->next_;

	}

	return nullptr;

}

unsigned int* DASPolyhedron::GetIndices() const {

	return indices_;

}

int DASPolyhedron::GetVertexDegree(DASVertex* v) {

	if (!v) return 0;
	if (!v->halfEdge_) return 0;

	int degree = 0;
	DASHalfEdge* he = v->halfEdge_;
	std::set<DASHalfEdge*> visited;

	while (he != nullptr && visited.insert(he).second) {

		++degree;
		if (he->pair_ == nullptr || he->pair_->next_ == nullptr) break;
		he = he->pair_->next_;

	}

	return degree;

}

Edges DASPolyhedron::GetEdges() const {
	return edges_;
}

bool DASPolyhedron::IsInFace(DASVertex* v, DASPolygon* f) {

	if (!f || !v) return false;
	if (!f->halfEdge_) return false;

	bool is = false;

	DASHalfEdge* begin = f->halfEdge_;
	DASHalfEdge* he = begin;

	do {

		DASVertex* w = he->source_;
		if (w->id_ == v->id_) {
			is = true;
		}
		he = he->next_;

	} while (he != begin);

	return is;

}

DASHalfEdge* DASPolyhedron::GetHalfEdge(unsigned int id) {

	DASHalfEdge* res = nullptr;

	for (auto& vit : vertices_) {

		DASHalfEdge* he = vit.second->halfEdge_;
		std::set<DASHalfEdge*> visited;
		while (he != nullptr && visited.insert(he).second) {

			if (he->id_ == id) {
				res = he;
				break;
			}
			else if (he->pair_ != nullptr && he->pair_->id_ == id) {
				res = he->pair_;
				break;
			}
			if (he->pair_ == nullptr || he->pair_->next_ == nullptr) break;
			he = he->pair_->next_;

		}

	}

	return res;

}

DASHalfEdge* DASPolyhedron::GetHalfEdge(DASVertex* v, DASVertex* w) {

	if (!v || !w) return nullptr;
	if (!v->halfEdge_) return nullptr;

	DASHalfEdge* he = v->halfEdge_;
	std::set<DASHalfEdge*> visited;

	while (he != nullptr && visited.insert(he).second) {

		if (he->pair_ != nullptr && he->pair_->source_ == w) {
			return he;
		}
		if (he->pair_ == nullptr || he->pair_->next_ == nullptr) break;
		he = he->pair_->next_;

	}

	// v and w do not share an edge
	return nullptr;
	//std::exit(EXIT_FAILURE);

}

void DASPolyhedron::Scale(double scalingFactor) {

	const std::vector<std::vector<double>> sc{
		{ scalingFactor, 0.0, 0.0 },
		{ 0.0, scalingFactor, 0.0 },
		{ 0.0, 0.0, scalingFactor }
	};

	ublas::matrix<double> scaleMatrix = ADNVectorMath::CreateBoostMatrix(sc);
	for (const auto& originalVertex : originalVertices_) {

		std::vector<double> c = originalVertex.second->GetVectorCoordinates();
		ublas::vector<double> cB = ADNVectorMath::CreateBoostVector(c);
		ublas::vector<double> res = ublas::prod(scaleMatrix, cB);
		std::vector<double> r = ADNVectorMath::CreateStdVector(res);
		if (r.size() < 3) continue;
		DASVertex* w = vertices_.at(originalVertex.first);
		const SBPosition3 coords = SBPosition3(SBQuantity::angstrom(r[0]), SBQuantity::angstrom(r[1]), SBQuantity::angstrom(r[2]));
		w->SetCoordinates(coords);

	}

}

void DASPolyhedron::Center(const SBPosition3& center) {

	std::vector<std::vector<double>> sc;

	for (const auto& vertex : vertices_) {

		std::vector<double> coords = vertex.second->GetVectorCoordinates();
		sc.push_back(coords);

	}

	ublas::matrix<double> positions = ADNVectorMath::CreateBoostMatrix(sc);
	ublas::vector<double> cm = ADNVectorMath::CalculateCM(positions);
	std::vector<double> cm_std = ADNVectorMath::CreateStdVector(cm);
	if (cm_std.size() < 3) return;
	const SBPosition3 cm_sb = SBPosition3(SBQuantity::angstrom(cm_std[0]), SBQuantity::angstrom(cm_std[1]), SBQuantity::angstrom(cm_std[2]));
	const SBPosition3 R = center - cm_sb;

	for (auto& vertex : vertices_) {

		SBPosition3 pos = vertex.second->GetSBPosition();
		vertex.second->SetCoordinates(pos + R);

	}

	for (auto& originalVertex : originalVertices_) {

		SBPosition3 pos = originalVertex.second->GetSBPosition();
		originalVertex.second->SetCoordinates(pos + R);

	}

}

SBPosition3 DASPolyhedron::GetCenter() const {

	SBPosition3 cm;
	const size_t numVertices = vertices_.size();

	for (const auto& vertex : vertices_)
		cm += vertex.second->GetSBPosition();

	cm /= static_cast<double>(numVertices);

	return cm;

}
