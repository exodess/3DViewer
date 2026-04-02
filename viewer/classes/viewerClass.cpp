#include <viewer/viewerSpec.h>

// Имплементация основного класса программы - Viewer

namespace s21 {

Viewer::Viewer(BaseFileReader* reader, BaseDrawerScene* drawer) noexcept 
	: reader_{reader}, 
	  drawer_{drawer}, 
	  scene_{new Scene(Figure())}
{

	std::cout << "[Viewer] Инициализация Viewer завершена\n";

}

Viewer::~Viewer() {

	std::cout << "[Viewer] Удаление Viewer\n";

	if(reader_) {
		delete reader_;
		reader_ = nullptr;
	}
	
	if(drawer_) {
		delete drawer_;
		drawer_ = nullptr;
	}

	if(scene_) {
		delete scene_;
		scene_ = nullptr;	
	}
}

ViewerOperationResult Viewer::DrawScene() {
	if(drawer_) {
		drawer_->DrawScene(*scene_);
		return ViewerOperationResult(true);
	}
	else {
		std::cout << "[Viewer] Ошибка: Drawer is null\n";
		return ViewerOperationResult(false, error_scene_);
	}
}

ViewerOperationResult Viewer::LoadScene(std::string path, NormalizationParameters param) {
	if(reader_) {
		if(scene_) {
			delete scene_;
			scene_ = nullptr;
		}
		
		scene_ = reader_->ReadScene(path, param);
		std::cout << "[Viewer] Сцена загружена из: " << path << "\n";
		
		return ViewerOperationResult(true);
	}

	std::cout << "[Viewer] Ошибка: Reader is null\n";
	return ViewerOperationResult(false, error_reader_);
}

} // namespace s21
