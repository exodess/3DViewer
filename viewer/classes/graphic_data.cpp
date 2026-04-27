#include <viewer/viewerSpec.h>

// реализация классов Surface, Vertex, Figure

namespace s21 {

// ===========================
// ========== Surface ===========
// ===========================


Surface::Surface(uint32_t ind1, uint32_t ind2, uint32_t ind3) noexcept : indices_(std::vector<uint32_t>{ind1, ind2, ind3}) {}

bool Surface::operator==(const Surface& other) const noexcept {
	return (indices_ == other.indices_);
}

	uint32_t Surface::operator[](int i) const noexcept {
		if (i < 0 || i > 2)
			return 0;

		return indices_[i];
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


Figure::Figure() noexcept : vertices_{}, surfaces_{} {}

Figure::Figure(std::vector<Vertex>& vertices, std::vector<Surface>& surfaces) noexcept
	: vertices_{vertices}, surfaces_{surfaces} {}

const std::vector<Vertex>& Figure::getVertices() const noexcept {
	return vertices_;
}

const std::vector<Surface>& Figure::getSurfaces() const noexcept {
	return surfaces_;
}

void Figure::Transform(const TransformMatrix& mtrx) {
	for(auto it = vertices_.begin(); it != vertices_.end(); ++it) {
		(*it).Transform(mtrx);
	}
	
}

// ===========================
// ========== Scene ==========
// ===========================

Scene::Scene(const Figure& f) noexcept : figure_{f} {}

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
