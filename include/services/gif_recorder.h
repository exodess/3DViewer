/**
 * @file gif_recorder.h
 * @brief Заголовочный файл сервиса для записи GIF-анимации
 * @author wilsoncl
 * @date 2026-03-29
 * @version 1.0
 *
 * @details Этот файл содержит объявление класса GifRecorder,
 *          который обеспечивает запись последовательности RGB кадров
 *          в GIF-анимацию с использованием библиотеки libcgif.
 *
 * @note Класс не зависит от Qt и может использоваться в любом контексте
 * @see cgif.h
 */

#ifndef S21_GIF_RECORDER_H
#define S21_GIF_RECORDER_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace viewer {

/**
 * @class GifRecorder
 * @brief Сервис для записи GIF-анимации из последовательности кадров
 * @details Обеспечивает:
 *          - Запись последовательности RGB кадров
 *          - Масштабирование кадров до заданного размера
 *          - Квантование цветов в 256-цветную палитру (5-6-5 разбиение)
 *          - Сохранение результата в файл
 *
 * @note Не зависит от Qt, использует только стандартную библиотеку и libcgif
 * @warning GIF формат ограничен 256 цветами на всю анимацию
 *
 * @example
 * @code
 * GifRecorder recorder;
 * GifRecorder::Config config;
 * config.fps = 10;
 * config.duration_seconds = 5;
 *
 * recorder.StartRecording(config,
 *     [](int frame, int total) { std::cout << frame << "/" << total << "\n"; },
 *     [](const std::string& path) { std::cout << "Saved: " << path << "\n"; },
 *     [](const std::string& error) { std::cerr << error << "\n"; }
 * );
 *
 * // Добавление кадров
 * recorder.AddFrame(rgb_data, width, height);
 *
 * // Или автоматически по достижении нужного количества
 * @endcode
 */
class GifRecorder {
 public:
  /**
   * @struct Config
   * @brief Параметры конфигурации записи GIF
   */
  struct Config {
    int width = 640;  ///< Ширина выходного GIF в пикселях
    int height = 480;  ///< Высота выходного GIF в пикселях
    int fps = 10;      ///< Частота кадров в секунду
    int duration_seconds = 5;  ///< Длительность записи в секундах
  };

  /**
   * @brief Тип функции обратного вызова для прогресса записи
   * @param frame Количество записанных кадров
   * @param total Общее количество кадров для записи
   */
  using ProgressCallback = std::function<void(int frame, int total)>;

  /**
   * @brief Тип функции обратного вызова при завершении записи
   * @param path Путь к сохранённому файлу
   */
  using CompleteCallback = std::function<void(const std::string& path)>;

  /**
   * @brief Тип функции обратного вызова при ошибке
   * @param error Текст ошибки
   */
  using ErrorCallback = std::function<void(const std::string& error)>;

  /**
   * @brief Конструктор класса GifRecorder
   * @details Инициализирует объект в неактивном состоянии
   */
  GifRecorder();

  /**
   * @brief Деструктор класса GifRecorder
   * @details Если запись активна, останавливает её
   */
  ~GifRecorder();

  /**
   * @brief Начало записи GIF-анимации
   * @param config Параметры записи (размер, fps, длительность)
   * @param onProgress Функция обратного вызова для обновления прогресса
   * @param onComplete Функция обратного вызова при завершении записи
   * @param onError Функция обратного вызова при ошибке
   * @details Очищает буфер кадров, резервирует память под ожидаемое количество
   * кадров, устанавливает флаг записи.
   * @warning Если запись уже идёт, вызов игнорируется
   * @see AddFrame
   * @see StopRecording
   */
  void StartRecording(const std::string& path, const Config& config, ProgressCallback onProgress,
                      CompleteCallback onComplete, ErrorCallback onError);

  /**
   * @brief Добавление кадра в GIF
   * @param rgb_data Указатель на данные в формате RGB (24 бита на пиксель)
   * @param width Ширина кадра в пикселях
   * @param height Высота кадра в пикселях
   * @details Сохраняет копию кадра в буфер. Если достигнуто необходимое
   * количество кадров (fps * duration_seconds), автоматически завершает запись.
   * @note Кадры сохраняются в оригинальном размере, масштабирование происходит
   * при кодировании
   * @warning Данные должны быть в формате RGB (порядок: R,G,B,R,G,B,...)
   */
  void AddFrame(const uint8_t* rgb_data, int width, int height);

  /**
   * @brief Принудительная остановка записи GIF
   * @details Завершает запись с текущими накопленными кадрами.
   *          Если кадров меньше ожидаемого, GIF будет короче.
   */
  void StopRecording();

  /**
   * @brief Проверка состояния записи
   * @return true если запись активна, false в противном случае
   */
  bool IsRecording() const { return is_recording_; }

 private:
  /**
   * @brief Завершение записи и сохранение GIF файла
   * @details Кодирует все накопленные кадры в GIF-анимацию:
   *          - Создаёт глобальную палитру из 256 цветов
   *          - Масштабирует кадры до целевого размера
   *          - Преобразует RGB в индексы палитры (квантование 5-6-5)
   *          - Сохраняет результат в ~/3DViewer_screenrecord.gif
   * @warning Если нет кадров, вызывается ошибка через error_cb_
   */
  void FinishRecording();

  bool is_recording_ = false;  ///< Флаг активности записи
  Config config_;              ///< Параметры записи
  std::vector<std::vector<uint8_t>> frames_;  ///< Буфер кадров в формате RGB
  ProgressCallback progress_cb_;  ///< Callback прогресса
  CompleteCallback complete_cb_;  ///< Callback завершения
  ErrorCallback error_cb_;        ///< Callback ошибки
  std::string output_file_path_; ///< Файл для загрузки итоговой Gif анимации
};

}

#endif
