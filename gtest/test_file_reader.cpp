#include <gtest/gtest.h>
#include "viewer/viewerSpec.h"

#ifndef TEST_SHAPES_DATA_PATH
#define TEST_SHAPES_DATA_PATH "."
#endif

TEST(ViewerClasses, FileReader_without_normales_Test) {

    std::string file1 = std::string(TEST_SHAPES_DATA_PATH) + "test1.vobj";
    s21::FileReader reader;

    // После нормализации модель центрируется и масштабируется
    // Проверяем только количество вершин и рёбер
    s21::Scene* result = reader.ReadScene(file1, {0.0f, 0.0f, 0.0f, 0.0f});

    auto vertices = result->getFigure().getVertices();
    auto edges = result->getFigure().getEdges();

    // test1.vobj содержит 5 вершин (после парсинга)
    EXPECT_EQ(vertices.size(), 5);

    // test1.vobj содержит 18 рёбер (6 граней * 3 ребра = 18, или меньше если есть дубликаты)
    EXPECT_EQ(edges.size(), 18);

    // Проверяем что вершины не пустые и находятся в разумных пределах после нормализации
    for(size_t i = 0; i < vertices.size(); ++i) {
        EXPECT_TRUE(vertices[i].getPosition().x >= -2.0f && vertices[i].getPosition().x <= 2.0f);
        EXPECT_TRUE(vertices[i].getPosition().y >= -2.0f && vertices[i].getPosition().y <= 2.0f);
        EXPECT_TRUE(vertices[i].getPosition().z >= -2.0f && vertices[i].getPosition().z <= 2.0f);
    }
}

TEST(ViewerClasses, FileReader_with_normales_Test) {
    std::string file1 = std::string(TEST_SHAPES_DATA_PATH) + "test2.vobj";
    s21::FileReader reader;

    s21::Scene* result = reader.ReadScene(file1, {0.0f, 0.0f, 0.0f, 0.0f});

    auto vertices = result->getFigure().getVertices();
    auto edges = result->getFigure().getEdges();

    ASSERT_EQ(vertices.size(), 6);
    ASSERT_EQ(edges.size(), 6);

    auto file_normales = std::vector<s21::Point3D> {
        s21::Point3D(0.0, 0.0, 0.0),
        s21::Point3D(1.0, 0.1, 0.3),
        s21::Point3D(-0.45, 0.987, -0.123),
        s21::Point3D(-1.0, -0.567, 0.333)
    };

    auto indices_normales = std::vector<uint32_t> {
        0, 0,
        0, 3,
        0, 3,
        1, 2,
        1, 0,
        2, 0
    };

    for (auto v : vertices) {
        EXPECT_TRUE(v.getPosition().x >= -2.0f && v.getPosition().x <= 2.0f);
        EXPECT_TRUE(v.getPosition().y >= -2.0f && v.getPosition().y <= 2.0f);
        EXPECT_TRUE(v.getPosition().z >= -2.0f && v.getPosition().z <= 2.0f);
    }

    int i = 0;
    for (auto e : edges) {
        EXPECT_TRUE(vertices[e.getBegin() - 1].getNormale() == file_normales[indices_normales[i]]);
        i++;
        EXPECT_TRUE(vertices[e.getEnd() - 1].getNormale() == file_normales[indices_normales[i]]);
        i++;
    }
}