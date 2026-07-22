#ifndef VIEWER_DATASTRUCTURES_H
#define VIEWER_DATASTRUCTURES_H

/**
 * @file data.h
 * @brief В этом файле хранятся основные структуры данных,
 * которые используются при хранении данных в базовых классах программы
 * @author Georgiy Kovalev
 */
namespace viewer {
	/**
	 * @class ProjectionType
	 * @brief Типы проекции при отображении фигуры
	 */
	enum ProjectionType {
		ORTHOGRAPHIC = 0, ///< Параллельная проекция
		PERSPECTIVE ///< Центральная проекция
	};

	/**
	 * @class VerticesMode
	 * @brief Перечисление для указания режима отображения вершин
	 */
	enum VerticesMode {
		NONE = 0, ///< Вершины не отображаются
		CIRCLE, ///< Вершины представлены кругами
		SQUARE ///< Вершины представлены квадратами
	};

	/**
	 * @class EdgesMode
	 * @brief Перечисление для указания режима отображения ребер
	 */
	enum EdgesMode {
		SOLID, ///< Ребра представлены сплошной линией
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
		RAY_TRACING, /// Трассировка лучей (пока что через PBR)
		COUNT ///< Константа для хранения размера перечисления DisplayType
	};

	enum RecordType {
		IMAGE_RECORD = 0, ///< Захват изображения
		GIF_RECORD ///< Запись GIF-анимации
	};

#define GIF_FPS_COUNT 10
#define GIF_DURATION_SEC 5
#define LIGHT_DEFAULT_INTENSITY 0.5f
#define MAX_POINT_LIGHTS 5
#define MAX_COUNT_FIGURES 5
#define MAX_VERTICES 1000000 ///< Максимальное количество вершин в Ray-tracing шейдерной программе

	/**
	* @class Point3D
	* @brief Хранение координаты точки в трехмерном пространстве.
	* Выравнивание до 16 байт необходимо для правильной загрузки в шейдерный буфер
	*/
	struct alignas(16) Point3D {

		float x, y, z;

		Point3D() : x{0.0f}, y{0.0f}, z{0.0f} {}
		Point3D(float x_value, float y_value, float z_value) : x{x_value}, y{y_value}, z{z_value} {}

		bool operator==(const Point3D& other) const noexcept {
			return (x == other.x && y == other.y && z == other.z);
		}

		Point3D operator+(const Point3D& other) const noexcept {
			return Point3D(x + other.x, y + other.y, z + other.z);
		}

		Point3D(const Point3D& other) noexcept : x{other.x}, y{other.y}, z{other.z} {}
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

	/**
	 * @class EdgeInfo
	 * @brief Вспомогательная структура для хранения информации о ребрах в фигуре
	 */
	struct EdgeInfo {
		Point3D edgColor_; ///< Цвет ребер
		float edgSize_; ///< Размер ребер
		EdgesMode edgMode_; ///< Режим отображения ребер

		EdgeInfo() noexcept : edgColor_(0.0, 0.0, 0.0), edgSize_(0.001f), edgMode_(EdgesMode::SOLID) {}

		/**
		 * @brief Метод для доступа к полю цвета
		 * @return Текущий цвет ребра
		 */
		Point3D& color() noexcept {
			return edgColor_;
		}

		/**
		 * @brief Метод для доступа к полю толщины ребра
		 * @return Текущий размер ребра
		 */
		float& size() noexcept {
			return edgSize_;
		}

		/**
		 * @brief Метод для доступа к полю режима отображения ребра
		 * @return Текущий режим отображения ребра
		 */
		EdgesMode& mode() noexcept {
			return edgMode_;
		}
	};

	/**
	 * @class VertexInfo
	 * @brief Вспомогательная структура для хранения информации о том,
	 * как должны быть отображены вершины у конкретной фигуры в каркасном режиме
	 */
	struct VertexInfo {
		Point3D vertColor_; ///< Цвет вершины
		float vertSize_; ///< Размер вершины
		VerticesMode vertMode_; ///< Режим отображения вершины

		VertexInfo() noexcept : vertColor_(0.0, 0.0, 0.0), vertSize_(0.01f), vertMode_(VerticesMode::CIRCLE) {}

		/**
		 * @brief Осуществление доступа к полю цвета вершины
		 * @return Текущий цвет в формате RGB
		 */
		Point3D& color() noexcept {
			return vertColor_;
		}

		/**
		 * @brief Осуществление доступа к полю размера вершины
		 * @return Текущий размер вершины
		 */
		float& size() noexcept {
			return vertSize_;
		}

		/**
		 * @brief Осуществление доступа к полю, хранящему режим отображения вершины
		 * @return Текущий режим отображения вершины
		 */
		VerticesMode& mode() noexcept {
			return vertMode_;
		}

 	};

	struct MaterialData {
		Point3D baseColor_; ///< Цвет материала в формате RGB
		/**
		 * @brief Шероховатость поверхности, где 0 — абсолютно гладкая зеркальная поверхность,
		 * а 1 — шероховатая матовая поверхность
		 */
		float roughness_;
		/**
		 * @brief Металличность поверхности,
		 * где 0 — диэлектрическая поверхность (пластик, стекло, резина),
		 * а 1 — металл
		 */
		float metallic_;
		/**
		 * @brief Коэффициент преломления,
		 * где 0 - нет преломления,
		 * а 1 - все лучи преломляются
		 */
		float refractive_;

		/**
		 * @brief Коэффициент отражения,
		 * где 0 - абсолютно черная поверхность (поглощает весь свет),
		 * 1 - абсолютно белая поверхность (отражает весь свет)
		 */
		float reflectivity_;
		/**
		 * @brief
		 * Коэффициент прозрачности,
		 * где 0 - абсолютно прозрачная,
		 * 1 - абсолютно непрозрачная
		 */
		float alpha_;
		float padding[3];

		Point3D& color() noexcept { return baseColor_; } ///< Получение доступа к базовому цвету
		float& roughness() noexcept { return roughness_; } ///< Получение доступа к коэффициенту шероховатости
		float& metallic() noexcept { return metallic_; } ///< Получение доступа к коэффициенту металличности
		float& refractive() noexcept { return refractive_; } ///< Получение доступа к коэффициенту преломления
		float& reflectivity() noexcept { return reflectivity_; } ///< Получение доступа к коэффициенту отражения
		float& alpha() noexcept { return alpha_; } ///< Получение доступа к коэффициенту прозрачности
	};

	struct CameraData {
		float projectionMatrix_[4][4]; ///< Матрица проекции камеры
		float viewMatrix_[4][4]; ///< Матрица вида (матрица модели, созданная с настройками камеры)
		Point3D position_; ///< Координата камеры на сцене (смещение относительно начала координат)

		CameraData() noexcept : projectionMatrix_{0}, viewMatrix_{0} {}
		CameraData(const CameraData& other) {
			for (auto i = 0; i < 4; ++i) {
				for (auto j = 0; j < 4; ++j) {
					projectionMatrix_[i][j] = other.projectionMatrix_[i][j];
					viewMatrix_[i][j] = other.viewMatrix_[i][j];
				}
			}

			position_ = other.position_;
		}
	};

	/**
	 * @struct GPURayFigure
	 * @brief Структура для хранения основной информации о фигуре,
	 * которая загружена в Ray-tracing шейдерную программу и
	 * должна быть обработана.
	 */
	struct GPURayFigure {
		float modelMatrix[16];
		float normalMatrix[16];
		int firstVertex; ///< Смещение начала вершин в общем буфере
		int firstIndex; ///< Смещение начала индексов в общем буфере
		int indexCount; ///< Количество индексов фигуры
		int padding; ///< Выравнивание 16 байт
	};

}

#endif