#include <glad/glad.h>
#include "viewer/graphic_spec.h"
#include <sstream>

namespace viewer {

	std::map<std::string, int32_t> ShaderProgram::handler_location_;

	// ==================================================
	// =============== ПУБЛИЧНЫЕ МЕТОДЫ =================
	// ==================================================

	ShaderProgram::ShaderProgram(const std::string& vert_shader_path,
	                             const std::string& geom_shader_path,
	                             const std::string& frag_shader_path) {
		GLuint vertexShader, geometryShader{}, fragmentShader;
		std::string strVertexCode, strGeometryCode = "", strFragmentCode;

		std::cout << "Чтение файла вершинного шейдера" << std::endl;
		strVertexCode = readShaderSource(vert_shader_path);
		if (!geom_shader_path.empty()) {
			std::cout << "Чтение файла геометрического шейдера" << std::endl;
			strGeometryCode = readShaderSource(geom_shader_path);
		}
		std::cout << "Чтение файла фрагментарного шейдера" << std::endl;
		strFragmentCode = readShaderSource(frag_shader_path);

		const char* vShaderCode = strVertexCode.c_str();
		const char* gShaderCode = strGeometryCode.c_str();
		const char* fShaderCode = strFragmentCode.c_str();

		std::cout << "Создаем вершинный шейдер" << std::endl;
		vertexShader = createShader(vShaderCode, GL_VERTEX_SHADER);
		if (!geom_shader_path.empty()) {
			std::cout << "Создаем геометрический шейдер" << std::endl;
			geometryShader = createShader(gShaderCode, GL_GEOMETRY_SHADER);
		}
		std::cout << "Создаем фрагментарный шейдер" << std::endl;
		fragmentShader = createShader(fShaderCode, GL_FRAGMENT_SHADER);

		std::cout << "Создаем шейдерную программу и сохраняем ее в приватном поле класса" << std::endl;
		if (!geom_shader_path.empty()) {
			ProgramID_ = createShaderProgram(vertexShader, geometryShader, fragmentShader);
		}
		else {
			ProgramID_ = createShaderProgram(vertexShader, fragmentShader);
		}

		use();
		std::cout << "Сохраняем индексы всех uniform, используемых в шейдерной программе" << std::endl;
	}

	ShaderProgram::~ShaderProgram() {
		glDeleteProgram(ProgramID_);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_PROGRAM_POINT_SIZE);
		glDisable(GL_BLEND);
	}

	void ShaderProgram::use() noexcept {
		glUseProgram(ProgramID_);
	}

	void ShaderProgram::initOpenGLTools() noexcept {
		std::cout << "Инициализация функций OpenGL... ";
		if(gladLoadGL()) std::cout << "+\n";
		else std::cout << "ERROR!\n";

		std::cout << "Включаем поддержки OpenGL для работы с графикой... ";

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthFunc(GL_LESS);

		std::cout << "+\n";
	}

	// ==================================================
	// =============== ПРИВАТНЫЕ МЕТОДЫ =================
	// ==================================================

	GLint ShaderProgram::getUniformLocation(char * name_uniform) noexcept {
		if (handler_location_.contains(name_uniform)) {
			return handler_location_[name_uniform];
		}

		GLint location = glGetUniformLocation(ProgramID_, name_uniform);
		handler_location_[name_uniform] = location;

		return location;
	}

	GLuint ShaderProgram::createShaderProgram(GLuint vertexShader,
	                                          GLuint geometryShader,
	                                          GLuint fragmentShader) {

		std::cout << "\tСоздаем шейдерную программу...";
		GLuint shaderProgram = glCreateProgram();
		std::cout << " +\n";

		std::cout << "\tПрисоединяем собранные ранее шейдеры к программе...";
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, geometryShader);
		glAttachShader(shaderProgram, fragmentShader);
		std::cout << " +\n";

		std::cout << "\tСвязываем шейдеры...";
		glLinkProgram(shaderProgram);
		std::cout << " +\n";

		std::cout << "\tПроверка шейдерной программы...\n";

		GLint success;
		GLchar infoLog[512];

		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if(!success) {

			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		}
		else std::cout << "\tПрограмма успешно скомпилирована!\n";

		std::cout << "\tУдаляем шейдеры после связывания";
		glDeleteShader(vertexShader);
		glDeleteShader(geometryShader);
		glDeleteShader(fragmentShader);
		std::cout << " +\n";

		return shaderProgram;
	}

	GLuint ShaderProgram::createShaderProgram(GLuint vertexShader, GLuint fragmentShader) noexcept {

		std::cout << "\tСоздаем шейдерную программу...";
		GLuint shaderProgram = glCreateProgram();
		std::cout << " +\n";

		std::cout << "\tПрисоединяем собранные ранее шейдеры к программе...";
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		std::cout << " +\n";

		std::cout << "\tСвязываем шейдеры...";
		glLinkProgram(shaderProgram);
		std::cout << " +\n";

		std::cout << "\tПроверка шейдерной программы...\n";

		GLint success;
		GLchar infoLog[512];

		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if(!success) {

			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		}
		else std::cout << "\tПрограмма успешно скомпилирована!\n";

		std::cout << "\tУдаляем шейдеры после связывания";
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		std::cout << " +\n";

		return shaderProgram;

	}

	std::string ShaderProgram::readShaderSource(const std::string& path) const noexcept {

		std::ifstream shaderFile;

		shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try {
			std::cout << "\tСчитываем исходный код шейдера из файла\n";
			std::stringstream fileStream;

			shaderFile.open(path);

			fileStream << shaderFile.rdbuf();
			shaderFile.close();

			return fileStream.str();

		}

		catch(std::ifstream::failure& e) {

			std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: \"" << \
				 path << "\", " << e.what() << std::endl;
			return "";
		}

	}

	GLuint ShaderProgram::createShader(const char* sourceShaderCode, GLenum shaderType) const noexcept {

		// Данный метод создает и компилирует шейдеры

		std::cout << "\tСоздаем шейдер...";
		GLuint shader = glCreateShader(shaderType);
		std::cout << " +\n";

		std::cout << "\tЗагружаем исходный код шейдера в память графического устройства...";
		glShaderSource(shader, 1, &sourceShaderCode, NULL);
		std::cout << " +\n";

		std::cout << "\tКомпиляция шейдера";
		glCompileShader(shader);
		std::cout << " +\n";

		std::cout << "\tПроверяем на ошибки компиляции\n";
		GLint success;
		GLchar infoLog[512];

		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if(!success) {

			std::string nameShader = "[UNKNOW_SHADER_NAME]";
			if(shaderType == GL_VERTEX_SHADER) nameShader = "VERTEX";
			else if(shaderType == GL_GEOMETRY_SHADER) nameShader = "GEOMETRY";
			else if(shaderType == GL_FRAGMENT_SHADER) nameShader = "FRAGMENT";

			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::" << nameShader << "::COMPILATION_FAILED\n" << infoLog << std::endl;

		}
		else std::cout << "\tШейдер успешно скомпилировался!\n";

		return shader;
	}
	
}
