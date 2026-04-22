#ifndef S21_MAINWINDOW_H
#define S21_MAINWINDOW_H

#include "ui_mainwindow.h"
#include "glwidget.h"
#include <viewer/viewerSpec.h>

namespace s21 {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_Open_triggered();
    void on_action_Exit_triggered();
    void on_action_Orthographic_triggered();
    void on_action_Perspective_triggered();
    void on_action_Wireframe_triggered();
    void on_action_FlatShading_triggered();
    void on_action_SmoothShading_triggered();
    
    void on_btn_VertexColor_clicked();
    void onVertexSizeChanged(float value);
    void onVertexTypeChanged(int value);
    
    void on_btn_EdgeColor_clicked();
    void onEdgeWidthChanged(float value);
    void onEdgeTypeChanged(int value);

    void on_btn_BackgroundColor_clicked();
    void onCameraZoomChanged(int value);

private:
    void loadScene(const QString& path);
    void updateInfoLabels();
    void connectSignals() noexcept;

    Ui::MainWindow *ui;
    GLWidget *glWidget_;
    Viewer *viewer_;
    QString currentFileName_;
    int vertexCount_; // можно убрать
    int edgeCount_; // можно убрать
};

} // namespace s21

#endif // S21_MAINWINDOW_H
