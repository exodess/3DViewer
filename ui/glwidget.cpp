#include "ui/glwidget.h"

namespace viewer {

	GLWidget::GLWidget(QWidget* parent)
	    : QOpenGLWidget(parent)
		, current_program_id_(0)
	    , mesh_(nullptr)
		, aspect_(1.0f)
		, gif_recorder_(std::make_unique<GifRecorder>())
		, record_timer_(std::make_unique<QTimer>(this))
	{
		setFocusPolicy(Qt::StrongFocus);

		// Инициализация GIF рекордера
		connect(record_timer_.get(), &QTimer::timeout, this, &GLWidget::captureFrame);
		shader_programs_.reserve(DisplayType::COUNT + 1);

		std::cout << "[GLWidget] Инициализация GLWidget\n";
	}

	void GLWidget::initializeGL() {
		initializeOpenGLFunctions();
		ShaderProgram::initOpenGLTools();

		compileShaders();
		mesh_ = std::make_unique<Mesh>();
		std::cout << "[GLWidget] OpenGL инициализирован\n";
	}

	void GLWidget::paintGL() {
		Mesh::clear();
		shader_programs_[current_program_id_]->use();
		mesh_->render(current_program_id_ == RAY_TRACING + 1);
	}

	void GLWidget::resizeGL(int w, int h) {
		if (h == 0) h = 1;
		glViewport(0, 0, w, h);
		aspect_ = static_cast<float>(w) / static_cast<float>(h);
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

	void GLWidget::DrawScene(Scene* scene) {
		makeCurrent();
		if (mesh_ && !shader_programs_.empty()) {
			// Выбираем необходимую шейдерную программу
			current_program_id_ = scene->displayType() + 1;
			Mesh::clear();

			// Сначала, если нужно, отображаем пол
			if (scene->displayFloor() && scene->displayType() != RAY_TRACING) {
				shader_programs_[0]->use();
				mesh_->loadBackgroundColor(
					shader_programs_[0]->getUniformLocation((char*)UNIFORM_FLOOR_BACKGROUND_COLOR),
					scene->backgroundColor());

				mesh_->renderFloor();
			}

			shader_programs_[current_program_id_]->use();
			glClearColor(scene->backgroundColor().x, scene->backgroundColor().y, scene->backgroundColor().z, 1.0f);

			// Везде необходима информация о камере
			mesh_->loadCameraStructure(scene->getCamera().getData(aspect_));

			if (scene->displayType() == WIREFRAME_MODEL) {
				mesh_->loadAspectRatio(
					shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_ASPECT_RATIO),
					aspect_);

				// Загружаем информацию о каждой фигуре

				for (auto i = 0; i < scene->countFigures(); ++i) {
					auto current_figure = scene->getFigure(i + 1);

					mesh_->loadGeometryData(current_figure);
					mesh_->loadModelMatrix(
						shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_MODEL_MATRIX),
						current_figure.getModelMatrix());

					// Загружаем информацию о вершинах
					mesh_->loadVerticesSize(
						shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_VERTICES_SIZE),
						current_figure.vertexInfo().size());
					mesh_->loadVerticesColor(
						shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_VERTICES_COLOR),
						current_figure.vertexInfo().color());
					mesh_->loadVerticesMode(
						shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_VERTICES_SIZE),
						shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_VERTICES_MODE),
						current_figure.vertexInfo().mode());

					// Загружаем информацию о ребрах
					mesh_->loadEdgesSize(
						shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_EDGES_SIZE),
						current_figure.edgeInfo().size());
					mesh_->loadEdgesColor(
						shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_EDGES_COLOR),
						current_figure.edgeInfo().color());
					mesh_->loadEdgesMode(
						shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_EDGES_MODE),
						current_figure.edgeInfo().mode());

					mesh_->render(false);
				}
			}

			if (scene->displayType() == FLAT_SHADING_MODEL) {
				for (auto i = 0; i < scene->countFigures(); ++i) {
					auto current_figure = scene->getFigure(i + 1);

					mesh_->loadGeometryData(current_figure);
					mesh_->loadModelMatrix(
						shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_MODEL_MATRIX),
						current_figure.getModelMatrix());

					mesh_->loadCountActiveLight(
						shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_FLAT_ACTIVE_LIGHTS),
						scene->countLights());
					mesh_->loadLightStructure(scene->getSceneLightsData());

					mesh_->render(false);
				}
			}

			else if (scene->displayType() == SMOOTH_SHADING_MODEL) {
				for (auto i = 0; i < scene->countFigures(); ++i) {
					auto current_figure = scene->getFigure(i + 1);

					mesh_->loadGeometryData(current_figure);
					mesh_->loadModelMatrix(
						shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_MODEL_MATRIX),
						current_figure.getModelMatrix());
					mesh_->loadNormalMatrix(
						shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_NORMAL_MATRIX),
						current_figure.getModelMatrix());

					mesh_->loadCountActiveLight(
						shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_SMOOTH_ACTIVE_LIGHTS),
						scene->countLights());
					mesh_->loadLightStructure(scene->getSceneLightsData());
					mesh_->loadMaterialStructure(current_figure.material());

					mesh_->render(false);
				}
			}

			else if (scene->displayType() == RAY_TRACING) {
				mesh_->loadBackgroundColor(
					shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_RAY_BACKGROUND_COLOR),
					scene->backgroundColor());
				mesh_->loadLightStructure(scene->getSceneLightsData());
				mesh_->loadCountActiveLight(
					shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_RAY_TRACING_ACTIVE_LIGHTS),
					scene->countLights());
				mesh_->loadTotalFigures(
					shader_programs_[current_program_id_]->getUniformLocation((char*)UNIFORM_TOTAL_FIGURES),
					scene->countFigures());

				mesh_->loadFiguresData(scene->getAllFigure());

				mesh_->render(true);
			}
		}

		else {
			std::cout << "[GLWidget] Инструменты OpenGL недоступны!" << std::endl;
		}

		doneCurrent();
	}

	void GLWidget::record(const std::string &dist, RecordType type) noexcept {
		if (type == IMAGE_RECORD) {
			saveImage(QString::fromStdString(dist));
		}

		else {
			startRecording(QString::fromStdString(dist), GIF_FPS_COUNT, GIF_DURATION_SEC);
		}
	}

	void GLWidget::compileShaders() {
		QString shadersPath = QCoreApplication::applicationDirPath() + "/shaders/";
		QStringList shader_directories;
		shader_directories << shadersPath + "floor/";
		shader_directories << shadersPath + "wireframe/";
		shader_directories << shadersPath + "flat_shading/";
		shader_directories << shadersPath + "smooth_shading/";
		shader_directories << shadersPath + "ray_tracing/";

		QStringList filters;
		filters << "*.vert" << "*.geom" << "*.frag";
		for (const auto& path : shader_directories) {
			QString vertShader, geomShader, fragShader;
			QDir dir(path);

			std::cout << "Поиск файлов шейдеров в директории " + path.toStdString() + "\n";

			// Ищем файлы шейдеров
			dir.setNameFilters(filters);
			dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

			QFileInfoList fileList = dir.entryInfoList();

			for (const auto& file : fileList) {
				if (file.fileName() == "shader.vert") vertShader = file.absoluteFilePath();
				if (file.fileName() == "shader.geom") geomShader = file.absoluteFilePath();
				if (file.fileName() == "shader.frag") fragShader = file.absoluteFilePath();
			}

			shader_programs_.push_back(std::make_shared<ShaderProgram>(vertShader.toStdString(), geomShader.toStdString(), fragShader.toStdString()));
		}
		std::cout << "[GLWidget] Шейдеры скомпилированы\n";
	}

}
