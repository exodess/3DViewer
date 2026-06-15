#include "ui/glwidget.h"

// Что еще нужно исправить:
// 1. Движение камеры
// 3. Починить масштабирование с помощью колесика мыши
// 4. Заменить LightData на Light

namespace viewer {

	GLWidget::GLWidget(QWidget* parent)
	    : QOpenGLWidget(parent)
	    , shaderProgram_(nullptr)
	    , mesh_(nullptr)
		, aspect_(1.0f)
		, gif_recorder_(std::make_unique<GifRecorder>())
		, record_timer_(new QTimer(this))
	{
		setFocusPolicy(Qt::StrongFocus);

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
	}

	void GLWidget::initializeGL() {
		initializeOpenGLFunctions();

		compileShaders();
		mesh_ = new Mesh();
		std::cout << "[GLWidget] OpenGL инициализирован\n";
	}

	void GLWidget::paintGL() {
		// Mesh::clear();
		shaderProgram_->use();
		mesh_->render();
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
		if (mesh_ && shaderProgram_) {
			Mesh::clear();
			shaderProgram_->use();

			glClearColor(scene->backgroundColor().x, scene->backgroundColor().y, scene->backgroundColor().z, 1.0f);
			mesh_->loadCameraStructure(scene->getCamera().getData(aspect_));

			// Загружаем данные фигур и поочередно их отрисовываем
			for (auto i = 0; i < scene->countFigures(); ++i) {
				auto current_figure = scene->getFigure(i + 1);

				mesh_->loadData(current_figure);
				mesh_->loadDisplayType(
					shaderProgram_->getUniformLocation((char*)UNIFORM_DISPLAY_TYPE),
					current_figure.displayType());
				mesh_->loadModelMatrix(
					shaderProgram_->getUniformLocation((char*)UNIFORM_MODEL_MATRIX),
					current_figure.getModelMatrix());
				mesh_->loadNormalMatrix(
					shaderProgram_->getUniformLocation((char*)UNIFORM_NORMAL_MATRIX),
					current_figure.getModelMatrix());

				if (current_figure.displayType() == WIREFRAME_MODEL) {
					// Для каркасной модели необходимы сведения о вершинах и ребрах
					mesh_->loadAspectRatio(
						shaderProgram_->getUniformLocation((char*)UNIFORM_ASPECT_RATIO),
						aspect_ );

					// Размер, цвет и форма отображения вершин
					mesh_->loadVerticesSize(
						shaderProgram_->getUniformLocation((char*)UNIFORM_VERTICES_SIZE),
						current_figure.vertexInfo().size());
					mesh_->loadVerticesColor(
						shaderProgram_->getUniformLocation((char*)UNIFORM_VERTICES_COLOR),
						current_figure.vertexInfo().color());
					mesh_->loadVerticesMode(
						shaderProgram_->getUniformLocation((char*)UNIFORM_VERTICES_SIZE),
						shaderProgram_->getUniformLocation((char*)UNIFORM_VERTICES_MODE),
						current_figure.vertexInfo().mode());

					// Размер, цвет и способ отображения ребер
					mesh_->loadEdgesSize(
						shaderProgram_->getUniformLocation((char*)UNIFORM_EDGES_SIZE),
						current_figure.edgeInfo().size());
					mesh_->loadEdgesColor(
						shaderProgram_->getUniformLocation((char*)UNIFORM_EDGES_COLOR),
						current_figure.edgeInfo().color());
					mesh_->loadEdgesMode(
						shaderProgram_->getUniformLocation((char*)UNIFORM_EDGES_MODE),
						current_figure.edgeInfo().mode());
				}
				else {
					mesh_->loadCountActiveLight(
						shaderProgram_->getUniformLocation((char*)UNIFORM_ACTIVE_LIGHTS),
						scene->countLights());

					mesh_->loadLightStructure(scene->getSceneLightsData());
					mesh_->loadMaterialStructure(current_figure.material());
				}
				mesh_->render();
			}
		}

		else {
			std::cout << "[GLWidget] Инструменты OpenGL недоступны!" << std::endl;
		}

		doneCurrent();
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
