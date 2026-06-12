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
	, current_light_(1)
	, count_lights_(1)
	{
		auto glWidget_ = new GLWidget(this);
		ui->setupUi(this);
		this->setMinimumSize(300, 650);
		viewer_ = new Viewer(new FileReader(), glWidget_);
		loadSettingsFromFile("settings_viewer.json");

		ui->contentLayout->insertWidget(0, glWidget_, 1); // 1 - это stretch factor (растяжение)
		connectSignals();

		setFigureValues();
		setLightValues();
		setGeneralValues();

		std::cout << "[MainWindow] Инициализация завершена\n";
	}

	MainWindow::~MainWindow() {
		saveSettingsToFile("settings_viewer.json");

		if (viewer_) delete viewer_;
		if (ui) delete ui;
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
		connect(ui->btn_AddLight, &QPushButton::clicked, this, &MainWindow::on_btn_AddLight_clicked);

		// СОЕДИНЕНИЕ ВЫПАДАЮЩИХ СПИСКОВ
		connect(ui->combo_VertexType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onVertexTypeChanged);
		connect(ui->combo_EdgeType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onEdgeTypeChanged);
		connect(ui->combo_LightSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onLightChanged);
		connect(ui->combo_FigureSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFigureChanged);

		auto updateTransform = [this]() {
			if (count_figures_ > 0) {
				viewer_->getScene()->getFigure(current_figure_).vertexInfo().size() = ui->spin_VertexSize->value() / 100;
				viewer_->getScene()->getFigure(current_figure_).edgeInfo().size() = ui->spin_EdgeWidth->value() / 1000.0;
				viewer_->getScene()->getFigure(current_figure_).translation() = Point3D(ui->spin_transX->value(), ui->spin_transY->value(), ui->spin_transZ->value());
				viewer_->getScene()->getFigure(current_figure_).rotation() = Point3D(ui->spin_rotX->value(), ui->spin_rotY->value(), ui->spin_rotZ->value());
				viewer_->getScene()->getFigure(current_figure_).scale() = Point3D(ui->spin_scaleX->value(), ui->spin_scaleY->value(), ui->spin_scaleZ->value());
			}
			viewer_->getScene()->getLight(current_light_).position() = Point3D(ui->light_transX->value(), ui->light_transY->value(), ui->light_transZ->value());
			viewer_->getScene()->getLight(current_light_).intensity() = ui->light_intensity->value();
		};

		// СОЕДИНЕНИЕ ВСЕХ СПИНБОКСОВ
		for(auto s : {
			ui->spin_transX, ui->spin_transY, ui->spin_transZ,
			ui->spin_rotX, ui->spin_rotY, ui->spin_rotZ,
			ui->spin_scaleX, ui->spin_scaleY, ui->spin_scaleZ,
			ui->light_transX, ui->light_transY, ui->light_transZ,
			ui->spin_VertexSize, ui->spin_EdgeWidth, ui->light_intensity}) {
		    connect(s, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updateTransform);
		}
	}

	void MainWindow::setFigureValues() noexcept {
		if (current_figure_ > 0) {
			// Устанавливаем значения на кнопках:
			auto vertColorPoint = viewer_->getScene()->getFigure(current_figure_).vertexInfo().color();
			auto edgColorPoint = viewer_->getScene()->getFigure(current_figure_).edgeInfo().color();
			auto lightColorPoint = viewer_->getScene()->getLight(current_light_).color();
			QColor vertColor = QColor(vertColorPoint.x * 255, vertColorPoint.y * 255, vertColorPoint.z * 255);
			QColor edgColor = QColor(edgColorPoint.x * 255, edgColorPoint.y * 255, edgColorPoint.z * 255);
			QColor lightColor = QColor(lightColorPoint.x * 255, lightColorPoint.y * 255, lightColorPoint.z * 255);

			ui->btn_VertexColor->setStyleSheet(QString("background-color: %1").arg(vertColor.name()));
			ui->btn_EdgeColor->setStyleSheet(QString("background-color: %1").arg(edgColor.name()));
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

			ui->spin_transX->setValue(viewer_->getScene()->getFigure(current_figure_).translation().x);
			ui->spin_transY->setValue(viewer_->getScene()->getFigure(current_figure_).translation().y);
			ui->spin_transZ->setValue(viewer_->getScene()->getFigure(current_figure_).translation().z);

			ui->spin_rotX->setValue(viewer_->getScene()->getFigure(current_figure_).rotation().x);
			ui->spin_rotY->setValue(viewer_->getScene()->getFigure(current_figure_).rotation().y);
			ui->spin_rotZ->setValue(viewer_->getScene()->getFigure(current_figure_).rotation().z);

			ui->spin_scaleX->setValue(viewer_->getScene()->getFigure(current_figure_).scale().x);
			ui->spin_scaleY->setValue(viewer_->getScene()->getFigure(current_figure_).scale().y);
			ui->spin_scaleZ->setValue(viewer_->getScene()->getFigure(current_figure_).scale().z);

			updateInfoLabels();
		}
	}

	void MainWindow::setLightValues() noexcept {
		ui->light_transX->setValue(viewer_->getScene()->getLight(current_light_).position().x);
		ui->light_transY->setValue(viewer_->getScene()->getLight(current_light_).position().y);
		ui->light_transZ->setValue(viewer_->getScene()->getLight(current_light_).position().z);

		ui->light_intensity->setValue(viewer_->getScene()->getLight(current_light_).intensity());

		auto color = viewer_->getScene()->getLight(current_light_).color();
		QColor qcolor = QColor(color.x * 255, color.y * 255, color.z * 255);

		ui->btn_LightColor->setStyleSheet(QString("background-color: %1").arg(qcolor.name()));
	}

	void MainWindow::setGeneralValues() noexcept {
		auto backColorPoint = viewer_->getScene()->backgroundColor();
		QColor backColor = QColor(backColorPoint.x * 255, backColorPoint.y * 255, backColorPoint.z * 255);
		ui->btn_BackgroundColor->setStyleSheet(QString("background-color: %1").arg(backColor.name()));
	}


	void MainWindow::mousePressEvent(QMouseEvent* event) {
		lastPos_ = event->pos();
	}

	void MainWindow::mouseMoveEvent(QMouseEvent* event) {
		float dx = event->position().x() - lastPos_.x();
		float dy = event->position().y() - lastPos_.y();

		if (event->buttons() & Qt::LeftButton && current_figure_ > 0) {
			// Левая кнопка мыши - перемещение фигуры в фокусе

			viewer_->getScene()->getFigure(current_figure_).translation().x += dx * TRANSLATION_MOUSE_SENSITIVITY;
			viewer_->getScene()->getFigure(current_figure_).translation().y -= dy * TRANSLATION_MOUSE_SENSITIVITY;
			viewer_->getScene()->getFigure(current_figure_).translation().z = ui->spin_transZ->value();

			ui->spin_transX->setValue(viewer_->getScene()->getFigure(current_figure_).translation().x);
			ui->spin_transY->setValue(viewer_->getScene()->getFigure(current_figure_).translation().y);
		}

		else if (event->buttons() & Qt::RightButton && current_figure_) {
			// Правая кнопка мыши - вращение

			viewer_->getScene()->getFigure(current_figure_).rotation().x += static_cast<int>(dy * ROTATION_MOUSE_SENSITIVITY) % 360;
			viewer_->getScene()->getFigure(current_figure_).rotation().y += static_cast<int>(dx * ROTATION_MOUSE_SENSITIVITY) % 360;

			ui->spin_rotX->setValue(static_cast<int>(viewer_->getScene()->getFigure(current_figure_).rotation().x) % 360);
			ui->spin_rotY->setValue(static_cast<int>(viewer_->getScene()->getFigure(current_figure_).rotation().y) % 360);
		}

		lastPos_ = event->pos();
		viewer_->DrawScene();
	}

	void MainWindow::wheelEvent(QWheelEvent* event) {
		// Колесико мыши - зум

		if (current_figure_ > 0) {
			float det = event->angleDelta().y() * ZOOM_MOUSE_SENSITIVITY;
			viewer_->getScene()->getCamera().translation().z -= det;

			viewer_->DrawScene();
		}
	}

	// ===========================================================
	// ============= СОХРАНЕНИЕ И ЗАГРУЗКА НАСТРОЕК ==============
	// ===========================================================

	void MainWindow::saveSettingsToFile(const QString& filePath) {
		QJsonObject settings;

		// Сначала глобальные настройки сцены
		settings["backgroundColor"] = colorToJson(viewer_->getScene()->backgroundColor());
		settings["projectionType"] = viewer_->getScene()->getCamera().projectionType();

		// Сохраняем настройки глобального освещения
		QJsonObject globalLight_settings;
		globalLight_settings["color"] = colorToJson(viewer_->getScene()->getLight(1).color());
		globalLight_settings["intensity"] = viewer_->getScene()->getLight(1).intensity();
		settings["globalLight"] = globalLight_settings;

		// Сохраняем настройки каждой фигуры
		for (auto i = 2; i <= count_figures_; ++i) {
			QJsonObject figure_settings;

			figure_settings["path"] = QString::fromStdString(viewer_->getScene()->getFigure(i).path());

			figure_settings["translation"] = positionToJson(viewer_->getScene()->getFigure(i).translation());
			figure_settings["rotation"] = positionToJson(viewer_->getScene()->getFigure(i).rotation());
			figure_settings["scale"] = positionToJson(viewer_->getScene()->getFigure(i).scale());

			figure_settings["vertexColor"] = colorToJson(viewer_->getScene()->getFigure(i).vertexInfo().color());
			figure_settings["vertexSize"] = viewer_->getScene()->getFigure(i).vertexInfo().size();
			figure_settings["vertexDisplayType"] = viewer_->getScene()->getFigure(i).vertexInfo().mode();
			figure_settings["edgeColor"] = colorToJson(viewer_->getScene()->getFigure(i).edgeInfo().color());
			figure_settings["edgeSize"] = viewer_->getScene()->getFigure(i).edgeInfo().size();
			figure_settings["edgeIsDashed"] = viewer_->getScene()->getFigure(i).edgeInfo().mode();
			figure_settings["edgeDisplayType"] = viewer_->getScene()->getFigure(i).displayType();

			settings[QString("figure %1").arg(i)] = figure_settings;
		}

		// Сохраняем настройки каждого источника освещения
		for (auto i = 1; i <= count_lights_; ++i) {
			QJsonObject light_settings;

			light_settings["position"] = positionToJson(viewer_->getScene()->getLight(i).position());
			light_settings["intensity"] = viewer_->getScene()->getLight(i).intensity();
			light_settings["color"] = colorToJson(viewer_->getScene()->getLight(i).color());

			settings[QString("light %1").arg(i)] = light_settings;
		}

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

			// Загрузка глобальных настроек
			if (settings.contains("backgroundColor")) {
			  viewer_->getScene()->backgroundColor() = colorFromJson(settings["backgroundColor"].toObject());
			}
			if (settings.contains("projectionType")) {
			  viewer_->getScene()->getCamera().projectionType() = static_cast<ProjectionType>(settings["projectionType"].toInt());
			}

			// Загрузка настроек глобального освещения
			if (settings.contains("globalLight")) {
				auto globalLight_settings = settings["globalLight"].toObject();

				if (globalLight_settings.contains("color")) {
					viewer_->getScene()->getLight(1).color() = colorFromJson(globalLight_settings["color"].toObject());
				}

				if (globalLight_settings.contains("intensity")) {
					viewer_->getScene()->getLight(1).intensity() = static_cast<float>(settings["intensity"].toDouble());
				}
			}

			// Загрузка настроек фигур
			int i = 1;
			while (settings.contains("figure " + i)) {
				auto figure_settings = settings["figure " + i].toObject();

				if (viewer_->LoadFigure(figure_settings["path"].toString().toStdString()).isSuccess()) {
					if (figure_settings.contains("translation")) {
						viewer_->getScene()->getFigure(i).translation() = positionFromJson(figure_settings["translation"].toObject());
					}
					if (figure_settings.contains("rotation")) {
						viewer_->getScene()->getFigure(i).rotation() = positionFromJson(figure_settings["rotation"].toObject());
					}
					if (figure_settings.contains("scale")) {
						viewer_->getScene()->getFigure(i).scale() = positionFromJson(figure_settings["scale"].toObject());
					}
					if (figure_settings.contains("vertexColor")) {
						viewer_->getScene()->getFigure(i).vertexInfo().color() = colorFromJson(figure_settings["vertexColor"].toObject());
					}
					if (figure_settings.contains("vertexSize")) {
						viewer_->getScene()->getFigure(i).vertexInfo().size() = figure_settings["vertexSize"].toDouble();
					}
					if (figure_settings.contains("vertexDisplayType")) {
						viewer_->getScene()->getFigure(i).vertexInfo().mode() = static_cast<VerticesMode>(figure_settings["vertexDisplayType"].toInt());
					}
					if (figure_settings.contains("edgeColor")) {
						viewer_->getScene()->getFigure(i).edgeInfo().color() = colorFromJson(figure_settings["edgeColor"].toObject());
					}
					if (figure_settings.contains("edgeSize")) {
						viewer_->getScene()->getFigure(i).edgeInfo().size() = figure_settings["edgeSize"].toDouble();
					}
					if (figure_settings.contains("edgeDisplayType")) {
						viewer_->getScene()->getFigure(i).edgeInfo().mode() = static_cast<EdgesMode>(figure_settings["edgeDisplayType"].toInt());
					}

					i++;
				}
			}

			i = 2;
			while (settings.contains("light " + i)) {
				auto light_settings = settings["light " + i].toObject();

				viewer_->getScene()->addLight();

				if (light_settings.contains("position")) {
					viewer_->getScene()->getLight(i).position() = positionFromJson(light_settings["position"].toObject());
				}
				if (light_settings.contains("color")) {
					viewer_->getScene()->getLight(i).color() = colorFromJson(light_settings["color"].toObject());
				}
				if (light_settings.contains("intensity")) {
					viewer_->getScene()->getLight(i).intensity() = static_cast<float>(settings["intensity"].toDouble());
				}

				i++;
			}

			std::cout << "[GLWidget] Настройки загружены из файла: "
			          << filePath.toStdString() << std::endl;
		}
		else {
			std::cerr << "[GLWidget] Ошибка загрузки настроек из файла: "
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

	QJsonObject MainWindow::positionToJson(const Point3D& pos) noexcept {
		QJsonObject obj;

		obj["x"] = pos.x;
		obj["y"] = pos.y;
		obj["z"] = pos.z;

		return obj;
	}

	Point3D MainWindow::positionFromJson(const QJsonObject& obj) noexcept {
		Point3D pos;

		pos.x = static_cast<float>(obj["x"].toDouble());
		pos.y = static_cast<float>(obj["y"].toDouble());
		pos.z = static_cast<float>(obj["z"].toDouble());

		return pos;
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

	void MainWindow::onLightChanged(int value) {
		current_light_ = value + 1;

		setLightValues();
	}

	void MainWindow::onFigureChanged(int value) {
		current_figure_ = value + 1;
		std::cout << "Смена на фигуру " << current_figure_ << " из " << count_figures_ << std::endl;

		setFigureValues();
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
			viewer_->DrawScene();

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

	void MainWindow::on_btn_AddLight_clicked() {
		count_lights_++;
		viewer_->getScene()->addLight();
		ui->combo_LightSelect->addItem(QString("Источник %1").arg(viewer_->getScene()->countLights()));

		ui->combo_LightSelect->setCurrentIndex(count_lights_ - 1);
	}

	void MainWindow::loadScene(const QString& path) {
		auto result = viewer_->LoadFigure(path.toStdString());

		if (result.isSuccess()) {
			currentFileName_ = QFileInfo(path).fileName();
			count_figures_ ++;
			current_figure_ = count_figures_;
			viewer_->DrawScene();

			updateInfoLabels();
			ui->statusbar->showMessage("Загружено: " + currentFileName_);
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

	//Обработчик действия "Открыть файл"
	void MainWindow::on_action_Open_triggered() {
		QString fileName = QFileDialog::getOpenFileName(
			this, "Открыть 3D модель", QString(),
			"OBJ Files (*.obj *.vobj);;All Files (*)");

		if (!fileName.isEmpty()) {
			loadScene(fileName);
			ui->groupBox_Figure->setVisible(true);
			ui->combo_FigureSelect->addItem(QString("Фигура %1").arg(viewer_->getScene()->countFigures()));
			ui->combo_FigureSelect->setCurrentIndex(count_figures_ - 1);
		}
	}

	void MainWindow::on_action_Exit_triggered() {
		std::cout << "[MainWindow] Выход из приложения\n";
		close();
	}

	void MainWindow::on_action_Orthographic_triggered() {
		viewer_->getScene()->getCamera().projectionType() = ORTHOGRAPHIC;

		ui->label_projectionType->setText("Параллельная проекция");
	}

	void MainWindow::on_action_Perspective_triggered() {
		viewer_->getScene()->getCamera().projectionType() = PERSPECTIVE;

		ui->label_projectionType->setText("Центральная проекция");
	}

	void MainWindow::on_action_Wireframe_triggered() {
		if (current_figure_ > 0) {
			viewer_->getScene()->getFigure(current_figure_).displayType() = WIREFRAME_MODEL;
			ui->groupBox_Edges->setVisible(true);
			ui->groupBox_Vertices->setVisible(true);

			ui->groupBox_Lights->setVisible(false);

			ui->label_displayType->setText("Отображение только ребер и вершин");
		}

		else {
			ui->statusbar->showMessage("Для начала загрузите фигуру");
		}
	}

	void MainWindow::on_action_FlatShading_triggered() {
		if (current_figure_ > 0) {
			viewer_->getScene()->getFigure(current_figure_).displayType() = FLAT_SHADING_MODEL;
			ui->groupBox_Edges->setVisible(false);
			ui->groupBox_Vertices->setVisible(false);

			ui->groupBox_Lights->setVisible(true);

			ui->label_displayType->setText("Плоское затенение");
		}

		else {
			ui->statusbar->showMessage("Для начала загрузите фигуру");
		}
	}

	void MainWindow::on_action_SmoothShading_triggered() {
		if (current_figure_ > 0) {
			viewer_->getScene()->getFigure(current_figure_).displayType() = SMOOTH_SHADING_MODEL;
			ui->groupBox_Edges->setVisible(false);
			ui->groupBox_Vertices->setVisible(false);

			ui->groupBox_Lights->setVisible(true);

			ui->label_displayType->setText("Мягкое затенение");
		}

		else {
			ui->statusbar->showMessage("Для начала загрузите фигуру");
		}
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
