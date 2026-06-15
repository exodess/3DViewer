#ifndef VIEWER_GLWIDGET_H
#define VIEWER_GLWIDGET_H

#include <QMouseEvent>
#include <QCoreApplication>
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QColor>
#include <QTimer>
#include <iostream>

#include "viewer/graphicSpec.h"
#include "ui/ui_mainwindow.h"
#include "services/gif_recorder.h"

#define TRANSLATION_MOUSE_SENSITIVITY 0.005f
#define ROTATION_MOUSE_SENSITIVITY 0.2f
#define ZOOM_MOUSE_SENSITIVITY 0.0006f

namespace viewer {

    class GLWidget : public QOpenGLWidget, public BaseDrawerScene, protected QOpenGLFunctions {
        Q_OBJECT

    public:
        GLWidget(QWidget* parent);
        ~GLWidget();

        void DrawScene(Scene* scene) override;

        /**
         * @brief Сохранение текущего изображения в файл
         * @param path Путь к файлу для сохранения
         * @details Поддерживает форматы BMP, JPEG, PNG
         */
        void saveImage(const QString& path);

        // ==============================================
        // ========== МЕТОДЫ ДЛЯ ЗАПИСИ GIF =============
        // ==============================================

        /**
         * @brief Начало записи GIF анимации
         * @param fps Частота кадров в секунду (5-30)
         * @param duration_sec Длительность записи в секундах (1-10)
         * @details Запускает таймер для захвата кадров с заданной частотой.
         *          После завершения записи испускается сигнал recordingFinished.
         */
        void startRecording(const QString& path, int fps, int duration_sec);

        /**
         * @brief Остановка записи GIF анимации
         * @details Останавливает таймер и завершает запись
         */
        void stopRecording();

        /**
         * @brief Проверка состояния записи GIF
         * @return true если запись активна, иначе false
         */
        bool isRecording() const;

        signals:
          /**
           * @brief Сигнал прогресса записи GIF
           * @param frame Количество записанных кадров
           * @param total Общее количество кадров для записи
           */
        void recordingProgress(int frame, int total);

        /**
         * @brief Сигнал завершения записи GIF
         * @param path Путь к сохранённому файлу
         */
        void recordingFinished(const QString& path);

        /**
         * @brief Сигнал ошибки при записи GIF
         * @param error Текст ошибки
         */
        void recordingError(const QString& error);


    protected:
        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int w, int h) override;

        /**
         * @brief Захват текущего кадра для GIF
         * @details Устанавливает цвет фона, вызывает отрисовку,
         * захватывает пиксели через glReadPixels
         */
        void captureFrame();

    private:
        void compileShaders();

        ShaderProgram* shaderProgram_;
        Mesh* mesh_;
        float aspect_;

        // GIF запись
        std::unique_ptr<GifRecorder> gif_recorder_;  ///< Рекордер GIF
        QTimer* record_timer_ = nullptr;  ///< Таймер для захвата кадров

    };

}

#endif
