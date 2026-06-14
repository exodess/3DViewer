#include "viewer/graphicSpec.h"
#include <glad/glad.h>

namespace viewer {
	GLuint Mesh::countVertices() noexcept {
		return count_vertices_;
	}

	GLuint Mesh::countSurfaces() noexcept {
		return count_surfaces_;
	}

	Mesh::Mesh() noexcept :
		vao_{VAO()},
		vbo_{BO(GL_ARRAY_BUFFER)},
		ebo_{BO(GL_ELEMENT_ARRAY_BUFFER)},
		ssboCamera_{SSBO(sizeof(CameraData), 0)},
		ssboLights_{SSBO(sizeof(LightData) * (MAX_POINT_LIGHTS + 1), 1)},
		ssboMaterial_{SSBO(sizeof(MaterialData), 2)},
		count_vertices_{0},
		count_surfaces_{0} {

		std::cout << "[Mesh] Создание объекта класса Mesh\n";
	}

	void Mesh::loadData(const Figure& figure) noexcept {

		auto vertices = figure.getVertices();
		auto surfaces = figure.getSurfaces();

		if (vertices.empty()) return;

		// Создаем плоские буферы исключительно из POD-типов (Plain Old Data)
		std::vector<float> raw_vertices;
		raw_vertices.reserve(vertices.size() * 6);

		for (const auto& v : vertices) {
			raw_vertices.push_back(v.getPosition().x);
			raw_vertices.push_back(v.getPosition().y);
			raw_vertices.push_back(v.getPosition().z);

			raw_vertices.push_back(v.getNormale().x);
			raw_vertices.push_back(v.getNormale().y);
			raw_vertices.push_back(v.getNormale().z);
		}

		std::vector<uint32_t> raw_indices;
		raw_indices.reserve(surfaces.size() * 3);

		for (const auto& s : surfaces) {
			raw_indices.push_back(s[0]);
			raw_indices.push_back(s[1]);
			raw_indices.push_back(s[2]);
		}

		vbo_.load(raw_vertices.data(), raw_vertices.size() * sizeof(float), GL_STATIC_DRAW);

		// Передаем плоский массив uint32_t
		if (!raw_indices.empty()) {
			ebo_.load(raw_indices.data(), raw_indices.size() * sizeof(uint32_t), GL_STATIC_DRAW);
		}

		vao_.use();

		// Первые три переменные в блоке относятся к координатам, смещение 0
		vbo_.setAttrib(6 * sizeof(float), (void*) 0, 0);

		// указываем, как в буфере расположены векторы нормалей
		// Векторы нормалей идут после 3 float переменных, поэтому смещение 3 * sizeof(float)
		vbo_.setAttrib(6 * sizeof(float), (void*) (3 * sizeof(float)), 1);

		// std::cout << "\tСохраняем количество обрабатываемых вершин и ребер:\n";
		count_vertices_ = vertices.size();
		count_surfaces_ = surfaces.size();

		VAO::disable();

		// std::cout << "\tcount_vertices = " << count_vertices_;
		// std::cout << ", count_surfaces = " << count_surfaces_;
		// std::cout << "\n";
	}

	bool Mesh::loadVerticesColor(GLint uniform_location, const Point3D& color) noexcept {

		if(uniform_location == -1) {
			std::cout << "ERROR::SHADER::FRAGMENT_SHADER::UNIFORM_NOT_FOUND: u_vertColor" << std::endl;
			return false;
		}

		glUniform3f(uniform_location, color.x, color.y, color.z);

		return true;
	}

	bool Mesh::loadVerticesSize(GLint uniform_location, float size) noexcept {

		if(uniform_location == -1) {
			std::cout << "ERROR::SHADER::GEOMETRY_SHADER::UNIFORM_NOT_FOUND: u_vertSize" << std::endl;
			return false;
		}

		glUniform1f(uniform_location, size);

		return true;
	}

	bool Mesh::loadVerticesMode(GLint u_sizeloc, GLint u_modeloc, VerticesMode mode) noexcept {
		int out_mode = 0; // будет ли точка отрисовываться как круг
		bool isEmpty = false;

		if(mode == NONE) {
			// размер вершины становится равен 0
			isEmpty = true;
		}
		else if(mode == CIRCLE) {
			out_mode = 1;
		}

		if(u_modeloc == -1) {
			std::cout << "ERROR::SHADER::FRAGMENT_SHADER::UNIFORM_NOT_FOUND: u_isVertCircle" << std::endl;
			return false;
		}
		if (u_sizeloc == -1) {
			std::cout << "ERROR::SHADER::GEOMETRY_SHADER::UNIFORM_NOT_FOUND: u_vertSize" << std::endl;
			return false;
		}

		glUniform1i(u_modeloc, out_mode);
		if(isEmpty) {
			glUniform1f(u_sizeloc, 0.0);
		}

		return true;
	}

	bool Mesh::loadEdgesColor(GLint uniform_location, const Point3D& color) noexcept {

		if(uniform_location == -1) {
			std::cout << "ERROR::SHADER::FRAGMENT_SHADER::UNIFORM_NOT_FOUND: u_edgesColor" << std::endl;
			return false;
		}

		glUniform3f(uniform_location, color.x, color.y, color.z);

		return true;
	}

	bool Mesh::loadEdgesSize(GLint uniform_location, float size) noexcept {

		if(uniform_location == -1) {
			std::cout << "ERROR::SHADER::GEOMETRY_SHADER::UNIFORM_NOT_FOUND: u_edgesSize" << std::endl;
			return false;
		}

		glUniform1f(uniform_location, size);

		return true;
	}

	bool Mesh::loadEdgesMode(GLint uniform_location, EdgesMode mode) noexcept {
		float dash_size = (mode == DASHED) ? 0.05f : 0.0f; // расстояние между штрихами

		if(uniform_location == -1) {
			std::cout << "ERROR::SHADER::FRAGMENT_SHADER::UNIFORM_NOT_FOUND: u_dashSize" << std::endl;
			return false;
		}

		glUniform1f(uniform_location, dash_size);

		return true;
	}

	bool Mesh::loadModelMatrix(GLint uniform_location, TransformMatrix modelMatrix) noexcept {

		if(uniform_location == -1) {
			std::cout << "ERROR::SHADER::VERTEX_SHADER::UNIFORM_NOT_FOUND: u_modelMatrix" << std::endl;
			return false;
		}

		float mtrx[4][4];
		for (auto i = 0; i < 4; ++i) {
			for (auto j = 0; j < 4; ++j) {
				mtrx[i][j] = modelMatrix(i, j);
			}
		}

		glUniformMatrix4fv(uniform_location, 1, GL_TRUE, &mtrx[0][0]);
		return true;
	}

	bool Mesh::loadNormalMatrix(GLint uniform_location, TransformMatrix modelMatrix) noexcept {
		if (uniform_location == -1) {
			std::cout << "ERROR::SHADER::VERTEX_SHADER::UNIFORM_NOT_FOUND: u_normalMatrix" << std::endl;
			return false;
		}

		auto normalMatrix = modelMatrix.inverse().transpose();
		float mtrx[3][3];
		for (auto i = 0; i < 3; ++i) {
			for (auto j = 0; j < 3; ++j) {
				mtrx[i][j] = normalMatrix(i, j);
			}
		}

		glUniformMatrix3fv(uniform_location, 1, GL_TRUE, &mtrx[0][0]);
		return true;
	}


	bool Mesh::loadAspectRatio(GLint uniform_location, float ratio) noexcept {

		if(uniform_location == -1) {
			std::cout << "ERROR::SHADER::GEOMETRY_SHADER::UNIFORM_NOT_FOUND: u_aspectRatio" << std::endl;
			return false;
		}

		glUniform1f(uniform_location, ratio);
		return true;
	}

	bool Mesh::loadDisplayType(int32_t u_location, DisplayType displayType) noexcept {
		if (u_location == -1) {
			std::cout << "ERROR::SHADER::GEOMETRY_SHADER::UNIFORM_NOT_FOUND: u_displayType" << std::endl;
			return false;
		}

		glUniform1i(u_location, static_cast<int>(displayType));
		return true;
	}

	bool Mesh::loadCountActiveLight(int32_t u_location, int count) noexcept {
		if (u_location == -1) {
			std::cout << "ERROR::SHADER::FRAGMENT_SHADER::UNIFORM_NOT_FOUND: u_activePointLights" << std::endl;
			return false;
		}

		glUniform1i(u_location, count);
		return true;
	}

	void Mesh::loadMaterialStructure(const MaterialData &material) noexcept {
		ssboMaterial_.loadSub(&material, sizeof(MaterialData), 0);
	}

	void Mesh::loadCameraStructure(const CameraData &cameraInfo) noexcept {
		ssboCamera_.loadSub(&cameraInfo, sizeof(CameraData), 0);
	}

	void Mesh::loadLightStructure(const std::vector<LightData>& scene_lights_data) noexcept {
		ssboLights_.loadSub(scene_lights_data.data(), sizeof(LightData) * scene_lights_data.size(), 0);
	}

	void Mesh::render() noexcept {
		vao_.use();
		glDrawElements(GL_TRIANGLES, count_surfaces_ * 3, GL_UNSIGNED_INT, nullptr);

		VAO::disable();
	}

	void Mesh::clear() noexcept {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	
}
