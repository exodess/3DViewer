#include "viewer/graphicSpec.h"
#include <glad/glad.h>

// В этом файле представлена реализация BO и VAO


namespace viewer {

	// Счетчики использований дескрипторов
	std::map<GLuint, GLuint> VAO::handler_count;
	std::map<GLuint, GLuint> BO::handler_count;

	// ==============================================
	// ================== КЛАСС BO =================
	// ==============================================


	// создание пустого буфера
	BO::BO(GLenum type) noexcept : type_{type}  {

		// создание буфера необходимого типа
		glGenBuffers(1, &boID_);
		use();

		handler_count[boID_] = 1;

	}

	// Создание буфера с загрузкой в него данных
	BO::BO(GLenum type, const void* mem, int size) noexcept : BO(type) {

		// загружаем туда данные
		load(mem, size, GL_STATIC_DRAW);

	}

	BO::BO(const BO& other) noexcept : boID_{other.boID_}, type_{other.type_} {

		handler_count[boID_]++; // теперь этим дескриптором пользуется еще один буфер

	}

	// Удаление буфера
	BO::~BO() {
		if(boID_) {
			// если буфер был создан и никем не используется, удаляем его
			if(!--handler_count[boID_]) {
				glDeleteBuffers(1, &boID_);
				handler_count.erase(boID_);
			}
			boID_ = 0;
		}

	}

	BO& BO::operator=(const BO& other) noexcept {

		// если буферы используют разные дескрипторы
		if(boID_ != other.boID_) {

				this->~BO(); // удаляем текущий буфер
				boID_ = other.boID_;

				handler_count[boID_] ++;
		}

		type_ = other.type_; // все равно меняем тип буфера

		return *this;

	}

	// загрузка данных в буфер
	void BO::load(const void* mem, int size, GLuint mode) noexcept{
		use();
		// mode - указание для драйвера, как лучше использовать буфер
		glBufferData(type_, size, mem, mode);

	}

	void BO::setAttrib(std::size_t stride, void* offset, int index) noexcept {
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(
	        index,
	        3,
	        GL_FLOAT,
	        GL_FALSE,
	        stride,
	        offset
		);
	}

	// Привязка элементного буфера
	void BO::use() noexcept {

		glBindBuffer(type_, boID_);
	}

	// ==============================================
	// ================== КЛАСС UBO =================
	// ==============================================

	UBO::UBO(int size, int binding) noexcept : BO(GL_UNIFORM_BUFFER, 0, size) {
		rebind(binding);
	}

	UBO::UBO(const void* data, int size, int binding) noexcept : BO(GL_UNIFORM_BUFFER, data, size) {
		rebind(binding);
	}

	void UBO::rebind(int binding) noexcept {
		glBindBufferBase(type_, binding, boID_);
	}

	void UBO::loadSub(const void* mem, int size, int offset) noexcept {
		use();
		glBufferSubData(type_, offset, size, mem);
	}

	// ==============================================
	// ================== КЛАСС VAO =================
	// ==============================================


	VAO::VAO() noexcept {

		// создаем VAO
		glGenVertexArrays(1, &vaoID_);
		glBindVertexArray(vaoID_);

		handler_count[vaoID_] = 1;

	}

	VAO::~VAO() {

		if(vaoID_) {
			// если буфер создан и никем не используется, удалим его
			if(!--handler_count[vaoID_]) {
				glDeleteVertexArrays(1, &vaoID_);
				handler_count.erase(vaoID_);
			}
			vaoID_ = 0;
		}

	}

	// Создаем копию VAO
	VAO::VAO(const VAO& copy) noexcept : vaoID_{copy.vaoID_} {

		// увеличиваем счетчик использования
		handler_count[vaoID_] ++;

	}

	VAO& VAO::operator=(const VAO& other) noexcept {

		if(vaoID_ != other.vaoID_) {

			this->~VAO(); // удаляем текущий объект

			vaoID_ = other.vaoID_;
			handler_count[vaoID_] ++;

		}

		return *this;

	}

	void VAO::use() noexcept {
		glBindVertexArray(vaoID_);
	}

	void VAO::disable() noexcept {
		glBindVertexArray(0); // отключение VAO
	}

}
