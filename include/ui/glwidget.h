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
        GLWidget(QWidget* parent, Ui::MainWindow* ui);
        ~GLWidget();

        // Сохранение и загрузка настроек
        void saveSettingsToFile(const QString& filePath);
        void loadSettingsFromFile(const QString& filePath);

        void DrawScene(Scene& scene) override;
        uint32_t countVertices() noexcept;
        uint32_t countSurfaces() noexcept;

        void setRotation(float x, float y, float z) noexcept;
        void setTranslation(float x, float y, float z) noexcept;
        void setScale(float x, float y, float z) noexcept;

        void setNewVerticesSize(float) noexcept;
        void setNewVerticesColor(float, float, float) noexcept;
        void setNewVerticesMode(VerticesMode) noexcept;

        void setNewEdgesSize(float) noexcept;
        void setNewEdgesColor(float, float, float) noexcept;
        void setNewEdgesMode(EdgesMode) noexcept;

        void setNewModelMatrix() noexcept;
        void setNewViewMatrix(float) noexcept;
        void setNewProjectionType(ProjectionType type) noexcept;

        void setNewAspectRatio(float) noexcept;
        void setNewBackgroundColor(float r, float g, float b) noexcept;

        void setNewLightPosition(float x, float y, float z) noexcept; ///< Смещение источника освещения
        void setNewLightColor(float r, float g, float b) noexcept; ///< Изменение цвета источника света
        void setNewDisplayType(DisplayType display_type) noexcept; ///< Смена режима отображения фигуры

        // Геттеры для интерфейса
        QColor getEdgeColor() noexcept;
        QColor getVertexColor() noexcept;
        QColor getBackgroundColor() noexcept;
        QColor getLightColor() noexcept;
        float getVertexSize() noexcept;
        float getEdgeSize() noexcept;
        Point3D getLightPosition() noexcept;
        int getVertexMode() noexcept;
        int getEdgeMode() noexcept;
        int getDisplayType() noexcept;
        int getProjectionType() noexcept;

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

        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void wheelEvent(QWheelEvent* event) override;

        /**
         * @brief Захват текущего кадра для GIF
         * @details Устанавливает цвет фона, вызывает отрисовку,
         * захватывает пиксели через glReadPixels
         */
        void captureFrame();

    private:
        void compileShaders();
        QJsonObject colorToJson(const Point3D& color) noexcept;
        Point3D colorFromJson(const QJsonObject& jobject) noexcept;

        Scene* scene_;
        ShaderProgram* shaderProgram_;
        Mesh* mesh_;

        Ui::MainWindow* ui_;

        // настройки для матриц
        float cameraZoom_;
        Point3D cameraVector_;
        Point3D rotationVector_;
        Point3D translationVector_;
        Point3D scaleVector_;

		    // матрицы
        TransformMatrix modelMatrix_;
        TransformMatrix viewMatrix_;
        TransformMatrix projectionMatrix_;

        // настройки для вершин
        Point3D vertColor_;
        float vertSize_;
        VerticesMode vertMode_;

        // настройки для ребер
        Point3D edgColor_;
        float edgSize_;
        EdgesMode edgMode_;

        // цвет фона
        Point3D backColor_;

        // тип проекции
        ProjectionType projectionType_;

        // тип отрисовки фигуры
        DisplayType displayType_;
        Point3D lightColor_;
        Point3D lightPosition_;

        QPoint lastPos_;
        Point3D rotation_;
        Point3D translation_;

        // GIF запись
        std::unique_ptr<GifRecorder> gif_recorder_;  ///< Рекордер GIF
        QTimer* record_timer_ = nullptr;  ///< Таймер для захвата кадров

    };

}

#endif
