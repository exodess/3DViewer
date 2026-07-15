#ifndef VIEWER_GRAPHIC_SPECIFICATION_H
#define VIEWER_GRAPHIC_SPECIFICATION_H

/**
@file graphic_spec.h
@brief В этом заголовочном файле описаны вспомогательные классы и константы, 
которые используются при обработке 3D графики OpenGL в коде viewer/opengl
@author Versiese
@date 2026-03-13
*/
#include "viewer_spec.h"
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
@brief Имя uniform переменной в вершинном шейдере для загрузки матрицы трансформации вектора нормали вершины
*/
#define UNIFORM_NORMAL_MATRIX "u_normalMatrix"

/**
@brief Имя uniform переменной в геометрическом шейдере для загрузки соотношения сторон экрана
*/
#define UNIFORM_ASPECT_RATIO "u_aspectRatio"

/**
@brief Имя uniform переменной в фрагментарном шейдере для загрузки режима отображения фигуры
*/
#define UNIFORM_DISPLAY_TYPE "u_displayType"

/**
 * @brief Имя uniform переменной в фрагментарном шейдере для загрузки количества направленных источников освещениия на сцене.
 * Используется в шейдерной программе для отрисовки с плоским затенением
 */
#define UNIFORM_FLAT_ACTIVE_LIGHTS "u_activeFlatPointLights"

/**
 * @brief Имя uniform переменной в фрагментарном шейдере для загрузки количества направленных источников освещениия на сцене.
 * Используется в шейдерной программе для отрисовки с мягким затенением
 */
#define UNIFORM_SMOOTH_ACTIVE_LIGHTS "u_activeSmoothPointLights"

/**
 * @brief Имя uniform переменной в фрагментарном шейдере для загрузки фонового цвета сцены
 */
#define UNIFORM_BACKGROUND_COLOR "u_backgroundColor"

/**
@brief Классы для управления логикой проекта реализованы внутри пространства имен viewer
*/
namespace viewer {

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
	поэтому вызывать эту функцию необходимо только один раз. Отслеживание этого происходит в handler_location
	*/
	class ShaderProgram {

	private:
		static std::map<std::string, int32_t> handler_location_;
		uint32_t ProgramID_; ///< Идентификатор шейдерной программы

		/**
		@brief Считывания исходного кода шейдера для его последующей компиляции
		@note Исходный код шейдеров расположены в папке shaders
		@param path Путь до файла с кодом шейдера
		@return Исходный код в виде одной строки
		*/
		std::string readShaderSource(const std::string& path) const noexcept;

		/**
		@brief Создание и компиляция шейдера
		@param sourceShaderCode Исходный код шейдера в виде C-строки
		@param shaderType Тип шейдера, который необходимо создать (вершинный или фрагментарный)
		@return Идентификатор скомпилированного шейдера
		*/
		uint32_t createShader(const char* sourceShaderCode, uint32_t shaderType) const noexcept;

		/**
		@brief Создание шейдерной программы, присоединение к ней скомпилированных ранее шейдеров
		@note Шейдерная программа создается вместе с геометрическим шейдером,
		служит для отрисовки в каркасном режиме
		@param vertexShader Идентификатор скомпилированного вершинного шейдера
		@param geometryShader Идентификатор скомпилированного геометрического шейдера
		@param fragmentShader Идентификатор скомпилированного фрагментарного шейдера
		@return Идентификатор шейдерной программы
		*/
		uint32_t createShaderProgram(uint32_t vertexShader, uint32_t geometryShader, uint32_t fragmentShader) noexcept;

		/**
		 * @brief Создание шейдерной программы, присоединение к ней скомпилированных ранее шейдеров.
		 * @note Шейдерная программа создается без геометрического шейдера,
		 * служит для отрисовки сцены со светом
		 * @param vertexShader Идентификатор скомпилированного вершинного шейдера
		 * @param fragmentShader Идентификатор скомпилированного фрагментарного шейдера
		 * @return Идентификатор шейдерной программы
		 */
		uint32_t createShaderProgram(uint32_t vertexShader, uint32_t fragmentShader) noexcept;

	public:
		/**
		@brief Создание шейдерной программы для ее использования в каркасном режиме отрисовки
		@param vert_shader_path Путь до исходного кода вершинного шейдера
		@param geom_shader_path Путь до исходного кода геометрического шейдера
		@param frag_shader_path Путь до исходного кода фрагментарного шейдера
		@note Шейдерная программа создается один раз в начале программы
		*/
		ShaderProgram(const std::string& vert_shader_path, const std::string& geom_shader_path, const std::string& frag_shader_path);
		~ShaderProgram(); ///< Удаление шейдерной программы

		/**
		 * @brief Инициализирует функции OpenGL и
		 * включает необходимые поддержки для работы с графикой
		 */
		static void initOpenGLTools() noexcept;

		/**
		 * @brief Находит расположение необходимой uniform'ы в шейдерной программе\n
		 * Необходимо для последующей загрузки в uniform'у соотвествующих данных\n
		 * @param name_uniform Имя uniform'ы в шейдерной программ
		 * @return Индекс формы, иначе -1, если такой формы не существует
		 */
		int32_t getUniformLocation(char * name_uniform) noexcept;

		/**
		@brief Указание коду OpenGL использовать нашу созданную шейдерную программу
		*/
		void use() noexcept;
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
	protected:
		uint32_t boID_; ///< Идентификатор объекта
		static std::map<uint32_t, uint32_t> handler_count; ///< Счетчик использования дескриптора

		/**
		@brief Определяет тип буфера
		- Для VBO - GL_ARRAY_BUFFER
		- Для EBO - GL_ELEMENT_ARRAY_BUFFER
		- Для SSBO - GL_SHADER_STORAGE_BUFFER
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
	 * @class SSBO
	 * @brief Класс шейдерного буфера (Shader Storage Buffer Object), который нужен для загрузки данных в буферы хранения,
	 * такие как u_CameraStruct, u_LightStruct, u_ModelStruct в фрагментарном и вершинном шейдерах
	 */
	class SSBO : public BO {
	public:
		/**
		 * @brief Создание пустого шейдерного буфера заданного размера с автоматической привязкой
		 * @param size Необходимый размер буфера
		 * @param binding Привязка буфера
		 */
		SSBO(int size, int binding) noexcept;

		/**
		 * @brief Создание шейдерного буфера заданного размера с автоматической привязкой и загрузка в него данных
		 * @param data Адрес первой ячейки памяти, по которой находятся данные
		 * @param size Размер данных, которые нужно загрузить в буфер
		 * @param binding Привязка uniform буфера
		 */
		SSBO(const void* data, int size, int binding) noexcept;

		/**
		 * @brief Перепривязка буфера
		 * @param binding Необходимая привязка буфера
		 */
		void rebind(int binding) noexcept;

		/**
		 * @brief Загрузка данных в шейдерный буфер
		 * @param mem Адрес первой ячейки памяти, по которой расположены данные
		 * @param size Размер данных, которые нужно загрузить в буфер
		 * @param offset Отступ от начала буфера
		 */
		void loadSub(const void* mem, int size, int offset = 0) noexcept;
	};

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
		SSBO ssboCamera_; ///< Uniform буфер камеры
		SSBO ssboLights_; ///< Uniform буфер источника освещения
		SSBO ssboMaterial_; ///< Uniform буфер материала фигуры

		uint32_t count_vertices_; ///< Количество вершин в фигуре
		uint32_t count_surfaces_; ///< Количество поверхностей в вершине

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
		@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getUniformLocation("UNIFORM_VERTICES_SIZE")
		*/
		bool loadVerticesSize(int32_t u_location, float size) noexcept;

		/**
		@brief Загружает значение цвета вершин фигуры в фрагментарный шейдер
		@param color Пользовательское значение цвета в формате RGB
		@param u_location Идентификатор uniform'ы
		@return true, если uniform'а существует, иначе false
		@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getUniformLocation("UNIFORM_VERTICES_COLOR")
		*/
		bool loadVerticesColor(int32_t u_location, const Point3D& color) noexcept;

		/**
		@brief Загружает режим отображения вершин в фрагментарный шейдер\n
		Метод требует также расположение uniform'ы размера вершины,
		так как при задании	режима NONE размер вершины устанавливается в 0
		@param mode Режим отображения (нет, круг, квадрат)
		@param u_sizelocation Идентификатор uniform'ы u_vertSize
		@return true, если uniform'а существует, иначе false
		@note Идентификатор uniform'ы u_sizelocation передается с помощью метода ShaderProgram::getUniformLocation("UNIFORM_VERTICES_MODE")
		@warning Вызывается строго после loadVerticesSize()!
		*/
		bool loadVerticesMode(int32_t u_sizelocation, int32_t u_modelocation, VerticesMode mode) noexcept;

		/**
		@brief Загружает размер ребер в вершинный шейдер
		@param size Пользовательское значение размера ребер
		@param u_location Идентификатор uniform'ы
		@return true, если uniform'а существует, иначе false
		@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getUniformLocation("UNIFORM_EDGES_SIZE")
		*/
		bool loadEdgesSize(int32_t u_location, float size) noexcept;

		/**
		@brief Загружает значение цвета ребер в фрагментарный шейдер
		@param color Пользовательское значение цвета в формате RGB
		@param u_location Идентификатор uniform'ы
		@return true, если uniform'а существует, иначе false
		@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getUniformLocation("UNIFORM_EDGES_COLOR")
		*/
		bool loadEdgesColor(int32_t u_location, const Point3D& color) noexcept;

		/**
		@brief Загружает режим отображения ребер в фрагментарный шейдер
		@param mode Режим отображения (cплошная линия, пунктирная линия)
		@param u_location Идентификатор uniform'ы u_dashMode
		@return true, если uniform'а существует, иначе false
		@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getUniformLocation("UNIFORM_EDGES_MODE")
		*/
		bool loadEdgesMode(int32_t u_location, EdgesMode mode) noexcept;

		/**
		@brief Загружает матрицу модели в вершинный шейдер\n
		Позволяет изменить положение фигуры в пространстве:
		- Координату
		- Угол поворота
		- Масштаб
		@param u_location Идентификатор uniform'ы
		@param modelMatrix Матрица трансформации 4 * 4
		@return true, если uniform'а существует, иначе false
		@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getUniformLocation(UNIFORM_MODEL_MATRIX)
		*/
		bool loadModelMatrix(int32_t u_location, TransformMatrix modelMatrix) noexcept;

		/**
		 * @brief Преобразовывает матрицу модели в матрицу трансформации вектора нормали вершины и загружает ее в вершинный шейдер\n
		 * Позволяет перевести вектор нормали в глобальные координаты
		 * @param u_location Идентификатор uniform'ы
		 * @param modelMatrix Матрица модели фигуры
		 * @return true, если uniform'а существует, иначе false
		 * @note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getUniformLocation(UNIFORM_NORMAL_MATRIX)
		 * @warning Матрица трансформации имеет размер 3*3, в отличие от остальных матриц!
		 */
		bool loadNormalMatrix(int32_t u_location, TransformMatrix modelMatrix) noexcept;

		/**
		@brief Загружает соотношение экрана в геометрический шейдер\n
		Позволяет сохранять масштаб при изменении размеров экрана
		@param u_location Идентификатор uniform'ы
		@param ratio Коэффиициент width / height
		@return true, если uniform'а существует, иначе false
		@note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getUniformLocation(UNIFORM_ASPECT_RATIO)
		*/
		bool loadAspectRatio(int32_t u_location, float ratio) noexcept;

		/**
		 * @brief Задает способ отображения фигуры
		 * @param u_location Идентификатор uniform'ы
		 * @param displayType Тип отображения объекта
		 * @return true, если uniform'а существует, иначе false
		 * @note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getUniformLocation(UNIFORM_DISPLAY_TYPE)
		 */
		bool loadDisplayType(int32_t u_location, DisplayType displayType) noexcept;

		/**
		 * @brief Задает количество направленных источников освещения на сцене
		 * @param u_location Идентификатор uniform'ы
		 * @param count Количество направленных источников освещения
		 * @return true, если uniform'а существует, иначе false
		 * @note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getUniformLocation(UNIFORM_ACTIVE_LIGHTS)
		 */
		bool loadCountActiveLight(int32_t u_location, int count) noexcept;

		/**
		 * @brief Задает информацию, отображать ли пол на сцене
		 * @param u_location Идентификатор uniform'ы
		 * @param is_display Будет ли отрисован пол или нет
		 * @return true, если uniform'а существует, иначе false
		 * @note Идентификатор uniform'ы передается с помощью метода ShaderProgram::getUniformLocation(UNIFORM_DISPLAY_FLOOR)
		 */
		bool loadDisplayFloor(int32_t u_location, bool is_display) noexcept;

		/**
		 * @brief Задает характеристики поверхности фигуры
		 * @param material Визуальные свойства материала, которые влияют на отображение
		 */
		void loadMaterialStructure(const MaterialData& material) noexcept;

		/**
		 * @brief Задает характеристики камеры на сцене
		 * @param cameraInfo Информация о том, как следует отображать сцену относительно пользователя
		 */
		void loadCameraStructure(const CameraData& cameraInfo) noexcept;

		/**
		 * @brief Задает характеристики всех источников освещения на сцене (фоновый и направленные вместе)
		 * @param sceneLightsData Информация об источниках освещения
		 */
		void loadLightStructure(const std::vector<Light>& sceneLightsData) noexcept;

		static void clear() noexcept; ///< Очищает буфер кадра

		/**
		@brief Отрисовка фигуры\n
		@note Вызывается после того, как были загружены все данные в uniform'ы (методы load*)
		*/
		void renderFigure() noexcept;

		/**
		 * @brief Отрисовка пола сцены
		 * @note Внутри себя имлементирует всю логику загрузки,
		 * только перед этим нужно загрузить в фрагментный шейдер
		 * отображение пола через Mesh::loadDisplayFloor()
		 */
		void renderFloor() noexcept;

		/**
		@brief Получение количества обрабатываемых вершин
		для последующего вывода этой информации на экран
		@return Количество вершин в моделе
		*/
		uint32_t countVertices() noexcept;

		/**
		@brief Получение количества обрабатываемых поверхностей
		для последующего вывода этой информации на экран
		@return Количество поверхностей в моделе
		*/
		uint32_t countSurfaces() noexcept;

};

}

#endif
