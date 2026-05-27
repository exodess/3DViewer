#include "viewer/viewerSpec.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <charconv>
#include <random>

namespace viewer {

	Scene* FileReader::ReadScene(std::string path, NormalizationParameters param) {

		std::cout << "[FileReader] Считывание модели из файла " << path << std::endl;

		auto vertex_prefix = "v";
		auto surface_prefix = "f";
		auto normale_prefix = "vn";

		std::string line;
		std::string word;

		std::vector<Vertex> vertices;
		std::vector<Surface> surfaces;
		std::vector<Point3D> normals_coordinates;

		vertices.clear();
		surfaces.clear();

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
				vertices.push_back(Vertex(Point3D(arr[0], arr[1], arr[2]), Point3D()));
			}

			else if(word == surface_prefix) {
				// Считываем индекс вершины и индекс нормали
				// f v1/vt1/vn1 v2/vt2/vn2 ...
				std::vector<long> list_ind;

				while(ss >> word) {
					std::string temp;
					long vert_index = 0;
					long uv_index = 0;
					long norm_index = 0;

					// считываем индекс координаты вершины из списка
					std::getline(std::stringstream(word), temp, '/');
					std::from_chars(temp.data(), temp.data() + word.size(), vert_index);

					std::string str_vt = (word.find('/') != std::string::npos) ? word.substr(word.find('/') + 1) : "0";
					// считываем индекс нужной UV-координаты из списка
					std::getline(std::stringstream(str_vt), temp, '/');
					std::from_chars(temp.data(), temp.data() + temp.size(), uv_index);

					// считываем последнее число - индекс нормали вершины
					std::string str_vn = (str_vt.find('/') != std::string::npos) ? str_vt.substr(str_vt.find('/') + 1) : "0";
					std::from_chars(str_vn.data(), str_vn.data() + str_vn.size(), norm_index);

					if (vert_index < 0) {
						vert_index += vertices.size() + 1;
					}
					list_ind.push_back(vert_index - 1);

					if (norm_index < 0) {
						norm_index += normals_coordinates.size() + 1;
					}
					if (norm_index > 0 && vert_index > 0) {
						vertices[vert_index - 1].setNormals(normals_coordinates[norm_index - 1]);
					}
				}
				if (list_ind.size() == 3) {
					surfaces.push_back(Surface(list_ind[0], list_ind[1], list_ind[2]));
				}
				else {
					surfaces.push_back(Surface(list_ind[0], list_ind[1], list_ind[2]));
					surfaces.push_back(Surface(list_ind[0], list_ind[1], list_ind[3]));
					surfaces.push_back(Surface(list_ind[0], list_ind[2], list_ind[3]));
				}
			}
			else if (word == normale_prefix) {
				// считываем нормали, их всегда 3
				for (int i = 0; i < 3; ++i) {
					float value = 0.0f;
					if(ss >> word) {
						std::from_chars(word.data(), word.data() + word.size(), value);
					}
					arr[i] = value;
				}
				normals_coordinates.push_back(Point3D(arr[0], arr[1], arr[2]));
			}
		}

		Normalize(vertices, surfaces);

		std::cout << "\tСчитано " << vertices.size() << " вершин, ";
		std::cout << surfaces.size() << " поверхностей, ";
		std::cout << normals_coordinates.size() << " нормалей" << std::endl;

		return new Scene(Figure(vertices, surfaces));
	}

	void FileReader::Normalize(std::vector<Vertex>& vertices, std::vector<Surface>& surfaces) {
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

		if (vertices.size()  && vertices[0].getNormale() == Point3D(0.0f, 0.0f, 0.0f)) {
			// Усреднение нормалей, если
			for (auto& s : surfaces) {
				Point3D v1 = vertices[s[0]].getPosition();
				Point3D v2 = vertices[s[1]].getPosition();
				Point3D v3 = vertices[s[2]].getPosition();

				Point3D edge1 = Point3D(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
				Point3D edge2 = Point3D(v3.x - v1.x, v3.y - v1.y, v3.z - v1.z);

				// cross(edge1, edge2) - векторное произведение
				Point3D faceNormal = Point3D(
					edge1.y * edge2.z - edge1.z * edge2.y,
					edge1.z * edge2.x - edge1.x * edge2.z,
					edge1.x * edge2.y - edge1.y * edge2.x);

				vertices[s[0]].getNormale() = vertices[s[0]].getNormale() + faceNormal;
				vertices[s[1]].getNormale() = vertices[s[1]].getNormale() + faceNormal;
				vertices[s[2]].getNormale() = vertices[s[2]].getNormale() + faceNormal;
			}

			for (auto& v : vertices) {
				Point3D normal = v.getNormale();

				float len = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
				normal.x /= len;
				normal.y /= len;
				normal.z /= len;

				v.getNormale() = normal;
			}
		}
	}

};
