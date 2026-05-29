#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow {
public:
    QAction *action_Open;
    QAction *action_Exit;
    QAction *action_Orthographic;
    QAction *action_Perspective;
    QAction *action_Wireframe;
    QAction *action_FlatShading;
    QAction *action_SmoothShading;
    QAction *action_RayTracing;
    QAction *action_SaveScreenshot;
    QAction *action_SaveGif;
    QWidget *centralWidget;
    QVBoxLayout *mainVerticalLayout;
    QHBoxLayout *contentLayout;

    // Справа: Панель настроек
    QGroupBox *groupBox_Settings;
    QVBoxLayout *settingsLayout;

    // --- Секция вершин (сворачиваемая) ---
    QGroupBox  *groupBox_Vertices;     // checkable groupbox = встроенный toggle
    QVBoxLayout *verticesLayout;
    QWidget    *verticesContent;       // контейнер содержимого
    QVBoxLayout *verticesContentLayout;
    QPushButton *btn_CollapseVertices; // кнопка ▾/▸

    QPushButton *btn_VertexColor;
    QDoubleSpinBox *spin_VertexSize;
    QComboBox *combo_VertexType;

    // --- Секция рёбер (сворачиваемая) ---
    QGroupBox  *groupBox_Edges;
    QVBoxLayout *edgesLayout;
    QWidget *edgesContent;
    QVBoxLayout *edgesContentLayout;
    QPushButton *btn_CollapseEdges;

    QPushButton *btn_EdgeColor;
    QDoubleSpinBox *spin_EdgeWidth;
    QComboBox *combo_EdgeType;
    QPushButton *btn_BackgroundColor;

    // Трансформация фигуры
    QHBoxLayout *rotLayout;
    QDoubleSpinBox *spin_rotX, *spin_rotY, *spin_rotZ;
    QHBoxLayout *transLayout;
    QDoubleSpinBox *spin_transX, *spin_transY, *spin_transZ;
    QHBoxLayout *scaleLayout;
    QDoubleSpinBox *spin_scaleX, *spin_scaleY, *spin_scaleZ;

    // Настройка источника света
    QHBoxLayout *lightLayout;
    QDoubleSpinBox *light_transX, *light_transY, *light_transZ;
    QPushButton *btn_LightColor;

    // Нижняя информационная панель
    QHBoxLayout *infoLayout;
    QLabel *label_FileInfo;
    QLabel *label_VertexCount;
    QLabel *label_EdgeCount;
    QLabel *label_displayType;
    QLabel *label_projectionType;

    QSpacerItem *horizontalSpacer;

    QMenuBar *menubar;
    QMenu *menu_File;
    QMenu *menu_Projection;
    QMenu *menu_DisplayType;
    QMenu *menu_Export;
    QStatusBar *statusbar;

    // ----------------------------------------------------------------
    //  Вспомогательная функция: подключает кнопку-переключатель
    //  к QWidget-контейнеру внутри groupbox.
    // ----------------------------------------------------------------
    static void connectCollapseButton(QPushButton *btn, QWidget *content) {
        // Лямбда переключает видимость и меняет символ кнопки
        QObject::connect(btn, &QPushButton::clicked, [btn, content]() {
            bool visible = !content->isVisible();
            content->setVisible(visible);
            btn->setText(visible ? "▾" : "▸");
        });
    }

    void setupUi(QMainWindow *MainWindow) {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1200, 800);
        MainWindow->setMinimumSize(300, 650);

        // Инициализация действий
        action_Open = new QAction(MainWindow);
        action_Exit = new QAction(MainWindow);
        action_Orthographic = new QAction(MainWindow);
        action_Perspective = new QAction(MainWindow);
        action_Wireframe = new QAction(MainWindow);
        action_FlatShading = new QAction(MainWindow);
        action_SmoothShading = new QAction(MainWindow);
        action_RayTracing = new QAction(MainWindow);
        action_SaveGif = new QAction(MainWindow);
        action_SaveScreenshot = new QAction(MainWindow);

        centralWidget = new QWidget(MainWindow);
        mainVerticalLayout = new QVBoxLayout(centralWidget);

        // Создаем горизонтальный слой для разделения контента
        contentLayout = new QHBoxLayout();

        // Панель настроек (будет справа)
        groupBox_Settings = new QGroupBox("Настройки вида", centralWidget);
        groupBox_Settings->setFixedWidth(250);
        settingsLayout = new QVBoxLayout(groupBox_Settings);

        // =============================================

        // ===========================
        // --- НАСТРОЙКА ОСВЕЩЕНИЯ ---
        // ===========================
        settingsLayout->addWidget(new QLabel("--- ИСТОЧНИК СВЕТА ---"));
        lightLayout = new QHBoxLayout();
        light_transX = new QDoubleSpinBox();
        light_transY = new QDoubleSpinBox();
        light_transZ = new QDoubleSpinBox();
        for (auto s : {light_transX, light_transY, light_transZ}) {
            s->setRange(-10.0, 10.0);
            s->setSingleStep(0.1);
            lightLayout->addWidget(s);
        }
        settingsLayout->addLayout(lightLayout);

        btn_LightColor = new QPushButton("Цвет источника света");
        settingsLayout->addWidget(btn_LightColor);

        settingsLayout->addSpacing(20);

        // =======================================
        // --- ВЕРШИНЫ (сворачиваемая секция) ---
        // =======================================
        groupBox_Vertices = new QGroupBox("ВЕРШИНЫ", centralWidget);
        verticesLayout    = new QVBoxLayout(groupBox_Vertices);
        verticesLayout->setContentsMargins(4, 4, 4, 4);

        // Строка-заголовок с кнопкой-переключателем
        btn_CollapseVertices = new QPushButton("▾");
        btn_CollapseVertices->setFixedSize(20, 20);
        btn_CollapseVertices->setFlat(true);
        btn_CollapseVertices->setToolTip("Свернуть / Развернуть");
        {
            auto *headerRow = new QHBoxLayout();
            headerRow->addStretch();
            headerRow->addWidget(btn_CollapseVertices);
            verticesLayout->addLayout(headerRow);
        }

        // Содержимое секции
        verticesContent       = new QWidget();
        verticesContentLayout = new QVBoxLayout(verticesContent);
        verticesContentLayout->setContentsMargins(0, 0, 0, 0);

        btn_VertexColor = new QPushButton("Цвет вершин");
        verticesContentLayout->addWidget(btn_VertexColor);

        verticesContentLayout->addWidget(new QLabel("Размер:"));
        spin_VertexSize = new QDoubleSpinBox();
        spin_VertexSize->setRange(0.0, 20.0);
        spin_VertexSize->setValue(1.0);
        spin_VertexSize->setSingleStep(0.01);
        verticesContentLayout->addWidget(spin_VertexSize);

        verticesContentLayout->addWidget(new QLabel("Тип:"));
        combo_VertexType = new QComboBox();
        combo_VertexType->addItems({"Нет", "Круг", "Квадрат"});
        verticesContentLayout->addWidget(combo_VertexType);

        verticesLayout->addWidget(verticesContent);
        connectCollapseButton(btn_CollapseVertices, verticesContent);

        settingsLayout->addWidget(groupBox_Vertices);
        settingsLayout->addSpacing(10);

        // =====================================
        // --- РЁБРА (сворачиваемая секция) ---
        // =====================================
        groupBox_Edges  = new QGroupBox("РЁБРА", centralWidget);
        edgesLayout     = new QVBoxLayout(groupBox_Edges);
        edgesLayout->setContentsMargins(4, 4, 4, 4);

        btn_CollapseEdges = new QPushButton("▾");
        btn_CollapseEdges->setFixedSize(20, 20);
        btn_CollapseEdges->setFlat(true);
        btn_CollapseEdges->setToolTip("Свернуть / Развернуть");
        {
            auto *headerRow = new QHBoxLayout();
            headerRow->addStretch();
            headerRow->addWidget(btn_CollapseEdges);
            edgesLayout->addLayout(headerRow);
        }

        edgesContent       = new QWidget();
        edgesContentLayout = new QVBoxLayout(edgesContent);
        edgesContentLayout->setContentsMargins(0, 0, 0, 0);

        btn_EdgeColor = new QPushButton("Цвет рёбер");
        edgesContentLayout->addWidget(btn_EdgeColor);

        edgesContentLayout->addWidget(new QLabel("Толщина:"));
        spin_EdgeWidth = new QDoubleSpinBox();
        spin_EdgeWidth->setRange(0.0, 20.0);
        spin_EdgeWidth->setValue(2.0);
        spin_EdgeWidth->setSingleStep(0.01);
        edgesContentLayout->addWidget(spin_EdgeWidth);

        edgesContentLayout->addWidget(new QLabel("Тип:"));
        combo_EdgeType = new QComboBox();
        combo_EdgeType->addItems({"Сплошной", "Пунктир"});
        edgesContentLayout->addWidget(combo_EdgeType);

        edgesLayout->addWidget(edgesContent);
        connectCollapseButton(btn_CollapseEdges, edgesContent);

        settingsLayout->addWidget(groupBox_Edges);
        settingsLayout->addSpacing(20);

        // ===========================
        // ПЕРЕМЕЩЕНИЕ (Translation)
        // ===========================
        settingsLayout->addWidget(new QLabel("--- ПЕРЕМЕЩЕНИЕ ФИГУРЫ ---"));
        transLayout = new QHBoxLayout();
        spin_transX = new QDoubleSpinBox();
        spin_transY = new QDoubleSpinBox();
        spin_transZ = new QDoubleSpinBox();
        for (auto s : {spin_transX, spin_transY, spin_transZ}) {
            s->setRange(-10.0, 10.0);
            s->setSingleStep(0.1);
            transLayout->addWidget(s);
        }
        settingsLayout->addLayout(transLayout);

        // ===========================
        // --- ПОВОРОТ (Rotation) ---
        // ===========================
        settingsLayout->addWidget(new QLabel("--- ПОВОРОТ ФИГУРЫ ---"));
        rotLayout = new QHBoxLayout();
        spin_rotX = new QDoubleSpinBox();
        spin_rotY = new QDoubleSpinBox();
        spin_rotZ = new QDoubleSpinBox();
        for (auto s : {spin_rotX, spin_rotY, spin_rotZ}) {
            s->setRange(-360.0, 360.0);
            rotLayout->addWidget(s);
        }
        settingsLayout->addLayout(rotLayout);

        // ===========================
        //      МАСШТАБ (Scale)
        // ===========================
        settingsLayout->addWidget(new QLabel("--- МАСШТАБ ФИГУРЫ ---"));
        scaleLayout = new QHBoxLayout();
        spin_scaleX = new QDoubleSpinBox();
        spin_scaleY = new QDoubleSpinBox();
        spin_scaleZ = new QDoubleSpinBox();
        for (auto s : {spin_scaleX, spin_scaleY, spin_scaleZ}) {
            s->setRange(0.1, 10.0);
            s->setValue(1.0); // По умолчанию масштаб 1:1
            s->setSingleStep(0.1);
            scaleLayout->addWidget(s);
        }
        settingsLayout->addLayout(scaleLayout);

        settingsLayout->addSpacing(20);

        // --- СЕКЦИЯ ОБЩИХ НАСТРОЕК ---
        settingsLayout->addWidget(new QLabel("--- ОБЩИЕ ---"));
        btn_BackgroundColor = new QPushButton("Цвет фона");
        settingsLayout->addWidget(btn_BackgroundColor);

        // =========================================

        settingsLayout->addStretch(); // Пружина вниз

        // Добавляем панель в горизонтальный слой
        contentLayout->addWidget(groupBox_Settings);

        // Добавляем горизонтальный слой в главный вертикальный
        mainVerticalLayout->addLayout(contentLayout);

        // ===========================
        // Нижняя информационная панель
        // ===========================
        infoLayout = new QHBoxLayout();
        label_FileInfo = new QLabel("Файл: не выбран");
        label_VertexCount = new QLabel("");
        label_EdgeCount = new QLabel("");
        label_projectionType = new QLabel("");
        label_displayType = new QLabel("");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        infoLayout->addWidget(label_FileInfo);
        infoLayout->addSpacing(15);
        infoLayout->addWidget(label_VertexCount);
        infoLayout->addSpacing(15);
        infoLayout->addWidget(label_EdgeCount);
        infoLayout->addSpacing(15);
        infoLayout->addWidget(label_projectionType);
        infoLayout->addSpacing(15);
        infoLayout->addWidget(label_displayType);

        infoLayout->addItem(horizontalSpacer);

        mainVerticalLayout->addLayout(infoLayout);
        MainWindow->setCentralWidget(centralWidget);

        // ================================
        // ========= Верхнее меню =========
        // ================================

        menubar = new QMenuBar(MainWindow);
        menu_File = new QMenu("Файл", menubar);
        menu_Projection = new QMenu("Проекция", menubar);
        menu_DisplayType = new QMenu("Отображение", menubar);
        menu_Export = new QMenu("Запись", menubar);
        MainWindow->setMenuBar(menubar);

        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menu_File->menuAction());
        menubar->addAction(menu_Projection->menuAction());
        menubar->addAction(menu_DisplayType->menuAction());
        menubar->addAction(menu_Export->menuAction());

        menu_File->addAction(action_Open);
        menu_File->addAction(action_Exit);

        menu_Projection->addAction(action_Orthographic);
        menu_Projection->addAction(action_Perspective);

        menu_DisplayType->addAction(action_Wireframe);
        menu_DisplayType->addAction(action_FlatShading);
        menu_DisplayType->addAction(action_SmoothShading);
        menu_DisplayType->addAction(action_RayTracing);
        menu_Export->addAction(action_SaveScreenshot);
        menu_Export->addAction(action_SaveGif);

        retranslateUi(MainWindow);
    }

    void retranslateUi(QMainWindow *MainWindow) {
        action_Open->setText("Открыть...");
        action_Exit->setText("Выход");
        action_Orthographic->setText("Параллельная");
        action_Perspective->setText("Центральная");
        action_Wireframe->setText("Каркасная модель");
        action_FlatShading->setText("Плоское затенение");
        action_SmoothShading->setText("Мягкое затенение");
        action_RayTracing->setText("Трассировка лучей");
        action_SaveScreenshot->setText("Скриншот");
        action_SaveGif->setText("Анимация (GIF)");
    }

    QFrame* createSeparator() {
        QFrame *line = new QFrame();
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setStyleSheet("background-color: #000;");
        return line;
    }
};

namespace Ui { class MainWindow: public Ui_MainWindow {}; }
QT_END_NAMESPACE
#endif
