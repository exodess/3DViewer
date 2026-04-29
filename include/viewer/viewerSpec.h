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

namespace viewer {

	// =====================================
	// ========== Общее положение ==========
	// =====================================

	// Хранение данных:
	struct Point3D;
	struct NormalizationParameters;
	struct ViewerOperationResult;

	// Интерфейсы:
	class BaseFileReader;
	class BaseDrawerScene;
	class BaseSceneObject;

	// Основные классы:
	class Viewer;
	class Scene;
	class Figure;
	class Surface;
	class Vertex;

	class TransformMatrix;
	class TransformMatrixBuilder;
	class FileReader;

	// =====================================
	// ========== Структуры данных =========
	// =====================================

	/**
	@class Point3D
	@brief Хранение координаты точки в трехмерном пространстве
	*/
	struct Point3D {

		float x, y, z;

		Point3D() : x{0.0f}, y{0.0f}, z{0.0f} {}
		Point3D(float x_value, float y_value, float z_value) : x{x_value}, y{y_value}, z{z_value} {}

		bool operator==(const Point3D& other) const noexcept {
			return (x == other.x && y == other.y && z == other.z);
		}

		Point3D operator+(const Point3D& other) const noexcept {
			return Point3D(x + other.x, y + other.y, z + other.z);
		}
	};

	/**
	@class NormalizationParameters
	@brief Параметры загрузки сцены
	@note Пока что не используется
	*/
	struct NormalizationParameters {

		float min, max;
		float dxStep, dyStep;

	};

	/**
	@class ViewerOperationResult
	@brief Структура для хранения информации о результате работы внутри класса
	Viewer при вызове какого-либо метода

	- Если операция прошла успешно, то isSuccess_ = true, а поле message_ пустое
	- Иначе, в поле message_ будет хранится краткая информация о произошедшей ошибке

	Структура передается в главный класс отрисовки приложения для сигнализации пользователя
	о результатах операций
	*/
	struct ViewerOperationResult {

	private:
		bool isSuccess_; ///< Успешность операции
		std::string message_; ///< Информация о ошибке, если она есть

	public:
		ViewerOperationResult(bool success, std::string mess = "") : isSuccess_{success}, message_{mess} {}

		bool isSuccess() noexcept { return isSuccess_; } ///< Получение информации о том, успешно ли прошла операция или нет
		std::string& getMessage() noexcept { return message_; } ///< Передает краткую аннотацию, почему операция завершилась с ошибкой

	};

	// =====================================
	// ======== Используемые классы ========
	// =====================================

	/**
	@class BaseSceneObject
	@brief Абстрактный базовый класс, от которого наследуются все объекты, находящиеся на сцене\n
	Предоставляет интерфейс для классов-наследников, как они должны быть реализованы

	@note От данного класса наследуются классы Figure, Surface, Vertex
	*/
	class BaseSceneObject {

	public:
		/**
		@brief Каждый класс, представленный на сцене,
		должен иметь метод для изменения текущего положения в пространстве
		@param trans_matrix Матрица трансформации, применяемая к каждой вершине
		*/
		virtual void Transform(const TransformMatrix& trans_matrix) = 0;
		virtual ~BaseSceneObject() = default;
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
		@param param Параметры, с которыми считывается фигура
		@return Готовая к отображению сцена, которая сохраняется в классе Viewer
		*/
		virtual Scene* ReadScene(std::string path, NormalizationParameters param) = 0;
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
		virtual void DrawScene(Scene&) = 0;
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
		@brief Изменение координаты точки путем умножения ее координаты на аффинную матрицу\n
		@param point Координата точки в трехмерном пространстве
		@return Новая координата точки после преобразования
		*/
		Point3D TransformPoint(const Point3D& point) const noexcept;


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
	class Vertex : public BaseSceneObject {

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
		void Transform(const TransformMatrix&) override; ///< Не используется

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
		Viewer(BaseFileReader*, BaseDrawerScene*) noexcept;

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
		@brief Загрузка сцены из файла
		@param path Путь к файлу модели
		@param param Параметры нормализации
		@return Результат операции
		*/
		ViewerOperationResult LoadScene(std::string path, NormalizationParameters);


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
		std::vector<Vertex> vertices_; ///< Множество вершин (координат в трехмерном пространстве)
		std::vector<Surface> surfaces_; ///< Множество поверхностей (связей между вершинами)

	public:

		Figure() noexcept; ///< Инициализация пустой фигуры
		/**
		@brief Создание фигуры из исходного множества вершин и ребер
		@param vertices Считанные вершины
		@param Surfaces Считанные поверхности
		*/
		Figure(std::vector<Vertex>& vertices, std::vector<Surface>& Surfaces) noexcept;

		const std::vector<Vertex>& getVertices() const noexcept; ///< Доступ к списку вершин фигуры
		const std::vector<Surface>& getSurfaces() const noexcept; ///< Доступ к списку поверхностей фигуры

		void Transform(const TransformMatrix&) override; ///< Не используется

	};

	// =====================================
	// ========== Scene (Сцена) ===========
	// =====================================

	/**
	@class Scene
	@brief Класс для хранения текущей фигуры, которая должна быть отрисована
	@note В каждый момент времени может быть только одна фигура на сцене!
	*/
	class Scene {

	private:

		Figure figure_; ///< Фигура, находящаяся на сцене

	public:

		Scene(const Figure& figure) noexcept; ///< Инициализация сцены фигурой

		Figure& getFigure() noexcept; ///< Получение текущей фигуры
		const Figure& getFigure() const noexcept;
		void TransformFigure(const TransformMatrix&); ///< Не используется

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
		@brief Чтение сцены
		@param path Расположение файла .obj
		@param param Параметры нормализации, которые необходимо применить для считываемой фигуры (не используется)
		@return Готовая для отображения на экран сцена, содержащая загруженную фигуру
		*/
		Scene* ReadScene(std::string path, NormalizationParameters param) override;

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
