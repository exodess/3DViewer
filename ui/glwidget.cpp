#include "ui/glwidget.h"

namespace viewer {

	static float global_matrix[4][4];

	GLWidget::GLWidget(QWidget* parent, Ui::MainWindow* ui)
	    : QOpenGLWidget(parent)
	    , scene_(nullptr)
	    , shaderProgram_(nullptr)
	    , mesh_(nullptr)
		, ui_(ui)
	    , backColor_(Point3D(1.0, 1.0, 1.0))
		, lastPos_(0, 0)
		, rotation_(Point3D(0.0f, 0.0f, 0.0f))
		, translation_(Point3D(0.0f, 0.0f, 0.0f))
		, gif_recorder_(std::make_unique<GifRecorder>())
		, record_timer_(new QTimer(this))
	{
		setFocusPolicy(Qt::StrongFocus);
		loadSettingsFromFile("settings_viewer.json");

		// Инициализация GIF рекордера
		connect(record_timer_, &QTimer::timeout, this, &GLWidget::captureFrame);

		std::cout << "[GLWidget] Инициализация GLWidget\n";
	}

	GLWidget::~GLWidget() {
		std::cout << "[GLWidget] Удаление GLWidget\n";
		if (mesh_) {
		    delete mesh_;
		}
		if (shaderProgram_) {
		    delete shaderProgram_;
		}

		if (record_timer_ && record_timer_->isActive()) {
			record_timer_->stop();
		}
		if (gif_recorder_ && gif_recorder_->IsRecording()) {
			gif_recorder_->StopRecording();
		}

		saveSettingsToFile("settings_viewer.json");
	}

	void GLWidget::initializeGL() {
		initializeOpenGLFunctions();

		compileShaders();
		setNewBackgroundColor(backColor_.x, backColor_.y, backColor_.z);
		std::cout << "[GLWidget] OpenGL инициализирован\n";
	}

	void GLWidget::paintGL() {
		if (mesh_ && shaderProgram_) {
		    shaderProgram_->use();

		    mesh_->render();
		}
	}

	void GLWidget::resizeGL(int w, int h) {
		glViewport(0, 0, w, h);
		setNewProjectionType(projectionType_);
	}

	void GLWidget::saveImage(const QString& path) {
		if (path.isEmpty()) {
			std::cerr << "[GLWidget] Путь к файлу пуст\n";
			return;
		}

		QFileInfo fileInfo(path);
		QDir dir = fileInfo.absoluteDir();

		if (!dir.exists()) {
			std::cerr << "[GLWidget] Директория не существует: " << dir.absolutePath().toStdString() << "\n";
			return;
		}

		makeCurrent();

		if (!isValid()) {
			std::cerr << "[GLWidget] OpenGL контекст не валиден\n";
			doneCurrent();
			return;
		}

		QImage image = grabFramebuffer();
		if (image.isNull()) {
			std::cerr << "[GLWidget] Не удалось захватить изображение\n";
			doneCurrent();
			return;
		}

		QString format = fileInfo.suffix().toLower();
		bool success = false;

		if (format == "bmp") {
			success = image.save(path, "BMP");
		} else if (format == "jpg" || format == "jpeg") {
			success = image.save(path, "JPEG", 95);
		} else if (format == "png") {
			success = image.save(path, "PNG");
		} else {
			success = image.save(path + ".png", "PNG");
		}

		if (success) {
			std::cout << "[GLWidget] Изображение сохранено: " << path.toStdString() << "\n";
		} else {
			std::cerr << "[GLWidget] Ошибка сохранения изображения в файл: " << path.toStdString() << "\n";
		}

		doneCurrent();
	}

	// ==============================================
	// ========== МЕТОДЫ ДЛЯ ЗАПИСИ GIF =============
	// ==============================================

	void GLWidget::startRecording(const QString& path, int fps, int duration_sec) {
		if (!gif_recorder_) {
			gif_recorder_ = std::make_unique<GifRecorder>();
		}

		if (gif_recorder_->IsRecording()) {
			std::cout << "[GLWidget] Уже идет запись GIF\n";
			return;
		}
		GifRecorder::Config config;
		config.fps = fps;
		config.duration_seconds = duration_sec;
		config.width = width();
		config.height = height();

		// Сигнатура сервиса к сигналам Qt
		gif_recorder_->StartRecording(
			path.toStdString(), config,
			[this](int frame, int total) { emit recordingProgress(frame, total); },
			[this](const std::string& path) {
			  emit recordingFinished(QString::fromStdString(path));
			},
			[this](const std::string& error) {
			  emit recordingError(QString::fromStdString(error));
			});

		// Запуск таймера для захвата кадров с заданной частотой
		int interval_ms = 1000 / fps;
		record_timer_->start(interval_ms);

		std::cout << "[GLWidget] Начата запись GIF: " << fps << " fps, "
				  << duration_sec << " сек\n";
	}

	void GLWidget::stopRecording() {
		if (!gif_recorder_ || !gif_recorder_->IsRecording()) {
			return;
		}

		if (record_timer_->isActive()) {
			record_timer_->stop();
		}

		gif_recorder_->StopRecording();
		std::cout << "[GLWidget] Запись GIF остановлена\n";
	}

	bool GLWidget::isRecording() const {
		return gif_recorder_ && gif_recorder_->IsRecording();
	}

	void GLWidget::captureFrame() {
		if (!gif_recorder_ || !gif_recorder_->IsRecording()) {
			return;
		}

		makeCurrent();

		if (isValid()) {
			// Вызов отрисовки напрямую
			paintGL();

			glFinish();

			// Захват кадра через glReadPixels
			QImage frame(width(), height(), QImage::Format_RGB888);
			glReadPixels(0, 0, width(), height(), GL_RGB, GL_UNSIGNED_BYTE,
						 frame.bits());

			QImage flipped = frame.mirrored(false, true);
			QImage rgb = flipped.convertToFormat(QImage::Format_RGB888);

			gif_recorder_->AddFrame(rgb.constBits(), rgb.width(), rgb.height());
		}

		doneCurrent();
	}

	void GLWidget::mousePressEvent(QMouseEvent* event) {
		lastPos_ = event->pos();
	}

	void GLWidget::mouseMoveEvent(QMouseEvent* event) {
		float dx = event->position().x() - lastPos_.x();
		float dy = event->position().y() - lastPos_.y();

		if (event->buttons() & Qt::LeftButton) {
			// Левая кнопка мыши - перемещение
			translation_.x += dx * TRANSLATION_MOUSE_SENSITIVITY;
			translation_.y -= dy * TRANSLATION_MOUSE_SENSITIVITY;

			setTranslation(translation_.x, translation_.y, ui_->spin_transZ->value());
			ui_->spin_transX->setValue(translation_.x);
			ui_->spin_transY->setValue(translation_.y);
		}

		else if (event->buttons() & Qt::RightButton) {
			// Правая кнопка мыши - вращение
			rotation_.x += dx * ROTATION_MOUSE_SENSITIVITY;
			rotation_.y += dy * ROTATION_MOUSE_SENSITIVITY;

			setRotation(rotation_.y, rotation_.x, ui_->spin_rotZ->value());
			ui_->spin_rotX->setValue(static_cast<int>(rotation_.y) % 360);
			ui_->spin_rotY->setValue(static_cast<int>(rotation_.x) % 360);
		}

		lastPos_ = event->pos();
		update();
	}

	void GLWidget::wheelEvent(QWheelEvent* event) {
		// Колесико мыши - зум

		float det = event->angleDelta().y() * ZOOM_MOUSE_SENSITIVITY;
		cameraZoom_ -= det * 0.5f;
		setNewViewMatrix(cameraZoom_);

		update();
	}

	// ===========================================================
	// ============= СОХРАНЕНИЕ И ЗАГРУЗКА НАСТРОЕК ==============
	// ===========================================================

	void GLWidget::saveSettingsToFile(const QString& filePath) {
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

	QJsonObject GLWidget::colorToJson(const Point3D& color) noexcept {
		QJsonObject obj;

		obj["r"] = color.x;
		obj["g"] = color.y;
		obj["b"] = color.z;

		return obj;
	}

	Point3D GLWidget::colorFromJson(const QJsonObject& obj) noexcept {

		Point3D color;

		color.x = static_cast<float>(obj["r"].toDouble());
		color.y = static_cast<float>(obj["g"].toDouble());
		color.z = static_cast<float>(obj["b"].toDouble());

		return color;
	}

	void GLWidget::loadSettingsFromFile(const QString& filePath) {
		QFile file(filePath);

		if (!file.exists()) {
			std::cout << "[GLWidget] Файл настроек не найден, используем значения по "
			             "умолчанию"
			          << std::endl;
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


	void GLWidget::DrawScene(Scene* scene) {
		makeCurrent();

		if(scene_ != scene) {
			// загружена новая фигура
			scene_ = scene;

			if (mesh_)
				delete mesh_;

			mesh_ = new Mesh();
			mesh_->loadData(scene_->getFigure());
		}

		setNewVerticesSize(vertSize_);
		setNewVerticesColor(vertColor_.x, vertColor_.y, vertColor_.z);
		setNewVerticesMode(vertMode_);
		setNewEdgesSize(edgSize_);
		setNewEdgesColor(edgColor_.x, edgColor_.y, edgColor_.z);
		setNewEdgesMode(edgMode_);

		setNewDisplayType(displayType_);
		setNewLightColor(lightColor_.x, lightColor_.y, lightColor_.z);
		setNewLightPosition(lightPosition_.x, lightPosition_.y, lightPosition_.z);

		setNewViewMatrix(cameraZoom_);
		setNewProjectionType(projectionType_);

		setNewAspectRatio(static_cast<float>(width()) / static_cast<float>(height()));
		setNewBackgroundColor(backColor_.x, backColor_.y, backColor_.z);
		setNewModelMatrix();

		std::cout << "[GLWidget] Сцена отрисована | Вершин: " << countVertices()
		          << " | Поверхностей: " << countSurfaces() << "\n";

		mesh_->render();
		update();
		doneCurrent();

	}

	void GLWidget::setNewBackgroundColor(float r, float g, float b) noexcept {

		makeCurrent();
		backColor_ = Point3D(r, g, b);
		glClearColor(r, g, b, 1.0f);
		update();
		doneCurrent();
	}

	void GLWidget::setNewModelMatrix() noexcept {

		makeCurrent();

		TransformMatrix scaleMatrix = \
			TransformMatrixBuilder::CreateScaleMatrix(scaleVector_.x, scaleVector_.y, scaleVector_.z);
		TransformMatrix rotationMatrix = \
			TransformMatrixBuilder::CreateRotationMatrix(rotationVector_.x, rotationVector_.y, rotationVector_.z);
		TransformMatrix translationMatrix = \
			TransformMatrixBuilder::CreateMoveMatrix(translationVector_.x, translationVector_.y, translationVector_.z);

		modelMatrix_ = translationMatrix * rotationMatrix * scaleMatrix;

		if (mesh_ && shaderProgram_) {

			for(int i = 0; i < 4; ++i) {
				for(int j = 0; j < 4; ++j) {
					global_matrix[i][j] = modelMatrix_(i, j);
				}
			}

			mesh_->loadModelMatrix(
				shaderProgram_->getModelMatrixUniformLocation(),
				global_matrix
			);

			float normal_matrix[3][3];
			TransformMatrix normal = modelMatrix_.inverse().transpose();

			for(int i = 0; i < 3; ++i) {
				for(int j = 0; j < 3; ++j) {
					normal_matrix[i][j] = normal(i, j);
				}
			}
			mesh_->loadNormalMatrix(
				shaderProgram_->getNormalMatrixUniformLocation(),
				normal_matrix
			);

			update();
		}
		doneCurrent();
	}

	void GLWidget::setNewViewMatrix(float zoomFactor) noexcept {

		makeCurrent();
		cameraZoom_ = zoomFactor;
		// Просто отодвигаем камеру по Z (без вращения!)
		viewMatrix_ = TransformMatrixBuilder::CreateMoveMatrix(cameraVector_.x, cameraVector_.y, -cameraVector_.z * zoomFactor);

		if(mesh_ && shaderProgram_) {
			for(int i = 0; i < 4; ++i) {
	 			for(int j = 0; j < 4; ++j) {
	 				global_matrix[i][j] = viewMatrix_(i, j);
	 			}
			}

			mesh_->loadViewMatrix(
			    shaderProgram_->getViewMatrixUniformLocation(),
			    global_matrix
			);
			update();

		}

		doneCurrent();

	}

	void GLWidget::setNewProjectionType(ProjectionType type) noexcept {
		makeCurrent();
		projectionType_ = type;

		if(mesh_ && shaderProgram_) {

			float aspect = static_cast<float>(width()) / static_cast<float>(height());

			if (type == ProjectionType::ORTHOGRAPHIC) {

			    float size = 2.0f;
			    projectionMatrix_ = TransformMatrixBuilder::CreateOrthographicMatrix(
			        -size * aspect, size * aspect, -size, size, -100.0f, 100.0f
			    );
			}
			else {
			    projectionMatrix_ = TransformMatrixBuilder::CreatePerspectiveMatrix(45.0f, aspect, 0.1f, 100.0f);
			}

			for(int i = 0; i < 4; ++i) {
				for(int j = 0; j < 4; ++j) {
					global_matrix[i][j] = projectionMatrix_(i, j);
				}
			}

			mesh_->loadProjectionMatrix(
				shaderProgram_->getProjectionMatrixUniformLocation(),
				global_matrix
			);

			update();
		}

		doneCurrent();
	}

	void GLWidget::setRotation(float x, float y, float z) noexcept {
		rotationVector_.x = x;
		rotationVector_.y = y;
		rotationVector_.z = z;

		setNewModelMatrix();
	}

	void GLWidget::setTranslation(float x, float y, float z) noexcept {
		translationVector_.x = x;
		translationVector_.y = y;
		translationVector_.z = z;

		setNewModelMatrix();
	}

	void GLWidget::setScale(float x, float y, float z) noexcept {
		scaleVector_.x = x;
		scaleVector_.y = y;
		scaleVector_.z = z;

		setNewModelMatrix();
	}

	void GLWidget::setNewAspectRatio(float ratio) noexcept {
		makeCurrent();

		if (mesh_ && shaderProgram_) {
			mesh_->loadAspectRatio(
			    shaderProgram_->getAspectRatioUniformLocation(),
			    ratio
			);
		}

		doneCurrent();
	}

	void GLWidget::setNewVerticesColor(float r, float g, float b) noexcept {
		makeCurrent();

		vertColor_ = Point3D(r, g, b);
		if (mesh_ && shaderProgram_) {
			mesh_->loadVerticesColor(
			    shaderProgram_->getColorVerticesUniformLocation(),
			    r, g, b
			);
			update();
		}

		doneCurrent();
	}

	void GLWidget::setNewVerticesSize(float new_size) noexcept {
		makeCurrent();

		vertSize_ = new_size;
		if (mesh_ && shaderProgram_) {
			mesh_->loadVerticesSize(
			    shaderProgram_->getSizeVerticesUniformLocation(),
			    new_size
			);
			update();
		}

		doneCurrent();
	}

	void GLWidget::setNewVerticesMode(VerticesMode mode) noexcept {
		makeCurrent();
		vertMode_ = mode;
		if (mesh_ && shaderProgram_) {

			// Если выбран вариант не NONE, то загружаем старое значение size
			mesh_->loadVerticesSize(
				shaderProgram_->getSizeVerticesUniformLocation(),
				vertSize_
			);

			mesh_->loadVerticesMode(
			    shaderProgram_->getSizeVerticesUniformLocation(),
			    shaderProgram_->getModeVerticesUniformLocation(),
			    mode
			);
			update();
		}
		doneCurrent();
	}

	void GLWidget::setNewEdgesColor(float r, float g, float b) noexcept {
		makeCurrent();

		edgColor_ = Point3D(r, g, b);
		if (mesh_ && shaderProgram_) {
			mesh_->loadEdgesColor(
			    shaderProgram_->getColorEdgesUniformLocation(),
			    r, g, b
			);
			update();
		}

		doneCurrent();
	}

	void GLWidget::setNewEdgesSize(float new_size) noexcept {
		makeCurrent();

		edgSize_ = new_size;
		if (mesh_ && shaderProgram_) {
			mesh_->loadEdgesSize(
			    shaderProgram_->getSizeEdgesUniformLocation(),
			    new_size
			);
			update();
		}

		doneCurrent();
	}

	void GLWidget::setNewEdgesMode(EdgesMode mode) noexcept {
		makeCurrent();

		edgMode_ = mode;
		if (mesh_ && shaderProgram_) {
			mesh_->loadEdgesMode(
			    shaderProgram_->getModeEdgesUniformLocation(),
			    mode
			);
			update();
		}

		doneCurrent();
	}

	void GLWidget::setNewLightPosition(float x, float y, float z) noexcept {
		makeCurrent();
		lightPosition_ = Point3D(x, y, z);

		if (mesh_ && shaderProgram_) {
			mesh_->loadLightPosition(
				shaderProgram_->getLightPositionUniformLocation(),
				x, y, z
				);
			update();
		}

		doneCurrent();
	}

	void GLWidget::setNewLightColor(float r, float g, float b) noexcept {
		makeCurrent();
		lightColor_ = Point3D(r, g, b);

		if (mesh_ && shaderProgram_) {
			mesh_->loadLightColor(
				shaderProgram_->getLightColorUniformLocation(),
				r, g, b
				);
			update();
		}

		doneCurrent();
	}

	void GLWidget::setNewDisplayType(DisplayType type) noexcept {
		makeCurrent();
		displayType_ = type;

		if (mesh_ && shaderProgram_) {
			mesh_->loadDisplayType(
				shaderProgram_->getDisplayTypeUniformLocation(),
				type
				);
			update();
		}

		doneCurrent();
	}

	// =======================================
	// ======= Гетеры для интерфейса =========
	// =======================================

	QColor GLWidget::getEdgeColor() noexcept {
		return QColor(edgColor_.x * 255, edgColor_.y * 255, edgColor_.z * 255, 1.0);
	}

	QColor GLWidget::getVertexColor() noexcept {
		return QColor(vertColor_.x * 255, vertColor_.y * 255, vertColor_.z * 255, 1.0);
	}

	QColor GLWidget::getBackgroundColor() noexcept {
		return QColor(backColor_.x * 255, backColor_.y * 255, backColor_.z * 255, 1.0);
	}

	QColor GLWidget::getLightColor() noexcept {
		return QColor(lightColor_.x * 255, lightColor_.y * 255, lightColor_.z * 255, 1.0);
	}

	float GLWidget::getVertexSize() noexcept {
		return vertSize_ * 100.0f;
	}

	float GLWidget::getEdgeSize() noexcept {
		return edgSize_ * 1000.0f;
	}

	Point3D GLWidget::getLightPosition() noexcept {
		return lightPosition_;
	}

	int GLWidget::getVertexMode() noexcept {
		return static_cast<int>(vertMode_);
	}

	int GLWidget::getEdgeMode() noexcept {
		return static_cast<int>(edgMode_);
	}

	int GLWidget::getDisplayType() noexcept {
		return displayType_;
	}

	int GLWidget::getProjectionType() noexcept {
		return projectionType_;
	}


	uint32_t GLWidget::countVertices() noexcept {
	    return mesh_ ? mesh_->countVertices() : 0;
	}

	uint32_t GLWidget::countSurfaces() noexcept {
	    return mesh_ ? mesh_->countSurfaces() : 0;
	}

	void GLWidget::compileShaders() {
		QString shaderPath = QCoreApplication::applicationDirPath() + "/shaders/";
		QString vertPath = shaderPath + "shader.vert";
		QString geomPath = shaderPath + "shader.geom";
		QString fragPath = shaderPath + "shader.frag";

		if (!QFile::exists(vertPath)) {
			vertPath = "shaders/shader.vert";
			geomPath = "shaders/shader.geom";
			fragPath = "shaders/shader.frag";
		}

		shaderProgram_ = new ShaderProgram(vertPath.toStdString(), geomPath.toStdString(), fragPath.toStdString());
		std::cout << "[GLWidget] Шейдеры скомпилированы\n";
	}

}
