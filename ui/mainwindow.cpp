#include "ui/mainwindow.h"
#include <QApplication>
#include <QColorDialog> // Обязательно для диалога выбора цвета
#include <iostream>

namespace viewer {

	MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, viewer_(nullptr)
	, current_figure_(0)
	, count_figures_(0)
	, current_light_(0)
	, count_lights_(1)
	{
		auto glWidget_ = new GLWidget(this);
		ui->setupUi(this);
		this->setMinimumSize(300, 650);
		viewer_ = new Viewer(new FileReader(), glWidget_);
		loadSettingsFromFile("settings_viewer.json");

		ui->contentLayout->insertWidget(0, glWidget_, 1); // 1 - это stretch factor (растяжение)
		connectSignals();

		std::cout << "[MainWindow] Инициализация завершена\n";
	}

	MainWindow::~MainWindow() {
		if (viewer_) { delete viewer_; }
		delete ui;
	}

	void MainWindow::connectSignals() noexcept {

		// СОЕДИНЕНИЕ СИГНАЛОВ МЕНЮ (Actions)
		connect(ui->action_Open, &QAction::triggered, this, &MainWindow::on_action_Open_triggered);
		connect(ui->action_Exit, &QAction::triggered, this, &MainWindow::on_action_Exit_triggered);
		connect(ui->action_Orthographic, &QAction::triggered, this, &MainWindow::on_action_Orthographic_triggered);
		connect(ui->action_Perspective, &QAction::triggered, this, &MainWindow::on_action_Perspective_triggered);
		connect(ui->action_Wireframe, &QAction::triggered, this, &MainWindow::on_action_Wireframe_triggered);
		connect(ui->action_FlatShading, &QAction::triggered, this, &MainWindow::on_action_FlatShading_triggered);
		connect(ui->action_SmoothShading, &QAction::triggered, this, &MainWindow::on_action_SmoothShading_triggered);
		connect(ui->action_SaveScreenshot, &QAction::triggered, this, &MainWindow::on_action_SaveScreenshot_triggered);
		connect(ui->action_SaveGif, &QAction::triggered, this, &MainWindow::on_action_SaveGif_triggered);

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
			viewer_->getScene()->getFigure(current_figure_).translation() = Point3D(ui->spin_transX->value(), ui->spin_transY->value(), ui->spin_transZ->value());
			viewer_->getScene()->getFigure(current_figure_).rotation() = Point3D(ui->spin_rotX->value(), ui->spin_rotY->value(), ui->spin_rotZ->value());
			viewer_->getScene()->getFigure(current_figure_).scale() = Point3D(ui->spin_scaleX->value(), ui->spin_scaleY->value(), ui->spin_scaleZ->value());
			viewer_->getScene()->getLight(current_light_).position() = Point3D(ui->light_transX->value(), ui->light_transY->value(), ui->light_transZ->value());
		};

		// Соединяем все спинбоксы с обновлением
		for(auto s : {ui->spin_transX, ui->spin_transY, ui->spin_transZ,
		              ui->spin_rotX, ui->spin_rotY, ui->spin_rotZ,
		              ui->spin_scaleX, ui->spin_scaleY, ui->spin_scaleZ,
		              ui->light_transX, ui->light_transY, ui->light_transZ}) {
		    connect(s, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updateTransform);
		}

		// Устанавливаем значения на кнопках:
		auto backColorPoint = viewer_->getScene()->backgroundColor();
		auto vertColorPoint = viewer_->getScene()->getFigure(current_figure_).vertexInfo().color();
		auto edgColorPoint = viewer_->getScene()->getFigure(current_figure_).edgeInfo().color();
		auto lightColorPoint = viewer_->getScene()->getLight(current_light_).color();
		QColor backColor = QColor(backColorPoint.x() * 255, backColorPoint.y() * 255, backColorPoint.z() * 255);
		QColor vertColor = QColor(vertColorPoint.x() * 255, vertColorPoint.y() * 255, vertColorPoint.z() * 255);
		QColor edgColor = QColor(edgColorPoint.x() * 255, edgColorPoint.y() * 255, edgColorPoint.z() * 255);
		QColor lightColor = QColor(lightColorPoint.x() * 255, lightColorPoint.y() * 255, lightColorPoint.z() * 255);

		ui->btn_VertexColor->setStyleSheet(QString("background-color: %1").arg(vertColor.name()));
		ui->btn_EdgeColor->setStyleSheet(QString("background-color: %1").arg(edgColor.name()));
		ui->btn_BackgroundColor->setStyleSheet(QString("background-color: %1").arg(backColor.name()));
		ui->btn_LightColor->setStyleSheet(QString("background-color: %1").arg(lightColor.name()));

		ui->spin_VertexSize->setValue(viewer_->getScene()->getFigure(current_figure_).vertexInfo().size() * 100);
		ui->spin_EdgeWidth->setValue(viewer_->getScene()->getFigure(current_figure_).edgeInfo().size() * 1000);
		if(viewer_->getScene()->getFigure(current_figure_).edgeInfo().mode() == 0) {
			ui->combo_EdgeType->setCurrentText("Сплошной");
		}
		else {
			ui->combo_EdgeType->setCurrentText("Пунктир");
		}

		if(viewer_->getScene()->getFigure(current_figure_).vertexInfo().mode() == 0) {
			ui->combo_VertexType->setCurrentText("Нет");
		}
		else if(viewer_->getScene()->getFigure(current_figure_).vertexInfo().mode() == 1) {
			ui->combo_VertexType->setCurrentText("Круг");
		}
		else {
			ui->combo_VertexType->setCurrentText("Квадрат");
		}

	}

	void MainWindow::mousePressEvent(QMouseEvent* event) {
		lastPos_ = event->pos();
	}

	void MainWindow::mouseMoveEvent(QMouseEvent* event) {
		float dx = event->position().x() - lastPos_.x();
		float dy = event->position().y() - lastPos_.y();

		if (event->buttons() & Qt::LeftButton) {
			// Левая кнопка мыши - перемещение фигуры в фокусе

			viewer_->getScene()->getFigure(current_figure_).translation().x() += dx * TRANSLATION_MOUSE_SENSITIVITY;
			viewer_->getScene()->getFigure(current_figure_).translation().y() -= dy * TRANSLATION_MOUSE_SENSITIVITY;
			viewer_->getScene()->getFigure(current_figure_).translation().z() = ui->spin_transZ->value();

			ui->spin_transX->setValue(viewer_->getScene()->getFigure(current_figure_).translation().x());
			ui->spin_transY->setValue(viewer_->getScene()->getFigure(current_figure_).translation().y());
		}

		else if (event->buttons() & Qt::RightButton) {
			// Правая кнопка мыши - вращение

			viewer_->getScene()->getFigure(current_figure_).rotation().x() = static_cast<int>(dy * ROTATION_MOUSE_SENSITIVITY) % 360;
			viewer_->getScene()->getFigure(current_figure_).rotation().y() = static_cast<int>(dx * ROTATION_MOUSE_SENSITIVITY) % 360;

			ui->spin_rotX->setValue(static_cast<int>(dy * ROTATION_MOUSE_SENSITIVITY) % 360);
			ui->spin_rotY->setValue(static_cast<int>(dx * ROTATION_MOUSE_SENSITIVITY) % 360);
		}

		lastPos_ = event->pos();
		update();
	}

	void MainWindow::wheelEvent(QWheelEvent* event) {
		// Колесико мыши - зум

		float det = event->angleDelta().y() * ZOOM_MOUSE_SENSITIVITY;
		viewer_->getScene()->getCamera().translation().z() -= det * 0.5f;

		update();
	}

	// ===========================================================
	// ============= СОХРАНЕНИЕ И ЗАГРУЗКА НАСТРОЕК ==============
	// ===========================================================

	void MainWindow::saveSettingsToFile(const QString& filePath) {
		QJsonObject settings;

		settings["backgroundColor"] = colorToJson(backColor_);
		settings["edgeColor"] = colorToJson(edgColor_);
		settings["vertexColor"] = colorToJson(vertColor_);
		settings["edgeSize"] = edgSize_;
		settings["vertexSize"] = vertSize_;
		settings["vertexDisplayType"] = static_cast<int>(vertMode_);
		settings["edgeIsDashed"] = static_cast<int>(edgMode_);
		settings["projectionType"] = static_cast<int>(projectionType_);
		settings["displayType"] = static_cast<int>(displayType_);
		settings["lightColor"] = colorToJson(lightColor_);

		QJsonDocument doc(settings);
		QFile file(filePath);

		if (file.open(QIODevice::WriteOnly)) {
			file.write(doc.toJson());
			file.close();
			std::cout << "[GLWidget] Настройки сохранены в файл: "
		            << filePath.toStdString() << std::endl;
		}
		else {
			std::cerr << "[GLWidget] Ошибка сохранения настроек в файл: "
			          << filePath.toStdString() << std::endl;
		}
	}

	QJsonObject MainWindow::colorToJson(const Point3D& color) noexcept {
		QJsonObject obj;

		obj["r"] = color.x;
		obj["g"] = color.y;
		obj["b"] = color.z;

		return obj;
	}

	Point3D MainWindow::colorFromJson(const QJsonObject& obj) noexcept {

		Point3D color;

		color.x = static_cast<float>(obj["r"].toDouble());
		color.y = static_cast<float>(obj["g"].toDouble());
		color.z = static_cast<float>(obj["b"].toDouble());

		return color;
	}

	void MainWindow::loadSettingsFromFile(const QString& filePath) {
		QFile file(filePath);

		if (!file.exists()) {
			std::cout << "[GLWidget] Файл настроек не найден, используем значения по умолчанию" << std::endl;
			return;
		}

		if (file.open(QIODevice::ReadOnly)) {
			QByteArray data = file.readAll();
			file.close();

			QJsonDocument doc = QJsonDocument::fromJson(data);
			QJsonObject settings = doc.object();

			// Загрузка настроек
			if (settings.contains("backgroundColor")) {
			  backColor_ = colorFromJson(settings["backgroundColor"].toObject());
			}
			if (settings.contains("edgeColor")) {
			  edgColor_ = colorFromJson(settings["edgeColor"].toObject());
			}
			if (settings.contains("vertexColor")) {
			  vertColor_ = colorFromJson(settings["vertexColor"].toObject());
			}
			if (settings.contains("edgeSize")) {
			  edgSize_ = static_cast<float>(settings["edgeSize"].toDouble());
			}
			if (settings.contains("vertexSize")) {
			  vertSize_ = static_cast<float>(settings["vertexSize"].toDouble());
			}
			if (settings.contains("vertexDisplayType")) {
			  vertMode_ = static_cast<VerticesMode>(settings["vertexDisplayType"].toInt());
			}
			if (settings.contains("edgeIsDashed")) {
			  edgMode_ = static_cast<EdgesMode>(settings["edgeIsDashed"].toInt());
			}
			if (settings.contains("projectionType")) {
			  projectionType_ = static_cast<ProjectionType>(settings["projectionType"].toInt());
			}
			if (settings.contains("displayType")) {
				displayType_ = static_cast<DisplayType>(settings["displayType"].toInt());
			}
			if (settings.contains("lightColor")) {
				lightColor_ = colorFromJson(settings["lightColor"].toObject());
			}

			std::cout << "[GLWidget] Настройки загружены из файла: "
			          << filePath.toStdString() << std::endl;
		}
		else {
			std::cerr << "[GLWidget] Ошибка загрузки настроек из файла: "
			          << filePath.toStdString() << std::endl;
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

		viewer_->getScene()->getFigure(current_figure_).vertexInfo().mode() = mode;
	}

	void MainWindow::onEdgeTypeChanged(int index) {
		// index: 0 - Сплошная, 1 - Пунктир
		EdgesMode mode = (index == 1) ? EdgesMode::DASHED : EdgesMode::SOLID;
		viewer_->getScene()->getFigure(current_figure_).edgeInfo().mode() = mode;
	}

	// Слот для изменения размера вершин
	void MainWindow::onVertexSizeChanged(float value) {
		viewer_->getScene()->getFigure(current_figure_).vertexInfo().size() = value / 100;
	}

	// Слот для изменения толщины ребер
	void MainWindow::onEdgeWidthChanged(float value) {
		viewer_->getScene()->getFigure(current_figure_).edgeInfo().size() = value / 1000.0;
	}

	// Слот для выбора цвета вершин
	void MainWindow::on_btn_VertexColor_clicked() {
		// Открываем стандартное окно выбора цвета
		QColor color = QColorDialog::getColor(Qt::blue, this, "Выберите цвет вершин");

		if (color.isValid()) {
			float r = static_cast<float>(color.redF());
			float g = static_cast<float>(color.greenF());
			float b = static_cast<float>(color.blueF());
			viewer_->getScene()->getFigure(current_figure_).vertexInfo().color() = Point3D(r, g, b);

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
			viewer_->getScene()->getFigure(current_figure_).edgeInfo().color() = Point3D(r, g, b);

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
			viewer_->getScene()->backgroundColor() = Point3D(r, g, b);

			ui->btn_BackgroundColor->setStyleSheet(QString("background-color: %1").arg(color.name()));
		}
	}

	void MainWindow::on_btn_LightColor_clicked() {
		QColor color = QColorDialog::getColor(Qt::blue, this, "Выберите цвет источника освещения");

		if (color.isValid()) {
			float r = color.redF();
			float g = color.greenF();
			float b = color.blueF();
			viewer_->getScene()->getLight(current_light_).color() = Point3D(r, g, b);

			ui->btn_LightColor->setStyleSheet(QString("background-color: %1").arg(color.name()));
		}
	}

	void MainWindow::loadScene(const QString& path) {
		auto result = viewer_->LoadFigure(path.toStdString());

		if (result.isSuccess()) {
			currentFileName_ = QFileInfo(path).fileName();
			count_figures_++;
			current_figure_ = count_figures_;
			viewer_->DrawScene();
			auto vertCount = viewer_->getScene()->getFigure(current_figure_).getVertices().size();
			int surfCount = viewer_->getScene()->getFigure(current_figure_).getSurfaces().size();
			updateInfoLabels();
			ui->statusbar->showMessage("Загружено: " + currentFileName_);
			std::cout << "[MainWindow] Файл загружен: " << currentFileName_.toStdString()
			          << " | Вершин: " << vertCount << " | Поверхностей: " << surfCount << "\n";
		}

		else {
			ui->statusbar->showMessage(QString::fromStdString(result.getMessage()));
			QMessageBox::critical(this, "Ошибка", "Не удалось загрузить файл: " + path);
			std::cout << "[MainWindow] Ошибка загрузки файла: " << path.toStdString() << "\n";
		}
	}

	void MainWindow::updateInfoLabels() {
		ui->label_FileInfo->setText("Файл: " + (currentFileName_.isEmpty() ? "не выбран" : currentFileName_));
		ui->label_VertexCount->setText("Вершин: " + QString::number(static_cast<int>(viewer_->getScene()->getFigure(current_figure_).getVertices().size())));
		ui->label_EdgeCount->setText("Поверхностей: " + QString::number(static_cast<int>(viewer_->getScene()->getFigure(current_figure_).getSurfaces().size())));

		if (viewer_->getScene()->getFigure(current_figure_).displayType() == DisplayType::WIREFRAME_MODEL)
			ui->label_displayType->setText("Отображение только ребер и вершин");
		else if (viewer_->getScene()->getFigure(current_figure_).displayType() == DisplayType::FLAT_SHADING_MODEL)
			ui->label_displayType->setText("Плоское затенение");
		else if (viewer_->getScene()->getFigure(current_figure_).displayType() == DisplayType::SMOOTH_SHADING_MODEL)
			ui->label_displayType->setText("Мягкое затенение");

		if (viewer_->getScene()->getCamera().projectionType() == ProjectionType::ORTHOGRAPHIC)
			ui->label_projectionType->setText("Параллельная проекция");
		else if (viewer_->getScene()->getCamera().projectionType() == ProjectionType::PERSPECTIVE)
			ui->label_projectionType->setText("Центральная проекция");
	}

	void MainWindow::on_action_Exit_triggered() {
		std::cout << "[MainWindow] Выход из приложения\n";
		close();
	}

	// ============================================
	// ============ МЕТОДЫ ПРОЕКЦИИ  ==============
	// ============================================

	void MainWindow::on_action_Orthographic_triggered() {

		viewer_->getScene()->getCamera().projectionType() = ORTHOGRAPHIC;

		ui->label_projectionType->setText("Параллельная проекция");
	}

	void MainWindow::on_action_Perspective_triggered() {

		viewer_->getScene()->getCamera().projectionType() = PERSPECTIVE;

		ui->label_projectionType->setText("Центральная проекция");
	}

	void MainWindow::on_action_Wireframe_triggered() {
		viewer_->getScene()->getFigure(current_figure_).displayType() = WIREFRAME_MODEL;

		ui->label_displayType->setText("Отображение только ребер и вершин");
	}

	void MainWindow::on_action_FlatShading_triggered() {
		viewer_->getScene()->getFigure(current_figure_).displayType() = FLAT_SHADING_MODEL;

		ui->label_displayType->setText("Плоское затенение");
	}

	void MainWindow::on_action_SmoothShading_triggered() {
		viewer_->getScene()->getFigure(current_figure_).displayType() = SMOOTH_SHADING_MODEL;

		ui->label_displayType->setText("Мягкое затенение");
	}

	void MainWindow::on_action_SaveScreenshot_triggered() {
		QString fileName = QFileDialog::getSaveFileName(this, "Сохранить скриншот",
														"", "Images (*.png *.jpg)");
		if (!fileName.isEmpty()) {
			// glWidget_->saveImage(fileName);
			ui->statusbar->showMessage("Скриншот сохранен в файле " + fileName);
		}
	}

	void MainWindow::on_action_SaveGif_triggered() {
		QString fileName = QFileDialog::getSaveFileName(this, "Сохранить анимацию",
														"", "Animation (*.gif)");
		if (!fileName.isEmpty()) {
			// glWidget_->startRecording(fileName, 10, 5);
			ui->statusbar->showMessage("Анимация сохранена в файле " + fileName);
		}
	}


}
