#ifndef VIEWER_MAINWINDOW_H
#define VIEWER_MAINWINDOW_H

#include "ui_mainwindow.h"
#include "glwidget.h"
#include "viewer/viewerSpec.h"

namespace viewer {

    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    protected:
        /**
         * @brief Отслеживание нажатия клавиш для управления сценой
         * - Левая кнопка мыши: Перемещение фигуры в фокусе
         * - Левая кнопка мыши + Ctrl: Перемещение камеры
         * - Правая кнопка мыши: Вращение фигуры в фокусе
         * - Правая кнопка мыши + Ctrl: Вращение камеры
         */
        void mousePressEvent(QMouseEvent* event) override;

        /**
         * @brief Отслеживание перемещения мыши для задания трансформации объектов на сцене.
         * Необходимо для упрощенного управления сценой как замена спинбоксам
         */
        void mouseMoveEvent(QMouseEvent* event) override;

        /**
         * @brief Отслеживание прокрутки колесика мыши
         * - Вращение колесика мыши: масштабирование фигуры в фокусе
         */
        void wheelEvent(QWheelEvent* event) override;

    private slots:
        // Действия в верхнем меню
        void on_action_Open_triggered();
        void on_action_Exit_triggered();
        void on_action_Orthographic_triggered();
        void on_action_Perspective_triggered();
        void on_action_Wireframe_triggered();
        void on_action_FlatShading_triggered();
        void on_action_SmoothShading_triggered();
        void on_action_SaveScreenshot_triggered();
        void on_action_SaveGif_triggered();

        // Кнопки для смены цветов
        void on_btn_VertexColor_clicked();
        void on_btn_EdgeColor_clicked();
        void on_btn_BackgroundColor_clicked();
        void on_btn_LightColor_clicked();

        void onVertexTypeChanged(int value);
        void onVertexSizeChanged(float value);
        void onEdgeWidthChanged(float value);
        void onEdgeTypeChanged(int value);

    private:
        void loadScene(const QString& path);
        void updateInfoLabels();
        void connectSignals() noexcept;

        /**
         * @brief Сохранение настроек программы для их восстановления при перезагрузке
         * @param filePath Путь до JSON файла, куда будут записаны настройки
         */
        void saveSettingsToFile(const QString& filePath);

        /**
         * @brief Загрузка сохраненных ранее настроек программы
         * @param filePath Путь до JSON файла с настройками
         */
        void loadSettingsFromFile(const QString& filePath);

        /**
         * @brief Преобразование цвета из Point3D в формат, понятный JSON
         * @param color Цвет, хранящийся в виде структуры RGB
         * @return JSON-формат, позволяющий хранить цвет
         */
        QJsonObject colorToJson(const Point3D& color) noexcept;

        /**
         * @brief Преобразование цвета из JSON файла в Point3D, понятный программе
         * @param jobject Цвет, хранящийся в JSON формате
         * @return Структура RGB
         */
        Point3D colorFromJson(const QJsonObject& jobject) noexcept;

        Ui::MainWindow *ui; ///< Элементы UI программы
        Viewer *viewer_; ///< Менеджер ресурсов
        QString currentFileName_; ///< Название последнего считанного файла
        int current_figure_; ///< Номер текущей фигуры в фокусе
        int count_figures_; ///< Количество фигур на сцене

        QPoint lastPos_; ///< Последняя позиция мыши
    };

} // namespace viewer

#endif
