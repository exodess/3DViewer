#include <viewer/viewerSpec.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <charconv>

namespace s21 {

Scene* FileReader::ReadScene(std::string path, NormalizationParameters param) {

	std::cout << "[FileReader] Считывание модели из файла " << path << std::endl;

	auto vertex_prefix = "v";
	auto surface_prefix = "f";

	std::string line;
	std::string word;

	std::vector<Vertex> vertices;
	std::vector<Edge> edges;

	vertices.clear();
	edges.clear();

	std::ifstream file(path, std::ios::in);

	if(!file) {
		std::cerr << "file \"" << path << "\" not found!" << std::endl;
	}

	std::cout << "\tПараметры нормализации:\n\t[ min = " << param.min << ", max = " << param.max;
	std::cout << ", dxStep = " << param.dxStep << ", dyStep = " << param.dyStep << " ]" <<  std::endl;

	while(file) {
		getline(file, line);
		std::stringstream ss(line);

		ss >> word;
		float arr[3]{0.0f};
		unsigned int list_ind[3]{0};
		if(word == vertex_prefix) {
			// считываем координаты вершины

			for(int i = 0; i < 3; ++i) {
				float value = 0.0f;

				if(ss >> word) {
					std::from_chars(word.data(), word.data() + word.size(), value);

				}

				arr[i] = value;

			}
			// добавляем вычисленную вершину в список
			vertices.push_back(Vertex(Point3D(arr[0], arr[1], arr[2])));
		}

		else if(word == surface_prefix) {

			for(int i = 0; i < 3; ++i) {
				std::string temp;
				unsigned int value = 0;

				if(ss >> word) {
					std::getline(std::stringstream(word), temp, '/');
					std::from_chars(temp.data(), temp.data() + word.size(), value);

				}

				if(value > 0) value --;
				list_ind[i] = value;
			}
			edges.push_back( Edge(list_ind[0], list_ind[1]) );
			edges.push_back( Edge(list_ind[1], list_ind[2]) );
			edges.push_back( Edge(list_ind[2], list_ind[0]) );
		}
	}

	Normalize(vertices);

	std::cout << "\tСчитано " << vertices.size() << " вершин, " << edges.size() << " ребер" << std::endl;
	return new Scene(Figure(vertices, edges));
	
}

void FileReader::Normalize(std::vector<Vertex>& vertices) {
    if (vertices.empty()) return;

    // Шаг 1: Находим min и max по всем осям
    float minX = vertices[0].getPosition().x, maxX = vertices[0].getPosition().x;
    float minY = vertices[0].getPosition().y, maxY = vertices[0].getPosition().y;
    float minZ = vertices[0].getPosition().z, maxZ = vertices[0].getPosition().z;

    for (const auto& v : vertices) {
        if (v.getPosition().x < minX) minX = v.getPosition().x; if (v.getPosition().x > maxX) maxX = v.getPosition().x;
        if (v.getPosition().y < minY) minY = v.getPosition().y; if (v.getPosition().y > maxY) maxY = v.getPosition().y;
        if (v.getPosition().z < minZ) minZ = v.getPosition().z; if (v.getPosition().z > maxZ) maxZ = v.getPosition().z;
    }

    // Шаг 2: Находим центр и максимальный размах (диапазон)
    float centerX = (minX + maxX) / 2.0f;
    float centerY = (minY + maxY) / 2.0f;
    float centerZ = (minZ + maxZ) / 2.0f;

    float diffX = maxX - minX;
    float diffY = maxY - minY;
    float diffZ = maxZ - minZ;
    
    // Выбираем самый большой размер, чтобы сохранить пропорции (не сплющить модель)
    float maxDiff = std::max(diffX, diffY);
    maxDiff = std::max(maxDiff, diffZ);
    float scale = (maxDiff != 0) ? (2.0f / maxDiff) : 1.0f;

    // Шаг 3: Применяем трансформацию к каждой вершине
    for (auto& v : vertices) {
        v.getPosition().x = (v.getPosition().x - centerX) * scale;
        v.getPosition().y = (v.getPosition().y - centerY) * scale;
        v.getPosition().z = (v.getPosition().z - centerZ) * scale;
    }
}

};
