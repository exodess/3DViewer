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

		viewer_->DrawScene();

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
		connect(ui->action_RayTracing, &QAction::triggered, this, &MainWindow::on_action_RayTracing_triggered);
		connect(ui->action_SaveScreenshot, &QAction::triggered, this, &MainWindow::on_action_SaveScreenshot_triggered);
		connect(ui->action_SaveGif, &QAction::triggered, this, &MainWindow::on_action_SaveGif_triggered);

		// СОЕДИНЕНИЕ КНОПОК ЦВЕТА
		connect(ui->btn_BackgroundColor, &QPushButton::clicked, this, &MainWindow::on_btn_BackgroundColor_clicked);
		connect(ui->btn_VertexColor, &QPushButton::clicked, this, &MainWindow::on_btn_VertexColor_clicked);
		connect(ui->btn_EdgeColor, &QPushButton::clicked, this, &MainWindow::on_btn_EdgeColor_clicked);
		connect(ui->btn_LightColor, &QPushButton::clicked, this, &MainWindow::on_btn_LightColor_clicked);
		connect(ui->btn_AddLight, &QPushButton::clicked, this, &MainWindow::on_btn_AddLight_clicked);
		connect(ui->btn_MaterialColor, &QPushButton::clicked, this, &MainWindow::on_btn_MaterialColor_clicked);

		// СОЕДИНЕНИЕ ВЫПАДАЮЩИХ СПИСКОВ
		connect(ui->combo_VertexType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onVertexTypeChanged);
		connect(ui->combo_EdgeType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onEdgeTypeChanged);
		connect(ui->combo_LightSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onLightChanged);
		connect(ui->combo_FigureSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onFigureChanged);

		connect(ui->check_FloorDisplay, QOverload<int>::of(&QCheckBox::stateChanged), this, &MainWindow::onFloorDisplayChanged);

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

			viewer_->DrawScene();
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

		auto updateMaterial = [this]() {
			if (current_figure_ > 0) {
				viewer_->getScene()->getFigure(current_figure_).material().roughness() = ui->roughnessSlider->value() / 100.0;
				viewer_->getScene()->getFigure(current_figure_).material().metallic() = ui->metallicSlider->value() / 100.0;
				viewer_->getScene()->getFigure(current_figure_).material().refractive() = ui->refractiveSlider->value() / 100.0;
				viewer_->getScene()->getFigure(current_figure_).material().reflectivity() = ui->reflectivitySlider->value() / 100.0;
				viewer_->getScene()->getFigure(current_figure_).material().alpha() = ui->alphaSlider->value() / 100.0;
			}

			viewer_->DrawScene();
		};

		for (auto s : {
			ui->roughnessSlider, ui->metallicSlider, ui->refractiveSlider,
			ui->reflectivitySlider, ui->alphaSlider }) {

			connect(s, QOverload<int>::of(&QSlider::valueChanged), this, updateMaterial);
		}

		auto updateGeneral = [this]() {
			viewer_->getScene()->getCamera().translation() = Point3D(ui->spin_camTransX->value(), ui->spin_camTransY->value(), ui->spin_camTransZ->value());
			viewer_->getScene()->getCamera().rotation() = Point3D(ui->spin_camRotX->value(), ui->spin_camRotY->value(), ui->spin_camRotZ->value());
			viewer_->getScene()->getCamera().scale() = Point3D(ui->spin_camScaleX->value(), ui->spin_camScaleY->value(), ui->spin_camScaleZ->value());

			viewer_->DrawScene();
		};

		for (auto s : {
			ui->spin_camTransX, ui->spin_camTransY, ui->spin_camTransZ,
			ui->spin_camRotX, ui->spin_camRotY, ui->spin_camRotZ,
			ui->spin_camScaleX, ui->spin_camScaleY, ui->spin_camScaleZ}) {

			connect(s, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updateGeneral);
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

			float vertSize = viewer_->getScene()->getFigure(current_figure_).vertexInfo().size() * 100;
			float edgeSize = viewer_->getScene()->getFigure(current_figure_).edgeInfo().size() * 1000;

			MaterialData mat = viewer_->getScene()->getFigure(current_figure_).material();
			Point3D translFigure = viewer_->getScene()->getFigure(current_figure_).translation();
			Point3D rotFigure = viewer_->getScene()->getFigure(current_figure_).rotation();
			Point3D scaleFigure = viewer_->getScene()->getFigure(current_figure_).scale();

			ui->btn_VertexColor->setStyleSheet(QString("background-color: %1").arg(vertColor.name()));
			ui->btn_EdgeColor->setStyleSheet(QString("background-color: %1").arg(edgColor.name()));
			ui->btn_LightColor->setStyleSheet(QString("background-color: %1").arg(lightColor.name()));

			ui->spin_VertexSize->setValue(vertSize);
			ui->spin_EdgeWidth->setValue(edgeSize);

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

			// Настройка области материалов
			Point3D materialColor = mat.color();
			QColor color = QColor(materialColor.x, materialColor.y, materialColor.z);
			ui->btn_MaterialColor->setStyleSheet(QString("background-color: %1").arg(color.name()));

			ui->roughnessSlider->setValue(mat.roughness() * 100);
			ui->metallicSlider->setValue(mat.metallic() * 100);
			ui->refractiveSlider->setValue(mat.refractive() * 100);
			ui->reflectivitySlider->setValue(mat.reflectivity() * 100);
			ui->alphaSlider->setValue(mat.alpha() * 100);

			// ТРАНСФОРМАЦИЯ ФИГУРЫ
			ui->spin_transX->setValue(translFigure.x);
			ui->spin_transY->setValue(translFigure.y);
			ui->spin_transZ->setValue(translFigure.z);

			ui->spin_rotX->setValue(rotFigure.x);
			ui->spin_rotY->setValue(rotFigure.y);
			ui->spin_rotZ->setValue(rotFigure.z);

			ui->spin_scaleX->setValue(scaleFigure.x);
			ui->spin_scaleY->setValue(scaleFigure.y);
			ui->spin_scaleZ->setValue(scaleFigure.z);

			updateInfoLabels();
		}
	}

	void MainWindow::setLightValues() noexcept {
		float intensity = viewer_->getScene()->getLight(current_light_).intensity();
		Point3D posLight = viewer_->getScene()->getLight(current_light_).position();
		auto color = viewer_->getScene()->getLight(current_light_).color();
		QColor qcolor = QColor(color.x * 255, color.y * 255, color.z * 255);

		ui->label_LightPos->setVisible(current_light_ != 1);
		ui->light_transX->setVisible(current_light_ != 1);
		ui->light_transY->setVisible(current_light_ != 1);
		ui->light_transZ->setVisible(current_light_ != 1);

		ui->light_transX->setValue(posLight.x);
		ui->light_transY->setValue(posLight.y);
		ui->light_transZ->setValue(posLight.z);

		ui->light_intensity->setValue(intensity);

		ui->btn_LightColor->setStyleSheet(QString("background-color: %1").arg(qcolor.name()));
	}

	void MainWindow::setGeneralValues() noexcept {
		auto backColorPoint = viewer_->getScene()->backgroundColor();
		QColor backColor = QColor(backColorPoint.x * 255, backColorPoint.y * 255, backColorPoint.z * 255);

		Point3D cameraTrans = viewer_->getScene()->getCamera().translation();
		Point3D cameraRot = viewer_->getScene()->getCamera().rotation();
		Point3D cameraScale = viewer_->getScene()->getCamera().scale();

		bool is_floor_display = viewer_->getScene()->displayFloor();

		ui->btn_BackgroundColor->setStyleSheet(QString("background-color: %1").arg(backColor.name()));
		ui->check_FloorDisplay->setCheckState(static_cast<Qt::CheckState>(is_floor_display));

		ui->spin_camTransX->setValue(cameraTrans.x);
		ui->spin_camTransY->setValue(cameraTrans.y);
		ui->spin_camTransZ->setValue(cameraTrans.z);

		ui->spin_camRotX->setValue(cameraRot.x);
		ui->spin_camRotY->setValue(cameraRot.y);
		ui->spin_camRotZ->setValue(cameraRot.z);

		ui->spin_camScaleX->setValue(cameraScale.x);
		ui->spin_camScaleY->setValue(cameraScale.y);
		ui->spin_camScaleZ->setValue(cameraScale.z);
	}

	void MainWindow::setUISettings(DisplayType type) noexcept {
		if (type == WIREFRAME_MODEL) {
			ui->groupBox_Edges->setVisible(true);
			ui->groupBox_Vertices->setVisible(true);

			ui->groupBox_Lights->setVisible(false);
			ui->groupBox_Material->setVisible(false);

			ui->label_displayType->setText("Отображение только ребер и вершин");
		}

		else if (type == FLAT_SHADING_MODEL) {
			ui->groupBox_Edges->setVisible(false);
			ui->groupBox_Vertices->setVisible(false);

			ui->groupBox_Lights->setVisible(true);
			ui->groupBox_Material->setVisible(false);

			ui->label_displayType->setText("Плоское затенение");
		}

		else if (type == SMOOTH_SHADING_MODEL || type == RAY_TRACING) {
			ui->groupBox_Edges->setVisible(false);
			ui->groupBox_Vertices->setVisible(false);

			ui->groupBox_Lights->setVisible(true);
			ui->groupBox_Material->setVisible(true);

			ui->label_displayType->setText("Мягкое затенение");
		}
	}


	void MainWindow::mousePressEvent(QMouseEvent* event) {
		lastPos_ = event->pos();
	}

	void MainWindow::mouseMoveEvent(QMouseEvent* event) {
		float dx = event->position().x() - lastPos_.x();
		float dy = event->position().y() - lastPos_.y();

		bool isCtrlPressed = (event->modifiers() & Qt::ControlModifier);

		if (event->buttons() & Qt::LeftButton && current_figure_ > 0) {
			// Левая кнопка мыши с Ctrl - перемещение камеры
			if (isCtrlPressed) {
				viewer_->getScene()->getCamera().translation().x += dx * TRANSLATION_MOUSE_SENSITIVITY;
				viewer_->getScene()->getCamera().translation().y -= dy * TRANSLATION_MOUSE_SENSITIVITY;

				ui->spin_camTransX->setValue(viewer_->getScene()->getCamera().translation().x);
				ui->spin_camTransY->setValue(viewer_->getScene()->getCamera().translation().y);
			}

			// Левая кнопка мыши - перемещение фигуры в фокусе
			else {
				viewer_->getScene()->getFigure(current_figure_).translation().x += dx * TRANSLATION_MOUSE_SENSITIVITY;
				viewer_->getScene()->getFigure(current_figure_).translation().y -= dy * TRANSLATION_MOUSE_SENSITIVITY;

				ui->spin_transX->setValue(viewer_->getScene()->getFigure(current_figure_).translation().x);
				ui->spin_transY->setValue(viewer_->getScene()->getFigure(current_figure_).translation().y);
			}
		}

		else if (event->buttons() & Qt::RightButton && current_figure_) {
			// Правая кнопка мыши с Ctrl - вращение камеры
			if (isCtrlPressed) {
				float new_x = viewer_->getScene()->getCamera().rotation().x + dy * ROTATION_MOUSE_SENSITIVITY;
				float new_y = viewer_->getScene()->getCamera().rotation().y + dx * ROTATION_MOUSE_SENSITIVITY;

				if (std::abs(new_x) >= 360.0) {
					new_x += (new_x > 0.0) ? -360.0 : 360.0;
				}
				if (std::abs(new_y) >= 360.0) {
					new_y += (new_y > 0.0) ? -360.0 : 360.0;
				}

				ui->spin_camRotX->setValue(new_x);
				ui->spin_camRotY->setValue(new_y);
			}

			// Правая кнопка мыши - вращение
			else {
				float new_x = viewer_->getScene()->getFigure(current_figure_).rotation().x + dy * ROTATION_MOUSE_SENSITIVITY;
				float new_y = viewer_->getScene()->getFigure(current_figure_).rotation().y + dx * ROTATION_MOUSE_SENSITIVITY;

				if (std::abs(new_x) >= 360.0) {
					new_x += (new_x > 0.0) ? -360.0 : 360.0;
				}
				if (std::abs(new_y) >= 360.0) {
					new_y += (new_y > 0.0) ? -360.0 : 360.0;
				}

				ui->spin_rotX->setValue(new_x);
				ui->spin_rotY->setValue(new_y);
			}
		}

		lastPos_ = event->pos();
		viewer_->DrawScene();
	}

	void MainWindow::wheelEvent(QWheelEvent* event) {
		// Колесико мыши - зум
		bool isCtrlPressed = (event->modifiers() & Qt::ControlModifier);

		if (current_figure_ > 0) {
			float det = event->angleDelta().y() * ZOOM_MOUSE_SENSITIVITY;

			if (isCtrlPressed) {
				ui->spin_camTransZ->setValue(ui->spin_camTransZ->value() - det);
			}

			else {
				ui->spin_scaleX->setValue(ui->spin_scaleX->value() + det);
				ui->spin_scaleY->setValue(ui->spin_scaleY->value() + det);
				ui->spin_scaleZ->setValue(ui->spin_scaleZ->value() + det);
			}

			viewer_->DrawScene();
			update();
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
		for (auto i = 1; i <= count_figures_; ++i) {
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
			figure_settings["edgeDisplayType"] = viewer_->getScene()->getFigure(i).edgeInfo().mode();

			figure_settings["displayType"] = viewer_->getScene()->getFigure(i).displayType();
			figure_settings["material"] = materialToJson(viewer_->getScene()->getFigure(i).material());

			settings[QString("figure %1").arg(i)] = figure_settings;
		}

		// Сохраняем настройки каждого источника освещения
		for (auto i = 2; i <= count_lights_; ++i) {
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
			std::cout << "[MainWindow] Настройки сохранены в файл: "
		            << filePath.toStdString() << std::endl;
		}
		else {
			std::cerr << "[MainWindow] Ошибка сохранения настроек в файл: "
			          << filePath.toStdString() << std::endl;
		}
	}

	void MainWindow::loadSettingsFromFile(const QString& filePath) {
		QFile file(filePath);

		if (!file.exists()) {
			std::cout << "[MainWindow] Файл настроек не найден, используем значения по умолчанию" << std::endl;
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
			while (settings.contains(QString("figure %1").arg(i))) {
				auto figure_settings = settings[QString("figure %1").arg(i)].toObject();

				if (loadScene(figure_settings["path"].toString())) {
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
					if (figure_settings.contains("displayType")) {
						viewer_->getScene()->getFigure(i).displayType() = static_cast<DisplayType>(figure_settings["displayType"].toInt());
					}
					if (figure_settings.contains("material")) {
						viewer_->getScene()->getFigure(i).material() = materialFromJson(figure_settings["material"].toObject());
					}

					setFigureValues();
					setUISettings(viewer_->getScene()->getFigure(i).displayType());

					i++;
				}
				else {
					std::cout << "Не удалось загрузить фигуру: " << figure_settings["path"].toString().toStdString() << std::endl;
				}
			}

			i = 2;
			while (settings.contains(QString("light %1").arg(i))) {
				auto light_settings = settings[QString("light %1").arg(i)].toObject();
				on_btn_AddLight_clicked();

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

			std::cout << "[MainWindow] Настройки загружены из файла: "
			          << filePath.toStdString() << std::endl;
		}
		else {
			std::cerr << "[MainWindow] Ошибка загрузки настроек из файла: "
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

	QJsonObject MainWindow::materialToJson(const MaterialData &mat) noexcept {
		QJsonObject obj;

		obj["color"] = colorToJson(mat.baseColor_);
		obj["roughness"] = mat.roughness_;
		obj["metallic"] = mat.metallic_;
		obj["refractive"] = mat.refractive_;
		obj["reflectivity"] = mat.reflectivity_;
		obj["alpha"] = mat.alpha_;

		return obj;
	}

	MaterialData MainWindow::materialFromJson(const QJsonObject &jobject) noexcept {
		MaterialData data;

		data.baseColor_ = colorFromJson(jobject["color"].toObject());
		data.roughness_ = static_cast<float>(jobject["roughness"].toDouble());
		data.metallic_ = static_cast<float>(jobject["metallic"].toDouble());
		data.refractive_ = static_cast<float>(jobject["refractive"].toDouble());
		data.reflectivity_ = static_cast<float>(jobject["reflectivity"].toDouble());
		data.alpha_ = static_cast<float>(jobject["alpha"].toDouble());

		return data;
	}

	// Слоты для ComboBox
	void MainWindow::onVertexTypeChanged(int index) {
		// index: 0 - Нет, 1 - Круг, 2 - Квадрат
		VerticesMode mode = VerticesMode::SQUARE;

		if(index == 0) mode = VerticesMode::NONE;
		else if(index == 1) mode = VerticesMode::CIRCLE;

		viewer_->getScene()->getFigure(current_figure_).vertexInfo().mode() = mode;
		viewer_->DrawScene();
	}

	void MainWindow::onEdgeTypeChanged(int index) {
		// index: 0 - Сплошная, 1 - Пунктир
		EdgesMode mode = (index == 1) ? EdgesMode::DASHED : EdgesMode::SOLID;

		viewer_->getScene()->getFigure(current_figure_).edgeInfo().mode() = mode;
		viewer_->DrawScene();
	}

	void MainWindow::onLightChanged(int value) {
		current_light_ = value + 1;

		setLightValues();
	}

	void MainWindow::onFigureChanged(int value) {
		current_figure_ = value + 1;

		setFigureValues();
		setUISettings(viewer_->getScene()->getFigure(current_figure_).displayType());
	}

	void MainWindow::onFloorDisplayChanged() {
		viewer_->getScene()->displayFloor() = !viewer_->getScene()->displayFloor();

		viewer_->DrawScene();
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
			viewer_->DrawScene();

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
			viewer_->DrawScene();

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
			viewer_->DrawScene();

			ui->btn_LightColor->setStyleSheet(QString("background-color: %1").arg(color.name()));
		}
	}

	void MainWindow::on_btn_AddLight_clicked() {
		count_lights_++;
		current_light_ = count_lights_;
		viewer_->getScene()->addLight();
		ui->combo_LightSelect->addItem(QString("Источник %1").arg(viewer_->getScene()->countLights()));
		viewer_->DrawScene();

		ui->combo_LightSelect->setCurrentIndex(count_lights_ - 1);
	}

	void MainWindow::on_btn_MaterialColor_clicked() {
		QColor color = QColorDialog::getColor(Qt::blue, this, "Выберите цвет источника освещения");

		if (color.isValid()) {
			float r = color.redF();
			float g = color.greenF();
			float b = color.blueF();

			viewer_->getScene()->getFigure(current_figure_).material().color() = Point3D(r, g, b);
			viewer_->DrawScene();

			ui->btn_MaterialColor->setStyleSheet(QString("background-color: %1").arg(color.name()));
		}
	}


	bool MainWindow::loadScene(const QString& path) {
		auto result = viewer_->LoadFigure(path.toStdString());

		if (result.isSuccess()) {
			currentFileName_ = QFileInfo(path).fileName();
			count_figures_ ++;
			current_figure_ = count_figures_;

			updateInfoLabels();

			ui->groupBox_Figure->setVisible(true);
			ui->combo_FigureSelect->addItem(QString("Фигура %1").arg(viewer_->getScene()->countFigures()));
			ui->combo_FigureSelect->setCurrentIndex(count_figures_ - 1);
			ui->statusbar->showMessage("Загружено: " + currentFileName_);
		}

		else {
			ui->statusbar->showMessage(QString::fromStdString(result.getMessage()));
			QMessageBox::critical(this, "Ошибка", "Не удалось загрузить файл: " + path);
			std::cout << "[MainWindow] Ошибка загрузки файла: " << path.toStdString() << "\n";
		}

		return result.isSuccess();
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
		else if (viewer_->getScene()->getFigure(current_figure_).displayType() == RAY_TRACING)
			ui->label_displayType->setText("Трассировка лучей");

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
			viewer_->DrawScene();
		}
	}

	void MainWindow::on_action_Exit_triggered() {
		std::cout << "[MainWindow] Выход из приложения\n";
		close();
	}

	void MainWindow::on_action_Orthographic_triggered() {
		viewer_->getScene()->getCamera().projectionType() = ORTHOGRAPHIC;
		viewer_->DrawScene();

		ui->label_projectionType->setText("Параллельная проекция");
	}

	void MainWindow::on_action_Perspective_triggered() {
		viewer_->getScene()->getCamera().projectionType() = PERSPECTIVE;
		viewer_->DrawScene();

		ui->label_projectionType->setText("Центральная проекция");
	}

	void MainWindow::on_action_Wireframe_triggered() {
		if (current_figure_ > 0) {
			viewer_->getScene()->getFigure(current_figure_).displayType() = WIREFRAME_MODEL;
			setUISettings(WIREFRAME_MODEL);

			viewer_->DrawScene();
		}

		else {
			ui->statusbar->showMessage("Для начала загрузите фигуру");
		}
	}

	void MainWindow::on_action_FlatShading_triggered() {
		if (current_figure_ > 0) {
			viewer_->getScene()->getFigure(current_figure_).displayType() = FLAT_SHADING_MODEL;
			setUISettings(FLAT_SHADING_MODEL);

			viewer_->DrawScene();
		}

		else {
			ui->statusbar->showMessage("Для начала загрузите фигуру");
		}
	}

	void MainWindow::on_action_SmoothShading_triggered() {
		if (current_figure_ > 0) {
			viewer_->getScene()->getFigure(current_figure_).displayType() = SMOOTH_SHADING_MODEL;
			setUISettings(SMOOTH_SHADING_MODEL);

			viewer_->DrawScene();
		}

		else {
			ui->statusbar->showMessage("Для начала загрузите фигуру");
		}
	}

	void MainWindow::on_action_RayTracing_triggered() {
		if (current_figure_ > 0) {
			viewer_->getScene()->getFigure(current_figure_).displayType() = RAY_TRACING;
			ui->groupBox_Edges->setVisible(false);
			ui->groupBox_Vertices->setVisible(false);

			ui->groupBox_Lights->setVisible(true);
			ui->groupBox_Material->setVisible(true);

			ui->label_displayType->setText("Трассировка лучей");

			viewer_->DrawScene();
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
