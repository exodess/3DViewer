#include <glad/glad.h>
#include <viewer/graphicSpec.h>
#include <sstream>
namespace s21 {

// ==================================================
// =============== ПУБЛИЧНЫЕ МЕТОДЫ =================
// ==================================================

GLint ShaderProgram::getColorVerticesUniformLocation() noexcept 
	{ return uLoc_verticesColor_; }

GLint ShaderProgram::getSizeVerticesUniformLocation() noexcept
	{ return uLoc_verticesSize_; }

GLint ShaderProgram::getModeVerticesUniformLocation() noexcept 
	{ return uLoc_verticesMode_; }

GLint ShaderProgram::getColorEdgesUniformLocation() noexcept 
	{ return uLoc_edgesColor_; }

GLint ShaderProgram::getSizeEdgesUniformLocation() noexcept 
	{ return uLoc_edgesSize_; }

GLint	ShaderProgram::getModeEdgesUniformLocation() noexcept 
	{ return uLoc_edgesDashSize_; }

GLint ShaderProgram::getModelMatrixUniformLocation() noexcept 
	{ return uLoc_modelMatrix_; }

GLint ShaderProgram::getViewMatrixUniformLocation() noexcept 
	{ return uLoc_viewMatrix_; }

GLint ShaderProgram::getProjectionMatrixUniformLocation() noexcept 
	{ return uLoc_projectionMatrix_; }

GLint ShaderProgram::getAspectRatioUniformLocation() noexcept
	{ return uLoc_aspectRatio_; }

ShaderProgram::ShaderProgram(std::string vertShaderPath, 
                             std::string geomShaderPath,
                             std::string fragShaderPath) {

	// Читаем код шейдеров и сохраняем их именно в C-строки 
	// 	для нормальной работы OpenGL функций

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
	
	std::cout << "Чтение файла вершинного шейдера" << std::endl;
	std::string strVertexCode = readShaderSource(vertShaderPath);
	std::cout << "Чтение файла геометрического шейдера" << std::endl;
	std::string strGeometryCode = readShaderSource(geomShaderPath);
	std::cout << "Чтение файла фрагментарного шейдера" << std::endl;
	std::string strFragmentCode = readShaderSource(fragShaderPath);

	const char* vShaderCode = strVertexCode.c_str();
	const char* gShaderCode = strGeometryCode.c_str();
	const char* fShaderCode = strFragmentCode.c_str();

	std::cout << "Создаем вершинный шейдер" << std::endl;
	GLuint vertexShader = createShader(vShaderCode, GL_VERTEX_SHADER);
	std::cout << "Создаем геометрический шейдер" << std::endl;
	GLuint geometryShader = createShader(gShaderCode, GL_GEOMETRY_SHADER);
	std::cout << "Создаем фрагментарный шейдер" << std::endl;
	GLuint fragmentShader = createShader(fShaderCode, GL_FRAGMENT_SHADER);

	std::cout << "Создаем шейдерную программу и сохраняем ее в приватном поле класса" << std::endl;
	ProgramID_ = createShaderProgram(vertexShader, geometryShader, fragmentShader);
	use();
	std::cout << "Сохраняем индексы всех uniform, используемых в шейдерной программе" << std::endl;
	uLoc_verticesColor_ = getUniformLocation(UNIFORM_VERTICES_COLOR);
	uLoc_verticesSize_ = getUniformLocation(UNIFORM_VERTICES_SIZE);
	uLoc_verticesMode_ = getUniformLocation(UNIFORM_VERTICES_MODE);
	uLoc_edgesColor_ = getUniformLocation(UNIFORM_EDGES_COLOR);
	uLoc_edgesSize_ = getUniformLocation(UNIFORM_EDGES_SIZE);
	uLoc_edgesDashSize_ = getUniformLocation(UNIFORM_EDGES_MODE);
	
	uLoc_modelMatrix_ = getUniformLocation(UNIFORM_MODEL_MATRIX);
	uLoc_viewMatrix_ = getUniformLocation(UNIFORM_VIEW_MATRIX);
	uLoc_projectionMatrix_ = getUniformLocation(UNIFORM_PROJECTION_MATRIX);
	uLoc_aspectRatio_ = getUniformLocation(UNIFORM_ASPECT_RATIO);
	
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

// ==================================================
// =============== ПРИВАТНЫЕ МЕТОДЫ =================
// ==================================================

GLint ShaderProgram::getUniformLocation(char * name_uniform) noexcept {
	GLint location = glGetUniformLocation(ProgramID_, name_uniform);

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

std::string ShaderProgram::readShaderSource(std::string path) {

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

GLuint ShaderProgram::createShader(const char* sourceShaderCode, GLenum shaderType) {

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
	
} // namespace s21
