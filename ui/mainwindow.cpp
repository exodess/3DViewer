#include "ui/mainwindow.h"
#include <QApplication>
#include <QColorDialog> // Обязательно для диалога выбора цвета
#include <iostream>

namespace viewer {

	MainWindow::MainWindow(QWidget *parent)
	    : QMainWindow(parent)
	    , ui(new Ui::MainWindow)
	    , glWidget_(nullptr)
	    , viewer_(nullptr)
	{
		ui->setupUi(this);
		this->setMinimumSize(300, 650);

		glWidget_ = new GLWidget(this, ui);
		viewer_ = new Viewer(new FileReader(), glWidget_);

		ui->contentLayout->insertWidget(0, glWidget_, 1); // 1 - это stretch factor (растяжение)
		connectSignals();

		std::cout << "[MainWindow] Инициализация завершена\n";
	}

	MainWindow::~MainWindow() {
		if (viewer_) { delete viewer_; }
		delete ui;
	}

	void MainWindow::connectSignals() noexcept {

		connect(ui->slider_Zoom, &QSlider::valueChanged, this, &MainWindow::onCameraZoomChanged);

		// СОЕДИНЕНИЕ СИГНАЛОВ МЕНЮ (Actions)
		connect(ui->action_Open, &QAction::triggered, this, &MainWindow::on_action_Open_triggered);
		connect(ui->action_Exit, &QAction::triggered, this, &MainWindow::on_action_Exit_triggered);
		connect(ui->action_Orthographic, &QAction::triggered, this, &MainWindow::on_action_Orthographic_triggered);
		connect(ui->action_Perspective, &QAction::triggered, this, &MainWindow::on_action_Perspective_triggered);
		connect(ui->action_Wireframe, &QAction::triggered, this, &MainWindow::on_action_Wireframe_triggered);
		connect(ui->action_FlatShading, &QAction::triggered, this, &MainWindow::on_action_FlatShading_triggered);
		connect(ui->action_SmoothShading, &QAction::triggered, this, &MainWindow::on_action_SmoothShading_triggered);

		// СОЕДИНЕНИЕ КНОПОК ЦВЕТА
		connect(ui->btn_BackgroundColor, &QPushButton::clicked, this, &MainWindow::on_btn_BackgroundColor_clicked);
		connect(ui->btn_VertexColor, &QPushButton::clicked, this, &MainWindow::on_btn_VertexColor_clicked);
		connect(ui->btn_EdgeColor, &QPushButton::clicked, this, &MainWindow::on_btn_EdgeColor_clicked);
		connect(ui->btn_LightColor, &QPushButton::clicked, this, &MainWindow::on_btn_LightColor_clicked);

		// Соединяем Спинбоксы размеров
		connect(ui->spin_VertexSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		        this, &MainWindow::onVertexSizeChanged);

		connect(ui->spin_EdgeWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		        this, &MainWindow::onEdgeWidthChanged);

		// СОЕДИНЕНИЕ ВЫПАДАЮЩИХ СПИСКОВ
		connect(ui->combo_VertexType, QOverload<int>::of(&QComboBox::currentIndexChanged),
		        this, &MainWindow::onVertexTypeChanged);
		connect(ui->combo_EdgeType, QOverload<int>::of(&QComboBox::currentIndexChanged),
		        this, &MainWindow::onEdgeTypeChanged);

		auto updateTransform = [this]() {
			glWidget_->setTranslation(ui->spin_transX->value(), ui->spin_transY->value(), ui->spin_transZ->value());
			glWidget_->setRotation(ui->spin_rotX->value(), ui->spin_rotY->value(), ui->spin_rotZ->value());
			glWidget_->setScale(ui->spin_scaleX->value(), ui->spin_scaleY->value(), ui->spin_scaleZ->value());
			glWidget_->setNewLightPosition(ui->light_transX->value(), ui->light_transY->value(), ui->light_transZ->value());
		};

		// Соединяем все спинбоксы с обновлением
		for(auto s : {ui->spin_transX, ui->spin_transY, ui->spin_transZ,
		              ui->spin_rotX, ui->spin_rotY, ui->spin_rotZ,
		              ui->spin_scaleX, ui->spin_scaleY, ui->spin_scaleZ,
		              ui->light_transX, ui->light_transY, ui->light_transZ}) {
		    connect(s, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updateTransform);
		}

		// Устанавливаем значения на кнопках:
		ui->btn_VertexColor->setStyleSheet(QString("background-color: %1").arg(glWidget_->getVertexColor().name()));
		ui->btn_EdgeColor->setStyleSheet(QString("background-color: %1").arg(glWidget_->getEdgeColor().name()));
		ui->btn_BackgroundColor->setStyleSheet(QString("background-color: %1").arg(glWidget_->getBackgroundColor().name()));
		ui->btn_LightColor->setStyleSheet(QString("background-color: %1").arg(glWidget_->getLightColor().name()));

		ui->spin_VertexSize->setValue(glWidget_->getVertexSize());
		ui->spin_EdgeWidth->setValue(glWidget_->getEdgeSize());
		if(glWidget_->getEdgeMode() == 0) {
			ui->combo_EdgeType->setCurrentText("Сплошной");
		}
		else {
			ui->combo_EdgeType->setCurrentText("Пунктир");
		}

		if(glWidget_->getVertexMode() == 0) {
			ui->combo_VertexType->setCurrentText("Нет");
		}
		else if(glWidget_->getVertexMode() == 1) {
			ui->combo_VertexType->setCurrentText("Круг");
		}
		else {
			ui->combo_VertexType->setCurrentText("Квадрат");
		}

	}

	//Обработчик действия "Открыть файл"
	void MainWindow::on_action_Open_triggered() {
		QString fileName = QFileDialog::getOpenFileName(
	        this, "Открыть 3D модель", QString(),
	        "OBJ Files (*.obj *.vobj);;All Files (*)");

		if (!fileName.isEmpty()) {
			loadScene(fileName);
		}
	}

	// Слоты для ComboBox
	void MainWindow::onVertexTypeChanged(int index) {
		// index: 0 - Нет, 1 - Круг, 2 - Квадрат
		VerticesMode mode = VerticesMode::SQUARE;

		if(index == 0) mode = VerticesMode::NONE;
		else if(index == 1) mode = VerticesMode::CIRCLE;

		glWidget_->setNewVerticesMode(mode);
	}

	void MainWindow::onEdgeTypeChanged(int index) {
		// index: 0 - Сплошная, 1 - Пунктир
		EdgesMode mode = (index == 1) ? EdgesMode::DASHED : EdgesMode::SOLID;
		glWidget_->setNewEdgesMode(mode);
	}

	// Слот для изменения размера вершин
	void MainWindow::onVertexSizeChanged(float value) {
		// Передаем float в GLWidget (например, 10 -> 10.0f)
		std::cout << "[MainWindow] Нажата кнопка для смены размера вершин...\n";
		glWidget_->setNewVerticesSize(value / 100);
	}

	// Слот для изменения толщины ребер
	void MainWindow::onEdgeWidthChanged(float value) {
		glWidget_->setNewEdgesSize(value / 1000);
	}

	// Слот для выбора цвета вершин
	void MainWindow::on_btn_VertexColor_clicked() {
		// Открываем стандартное окно выбора цвета
		QColor color = QColorDialog::getColor(Qt::blue, this, "Выберите цвет вершин");

		if (color.isValid()) {
			float r = static_cast<float>(color.redF());
			float g = static_cast<float>(color.greenF());
			float b = static_cast<float>(color.blueF());
			glWidget_->setNewVerticesColor(r, g, b);

			// Меняем цвет самой кнопки для наглядности
			ui->btn_VertexColor->setStyleSheet(QString("background-color: %1").arg(color.name()));
		}
	}

	// Слот для выбора цвета ребер
	void MainWindow::on_btn_EdgeColor_clicked() {
		QColor color = QColorDialog::getColor(Qt::blue, this, "Выберите цвет ребер");

		if (color.isValid()) {
			float r = static_cast<float>(color.redF());
			float g = static_cast<float>(color.greenF());
			float b = static_cast<float>(color.blueF());
			glWidget_->setNewEdgesColor(r, g, b);

			ui->btn_EdgeColor->setStyleSheet(QString("background-color: %1").arg(color.name()));
		}
	}

	// Слот для выбора цвета фона
	void MainWindow::on_btn_BackgroundColor_clicked() {
		QColor color = QColorDialog::getColor(Qt::blue, this, "Выберите цвет фона");

		if (color.isValid()) {
			float r = static_cast<float>(color.redF());
			float g = static_cast<float>(color.greenF());
			float b = static_cast<float>(color.blueF());
			glWidget_->setNewBackgroundColor(r, g, b);

			ui->btn_BackgroundColor->setStyleSheet(QString("background-color: %1").arg(color.name()));
		}
	}

	void MainWindow::on_btn_LightColor_clicked() {
		QColor color = QColorDialog::getColor(Qt::blue, this, "Выберите цвет источника освещения");

		if (color.isValid()) {
			float r = color.redF();
			float g = color.greenF();
			float b = color.blueF();
			glWidget_->setNewLightColor(r, g, b);

			ui->btn_LightColor->setStyleSheet(QString("background-color: %1").arg(color.name()));
		}
	}

	// Слот для передвижения камеры

	void MainWindow::onCameraZoomChanged(int value) {
		ui->label_ZoomValue->setText(QString::number(value) + "%");
		float zoomFactor = value / 100.0;
		glWidget_->setNewViewMatrix(zoomFactor);
	}

	void MainWindow::loadScene(const QString& path) {
		NormalizationParameters params{0.0f, 1.0f, 0.1f, 0.1f};
		auto result = viewer_->LoadScene(path.toStdString(), params);

		if (result.isSuccess()) {
			currentFileName_ = QFileInfo(path).fileName();
			viewer_->DrawScene();
			int vertexCount_ = static_cast<int>(glWidget_->countVertices());
			int edgeCount_ = static_cast<int>(glWidget_->countSurfaces());
			updateInfoLabels();
			ui->statusbar->showMessage("Загружено: " + currentFileName_);
			std::cout << "[MainWindow] Файл загружен: " << currentFileName_.toStdString()
			          << " | Вершин: " << vertexCount_ << " | Поверхностей: " << edgeCount_ << "\n";
		}

		else {
			ui->statusbar->showMessage(QString::fromStdString(result.getMessage()));
			QMessageBox::critical(this, "Ошибка", "Не удалось загрузить файл: " + path);
			std::cout << "[MainWindow] Ошибка загрузки файла: " << path.toStdString() << "\n";
		}
	}

	void MainWindow::updateInfoLabels() {
		ui->label_FileInfo->setText("Файл: " + (currentFileName_.isEmpty() ? "не выбран" : currentFileName_));
		ui->label_VertexCount->setText("Вершин: " + QString::number(static_cast<int>(glWidget_->countVertices())));
		ui->label_EdgeCount->setText("Поверхностей: " + QString::number(static_cast<int>(glWidget_->countSurfaces())));
	}

	void MainWindow::on_action_Exit_triggered() {
		std::cout << "[MainWindow] Выход из приложения\n";
		close();
	}

	// ============================================
	// ============ МЕТОДЫ ПРОЕКЦИИ  ==============
	// ============================================

	void MainWindow::on_action_Orthographic_triggered() {

		glWidget_->setNewProjectionType(ProjectionType::ORTHOGRAPHIC);

		ui->statusbar->showMessage("Параллельная проекция");
	}

	void MainWindow::on_action_Perspective_triggered() {

		glWidget_->setNewProjectionType(ProjectionType::PERSPECTIVE);

		ui->statusbar->showMessage("Центральная проекция");
	}

	void MainWindow::on_action_Wireframe_triggered() {
		glWidget_->setNewDisplayType(DisplayType::WIREFRAME_MODEL);

		ui->statusbar->showMessage("Отображение только ребер и вершин");
	}

	void MainWindow::on_action_FlatShading_triggered() {
		glWidget_->setNewDisplayType(DisplayType::FLAT_SHADING_MODEL);

		ui->statusbar->showMessage("Плоское затенение");
	}

	void MainWindow::on_action_SmoothShading_triggered() {
		glWidget_->setNewDisplayType(DisplayType::SMOOTH_SHADING_MODEL);

		ui->statusbar->showMessage("Мягкое затенение методом Гура");
	}

}
