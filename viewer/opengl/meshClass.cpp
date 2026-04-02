#include <viewer/graphicSpec.h>
#include <glad/glad.h>

namespace s21 {

GLuint Mesh::countVertices() noexcept {
	return count_vertices_;
}

GLuint Mesh::countEdges() noexcept {
	return count_edges_;
}

Mesh::Mesh() noexcept : vao_{VAO()}, 
                        vbo_{BO(GL_ARRAY_BUFFER)}, 
                        ebo_{BO(GL_ELEMENT_ARRAY_BUFFER)},
                        count_vertices_{0},
                        count_edges_{0} {
	std::cout << "[Mesh] Создание объекта класса Mesh\n";
}

void Mesh::loadData(const Figure& figure) noexcept {

	auto vertices = figure.getVertices();
	auto edges = figure.getEdges();

	if (vertices.empty()) return;

	// Создаем плоские буферы исключительно из POD-типов (Plain Old Data)
	std::vector<float> raw_vertices;
	raw_vertices.reserve(vertices.size() * 3);
	
	for (const auto& v : vertices) {
		raw_vertices.push_back(v.getPosition().x);
		raw_vertices.push_back(v.getPosition().y);
		raw_vertices.push_back(v.getPosition().z);
	}

	std::vector<uint32_t> raw_indices;
	raw_indices.reserve(edges.size() * 2);
	
	for (const auto& e : edges) {
		raw_indices.push_back(e.getBegin()); 
		raw_indices.push_back(e.getEnd());
	}

	vbo_.load(raw_vertices.data(), raw_vertices.size() * sizeof(float), GL_STATIC_DRAW);
	    
	// Передаем плоский массив uint32_t
	if (!raw_indices.empty()) {
		ebo_.load(raw_indices.data(), raw_indices.size() * sizeof(uint32_t), GL_STATIC_DRAW);
	}

	vao_.use();

	// Шаг ровно 12 байт (3 координаты)
	// Смещение 0, так как массив чистый
	vbo_.setAttrib(3 * sizeof(float), (void*) 0);

	std::cout << "\tСохраняем количество обрабатываемых вершин и ребер:\n";
	count_vertices_ = vertices.size();
	count_edges_ = edges.size() * 2;
	
	VAO::disable();
	
	std::cout << "\tcount_vertices = " << count_vertices_ << ", count_edges = " << count_edges_ << std::endl;
}


bool Mesh::loadVerticesColor(GLint uniform_location, float r, float g, float b) noexcept {

	if(uniform_location == -1) {
		std::cout << "ERROR::SHADER::FRAGMENT_SHADER::UNIFORM_NOT_FOUND\n" << std::endl;
		return false;
	}
	
	glUniform3f(uniform_location, r, g, b);

	return true;
}

bool Mesh::loadVerticesSize(GLint uniform_location, float size) noexcept {

	if(uniform_location == -1) {
		std::cout << "ERROR::SHADER::FRAGMENT_SHADER::UNIFORM_NOT_FOUND\n" << std::endl;
		return false;
	}
	
	glUniform1f(uniform_location, size);

	return true;
}

bool Mesh::loadVerticesMode(GLint u_sizeloc, GLint u_modeloc, VerticesMode mode) noexcept {

	std::string message_mode;
	int out_mode = 0; // будет ли точка отрисовываться как круг
	bool isEmpty = false;
	
	if(mode == NONE) {
		// размер вершины становится равен 0
		message_mode = "нет";
		isEmpty = true;
	}
	else if(mode == CIRCLE) {
		message_mode = "круг";
		out_mode = 1;
	}
	else if(mode == SQUARE)
		message_mode = "квадрат";
	else 
		message_mode = "[error_mode]";
		
	if(u_modeloc == -1 || u_sizeloc == -1) {
		std::cout << "ERROR::SHADER::FRAGMENT_SHADER::UNIFORM_NOT_FOUND\n" << std::endl;
		return false;
	}
	
	glUniform1i(u_modeloc, out_mode);
	if(isEmpty) {
		glUniform1f(u_sizeloc, 0.0);
	}

	return true;
}

bool Mesh::loadEdgesColor(GLint uniform_location, float r, float g, float b) noexcept {

	if(uniform_location == -1) {
		std::cout << "ERROR::SHADER::FRAGMENT_SHADER::UNIFORM_NOT_FOUND\n" << std::endl;
		return false;
	}
	
	glUniform3f(uniform_location, r, g, b);

	return true;
}

bool Mesh::loadEdgesSize(GLint uniform_location, float size) noexcept {

	if(uniform_location == -1) {
		std::cout << "ERROR::SHADER::FRAGMENT_SHADER::UNIFORM_NOT_FOUND\n" << std::endl;
		return false;
	}
	
	glUniform1f(uniform_location, size);

	return true;
}

bool Mesh::loadEdgesMode(GLint uniform_location, EdgesMode mode) noexcept {

	std::string message_mode;
	float dash_size = 0.0; // расстояние между штрихами
	
	if(mode == SOLID) 
		message_mode = "сплошная линия";
	else if(mode == DASHED) { 
		message_mode = "пунктирная линия";
		dash_size = 0.05;	
	}
	else 
		message_mode = "[error_mode]";
		
	if(uniform_location == -1) {
		std::cout << "ERROR::SHADER::FRAGMENT_SHADER::UNIFORM_NOT_FOUND\n" << std::endl;
		return false;
	}
	
	glUniform1f(uniform_location, dash_size);

	return true;
}

bool Mesh::loadModelMatrix(GLint uniform_location, float mtrx[4][4]) noexcept {

	if(uniform_location == -1) {
		std::cout << "ERROR::SHADER::VERTEX_SHADER::UNIFORM_NOT_FOUND\n" << std::endl;
		return false;
	}

	glUniformMatrix4fv(uniform_location, 1, GL_TRUE, &mtrx[0][0]);
	return true;
}

bool Mesh::loadViewMatrix(GLint uniform_location, float mtrx[4][4]) noexcept {

	if(uniform_location == -1) {
		std::cout << "ERROR::SHADER::VERTEX_SHADER::UNIFORM_NOT_FOUND\n" << std::endl;
		return false;
	}

	glUniformMatrix4fv(uniform_location, 1, GL_TRUE, &mtrx[0][0]);
	return true;
}

bool Mesh::loadProjectionMatrix(GLint uniform_location, float mtrx[4][4]) noexcept {

	if(uniform_location == -1) {
		std::cout << "ERROR::SHADER::VERTEX_SHADER::UNIFORM_NOT_FOUND\n" << std::endl;
		return false;
	}

	glUniformMatrix4fv(uniform_location, 1, GL_TRUE, &mtrx[0][0]);
	return true;
}

bool Mesh::loadAspectRatio(GLint uniform_location, float ratio) noexcept {

	if(uniform_location == -1) {
		std::cout << "ERROR::SHADER::VERTEX_SHADER::UNIFORM_NOT_FOUND\n" << std::endl;
		return false;
	}

	glUniform1f(uniform_location, ratio);
	return true;
}

void Mesh::render() noexcept {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	vao_.use(); 
	glDrawElements(GL_LINES, count_edges_, GL_UNSIGNED_INT, 0);

	VAO::disable();
}
	
} // namespace s21
