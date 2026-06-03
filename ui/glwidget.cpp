#include "ui/glwidget.h"

namespace viewer {

	static float global_matrix[4][4];

	GLWidget::GLWidget(QWidget* parent)
	    : QOpenGLWidget(parent)
	    , shaderProgram_(nullptr)
	    , mesh_(nullptr)
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
		mesh_->loadAspectRatio(
			shaderProgram_->getAspectRatioUniformLocation(),
			w / h);
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

		if(scene_ != scene) {
			// загружена новая фигура
			scene_ = scene;

			if (mesh_)
				delete mesh_;

			mesh_ = new Mesh();
			mesh_->loadData(scene_->getFigure());
		}

		std::cout << "[GLWidget] Сцена отрисована | Вершин: " << countVertices()
		          << " | Поверхностей: " << countSurfaces() << "\n";

		mesh_->render();
		update();
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
