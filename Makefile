NAME_PROJECT := 3DViewer
BUILD_DIR := build

all : install run

install:
	cmake -S . -B $(BUILD_DIR)
	cmake --build $(BUILD_DIR) --target $(NAME_PROJECT)

run : 

# создание документации
dvi :
	doxygen Doxyfile
	@echo -e "\nДля просмотра отчета откройте файл ./docs/html/index.html"

# Создание архива с программой
dist :
	mkdir -p $(NAME_PROJECT)
	cp -r viewer include gui materials $(NAME_PROJECT)/.
	cp Makefile CMakeLists.txt $(NAME_PROJECT)/.
	tar -czf Team_CPP4_$(NAME_PROJECT)_v2.0.tar.gz $(NAME_PROJECT)
	rm -rf $(NAME_PROJECT)

# удаление приложения
uninstall : clean
	rm -rf $(BUILD_DIR)
	
clean :
	rm -rf *.o *.out .clang-format *.gz 
	rm -rf docs
