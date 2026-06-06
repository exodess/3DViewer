#ifndef VIEWER_SPECIFICATION_H
#define VIEWER_SPECIFICATION_H

#include <vector>
#include <string>
#include <iostream>
#include <cstdint>

/**

@file viewerSpec.h
@author exodess
@date 2026-03-21
@brief Этот файл содержит описание структуры внутренней части программы 3DViewer\n

Классы, описанные в данном файле, осуществляют хранение информации о том, как будет представлены объекты в памяти 
и как ими управлять

Загружаемая фигура поддерживает только список вершин, поверхностей и нормалей\n

- Vertex - координата точки и ее вектор нормали
- Surface - поверхность фигуры (3 вершины)

При проектировании данного проекта были использованы несколько паттернов проектирования:
- классы BaseSceneObject, Figure и Vertex реализуют паттерн Strategy
- Класс Viewer реализует паттерн Facade
- TransformMatrixBuilder реализует паттерн Abstract Factory
	
*/

#include "dataStructures.h"
#include "viewerSpec.h"
#include "viewerSpec.h"

namespace viewer {

	// =====================================
	// ========== Общее положение ==========
	// =====================================

	// Интерфейсы:
	class BaseSceneObject;
	class BaseFileReader;
	class BaseDrawerScene;

	// Основные классы:
	class Viewer;
	class Scene;

	class Camera;
	class Figure;
	class Light;

	class Surface;
	class Vertex;

	class TransformMatrix;
	class TransformMatrixBuilder;
	class FileReader;

	// =====================================
	// ======== Используемые классы ========
	// =====================================

	/**
	 * @class BaseSceneObject
	 * @brief Абстрактный базовый класс, от которого наследуются классы, описывающие объекты на сцене.
	 * От этого класса наследуются Figure и Camera
	*/
	class BaseSceneObject {
	protected:
		Point3D translationVector_; ///< Смещение объекта относительно центра координат
		Point3D rotationVector_; ///< Поворот объекта относительно нулевого угла
		Point3D scaleVector_ = Point3D(1.0f, 1.0f, 1.0f); ///< Масштаб объекта в трехмерном пространстве

	public:
		Point3D& translation() noexcept; ///< Доступ к полю со смещением объекта
		Point3D& rotation() noexcept; ///< Доступ к полю угла вращения объекта
		Point3D& scale() noexcept; ///< Доступ к полю масштаба отображения объекта

		/**
		 * @brief Вычисляет и возвращает результирующую матрицу, которая потом будет загружена в шейдерную программу
		 * @return Матрица модели: Translation * Rotation * Scale
		 */
		TransformMatrix getModelMatrix() noexcept;
	};

	/**
	@class BaseFileReader
	@brief Абстрактный базовый класс, от которого наследуются классы,
	управляющие загрузкой фигуры из файла сохранения на сцену
	*/
	class BaseFileReader {

	public:
		/**
		@brief Класс для загрузки фигуры должен иметь специализированный метод для чтения фигуры
		@param path Путь до файла с фигурой
		@return Готовая к отображению фигура, которая сохраняется в текущей сцене
		*/
		virtual Figure ReadFigure(std::string path) = 0;
		virtual ~BaseFileReader() = default;
	};


	/**
	@class BaseDrawerScene
	@brief Абстрактный базовый класс, от которого наследуется класс для отображения фигуры на экране\n

	Управляет загрузкой всех настроек фигуры в соответствующие буферы OpenGL\n

	На стороне Qt от BaseDrawerScene наследуется класс GLWidget, который управляет в данном проекте отрисовкой фигуры

	Какие приватные члены должны точно быть в классе GLWidget:
	- Mesh* mesh_
	- ShaderProgram* shaderProgram_

	Взаимодействие с этими объектами в классе GLWidget:
	- В конструкторе создается объект класса ShaderProgram
	- После того, как пользователь нажал кнопку для открытия файла с фигурой, вызывается методы viewer.LoadScene()
	 и создается объект класса Mesh. Если он был создан, то удаляется и создается заново (для обновления данных)
	- После этого, когда пользователь будет изменять настройки, для их сохранения в шейдерах вызываются методы mesh_->load*
	 (в зависимости от того, для чего и какие настройки меняются, вызываются соответственный метод класса Mesh)
	- После этого, при каждой смене настроек в методе paintGL() вызывается mesh_->render() для загрузки данных в шейдеры OpenGL
	*/
	class BaseDrawerScene {

	public:
		virtual void DrawScene(Scene*) = 0;
		virtual ~BaseDrawerScene() = default;
	};

	// =====================================
	// ========== TransformMatrix ==========
	// =====================================

	/**
	@class TransformMatrix
	@brief Класс для хранения и взаимодействия матриц аффиного преобразования 4*4\n
	Такие матрицы нужны для изменения положения отдельной вершины (а значит, и фигуры в целом) в пространстве
	*/
	class TransformMatrix {

	private:
		float mtrx_[4][4]; ///< Хранение матрицы 4*4

	public:
		/**
		 * Вычисляет минор - определитель матрицы 3*3, получаемой путем
		 * вычеркивания из исходной матрицы элемента (i, j)
		 * @param i Индекс элемента по вертикали
		 * @param j Индекс элемента по горизонтали
		 * @return определитель подматрицы 3*3
		 */
		float calc_minor(int i, int j) const noexcept;
		float det() const noexcept; ///< Вычисление определителя матрицы
		TransformMatrix calcComplements() const noexcept; ///< Вычисление матрицы алгебраических дополнений

		TransformMatrix() noexcept; ///< Конструктор по умолчанию, создает единичную матрицу
		TransformMatrix(const TransformMatrix& other) noexcept;

		// Операторы
		TransformMatrix& operator=(const TransformMatrix& other) noexcept;
		TransformMatrix operator*(const TransformMatrix& other) const noexcept;
		float& operator()(int i, int j) ; ///< Получение элемента матрицы

		TransformMatrix inverse() const noexcept; ///< Создание новой инвертированной матрицы
		TransformMatrix transpose() const noexcept; ///< Создание новой транспонированной матрицы

		/**
		@brief Статический метод получения начальной матрицы
		@return Единичная матрица 4*4
		*/
		static TransformMatrix Identity() noexcept;
	};

	// =====================================
	// ======= TransformMatrixBuilder ======
	// =====================================

	/* Вспомогательный класс для генерации матриц трансформации */
	class TransformMatrixBuilder {

	public:
		/**
		 * @brief Создание комбинированной матрицы поворота
		 * @param x_deg Угол поворота вокруг оси X (градусы)
		 * @param y_deg Угол поворота вокруг оси Y (градусы)
		 * @param z_deg Угол поворота вокруг оси Z (градусы)
		 * @return Комбинированная матрица поворота 4×4
		*/
		static TransformMatrix CreateRotationMatrix(float x, float y, float z) noexcept;

		/**
		 * @brief Создание матрицы перемещения
		 *
		 * Матрица перемещения имеет вид:
		 * @code
		 * | 1  0  0  tx |
		 * | 0  1  0  ty |
		 * | 0  0  1  tz |
		 * | 0  0  0  1  |
		 * @endcode
		 *
		 * @param x Смещение по оси X
		 * @param y Смещение по оси Y
		 * @param z Смещение по оси Z
		 * @return Матрица перемещения 4×4
		*/
		static TransformMatrix CreateMoveMatrix(float x, float y, float z) noexcept;

		/**
		 * @brief Создание матрицы масштабирования
		 *
		 * Матрица масштабирования имеет вид:
		 * @code
		 * | sx  0   0   0 |
		 * | 0   sy  0   0 |
		 * | 0   0   sz  0 |
		 * | 0   0   0   1 |
		 * @endcode
		 *
		 * @param x Масштаб по оси X
		 * @param y Масштаб по оси Y
		 * @param z Масштаб по оси Z
		 * @return Матрица масштабирования 4×4
		*/
		static TransformMatrix CreateScaleMatrix(float x, float y, float z) noexcept;

		/**
		 * @brief Создание ортографической матрицы проекции
		 *
		 * Матрица ортографической проекции имеет вид:
		 * @code
		 * | 2/(r-l)    0         0       -(r+l)/(r-l) |
		 * | 0          2/(t-b)   0       -(t+b)/(t-b) |
		 * | 0          0         -2/(f-n) -(f+n)/(f-n)|
		 * | 0          0         0        1           |
		 * @endcode
		 *
		 * @param left Левая граница объема видимости
		 * @param right Правая граница объема видимости
		 * @param bottom Нижняя граница объема видимости
		 * @param top Верхняя граница объема видимости
		 * @param near Ближняя граница отсечения
		 * @param far Дальняя граница отсечения
		 * @return Матрица ортографической проекции 4×4
		*/
		static TransformMatrix CreateOrthographicMatrix(float left,
		                                                float right,
		                                                float bottom,
		                                                float top,
		                                                float near,
		                                                float far) noexcept;

		/**
		 * @brief Создание перспективной матрицы проекции
		 *
		 * Матрица перспективной проекции имеет вид:
		 * @code
		 * | 1/(aspect*tan)  0           0              0 |
		 * | 0               1/tan       0              0 |
		 * | 0               0           -(f+n)/(f-n)   -2fn/(f-n) |
		 * | 0               0           -1             0 |
		 * @endcode
		 *
		 * @param fov_degrees Поле зрения в градусах (обычно 45-90°)
		 * @param aspect_ratio Соотношение сторон окна (width/height)
		 * @param near Ближняя граница отсечения
		 * @param far Дальняя граница отсечения
		 * @return Матрица перспективной проекции 4×4
		*/
		static TransformMatrix CreatePerspectiveMatrix(float fov_degrees,
		                                               float aspect_ratio,
		                                               float near,
		                                               float far) noexcept;

	private:
		/**
		 * @brief Создание матрицы поворота вокруг оси X
		 *
		 * Матрица поворота вокруг оси X имеет вид:
		 * @code
		 * | 1   0      0     0 |
		 * | 0   cosθ  -sinθ   0 |
		 * | 0   sinθ   cosθ   0 |
		 * | 0   0      0     1 |
		 * @endcode
		 *
		 * @param angle_rad Угол поворота в радианах
		 * @return Матрица поворота 4×4
		*/
		static TransformMatrix CreateRotationX(float angle_rad) noexcept;

		/**
		 * @brief Создание матрицы поворота вокруг оси Y
		 *
		 * Матрица поворота вокруг оси Y имеет вид:
		 * @code
		 * | cosθ   0   sinθ   0 |
		 * | 0      1   0      0 |
		 * | -sinθ  0   cosθ   0 |
		 * | 0      0   0      1 |
		 * @endcode
		 *
		 * @param angle_rad Угол поворота в радианах
		 * @return Матрица поворота 4×4
		*/
		static TransformMatrix CreateRotationY(float angle_rad) noexcept;

		/**
		 * @brief Создание матрицы поворота вокруг оси Z
		 *
		 * Матрица поворота вокруг оси Z имеет вид:
		 * @code
		 * | cosθ  -sinθ   0   0 |
		 * | sinθ   cosθ   0   0 |
		 * | 0      0      1   0 |
		 * | 0      0      0   1 |
		 * @endcode
		 *
		 * @param angle_rad Угол поворота в радианах
		 * @return Матрица поворота 4×4
		*/
		static TransformMatrix CreateRotationZ(float angle_rad) noexcept;

	};

	// =====================================
	// ========== Vertex (Вершина) =========
	// =====================================

	/**
	@class Vertex
	@brief Представляет класс для хранения информации об отдельной вершине\n
	Каждая вершина хранит в себе:
	1. Координату в трехмерном пространстве
	2. Вектор нормали

	Вектор нормали необходим для корректной обработки освещения
	*/
	class Vertex {

	private:
		Point3D position_; ///< Координата вершины в трехмерном пространстве
		Point3D normale_; ///< Вектор нормали вершины для корректной обработки освещения

	public:
		/**
		@brief Инициализация вершины через задание ее координаты и вектора нормали
		@param point Координата, представленная в виде вектора (X, Y, Z)
		@param normale Вектор нормали вершины
		*/
		Vertex(const Point3D& point, const Point3D& normale) noexcept;
		Vertex(const Vertex&) noexcept;
		Vertex& operator=(const Vertex&) noexcept;
		bool operator==(const Vertex&) const noexcept;

		/**
		 * Изменяет вектор нормали вершины
		 * @param normale Новый вектор нормали для данной вершины
		 */
		void setNormals(const Point3D& normale) noexcept;
		Point3D& getPosition() noexcept; ///< Получение координаты вершины в трехмерном пространстве
		const Point3D& getPosition() const noexcept;
		const Point3D& getNormale() const noexcept; ///< Получение вектора нормали вершины
		Point3D& getNormale() noexcept;

	};

	// =================================
	// ===== Surface (Поверхность) =====
	// =================================

	class Surface {

	private:
		std::vector<uint32_t> indices_; ///< Индексы вершин из списка, считанного из файла

	public:
		/**
		@brief Инициализация поверхности из двух вершин
		@param ind1 Индекс первой вершины
		@param ind2 Индекс второй вершины
		@param ind3 Индекс третьей вершины
		*/
		Surface(uint32_t ind1, uint32_t ind2, uint32_t ind3) noexcept;
		bool operator==(const Surface&) const noexcept; ///< Две поверхности равны, если равны попарно индексы трех вершин

		uint32_t operator[](int i) const noexcept; ///< Доступ к индексам поверхности

	};

	// =====================================
	// ========== Viewer (Facade) ==========
	// =====================================

	/**
	@class Viewer
	@brief Главный класс программы, хранящий в себе остальные компоненты и
	управляющий их работой\n

	Создается вначале программы
	*/
	class Viewer {

	private:

		BaseFileReader* reader_; ///< Читатель файлов
		BaseDrawerScene* drawer_; ///< Отрисовщик фигуры на экране
		Scene* scene_; ///< Текущая сцена

		inline static std::string error_scene_ = "The scene has not been created";
		inline static std::string error_reader_ = "The file reader has not been created";
		inline static std::string error_drawer_ = "The drawer has not been created";

	public:

		/**
		@brief Конструктор класса Viewer
		@param reader Читатель файлов
		@param drawer Отрисовщик сцены
		*/
		Viewer(BaseFileReader* reader, BaseDrawerScene* drawer) noexcept;

		Viewer(const Viewer&) = delete;
		Viewer& operator=(const Viewer&) = delete;

		/**
		@brief Деструктор класса Viewer
		@note Освобождает все ресурсы
		*/
		~Viewer();

		/**
		@brief Отрисовка текущей сцены
		@return Результат операции
		*/
		ViewerOperationResult DrawScene();

		/**
		@brief Загрузка фигуры из файла
		@param path Путь к файлу модели
		@return Результат операции
		*/
		ViewerOperationResult LoadFigure(std::string path);

		Scene* getScene() noexcept;
	};

	/**
	 * @class Camera
	 * @brief Данный класс хранит информацию о том, как будет отображена вся сцена с точки зрения пользователя
	 * Кроме стандартных настроек изменения положения в пространстве,
	 * камера задает тип проекции сцены - ортографическая или перспективная.
	 * @note Матрица модели камеры является матрицей вида - View Matrix
	 */
	class Camera : public BaseSceneObject {
	private:
		ProjectionType type_; ///< Тип проекции

	public:
		/**
		 * @brief Создание камеры с стандартными настройками вида
		 */
		Camera() noexcept;

		/**
		 * @brief Создание проекционной матрицы, в зависимости от выбора пользователя
		 * @param aspect Соотношение сторон сцены
		 * @return Ортографическая или перспективная матрица проекции
		 */
		TransformMatrix getProjectionMatrix(float aspect) noexcept;

		/**
		 * @brief Метод для доступа к полю типу проекции сцены
		 * @return Информация о проекции
		 */
		ProjectionType& projectionType() noexcept;

		/**
		 * @brief Получение информации для загрузки ее в вершинный шейдер
		 * @return Результирующая информация о камере
		 */
		CameraData getData(float aspect) noexcept;
	};

	// =====================================
	// ========== Figure (Фигура) ==========
	// =====================================

	/**
	@class Figure
	@brief Представляет собой класс для хранения информации о целой фигуре\n
	Фигура - это множество вершин и поверхностей(связей между вершинами)\n
	*/
	class Figure : public BaseSceneObject {

	private:
		std::string name_; ///< Путь до файла с фигурой
		std::vector<Vertex> vertices_; ///< Множество вершин (координат в трехмерном пространстве)
		std::vector<Surface> surfaces_; ///< Множество поверхностей (связей между вершинами)
		DisplayType displayType_; ///< Хранит способ отображения фигуры на сцене
		EdgeInfo edgInfo_; ///< Способ отображения ребер в фигуре
		VertexInfo vertInfo_; ///< Способ отображения вершин в фигуре
		MaterialData material_; ///< Характеристики материала, из которого состоит фигура

	public:

		Figure() noexcept; ///< Инициализация пустой фигуры
		/**
		@brief Создание фигуры из исходного множества вершин и ребер
		@param vertices Считанные вершины
		@param Surfaces Считанные поверхности
		*/
		Figure(const std::string& path, const std::vector<Vertex>& vertices, const std::vector<Surface>& Surfaces) noexcept;

		/**
		 * @brief Получение имени файла, из которого была считана текущая фигура
		 * @return Абсолютный путь до файла
		 */
		const std::string& path() noexcept;

		const std::vector<Vertex>& getVertices() const noexcept; ///< Доступ к списку вершин фигуры
		const std::vector<Surface>& getSurfaces() const noexcept; ///< Доступ к списку поверхностей фигуры

		DisplayType& displayType() noexcept; ///< Получение информации о том, как должна отрисовываться фигура
		EdgeInfo& edgeInfo() noexcept; ///< Получение информации о способе представления ребер фигуры в каркасном режиме
		VertexInfo& vertexInfo() noexcept; ///< Получение информации о способе представления вершин фигуры в каркасном режиме
		MaterialData& material() noexcept; ///< Получение информации о характеристиках материала фигуры
	};

	// =====================================
	// ===== Light (Источник освещения) ====
	// =====================================

	class Light {
	private:
		Point3D position_; ///< Координата источника освещения в пространстве (для направленных источников)
		float intensity_; ///<
		Point3D color_; ///< Цвет в формате RGB

	public:
		Light() noexcept; ///< Инициализация стандартного источника освещения

		Point3D& position() noexcept; ///< Доступ к полю координаты объекта
		float& intensity() noexcept; ///< Доступ к полю интенсивности объекта
		Point3D& color() noexcept; ///< Доступ к полю цвета объекта

		/**
		 * @brief Получение информации для загрузки ее в шейдеры
		 * @return Результирующая информация об источнике освещения
		 */
		LightData getData() noexcept;
	};

	// =====================================
	// ========== Scene (Сцена) ===========
	// =====================================

	/**
	@class Scene
	@brief Класс для хранения текущих фигур, которые должны быть отрисованы
	@note Поддержка двух и более фигур одновременно на сцене
	*/
	class Scene {
	private:
		Point3D backColor_; ///< Цвет фона
		Camera camera_; ///< Камера сцены
		std::vector<Figure> figures_; ///< Список фигур, которые в данный момент находятся на сцене
		/**
		 * @brief Список источников освещения, которые расположены на сцене.
		 * Здесь хранится 1 глобальный и N направленных источников освещения
		 */
		std::vector<Light> lights_;

	public:
		/**
		 * @brief Инициализация сцены в момент загрузки прогаммы.
		 * По умолчанию создается глобальный источник освещения и камера,
		 * через которую будет происходить просмотр сцены
		 */
		Scene() noexcept;

		/**
		 * @brief Добавление фигуры на сцену для отрисовки
		 * @param figure Загруженная из файла модель
		 */
		void addFigure(const Figure& figure) noexcept;

		/**
		 * @brief Добавление нового направленного источника освещения на сцену
		 */
		void addLight() noexcept;

		/**
		 * @brief Доступ к информации о цвете фона сцены
		 * @return Цвет в формате RGB
		 */
		Point3D& backgroundColor() noexcept;

		/**
		 * @brief Получение фигуры из списка, которую выбрал пользователь. Это означает, что данная фигура
		 * переходит в активное состояние и для нее могут быть применены стандартные преобразования -
		 * перемещение, вращение и масштабирование
		 * @param number Номер фигуры в списке (Какой по счету она была загружена)
		 * @note Номер начинается с 1
		 * @return Ссылка на нужную фигуру
		 */
		Figure& getFigure(int number);

		/**
		 * @brief Получение источника освещения из списка, которого выбрал пользователь.
		 * Позволяет изменять его характеристики, что даст другое отображение сцены
		 * @param number Номер источника в списке (Каким по счету он был создан)
		 * @note 0 - глобальный источник освещения, от 1 до n - направленные источники освещения
		 * @return Ссылка на нужный направленный источник освещения
		 */
		Light& getLight(int number);

		/**
		 * @brief Доступ к камере сцены для изменения глобальных настроек представления сцены
		 * @return Текущая камера
		 */
		Camera& getCamera() noexcept;
	};

	// =====================================
	// ======== FileReader (Читатель) ======
	// =====================================

	/**
	@class FileReader
	@brief Класс для загрузки сцены из файла с 3d фигурой
	*/
	class FileReader : public BaseFileReader {

	public:
		/**
		@brief Чтение новой фигуры
		@param path Расположение файла .obj
		@return Готовая для отображения на экране загруженная фигура
		*/
		Figure ReadFigure(std::string path) override;

	private:
		/**
		@brief Нормализация фигуры, чтобы она корректно отображалась на экране
		@param vertices Считанные исходные вершины, образующие фигуру
		@surfaces Считанные поверхности из файла
		*/
		void Normalize(std::vector<Vertex>& vertices, std::vector<Surface>& surfaces);
	};

}

#endif
