#ifndef S21_GRAPHIC_SPECIFICATION_H
#define S21_GRAPHIC_SPECIFICATION_H

/**
@file graphicSpec.h
@brief В этом заголовочном файле описаны вспомогательные классы и константы, 
которые используются при обработке 3D графики OpenGL в коде viewer/opengl
@author Versiese
@date 2026-03-13
*/
#include "viewerSpec.h"
#include <iostream>
#include <fstream>
#include <map>
#include <cstdint> // для типов

// Имена uniform переменных для Shader::getUniformLocation()
/**
@brief Имя uniform переменной в фрагментарном шейдере для загрузки цвета вершин
*/
#define UNIFORM_VERTICES_COLOR "u_vertColor"

/**
@brief Имя uniform переменной в геометрическом шейдере для загрузки размера вершин
*/
#define UNIFORM_VERTICES_SIZE "u_vertSize"

/**
@brief Имя uniform переменной в фрагментарном шейдере для загрузки режима отображения вершин
*/
#define UNIFORM_VERTICES_MODE "u_isVertCircle"

/**
@brief Имя uniform переменной в фрагментарном шейдере для загрузки цвета ребер
*/
#define UNIFORM_EDGES_COLOR "u_edgesColor"

/**
@brief Имя uniform переменной в геометрическом шейдере для загрузки размера ребер
*/
#define UNIFORM_EDGES_SIZE "u_edgesSize"

/**
@brief Имя uniform переменной в фрагментарном шейдере для загрузки размера штриха в линии (0 - ребро сплошное)
*/
#define UNIFORM_EDGES_MODE "u_dashSize"

/**
@brief Имя uniform переменной в вершинном шейдере для загрузки матрицы модели
*/
#define UNIFORM_MODEL_MATRIX "u_modelMatrix"

/**
@brief Имя uniform переменной в вершинном шейдере для загрузки матрицы камеры
*/
#define UNIFORM_VIEW_MATRIX "u_viewMatrix"

/**
@brief Имя uniform переменной в вершинном шейдере для загрузки матрицы проекции
*/
#define UNIFORM_PROJECTION_MATRIX "u_projectionMatrix"

/**
@brief Имя uniform переменной в вершинном шейдере для загрузки матрицы трансформации вектора нормали вершины
*/
#define UNIFORM_NORMAL_MATRIX "u_normalMatrix"

/**
@brief Имя uniform переменной в геометрическом шейдере для загрузки соотношения сторон экрана
*/
#define UNIFORM_ASPECT_RATIO "u_aspectRatio"

/**
@brief Имя uniform переменной в фрагментарном шейдере для загрузки цвета источника света
*/
#define UNIFORM_LIGHT_COLOR "u_lightColor"

/**
@brief Имя uniform переменной в вершинном шейдере для загрузки координаты источника света в пространстве
*/
#define UNIFORM_LIGHT_POSITION "u_lightPosition"

/**
@brief Имя uniform переменной в вершинном шейдере для загрузки координаты камеры в пространстве
*/
#define UNIFORM_CAMERA_POSITION "u_cameraPosition"

/**
@brief Имя uniform переменной в фрагментарном шейдере для загрузки режима отображения фигуры
*/
#define UNIFORM_DISPLAY_TYPE "u_displayType"

/**
@brief Классы для управления логикой проекта реализованы внутри пространства имен s21
*/
namespace s21 {

/**
@class ShaderProgram
@brief Данный класс управляет загрузкой шейдеров и шейдерной программы для их последующего использования

Шейдеры, которые используются в логике программы:
- Вершинный шейдер - преобразовывает координаты вершин
- Фрагментарный шейдер - занимается вычислением цветов пикселей

Алгоритм загрузки и компиляции шейдеров может быть представлен в виде следующих этапов:
1. Создание необходимых шейдеров
2. Считывание шейдеров из файла
3. Загрузить исходные тексты в память графического устройства
4. Скомпилировать шейдеры
5. Проверить ошибки компиляции
6. Создать шейдерную программу
7. Скопировать шейдеры в шейдерную программу
8. Проверить шейдерную программу
9. Удалить шейдеры

@note При неудачной попытке генерации шейдерной программы OpenGL будет использовать свою
@note Вызов glGetUniformLocation в методе getUniformLocation() достаточно трудоемок,
поэтому сохраняем идентификатор каждой uniform'ы в приватных полях класса
*/
class ShaderProgram {

private:
	uint32_t ProgramID_; ///< Идентификатор шейдерной программы
	int32_t uLoc_verticesColor_; ///< Идентификатор uniform'ы для загрузки цвета
	int32_t uLoc_verticesSize_; ///< Идентификатор uniform'ы для загрузки размера вершин
	int32_t uLoc_verticesMode_; ///< Идентификатор uniform'ы для загрузки размера вершин
	int32_t uLoc_edgesColor_; ///< Идентификатор uniform'ы для загрузки цвета ребер
	int32_t uLoc_edgesSize_; /// Идентификатор uniform'ы для загрузки размера ребер
	int32_t uLoc_edgesDashSize_; ///< Идентификатор uniform'ы для загрузки размера штриха линии

	int32_t uLoc_modelMatrix_; ///< Идентификатор uniform'ы для загрузки матрицы модели
	int32_t uLoc_viewMatrix_; ///< Идентификатор uniform'ы для загрузки матрицы камеры
	int32_t uLoc_projectionMatrix_; ///< Идентификатор uniform'ы для загрузки матрицы проекции
	int32_t uLoc_normalMatrix_; ///< Идентификатор uniform'ы для загрузки матрицы трансформации вектора нормали
	int32_t uLoc_aspectRatio_; ///< Идентификатор uniform'ы для загрузки соотношения сторон области вывода фигуры

	int32_t uLoc_lightColor_; ///< Идентификатор uniform'ы для загрузки цвета источника света на сцене
	int32_t uLoc_lightPosition_; ///< Идентификатор uniform'ы для загрузки положения источника света на сцене
	int32_t uLoc_cameraPosition_; ///< Идентификатор uniform'ы для загрузки положения камеры на сцене
	int32_t uLoc_displayType_; ///< Идентификатор uniform'ы для загрузки режима отображения модели

	/**
	@brief Считывания исходного кода шейдера для его последующей компиляции
	@note Исходный код шейдеров расположены в папке shaders
	@param path Путь до файла с кодом шейдера
	@return Исходный код в виде одной строки
	*/
	std::string readShaderSource(std::string path);

	/**
	@brief Создание и компиляция шейдера
	@param sourceShaderCode Исходный код шейдера в виде C-строки
	@param shaderType Тип шейдера, который необходимо создать (вершинный или фрагментарный)
	@return Идентификатор скомпилированного шейдера
	*/
	uint32_t createShader(const char* sourceShaderCode, uint32_t shaderType);

	/**
	@brief Создание шейдерной программы, присоединение к ней скомпилированных ранее шейдеров
	@note После присоединения шейдеров к шейдерной программе они удаляются
	@param vertexShader Идентификатор скомпилированного вершинного шейдера
	@param geometryShader Идентификатор скомпилированного геометрического шейдера
	@param fragmentShader Идентификатор скомпилированного фрагментарного шейдера
	@return Идентификатор шейдерной программы
	*/
	uint32_t createShaderProgram(uint32_t vertexShader, uint32_t geometryShader, uint32_t fragmentShader);

	/**
	@brief Находит расположение необходимой uniform'ы в шейдерной программе\n
	Необходимо для последующей загрузки в uniform'у соотвествующих данных\n
	@param name_uniform Имя uniform'ы в шейдерной программ
	@return Индекс формы, иначе -1, если такой формы не существует
	*/
	int32_t getUniformLocation(char * name_uniform) noexcept;

public:
	/**
	@brief Создание шейдерной программы для ее использования в вычислениях над вершинами
	@param vertShaderPath Путь до исходного кода вершинного шейдера
	@param geomShaderPath Путь до исходного кода геометрического шейдера
	@param fragShaderPath Путь до исходного кода фрагментарного шейдера
	@note Шейдерная программа создается один раз в начале программы
	*/
	ShaderProgram(std::string vertShaderPath, std::string geomShaderPath, std::string fragShaderPath);
	~ShaderProgram(); ///< Удаление шейдерной программы
	/**
	@brief Указание коду OpenGL использовать нашу созданную шейдерную программу
	*/
	void use() noexcept;

	/**
	@brief Получает расположение uniform'ы для загрузки пользовательского цвета вершин\n
	Необходимо для метода Mesh::loadVerticesColor()
	@return Индекс формы, иначе -1, если такой формы не существует
	*/
	int32_t getColorVerticesUniformLocation() noexcept;

	/**
	@brief Получает расположение uniform'ы для загрузки пользовательского размера вершин\n
	Необходимо для метода Mesh::loadVerticesSize()
	@return Индекс формы, иначе -1, если такой формы не существует
	*/
	int32_t getSizeVerticesUniformLocation() noexcept;

	/**
	@brief Получает расположение uniform'ы для загрузки режима отображения вершин\n
	Необходимо для метода Mesh::loadVerticesMode()
	@return Индекс формы, иначе -1, если такой формы не существует
	*/
	int32_t getModeVerticesUniformLocation() noexcept;

	/**
	@brief Получает расположение uniform'ы для загрузки пользовательского цвета ребер\n
	Необходимо для метода Mesh::loadEdgesColor()
	@return Индекс формы, иначе -1, если такой формы не существует
	*/
	int32_t getColorEdgesUniformLocation() noexcept;

	/**
	@brief Получает расположение uniform'ы для загрузки размера ребер\n
	Необходимо для метода Mesh::loadEdgesSize()
	@return Индекс формы, иначе -1, если такой формы не существует
	*/
	int32_t getSizeEdgesUniformLocation() noexcept;

	/**
	@brief Получает расположение uniform'ы для загрузки режима отображения ребер\n
	Необходимо для метода Mesh::loadEdgesMode()
	@return Индекс формы, иначе -1, если такой формы не существует
	*/
	int32_t getModeEdgesUniformLocation() noexcept;

	/**
	@brief Получает расположение uniform'ы для загрузки матрицы модели\n
	Необходимо для метода Mesh::loadModelMatrix()
	@return Индекс формы, иначе -1, если такой формы не существует
	*/
	int32_t	getModelMatrixUniformLocation() noexcept;

	/**
	@brief Получает расположение uniform'ы для загрузки матрицы камеры\n
	Необходимо для метода Mesh::loadViewMatrix()
	@return Индекс формы, иначе -1, если такой формы не существует
	*/
	int32_t	getViewMatrixUniformLocation() noexcept;

	/**
	@brief Получает расположение uniform'ы для загрузки матрицы проекции\n
	Необходимо для метода Mesh::loadProjectionMatrix()
	@return Индекс формы, иначе -1, если такой формы не существует
	*/
	int32_t	getProjectionMatrixUniformLocation() noexcept;

	/**
	 * @brief Получает расположение uniform'ы для загрузки матрицы трансформации вектора нормали\n
	 * Необходимо для метода Mesh::loadNormalMatrix()
	 * @return Индекс формы, иначе -1, если такой формы не существует
	 */
	int32_t getNormalMatrixUniformLocation() noexcept;

	/**
	@brief Получает расположение uniform'ы для загрузки соотношения сторон экрана\n
	Необходимо для метода Mesh::loadAspectRatio()
	@return Индекс формы, иначе -1, если такой формы не существует
	*/
	int32_t getAspectRatioUniformLocation() noexcept;

	/**
	 * @brief Получает расположение uniform'ы для загрузки цвета источника света\n
	 * Необходим для метода Mesh::loadLightColor()
	 * @return Индекс формы, иначе -1, если такой формы не существует
	 */
	int32_t getLightColorUniformLocation() noexcept;

	/**
	 * @brief Получает расположение uniform'ы для загрузки координаты источника света в трехмерном пространстве\n
	 * Необходим для метода Mesh::loadLightPosition()
	 * @return Индекс формы, иначе -1, если такой формы не существует
	 */
	int32_t getLightPositionUniformLocation() noexcept;

	/**
	 * @brief Получает расположение uniform'ы для загрузки координаты камеры в трехмерном пространстве\n
	 * Необходим для метода Mesh::loadCameraPosition()
	 * @return Индекс формы, иначе -1, если такой формы не существует
	 */
	int32_t getCameraPositionUniformLocation() noexcept;

	/**
	 * @brief Получает расположение uniform'ы для загрузки режима отображения фигуры на экране\n
	 * Необходим для метода Mesh::loadDisplayType()
	 * @return Индекс формы, иначе -1, если такой формы не существует
	 */
	int32_t getDisplayTypeUniformLocation() noexcept;
};


/**
@class BO
@brief Buffer Object, хранит информацию о вершинах\n
С помощью этого класса реализуются объекты VBO и EBO

- VBO (Vertex Buffer Object) - хранит уникальные вершины
- EBO (Element Buffer Object) - хранит индексы (номера) вершин из VBO в том порядке,
в котором их нужно выводить на экран
*/
class BO {

private:
	uint32_t boID_; ///< Идентификатор объекта
	static std::map<uint32_t, uint32_t> handler_count; ///< Счетчик использования дескриптора

protected:
	/**
	@brief Определяет тип буфера

	- Для VBO - GL_ARRAY_BUFFER
	- Для EBO - GL_ELEMENT_ARRAY_BUFFER
	*/
	uint32_t type_;

public:
	BO(uint32_t type) noexcept; ///< Создает пустой буфер заданного типа
	BO(uint32_t type, const void* mem, int size) noexcept; ///< Создает буфер заданного типа и загружает туда данные
	BO(const BO&) noexcept; ///< Создает копию заданного буфера
	~BO(); ///< уничтожает буфер
	BO& operator=(const BO&) noexcept; ///< Оператор присваивания

	void setAttrib(std::size_t stride, void* offset, int index) noexcept; ///< Устанвливаем атрибуты
	void load(const void* mem, int size, uint32_t mode) noexcept; ///< Загружает данные в буфер
	void use() noexcept; ///< Привязка элементного буфера

}; // class BO


/**
@class VAO
@brief Vertex Array Object,
содержит один или несколько объектов буфера вершин,
предназначен для хранения информации о полном визуализируемом объекте
*/
class VAO {

private:
	uint32_t vaoID_; ///< Дескриптор
	static std::map<uint32_t, uint32_t> handler_count; ///< Счетчик использования дескрипторов

public:
	VAO() noexcept; ///< Создает VAO и активирует его
	VAO(const VAO& copy) noexcept; ///< Конструктор копирования
	VAO& operator=(const VAO& other) noexcept; ///< Оператор присваивания
	~VAO(); ///< Освобождение видеопамяти
	void use() noexcept; ///< Активация VAO
	static void disable() noexcept; ///< Деактивация активного VAO

}; // class VAO

/**
@class VerticesMode
@brief Перечисление для указания режима отображения вершин
*/
enum VerticesMode { NONE = 0, ///< Вершины не отображаются
                    CIRCLE, ///< Вершины представлены кругами
                    SQUARE ///< Вершины представлены квадратами
                  };

/**
@class EdgesMode
@brief Перечисление для указания режима отображения вершин
*/
enum EdgesMode { SOLID, ///< Ребра представлены сплошной линией
                 DASHED ///< Ребра представлены пунктирной линией
               };

/**
 * @class DisplayType
 * @brief Перечисление для указания типа отображения фигуры
 */
enum DisplayType {
	WIREFRAME_MODEL = 0, ///< Каркасная модель
	FLAT_SHADING_MODEL, ///< Плоское затенение
	SMOOTH_SHADING_MODEL, ///< Мягкое затенение
};
/**
@class Mesh
@brief Управляет созданием буферов для OpenGL и загрузкой в них данных для изменения фигуры\n

Инициализируется и используется в классе Viewer (пока что)\n

Алгоритм работы:
1. Инициализируем буферы VAO и VBO
2. Загружаем в них начальные координаты фигуры
3. Изменяем цвет/положение/масштаб посредством загрузки в шейдеры соответствующей информации

Какие настройки можно загрузить до отрисовки фигуры:
1. Размер вершин (если 0, значит точки не отображаются) : loadVerticesSize()
2. Цвет вершин (в формате RGB) : loadVerticesColor()
3. Способ отображения вершин (нет, круг или квадрат) : loadVerticesMode()
4. Размер ребер : loadEdgesSize()
5. Цвет ребер (в формате RGB) : loadEdgesColor()
6. Тип ребер (сплошная или пунктирная линия) : loadEdgesMode()

@note При загрузке данных в шейдерную программу, сначала необходимо ее активировать: Shader::use()
*/
class Mesh {

private:
	VAO vao_; ///< Объект
	BO vbo_; ///< Объект буфера вершин для непосредственного хранения вершин
	BO ebo_; ///< Объект буфера индексов

	uint32_t count_vertices_; ///< Количество вершин в фигуре
	uint32_t count_edges_; ///< Количество ребер в вершине

public:
	Mesh() noexcept; ///< Создаем буферы, куда будем загружать данные о вершинах фигуры

	/**
	@brief Загрузка данных о вершинах фигуры\n
	Вершины загружаются в буфер видеокарты следующей структурой:
	- Первые 3 float переменные относятся к координате вершины
	- Вторые 3 float переменные относятся к вектору нормали
	|  vX | vY | vZ  | nX | nY | nZ |
	| <-координата-> |  <-нормаль-> |
	@param figure Считанная из файла фигура
	*/
	void loadData(const Figure& figure) noexcept;

	/**
	@brief Загружает размер вершин в вершинный шейдер
	@param size Пользовательское значение размера вершин
	@param u_location Идентификатор uniform'ы
	@return true, если uniform'а существует, иначе false
	@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getSizeVerticesUniformLocation()
	*/
	bool loadVerticesSize(int32_t u_location, float size) noexcept;

	/**
	@brief Загружает значение цвета вершин фигуры в фрагментарный шейдер
	@param r, g, b Пользовательское значение цвета в формате RGB
	@param u_location Идентификатор uniform'ы
	@return true, если uniform'а существует, иначе false
	@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getColorVerticesUniformLocation()
	*/
	bool loadVerticesColor(int32_t u_location, float r, float g, float b) noexcept;

	/**
	@brief Загружает режим отображения вершин в фрагментарный шейдер\n
	Метод требует также расположение uniform'ы размера вершины,
	так как при задании	режима NONE размер вершины устанавливается в 0
	@param mode Режим отображения (нет, круг, квадрат)
	@param u_sizelocation Идентификатор uniform'ы u_vertSize
	@return true, если uniform'а существует, иначе false
	@note Идентификатор uniform'ы u_sizelocation передается с помощью метода ShaderProgram::getSizeVerticesUniformLocation()
	@note Идентификатор uniform'ы u_modelocation передается с помощью метода ShaderProgram::getModeVerticesUniformLocation()
	@warning Вызывается строго после loadVerticesSize()!
	*/
	bool loadVerticesMode(int32_t u_sizelocation, int32_t u_modelocation, VerticesMode mode) noexcept;

	/**
	@brief Загружает размер ребер в вершинный шейдер
	@param size Пользовательское значение размера ребер
	@param u_location Идентификатор uniform'ы
	@return true, если uniform'а существует, иначе false
	@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getSizeEdgesUniformLocation()
	*/
	bool loadEdgesSize(int32_t u_location, float size) noexcept;

	/**
	@brief Загружает значение цвета ребер в фрагментарный шейдер
	@param r, g, b Пользовательское значение цвета в формате RGB
	@param u_location Идентификатор uniform'ы
	@return true, если uniform'а существует, иначе false
	@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getColorEdgesUniformLocation()
	*/
	bool loadEdgesColor(int32_t u_location, float r, float g, float b) noexcept;

	/**
	@brief Загружает режим отображения ребер в фрагментарный шейдер
	@param mode Режим отображения (cплошная линия, пунктирная линия)
	@param u_location Идентификатор uniform'ы u_dashMode
	@return true, если uniform'а существует, иначе false
	@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getModeEdgesUniformLocation()
	*/
	bool loadEdgesMode(int32_t u_location, EdgesMode mode) noexcept;

	/**
	@brief Загружает матрицу модели в вершинный шейдер\n
	Позволяет изменить положение фигуры в пространстве:
	- Координату
	- Угол поворота
	- Масштаб
	@param u_location Идентификатор uniform'ы
	@param mtrx Матрица трансформации 4 * 4
	@return true, если uniform'а существует, иначе false
	@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getModelMatrixUniformLocation()
	*/
	bool loadModelMatrix(int32_t u_location, float mtrx[4][4]) noexcept;

	/**
	@brief Загружает матрицу камеры в вершинный шейдер\n
	Позволяет изменить положение камеры в пространстве
	@param u_location Идентификатор uniform'ы
	@param mtrx Матрица трансформации 4 * 4
	@return true, если uniform'а существует, иначе false
	@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getViewMatrixUniformLocation()
	*/
	bool loadViewMatrix(int32_t u_location, float mtrx[4][4]) noexcept;

	/**
	@brief Загружает матрицу проекции в вершинный шейдер\n
	Позволяет изменить способ проекции фигуры на экран
	@param u_location Идентификатор uniform'ы
	@param mtrx Матрица трансформации 4 * 4
	@return true, если uniform'а существует, иначе false
	@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getProjectionMatrixUniformLocation()
	*/
	bool loadProjectionMatrix(int32_t u_location, float mtrx[4][4]) noexcept;

	/**
	 * @brief Загружает матрицу трансформации вектора нормали вершины в вершинный шейдер\n
	 * Позволяет перевести вектор нормали в глобальные координаты
	 * @param u_location Идентификатор uniform'ы
	 * @param mtrx Матрица трансформации 3*3
	 * @return true, если uniform'а существует, иначе false
	 * @note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getNormalMatrixUniformLocation()
	 * @warning Матрица трансформации имеет размер 3*3, в отличие от остальных методов!
	 */
	bool loadNormalMatrix(int32_t u_location, float mtrx[3][3]) noexcept;

	/**
	@brief Загружает соотношение экрана в геометрический шейдер\n
	Позволяет сохранять масштаб при изменении размеров экрана
	@param u_location Идентификатор uniform'ы
	@param ratio Коэффиициент width / height
	@return true, если uniform'а существует, иначе false
	@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getAspectRatioUniformLocation()
	*/
	bool loadAspectRatio(int32_t u_location, float ratio) noexcept;

	/**
	 * @brief Загружает цвет источника света в фрагментарный шейдер\n
	 * Позволяет изменять цвет освещения фигуры на сцене
	 * @param u_location Идентификатор uniform'ы
	 * @param r, g, b Пользовательский цвет источника света в формате RGB
	 * @return true, если uniform'а существует, иначе false
	 * @note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getLightColorUniformLocation()
	 */
	bool loadLightColor(int32_t u_location, float r, float g, float b) noexcept;

	/**
	 * @brief Загружает координаты источника света в вершинный шейдер\n
	 * Позволяет изменять положение освещения фигуры
	 * @param u_location Идентификатор uniform'ы
	 * @param position Координата источника света в формате (X, Y, Z)
	 * @return true, если uniform'а существует, иначе false
	 * @note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getLightPositionUniformLocation()
	 */
	bool loadLightPosition(int32_t u_location, float x, float y, float z) noexcept;

	/**
	 * @brief Загружает координаты камеры в вершинный шейдер\n
	 * Позволяет изменять положения просмотра сцены
	 * @param u_location Идентификатор uniform'ы
	 * @param position Координата источника света в формате (X, Y, Z)
	 * @return true, если uniform'а существует, иначе false
	 * @note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getCameraPositionUniformLocation()
	 */
	bool loadCameraPosition(int32_t u_location, float x, float y, float z) noexcept;


	/**
	 * @brief Задает способ отображения фигуры
	 * @param u_location Идентификатор uniform'ы
	 * @param displayType Тип отображения объекта
	 * @return true, если uniform'а существует, иначе false
	 * @note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getDisplayTypeUniformLocation()
	 */
	bool loadDisplayType(int32_t u_location, DisplayType displayType) noexcept;

	/**
	@brief Отрисовка фигуры\n
	@note Вызывается после того, как были загружены все данные в uniform'ы (методы load*)
	*/
	void render() noexcept;

	/**
	@brief Получение количества обрабатываемых вершин
	для последующего вывода этой информации на экран
	@return Количество вершин в моделе
	*/
	uint32_t countVertices() noexcept;

	/**
	@brief Получение количества обрабатываемых вершин
	для последующего вывода этой информации на экран
	@return Количество ребер в моделе
	*/
	uint32_t countEdges() noexcept;

};

} // namespace s21

#endif // S21_GRAPHIC_SPECIFICATION_H
