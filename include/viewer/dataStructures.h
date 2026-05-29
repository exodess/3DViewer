#ifndef VIEWER_DATASTRUCTURES_H
#define VIEWER_DATASTRUCTURES_H

/**
 * @file dataStructures.h
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
	};

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

		EdgeInfo() noexcept : edgColor_(1.0, 1.0, 1.0), edgSize_(1.0), edgMode_(EdgesMode::SOLID) {}

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

		VertexInfo() noexcept : vertColor_(0.0, 0.0, 0.0), vertSize_(1.0), vertMode_(VerticesMode::NONE) {}

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
}

#endif