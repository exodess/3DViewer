#include <viewer/viewerSpec.h>

// реализация классов Edge, Vertex, Figure

namespace s21 {

// ===========================
// ========== Edge ===========
// ===========================


Edge::Edge(uint32_t v1, uint32_t v2) noexcept : begin_{v1}, end_{v2} {}

Edge::Edge(const Edge& other) noexcept : begin_{other.begin_}, end_{other.end_} {}

Edge& Edge::operator=(const Edge& other) noexcept {
	begin_ = other.begin_;
	end_ = other.end_;
	return *this;
}

bool Edge::operator==(const Edge& other) const noexcept {
	return (begin_ == other.begin_ && end_ == other.end_);
}

uint32_t Edge::getBegin() const noexcept {
	return begin_;
}

uint32_t Edge::getEnd() const noexcept {
	return end_;
}

// ===========================
// ========= Vertex ==========
// ===========================


Vertex::Vertex(const Point3D& point, const Point3D& normale) noexcept : position_{point}, normale_{normale} {}

Vertex::Vertex(const Vertex& other) noexcept : position_{other.position_}, normale_{other.normale_} {}

Vertex& Vertex::operator=(const Vertex& other) noexcept {
	position_ = other.position_;
	normale_ = other.normale_;
	return *this;
}

bool Vertex::operator==(const Vertex& other) const noexcept {
	return (position_ == other.position_) && (normale_ == other.normale_);
}

void Vertex::setNormals(const Point3D &normale) noexcept {
	normale_ = normale;
}


Point3D& Vertex::getPosition() noexcept {
	return position_;
}

const Point3D& Vertex::getPosition() const noexcept {
	return position_;
}

const Point3D& Vertex::getNormale() const noexcept {
	return normale_;
}

void Vertex::Transform(const TransformMatrix& mtrx) {
	position_ = mtrx.TransformPoint(position_);
}


// ===========================
// ========= Figure ==========
// ===========================


Figure::Figure() noexcept : vertices_{}, edges_{} {}

Figure::Figure(std::vector<Vertex>& vertices, std::vector<Edge>& edges) noexcept
	: vertices_{vertices}, edges_{edges} {}

Figure::Figure(const Figure& other) noexcept : vertices_{other.vertices_}, edges_{other.edges_} {}

Figure& Figure::operator=(const Figure& other) noexcept {
	vertices_ = other.vertices_;
	edges_ = other.edges_;
	return *this;
}

const std::vector<Vertex>& Figure::getVertices() const noexcept {
	return vertices_;
}

const std::vector<Edge>& Figure::getEdges() const noexcept {
	return edges_;
}

void Figure::Transform(const TransformMatrix& mtrx) {
	for(auto it = vertices_.begin(); it != vertices_.end(); ++it) {
		(*it).Transform(mtrx);
	}
	
}

// ===========================
// ========== Scene ==========
// ===========================

Scene::Scene(Figure f) noexcept : figure_{f} {}

Figure& Scene::getFigure() noexcept { 
	return figure_; 
}
const Figure& Scene::getFigure() const noexcept {
	return figure_;
}

void Scene::TransformFigure(const TransformMatrix& mtrx) { 
	figure_.Transform(mtrx); 
}

} // namespace s21
