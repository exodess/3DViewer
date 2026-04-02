#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
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
    QWidget *centralwidget;
    QVBoxLayout *mainVerticalLayout;   // Главный вертикальный слой
    QHBoxLayout *contentLayout;        // Слой для разделения "Экран | Настройки"
    
    // Справа: Панель настроек
    QGroupBox *groupBox_Settings;
    QVBoxLayout *settingsLayout;
    
    // Элементы управления цветом и объектами
    QPushButton *btn_VertexColor;
    QDoubleSpinBox *spin_VertexSize;
    QComboBox *combo_VertexType;
    QPushButton *btn_EdgeColor;
    QDoubleSpinBox *spin_EdgeWidth;
    QComboBox *combo_EdgeType;
    QPushButton *btn_BackgroundColor;
    QSlider *slider_Zoom;
    QLabel *label_ZoomValue;

    // Трансформация фигуры
    QHBoxLayout *rotLayout;
    QDoubleSpinBox* spin_rotX;
    QDoubleSpinBox* spin_rotY;
    QDoubleSpinBox* spin_rotZ;
    QHBoxLayout *transLayout;
    QDoubleSpinBox* spin_transX;
    QDoubleSpinBox* spin_transY;
    QDoubleSpinBox* spin_transZ;
    QHBoxLayout *scaleLayout;
    QDoubleSpinBox* spin_scaleX;
    QDoubleSpinBox* spin_scaleY;
    QDoubleSpinBox* spin_scaleZ;

    // Снизу: Инфо-панель
    QHBoxLayout *infoLayout;
    QLabel *label_FileInfo;
    QLabel *label_VertexCount;
    QLabel *label_EdgeCount;
    QSpacerItem *horizontalSpacer;
    
    QMenuBar *menubar;
    QMenu *menu_File;
    QMenu *menu_Projection;
    QStatusBar *statusbar;

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

        centralwidget = new QWidget(MainWindow);
        mainVerticalLayout = new QVBoxLayout(centralwidget);

        // Создаем горизонтальный слой для разделения контента
        contentLayout = new QHBoxLayout();

        // Панель настроек (будет справа)
        groupBox_Settings = new QGroupBox("Настройки вида", centralwidget);
        groupBox_Settings->setFixedWidth(250);
        settingsLayout = new QVBoxLayout(groupBox_Settings);

        // --- СЕКЦИЯ ОБЩИХ НАСТРОЕК (ФОН И ЗУМ) ---
        settingsLayout->addWidget(new QLabel("--- ОБЩИЕ ---"));
        btn_BackgroundColor = new QPushButton("Цвет фона");
        settingsLayout->addWidget(btn_BackgroundColor);

        settingsLayout->addWidget(new QLabel("Отдаление камеры:"));
        QHBoxLayout *zoomControlLayout = new QHBoxLayout();
        slider_Zoom = new QSlider(Qt::Horizontal);
        slider_Zoom->setRange(1, 200); // От 1% до 200%
        slider_Zoom->setValue(100);    // По умолчанию 100%
        label_ZoomValue = new QLabel("100%");
        zoomControlLayout->addWidget(slider_Zoom);
        zoomControlLayout->addWidget(label_ZoomValue);
        settingsLayout->addLayout(zoomControlLayout);

        settingsLayout->addSpacing(20);

        // Секция Вершин
        settingsLayout->addWidget(new QLabel("--- ВЕРШИНЫ ---"));
        btn_VertexColor = new QPushButton("Цвет вершин");
        settingsLayout->addWidget(btn_VertexColor);

        settingsLayout->addWidget(new QLabel("Размер:"));
        spin_VertexSize = new QDoubleSpinBox();
        spin_VertexSize->setRange(0.0, 20.0);
        spin_VertexSize->setValue(1.0);
        spin_VertexSize->setSingleStep(0.01);
        settingsLayout->addWidget(spin_VertexSize);

        settingsLayout->addWidget(new QLabel("Тип:"));
        combo_VertexType = new QComboBox();
        combo_VertexType->addItems({"Нет", "Круг", "Квадрат"});
        settingsLayout->addWidget(combo_VertexType);

        settingsLayout->addSpacing(20);

        // Секция Ребер
        settingsLayout->addWidget(new QLabel("--- РЕБРА ---"));
        btn_EdgeColor = new QPushButton("Цвет ребер");
        settingsLayout->addWidget(btn_EdgeColor);

        settingsLayout->addWidget(new QLabel("Толщина:"));
        spin_EdgeWidth = new QDoubleSpinBox();
        spin_EdgeWidth->setRange(0.0, 20.0);
        spin_EdgeWidth->setValue(2.0);
        spin_EdgeWidth->setSingleStep(0.01);
        settingsLayout->addWidget(spin_EdgeWidth);

        settingsLayout->addWidget(new QLabel("Тип:"));
        combo_EdgeType = new QComboBox();
        combo_EdgeType->addItems({"Сплошной", "Пунктир"});
        settingsLayout->addWidget(combo_EdgeType);

        settingsLayout->addSpacing(40);

        // ===========================
        // ПЕРЕМЕЩЕНИЕ (Translation)
        // ===========================
        settingsLayout->addWidget(new QLabel("--- ПЕРЕМЕЩЕНИЕ ФИГУРЫ ---"));
        transLayout = new QHBoxLayout();
        spin_transX = new QDoubleSpinBox(); spin_transY = new QDoubleSpinBox(); spin_transZ = new QDoubleSpinBox();
        
        for(auto s : {spin_transX, spin_transY, spin_transZ}) {
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
        spin_rotX = new QDoubleSpinBox(); spin_rotY = new QDoubleSpinBox(); spin_rotZ = new QDoubleSpinBox();
        for(auto s : {spin_rotX, spin_rotY, spin_rotZ}) {
            s->setRange(-360.0, 360.0);
            rotLayout->addWidget(s);
        }
        settingsLayout->addLayout(rotLayout);
        
        // ===========================
        //      МАСШТАБ (Scale)
        // =========================== 
        settingsLayout->addWidget(new QLabel("--- МАСШТАБ ФИГУРЫ ---"));
        scaleLayout = new QHBoxLayout();
        spin_scaleX = new QDoubleSpinBox(); spin_scaleY = new QDoubleSpinBox(); spin_scaleZ = new QDoubleSpinBox();
        for(auto s : {spin_scaleX, spin_scaleY, spin_scaleZ}) {
            s->setRange(0.1, 10.0);
            s->setValue(1.0); // По умолчанию масштаб 1:1
            s->setSingleStep(0.1);
            scaleLayout->addWidget(s);
        }
        settingsLayout->addLayout(scaleLayout);

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
        label_VertexCount = new QLabel("Вершин: 0");
        label_EdgeCount = new QLabel("Ребер: 0");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        infoLayout->addWidget(label_FileInfo);
        infoLayout->addWidget(label_VertexCount);
        infoLayout->addWidget(label_EdgeCount);
        infoLayout->addItem(horizontalSpacer);

        mainVerticalLayout->addLayout(infoLayout);
        MainWindow->setCentralWidget(centralwidget);

        // ================================
        // ========= Верхнее меню =========
        // ================================

        menubar = new QMenuBar(MainWindow);
        menu_File = new QMenu("Файл", menubar);
        menu_Projection = new QMenu("Проекция", menubar);
        MainWindow->setMenuBar(menubar);

        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menu_File->menuAction());
        menubar->addAction(menu_Projection->menuAction());
        menu_File->addAction(action_Open);
        menu_File->addAction(action_Exit);
        menu_Projection->addAction(action_Orthographic);
        menu_Projection->addAction(action_Perspective);

        retranslateUi(MainWindow);
    }

    void retranslateUi(QMainWindow *MainWindow) {
        action_Open->setText("Открыть...");
        action_Exit->setText("Выход");
        action_Orthographic->setText("Параллельная");
        action_Perspective->setText("Центральная");
    }
};

namespace Ui { class MainWindow: public Ui_MainWindow {}; }
QT_END_NAMESPACE
#endif
