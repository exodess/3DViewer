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
			scaleVector_.x, scaleVector_.y, scaleVector_.z);
		TransformMatrix rotationMatrix = TransformMatrixBuilder::CreateRotationMatrix(
			rotationVector_.x, rotationVector_.y, rotationVector_.z);
		TransformMatrix translationMatrix = TransformMatrixBuilder::CreateMoveMatrix(
			translationVector_.x, translationVector_.y, -translationVector_.z * 10.0f);

		return translationMatrix * rotationMatrix * scaleMatrix;
	}

	// ===========================
	// ========= Figure ==========
	// ===========================


	Figure::Figure() noexcept :
	displayType_(DisplayType::WIREFRAME_MODEL),
	material_(Point3D(), 0.5f, 0.0f, 0.5f, 0.5f, 0.5f) {}

	Figure::Figure(
		const std::string& path,
		const std::vector<Vertex>& vertices,
		const std::vector<Surface>& surfaces) noexcept :
	name_(path),
	vertices_{vertices},
	surfaces_{surfaces},
	displayType_(DisplayType::WIREFRAME_MODEL),
	material_(Point3D(), 0.5f, 0.5f, 0.5f, 0.5f, 0.5f) {}

	const std::string& Figure::path() noexcept {
		return name_;
	}


	const std::vector<Vertex>& Figure::getVertices() const noexcept {
		return vertices_;
	}

	const std::vector<Surface>& Figure::getSurfaces() const noexcept {
		return surfaces_;
	}

	DisplayType &Figure::displayType() noexcept {
		return displayType_;
	}

	EdgeInfo &Figure::edgeInfo() noexcept {
		return edgInfo_;
	}

	VertexInfo &Figure::vertexInfo() noexcept {
		return vertInfo_;
	}

	MaterialData &Figure::material() noexcept {
		return material_;
	}

	// ===========================
	// ========= Camera ==========
	// ===========================

	Camera::Camera() noexcept : type_(ProjectionType::ORTHOGRAPHIC) {}

	TransformMatrix Camera::getProjectionMatrix(float aspect) noexcept {

		TransformMatrix resultMatrix;

		if (type_ == ProjectionType::ORTHOGRAPHIC) {
			float size = 2.0f;
			resultMatrix = TransformMatrixBuilder::CreateOrthographicMatrix(
				  -size * aspect, size * aspect, -size, size, -100.0f, 100.0f);
		}
		else {
			resultMatrix = TransformMatrixBuilder::CreatePerspectiveMatrix(
				  45.0f, aspect, 0.1f, 100.0f);
		}

		return resultMatrix;
	}

	ProjectionType& Camera::projectionType() noexcept {
		return type_;
	}

	CameraData Camera::getData(float aspect) noexcept {
		CameraData data;

		TransformMatrix projectionMatrix = getProjectionMatrix(aspect);
		TransformMatrix viewMatrix = getModelMatrix();

		for (auto i = 0; i < 4; ++i) {
			for (auto j = 0; j < 4; ++j) {
				data.projectionMatrix_[i][j] = projectionMatrix(i, j);
				data.viewMatrix_[i][j] = viewMatrix(i, j);
			}
		}

		data.position_ = translation();

		return data;
	}

	// ===========================
	// ========== Scene ==========
	// ===========================

	Scene::Scene() noexcept : backColor_(Point3D(1.0, 1.0f, 1.0f)), camera_(Camera()) {
		lights_.push_back(Light());
	}

	void Scene::addFigure(const Figure &figure) noexcept {
		figures_.push_back(figure);
	}

	void Scene::addLight() noexcept {
		lights_.push_back(Light());
	}

	Light &Scene::getLight(int number) {
		if (number < 1 || number > lights_.size()) {
			throw std::out_of_range("Scene: getLight] This number is out of range: " + number);
		}

		return lights_[number - 1];
	}

	Point3D &Scene::backgroundColor() noexcept {
		return backColor_;
	}

	Figure &Scene::getFigure(int number) {
		if (number < 1 || number > figures_.size()) {
			throw std::out_of_range("[Scene: getFigure] This number is out of range: " + number);
		}

		return figures_[number - 1];
	}

	Camera &Scene::getCamera() noexcept {
		return camera_;
	}

	std::vector<LightData> Scene::getSceneLightsData() noexcept {
		std::vector<LightData> scene_lights;

		for (auto i = 0; i < countLights(); ++i) {
			auto data = getLight(i + 1).getData();

			scene_lights.push_back(data);
		}

		return scene_lights;
	}


	int Scene::countVertices() noexcept {
		int count = 0;

		for (auto& f : figures_) {
			count += f.getVertices().size();
		}

		return count;
	}

	int Scene::countSurfaces() noexcept {
		int count = 0;

		for (auto& f : figures_) {
			count += f.getSurfaces().size();
		}

		return count;
	}

	int Scene::countFigures() noexcept {
		return figures_.size();
	}

	int Scene::countLights() noexcept {
		// 1 Фоновое освещение + (n - 1) направленных источников
		return lights_.size();
	}

	// ===========================
	// ========== Light ==========
	// ===========================

	Light::Light() noexcept : intensity_(LIGHT_DEFAULT_INTENSITY), color_(Point3D(1.0f, 1.0f, 1.0f)) {}

	Point3D &Light::position() noexcept {
		return position_;
	}

	float &Light::intensity() noexcept {
		return intensity_;
	}

	Point3D &Light::color() noexcept {
		return color_;
	}

	LightData Light::getData() noexcept {
		return LightData( color_, position_, intensity_ );
	}


}
