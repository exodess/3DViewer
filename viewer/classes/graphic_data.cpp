#include <random>

#include "viewer/viewerSpec.h"

// реализация классов Surface, Vertex, Figure

namespace viewer {

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

	Point3D& Vertex::getNormale() noexcept {
		return normale_;
	}

	const Point3D& Vertex::getPosition() const noexcept {
		return position_;
	}

	const Point3D& Vertex::getNormale() const noexcept {
		return normale_;
	}

	// ===========================
	// ===== BaseSceneObject =====
	// ===========================

	Point3D& BaseSceneObject::translation() noexcept {
		return translationVector_;
	}

	Point3D& BaseSceneObject::rotation() noexcept {
		return rotationVector_;
	}

	Point3D &BaseSceneObject::scale() noexcept {
		return scaleVector_;
	}

	TransformMatrix BaseSceneObject::getModelMatrix() noexcept {
		TransformMatrix scaleMatrix = TransformMatrixBuilder::CreateScaleMatrix(
			scaleVector_.x(), scaleVector_.y(), scaleVector_.z());
		TransformMatrix rotationMatrix = TransformMatrixBuilder::CreateRotationMatrix(
			rotationVector_.x(), rotationVector_.y(), rotationVector_.z());
		TransformMatrix translationMatrix = TransformMatrixBuilder::CreateMoveMatrix(
			translationVector_.x(), translationVector_.y(), translationVector_.z());

		return translationMatrix * rotationMatrix * scaleMatrix;
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


	// ===========================
	// ========== Scene ==========
	// ===========================

	Scene::Scene() noexcept {}

	void Scene::addFigure(const Figure &figure) noexcept {
		figures_.push_back(figure);
	}

	Figure &Scene::getFigure(int number) {
		if (number < 1 || number > figures_.size()) {
			throw std::out_of_range("[Scene: getFigure] This number is out of range: " + number);
		}

		return figures_[number - 1];
	}


}
