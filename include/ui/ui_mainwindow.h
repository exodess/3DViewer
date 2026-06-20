#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QSlider>
#include <QCheckBox>

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

    // Прокручиваемая боковая панель
    QScrollArea *scrollArea_Settings;   // сам скролл-виджет
    QWidget *scrollContents;        // внутренний контейнер
    QVBoxLayout *scrollLayout;          // вертикальный слой внутри

    // ================================================================
    //  ЗОНА 1 — ИСТОЧНИКИ ОСВЕЩЕНИЯ
    // ================================================================
    QGroupBox *groupBox_Lights;
    QVBoxLayout *lightsLayout;

    QPushButton *btn_AddLight;

    QWidget *lightsControlWidget;
    QVBoxLayout *lightsControlLayout;

    QComboBox *combo_LightSelect;

    QWidget *lightSettingsWidget;
    QVBoxLayout *lightSettingsLayout;

    QLabel *label_LightPos;
    QHBoxLayout *lightPosLayout;
    QDoubleSpinBox *light_transX, *light_transY, *light_transZ;

    QLabel *label_LightIntensity;
    QDoubleSpinBox *light_intensity;

    QPushButton *btn_LightColor;

    int lightCounter = 0;

    // ================================================================
    //  ЗОНА 2 — ФИГУРА
    // ================================================================
    QGroupBox *groupBox_Figure;
    QVBoxLayout *figureLayout;

    QComboBox *combo_FigureSelect;

    // Вершины (сворачиваемая)
    QGroupBox *groupBox_Vertices;
    QVBoxLayout *verticesLayout;
    QWidget *verticesContent;
    QVBoxLayout *verticesContentLayout;
    QPushButton *btn_CollapseVertices;

    QPushButton *btn_VertexColor;
    QDoubleSpinBox *spin_VertexSize;
    QComboBox *combo_VertexType;

    // Рёбра (сворачиваемая)
    QGroupBox *groupBox_Edges;
    QVBoxLayout *edgesLayout;
    QWidget *edgesContent;
    QVBoxLayout *edgesContentLayout;
    QPushButton *btn_CollapseEdges;

    QPushButton *btn_EdgeColor;
    QDoubleSpinBox *spin_EdgeWidth;
    QComboBox *combo_EdgeType;

    // Настройки материала фигуры
    QGroupBox *groupBox_Material;
    QVBoxLayout *materialLayout;
    QWidget *materialContent;
    QVBoxLayout *materialContentLayout;

    QPushButton *btn_MaterialColor;

    QSlider *roughnessSlider;
    QSlider *metallicSlider;
    QSlider *refractiveSlider;
    QSlider *reflectivitySlider;
    QSlider *alphaSlider;

    // Трансформация фигуры
    QGroupBox *groupBox_Transform;
    QVBoxLayout *transformLayout;

    QLabel *label_FigTrans;
    QHBoxLayout *transLayout;
    QDoubleSpinBox *spin_transX, *spin_transY, *spin_transZ;

    QLabel *label_FigRot;
    QHBoxLayout *rotLayout;
    QDoubleSpinBox *spin_rotX, *spin_rotY, *spin_rotZ;

    QLabel *label_FigScale;
    QHBoxLayout *scaleLayout;
    QDoubleSpinBox *spin_scaleX, *spin_scaleY, *spin_scaleZ;

    // ================================================================
    //  ЗОНА 3 — ОБЩИЕ НАСТРОЙКИ
    // ================================================================
    QGroupBox *groupBox_General;
    QVBoxLayout *generalLayout;

    // Фон
    QPushButton *btn_BackgroundColor;

    // Отображение пола
    QCheckBox *check_FloorDisplay;

    // Камера — перемещение
    QLabel *label_CamTrans;
    QHBoxLayout *camTransLayout;
    QDoubleSpinBox *spin_camTransX, *spin_camTransY, *spin_camTransZ;

    // Камера — вращение
    QLabel *label_CamRot;
    QHBoxLayout *camRotLayout;
    QDoubleSpinBox *spin_camRotX, *spin_camRotY, *spin_camRotZ;

    // Камера — масштаб (zoom)
    QLabel *label_CamZoom;
    QHBoxLayout *camScaleLayout;
    QDoubleSpinBox *spin_camScaleX, *spin_camScaleY, *spin_camScaleZ;

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
    //  Вспомогательная: кнопка-переключатель для сворачиваемых секций
    // ----------------------------------------------------------------
    static void connectCollapseButton(QPushButton *btn, QWidget *content) {
        QObject::connect(btn, &QPushButton::clicked, [btn, content]() {
            bool v = !content->isVisible();
            content->setVisible(v);
            btn->setText(v ? "▾" : "▸");
        });
    }

    // ----------------------------------------------------------------
    //  Вспомогательная: создаёт горизонтальный ряд XYZ spinbox-ов
    // ----------------------------------------------------------------
    static QHBoxLayout* makeXYZRow(QDoubleSpinBox *&sx, QDoubleSpinBox *&sy,
                                   QDoubleSpinBox *&sz,
                                   double lo, double hi,
                                   double step, double defVal = 0.0)
    {
        auto *row = new QHBoxLayout();
        sx = new QDoubleSpinBox();
        sy = new QDoubleSpinBox();
        sz = new QDoubleSpinBox();
        for (QDoubleSpinBox * const &s : {sx, sy, sz}) {
            s->setRange(lo, hi);
            s->setSingleStep(step);
            s->setValue(defVal);
            s->setDecimals(2);
            row->addWidget(s);
        }
        return row;
    }

    static void createSlider(QSlider *&s, QVBoxLayout *layout) {
        s = new QSlider(Qt::Horizontal);

        s->setRange(0, 100);
        s->setStyleSheet(
            "QSlider::groove:horizontal {"
            "   border: 1px solid #444;"
            "   background: #333;" // Цвет пустой части
            "   height: 10px;"
            "}"
            "QSlider::sub-page:horizontal {"
            "   background: #4a82c9;" // Цвет заполненной части (синий)
            "}"
            "QSlider::handle:horizontal {"
            "   background: #eee;" // Цвет самого ползунка-бегунка
            "   width: 12px;"
            "}");
        layout->addWidget(s);
    }

    void setupUi(QMainWindow *MainWindow) {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1200, 800);
        MainWindow->setMinimumSize(400, 500);

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

        // Центральный виджет
        centralWidget = new QWidget(MainWindow);
        mainVerticalLayout = new QVBoxLayout(centralWidget);

        // Создаем горизонтальный слой для разделения контента
        contentLayout = new QHBoxLayout();

        // Прокручиваемая боковая панель
        scrollArea_Settings = new QScrollArea(centralWidget);
        scrollArea_Settings->setFixedWidth(300);
        scrollArea_Settings->setWidgetResizable(true);
        scrollArea_Settings->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea_Settings->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea_Settings->setFrameShape(QFrame::StyledPanel);

        scrollContents = new QWidget();
        scrollLayout = new QVBoxLayout(scrollContents);
        scrollLayout->setContentsMargins(6, 6, 6, 6);
        scrollLayout->setSpacing(8);

        // ============================================================
        //  ЗОНА 1 — ИСТОЧНИКИ ОСВЕЩЕНИЯ
        // ============================================================
        groupBox_Lights = new QGroupBox("Источники освещения");
        lightsLayout = new QVBoxLayout(groupBox_Lights);
        lightsLayout->setContentsMargins(6, 8, 6, 6);
        lightsLayout->setSpacing(5);

        btn_AddLight = new QPushButton("+ Добавить источник");
        lightsLayout->addWidget(btn_AddLight);

        // Контейнер (скрыт пока нет источников)
        lightsControlWidget = new QWidget();
        lightsControlLayout = new QVBoxLayout(lightsControlWidget);
        lightsControlLayout->setContentsMargins(0, 4, 0, 0);
        lightsControlLayout->setSpacing(4);

        combo_LightSelect = new QComboBox();
        combo_LightSelect->setToolTip("Выберите источник для настройки");
        combo_LightSelect->addItem("Фоновое освещение");
        lightsControlLayout->addWidget(combo_LightSelect);

        lightSettingsWidget = new QWidget();
        lightSettingsLayout = new QVBoxLayout(lightSettingsWidget);
        lightSettingsLayout->setContentsMargins(0, 2, 0, 2);
        lightSettingsLayout->setSpacing(4);

        label_LightPos = new QLabel("Положение (X / Y / Z):");
        lightSettingsLayout->addWidget(label_LightPos);
        lightPosLayout = new QHBoxLayout();
        light_transX = light_transY = light_transZ = nullptr;
        light_transX = new QDoubleSpinBox();
        light_transY = new QDoubleSpinBox();
        light_transZ = new QDoubleSpinBox();
        for (auto *s : {light_transX, light_transY, light_transZ}) {
            s->setRange(-100.0, 100.0); s->setSingleStep(0.1); s->setDecimals(2);
            lightPosLayout->addWidget(s);
        }
        lightSettingsLayout->addLayout(lightPosLayout);

        label_LightIntensity = new QLabel("Интенсивность:");
        lightSettingsLayout->addWidget(label_LightIntensity);
        light_intensity = new QDoubleSpinBox();
        light_intensity->setRange(0.0, 1.0);
        light_intensity->setSingleStep(0.05);
        light_intensity->setDecimals(2);
        lightSettingsLayout->addWidget(light_intensity);

        btn_LightColor = new QPushButton("Цвет источника");
        lightSettingsLayout->addWidget(btn_LightColor);

        lightsControlLayout->addWidget(lightSettingsWidget);
        lightsLayout->addWidget(lightsControlWidget);
        lightsControlWidget->setVisible(true);

        scrollLayout->addWidget(groupBox_Lights);

        // ============================================================
        //  ЗОНА 2 — ФИГУРА
        // ============================================================
        groupBox_Figure = new QGroupBox("Фигура");
        figureLayout = new QVBoxLayout(groupBox_Figure);
        figureLayout->setContentsMargins(6, 8, 6, 6);
        figureLayout->setSpacing(6);

        combo_FigureSelect = new QComboBox();
        combo_FigureSelect->setToolTip("Выберите фигуру для настройки");
        figureLayout->addWidget(combo_FigureSelect);

        // Вершины
        groupBox_Vertices = new QGroupBox("Вершины");
        verticesLayout = new QVBoxLayout(groupBox_Vertices);
        verticesLayout->setContentsMargins(4, 4, 4, 4);

        // Строка-заголовок с кнопкой-переключателем
        btn_CollapseVertices = new QPushButton("▾");
        btn_CollapseVertices->setFixedSize(20, 20);
        btn_CollapseVertices->setFlat(true);
        btn_CollapseVertices->setToolTip("Свернуть / Развернуть");

        auto *headerVertRow = new QHBoxLayout();
        headerVertRow->addStretch();
        headerVertRow->addWidget(btn_CollapseVertices);
        verticesLayout->addLayout(headerVertRow);

        // Содержимое секции
        verticesContent = new QWidget();
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
        figureLayout->addWidget(groupBox_Vertices);

        // Рёбра
        groupBox_Edges = new QGroupBox("Рёбра");
        edgesLayout = new QVBoxLayout(groupBox_Edges);
        edgesLayout->setContentsMargins(4, 4, 4, 4);

        btn_CollapseEdges = new QPushButton("▾");
        btn_CollapseEdges->setFixedSize(20, 20);
        btn_CollapseEdges->setFlat(true);
        btn_CollapseEdges->setToolTip("Свернуть / Развернуть");

        auto *headerEdgRow = new QHBoxLayout();
        headerEdgRow->addStretch();
        headerEdgRow->addWidget(btn_CollapseEdges);
        edgesLayout->addLayout(headerEdgRow);

        edgesContent = new QWidget();
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
        figureLayout->addWidget(groupBox_Edges);

        // Материал фигуры

        groupBox_Material = new QGroupBox("Материалы");
        materialLayout = new QVBoxLayout(groupBox_Material);
        materialLayout->setContentsMargins(4, 6, 4, 6);
        materialLayout->setSpacing(4);

        btn_MaterialColor = new QPushButton("Базовый цвет");
        materialLayout->addWidget(btn_MaterialColor);

        materialLayout->addWidget(new QLabel("Шероховатость"));
        createSlider(roughnessSlider, materialLayout);

        materialLayout->addWidget(new QLabel("Металличность"));
        createSlider(metallicSlider, materialLayout);

        materialLayout->addWidget(new QLabel("Преломление"));
        createSlider(refractiveSlider, materialLayout);

        materialLayout->addWidget(new QLabel("Отражение"));
        createSlider(reflectivitySlider, materialLayout);

        materialLayout->addWidget(new QLabel("Прозрачность"));
        createSlider(alphaSlider, materialLayout);

        groupBox_Material->setVisible(false);
        figureLayout->addWidget(groupBox_Material);

        // Трансформация фигуры
        groupBox_Transform = new QGroupBox("Трансформация");
        transformLayout = new QVBoxLayout(groupBox_Transform);
        transformLayout->setContentsMargins(4, 6, 4, 6);
        transformLayout->setSpacing(4);

        label_FigTrans = new QLabel("Перемещение (X / Y / Z):");
        transformLayout->addWidget(label_FigTrans);
        transLayout = new QHBoxLayout();
        spin_transX = new QDoubleSpinBox();
        spin_transY = new QDoubleSpinBox();
        spin_transZ = new QDoubleSpinBox();
        for (auto s : {spin_transX, spin_transY, spin_transZ}) {
            s->setRange(-10.0, 10.0);
            s->setSingleStep(0.1);
            transLayout->addWidget(s);
        }
        transformLayout->addLayout(transLayout);

        label_FigRot = new QLabel("Поворот (X / Y / Z):");
        transformLayout->addWidget(label_FigRot);
        rotLayout = new QHBoxLayout();
        spin_rotX = new QDoubleSpinBox();
        spin_rotY = new QDoubleSpinBox();
        spin_rotZ = new QDoubleSpinBox();
        for (auto s : {spin_rotX, spin_rotY, spin_rotZ}) {
            s->setRange(-360.0, 360.0);
            rotLayout->addWidget(s);
        }
        transformLayout->addLayout(rotLayout);

        label_FigScale = new QLabel("Масштаб (X / Y / Z):");
        transformLayout->addWidget(label_FigScale);
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
        transformLayout->addLayout(scaleLayout);

        figureLayout->addWidget(groupBox_Transform);
        scrollLayout->addWidget(groupBox_Figure);

        // Зона фигуры скрыта до открытия файла
        groupBox_Figure->setVisible(false);

        // ============================================================
        //  ЗОНА 3 — ОБЩИЕ НАСТРОЙКИ
        // ============================================================
        groupBox_General = new QGroupBox("Общие настройки");
        generalLayout = new QVBoxLayout(groupBox_General);
        generalLayout->setContentsMargins(6, 8, 6, 6);
        generalLayout->setSpacing(5);

        btn_BackgroundColor = new QPushButton("Цвет фона");
        generalLayout->addWidget(btn_BackgroundColor);

        check_FloorDisplay = new QCheckBox("Отображение пола");
        generalLayout->addWidget(check_FloorDisplay);

        // Камера — перемещение
        label_CamTrans = new QLabel("Камера — перемещение (X / Y / Z):");
        label_CamTrans->setWordWrap(true);
        generalLayout->addWidget(label_CamTrans);
        camTransLayout = new QHBoxLayout();
        spin_camTransX = new QDoubleSpinBox();
        spin_camTransY = new QDoubleSpinBox();
        spin_camTransZ = new QDoubleSpinBox();
        for (auto *s : {spin_camTransX, spin_camTransY, spin_camTransZ}) {
            s->setRange(-1000.0, 1000.0); s->setSingleStep(0.5); s->setDecimals(2);
            camTransLayout->addWidget(s);
        }
        generalLayout->addLayout(camTransLayout);

        // Камера — вращение
        label_CamRot = new QLabel("Камера — вращение (X / Y / Z):");
        label_CamRot->setWordWrap(true);
        generalLayout->addWidget(label_CamRot);
        camRotLayout = new QHBoxLayout();
        spin_camRotX = new QDoubleSpinBox();
        spin_camRotY = new QDoubleSpinBox();
        spin_camRotZ = new QDoubleSpinBox();
        for (auto *s : {spin_camRotX, spin_camRotY, spin_camRotZ}) {
            s->setRange(-360.0, 360.0); s->setSingleStep(1.0); s->setDecimals(1);
            camRotLayout->addWidget(s);
        }
        generalLayout->addLayout(camRotLayout);

        // Камера — масштаб
        label_CamZoom = new QLabel("Камера — масштаб:");
        label_CamZoom->setWordWrap(true);
        generalLayout->addWidget(label_CamZoom);
        camScaleLayout = new QHBoxLayout();
        spin_camScaleX = new QDoubleSpinBox();
        spin_camScaleY = new QDoubleSpinBox();
        spin_camScaleZ = new QDoubleSpinBox();

        for (auto *s : {spin_camScaleX, spin_camScaleY, spin_camScaleZ}) {
            s->setRange(0.1, 10.0);
            s->setSingleStep(0.1);
            s->setDecimals(1);
            camScaleLayout->addWidget(s);
        }
        generalLayout->addLayout(camScaleLayout);

        scrollLayout->addWidget(groupBox_General);

        // Пружина в конце, чтобы секции прижимались к верху
        scrollLayout->addStretch();

        // Собираем панель прокрутки
        scrollArea_Settings->setWidget(scrollContents);
        contentLayout->addWidget(scrollArea_Settings);
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
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line->setStyleSheet("background-color: #000;");
        return line;
    }
};

namespace Ui { class MainWindow: public Ui_MainWindow {}; }
QT_END_NAMESPACE
#endif