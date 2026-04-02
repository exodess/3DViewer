#ifndef S21_GLWIDGET_H
#define S21_GLWIDGET_H

#include <QCoreApplication>
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <iostream>

#include "viewer/graphicSpec.h"

namespace s21 {

class GLWidget : public QOpenGLWidget, public BaseDrawerScene, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit GLWidget(QWidget* parent = nullptr);
    ~GLWidget();
    
    // Сохранение и загрузка настроек
    void saveSettingsToFile(const QString& filePath);
    void loadSettingsFromFile(const QString& filePath);

    void DrawScene(Scene& scene) override;
    uint32_t countVertices() noexcept;
    uint32_t countEdges() noexcept;

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

    // Геттеры для интерфейса
    QColor getEdgeColor() noexcept;
    QColor getVertexColor() noexcept;
    QColor getBackgroundColor() noexcept;
    float getVertexSize() noexcept;
    float getEdgeSize() noexcept;
    int getVertexMode() noexcept;
    int getEdgeMode() noexcept;

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    void compileShaders();
    QJsonObject colorToJson(const Point3D& color) noexcept;
    Point3D colorFromJson(const QJsonObject& jobject) noexcept;

    Scene* scene_;
    ShaderProgram* shaderProgram_;
    Mesh* mesh_;

    // настройки для матриц
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

};

} // namespace s21

#endif // S21_GLWIDGET_H
