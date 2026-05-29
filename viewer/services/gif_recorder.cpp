/**
 * @file gif_recorder.cpp
 * @brief Реализация сервиса для записи GIF-анимации
 * @author wilsoncl
 * @date 2026-03-29
 * @version 1.0
 *
 * @details Этот файл содержит реализацию класса GifRecorder,
 *          который обеспечивает запись последовательности кадров
 *          в GIF-анимацию с использованием библиотеки libcgif.
 *
 * @see GifRecorder
 * @see cgif.h
 */

#include "services/gif_recorder.h"

#include <cgif.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>

namespace viewer {

/**
 * @brief Функция обратного вызова для записи данных GIF
 * @param pContext Указатель на контекст (в данном случае FILE*)
 * @param pData Указатель на данные для записи
 * @param numBytes Количество байт для записи
 * @return 0 при успешной записи, -1 при ошибке
 * @details Используется библиотекой libcgif для записи сжатых данных в файл.
 *          Возвращает 0 только если все байты успешно записаны.
 */
static int writeCallback(void* pContext, const uint8_t* pData,
                         size_t numBytes) {
  FILE* file = (FILE*)pContext;
  size_t written = fwrite(pData, 1, numBytes, file);
  return (written == numBytes) ? 0 : -1;
}

/**
 * @brief Конструктор класса GifRecorder
 * @details Инициализирует объект в неактивном состоянии (запись не идёт)
 */
GifRecorder::GifRecorder() = default;

/**
 * @brief Деструктор класса GifRecorder
 * @details Если запись активна, останавливает её и освобождает ресурсы
 */
GifRecorder::~GifRecorder() {
  if (is_recording_) StopRecording();
}

/**
 * @brief Начало записи GIF-анимации
 * @param config Параметры записи (размер, fps, длительность)
 * @param onProgress Функция обратного вызова для обновления прогресса
 * @param onComplete Функция обратного вызова при завершении записи
 * @param onError Функция обратного вызова при ошибке
 * @details Очищает буфер кадров, резервирует память под ожидаемое количество
 * кадров, устанавливает флаг записи и выводит информацию в консоль.
 * @warning Если запись уже идёт, вызов игнорируется
 */
void GifRecorder::StartRecording(const std::string& path, const Config& config,
                                 ProgressCallback onProgress,
                                 CompleteCallback onComplete,
                                 ErrorCallback onError) {
  if (is_recording_) return;

  output_file_path_ = path;
  config_ = config;
  progress_cb_ = std::move(onProgress);
  complete_cb_ = std::move(onComplete);
  error_cb_ = std::move(onError);

  frames_.clear();
  frames_.reserve(config_.fps * config_.duration_seconds);
  is_recording_ = true;

  std::cout << "[GifRecorder] Recording started: " << config_.fps << " fps, "
            << config_.duration_seconds << " sec\n";
}

/**
 * @brief Добавление кадра в GIF
 * @param rgb_data Указатель на данные в формате RGB (24 бита на пиксель)
 * @param width Ширина кадра в пикселях
 * @param height Высота кадра в пикселях
 * @details Сохраняет копию кадра в буфер. Если достигнуто необходимое
 * количество кадров, автоматически вызывает FinishRecording().
 * @note Кадры сохраняются в оригинальном размере, масштабирование происходит
 * при кодировании
 */
void GifRecorder::AddFrame(const uint8_t* rgb_data, int width, int height) {
  if (!is_recording_) return;

  // Сохранение оригинального кадра (RGB)
  std::vector<uint8_t> frame(rgb_data, rgb_data + (width * height * 3));
  frames_.push_back(std::move(frame));

  if (progress_cb_) {
    progress_cb_(frames_.size(), config_.fps * config_.duration_seconds);
  }

  if (static_cast<int>(frames_.size()) >=
      config_.fps * config_.duration_seconds) {
    FinishRecording();
  }
}

/**
 * @brief Принудительная остановка записи GIF
 * @details Вызывает FinishRecording() для завершения записи с текущими кадрами
 */
void GifRecorder::StopRecording() {
  if (!is_recording_) return;
  FinishRecording();
}

/**
 * @brief Завершение записи и сохранение GIF файла
 * @details Кодирует все накопленные кадры в GIF-анимацию:
 *          1. Создаёт глобальную палитру из 256 цветов (5-6-5 разбиение)
 *          2. Настраивает параметры GIF (размер, анимация)
 *          3. Для каждого кадра выполняет:
 *             - Масштабирование до целевого размера
 *             - Преобразование RGB в индексы палитры
 *             - Добавление кадра в GIF
 *          4. Сохраняет результат в файл и перемещает в домашнюю директорию
 * @warning Если нет кадров, вызывается ошибка
 * @note Результирующий файл сохраняется как ~/3DViewer_screenrecord.gif
 */
void GifRecorder::FinishRecording() {
  if (frames_.empty()) {
    if (error_cb_) error_cb_("No frames captured");
    is_recording_ = false;
    return;
  }

  std::cout << "[GifRecorder] Encoding " << frames_.size() << " frames... ";

  // Открытие файла для записи
  FILE* file = fopen(output_file_path_.c_str(), "wb");
  if (!file) {
    if (error_cb_) error_cb_("Cannot create output file");
    is_recording_ = false;
    return;
  }

  // Создаём оптимизированную палитру для GIF (используем базовые цвета)
  // Для простоты создаём палитру из 256 цветов (оттенки серого + основные
  // цвета)
  std::vector<uint8_t> globalPalette(256 * 3);
  for (int i = 0; i < 256; ++i) {
    // Создание разноцветной палитры с 5-6-5 разбиением
    // 3 бита для красного (8 значений), 3 бита для зелёного (8 значений),
    // 2 бита для синего (4 значения) -> всего 8*8*4 = 256 цветов
    int r = (i & 0xE0) >> 5;        // Старшие 3 бита
    int g = (i & 0x1C) >> 2;        // Средние 3 бита
    int b = (i & 0x03);             // Младшие 2 бита
    globalPalette[i * 3] = r * 85;  // 85 = 255 / 3
    globalPalette[i * 3 + 1] = g * 85;
    globalPalette[i * 3 + 2] = b * 85;
  }

  // Настройка GIF
  CGIF_Config gif_config = {0};
  gif_config.width = config_.width;
  gif_config.height = config_.height;
  gif_config.attrFlags = CGIF_ATTR_IS_ANIMATED;
  gif_config.pWriteFn = writeCallback;
  gif_config.pContext = file;
  gif_config.pGlobalPalette = globalPalette.data();
  gif_config.numGlobalPaletteEntries = 256;

  CGIF* gif = cgif_newgif(&gif_config);
  if (!gif) {
    if (error_cb_) error_cb_("Failed to create GIF");
    fclose(file);
    is_recording_ = false;
    return;
  }

  int delay = 100 / config_.fps;  // Задержка в сотых долях секунды

  for (const auto& frame : frames_) {
    // Создание индексированного изображения с цветами
    std::vector<uint8_t> indexed(config_.width * config_.height);

    int src_width = config_.width;
    int src_height = config_.height;

    // Масштабирование и преобразование в индексы палитры
    for (int y = 0; y < config_.height; ++y) {
      for (int x = 0; x < config_.width; ++x) {
        // Вычисление координат в исходном кадре
        int src_x = x * src_width / config_.width;
        int src_y = y * src_height / config_.height;
        int src_idx = (src_y * src_width + src_x) * 3;

        if (src_idx + 2 >= static_cast<int>(frame.size())) {
          src_idx = 0;
        }

        uint8_t r = frame[src_idx];
        uint8_t g = frame[src_idx + 1];
        uint8_t b = frame[src_idx + 2];

        // Ближайший цвет в палитре (упрощённо)
        int r_idx = r / 85;  // 0-2
        int g_idx = g / 85;  // 0-2
        int b_idx = b / 85;  // 0-2 (но синий имеет только 0-1 из-за 2 бит)
        // Корректировка синего для 2 бит (0-3)
        b_idx = b / 64;  // 64 = 255 / 4
        if (b_idx > 3) b_idx = 3;

        int index = (r_idx << 5) | (g_idx << 2) | b_idx;
        if (index > 255) index = 255;

        indexed[y * config_.width + x] = static_cast<uint8_t>(index);
      }
    }

    CGIF_FrameConfig frame_config = {0};
    frame_config.pImageData = indexed.data();
    frame_config.delay = delay;

    if (cgif_addframe(gif, &frame_config) != CGIF_OK) {
      if (error_cb_) error_cb_("Failed to add frame");
      cgif_close(gif);
      fclose(file);
      is_recording_ = false;
      return;
    }
  }

  cgif_close(gif);
  fclose(file);

  frames_.clear();
  is_recording_ = false;

  std::cout << "SUCCESS\n";
}

}
