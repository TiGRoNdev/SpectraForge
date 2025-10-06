#include <gtest/gtest.h>

#include "SpectraForge/Rendering/MeshConnectivity.hpp"

using namespace SpectraForge;

TEST(Phase1_Vertex, EqualityAndHash) {
    Rendering::TriangleSplattingVertex a(Math::Vector3(0.0f, 0.0f, 0.0f), Math::Vector3(1.0f, 0.0f, 0.0f), 1.0f);
    Rendering::TriangleSplattingVertex b(Math::Vector3(1e-7f, 0.0f, 0.0f), Math::Vector3(1.0f, 0.0f, 0.0f), 1.0f);

    EXPECT_TRUE(a == b);
    EXPECT_EQ(a.hash(), b.hash());

    a.add_adjacent_triangle(0);
    a.add_adjacent_triangle(0);  // не должен дублироваться
    EXPECT_EQ(a.adjacent_triangles.size(), 1u);

    a.add_adjacent_vertex(2);
    a.add_adjacent_vertex(2);  // не должен дублироваться
    EXPECT_EQ(a.adjacent_vertices.size(), 1u);
}

TEST(Phase1_Deduplicator, AddOrGet) {
    Rendering::VertexDeduplicator dedup;
    Rendering::TriangleSplattingVertex v0(Math::Vector3(0.0f, 0.0f, 0.0f), Math::Vector3(0.0f, 1.0f, 0.0f), 0.5f);
    Rendering::TriangleSplattingVertex v1(Math::Vector3(0.0f, 0.0f, 0.0f), Math::Vector3(0.0f, 1.0f, 0.0f), 0.5f);

    auto i0 = dedup.add_or_get_vertex(v0);
    auto i1 = dedup.add_or_get_vertex(v1);
    EXPECT_EQ(i0, i1);
    EXPECT_EQ(dedup.get_vertex_count(), 1u);
}

TEST(Phase1_Triangle, BarycentricAndInterpolation) {
    // Создаём простой треугольник
    Rendering::TriangleSplattingVertex v0(Math::Vector3(0.0f, 0.0f, 0.0f), Math::Vector3(1.0f, 0.0f, 0.0f), 1.0f);
    Rendering::TriangleSplattingVertex v1(Math::Vector3(1.0f, 0.0f, 0.0f), Math::Vector3(0.0f, 1.0f, 0.0f), 1.0f);
    Rendering::TriangleSplattingVertex v2(Math::Vector3(0.0f, 1.0f, 0.0f), Math::Vector3(0.0f, 0.0f, 1.0f), 1.0f);

    std::vector<Rendering::TriangleSplattingVertex> verts = {v0, v1, v2};

    Rendering::ConnectedTriangle tri(0, 1, 2);
    tri.compute_geometry(verts);
    EXPECT_GT(tri.area, 0.0f);

    // Центр треугольника
    Math::Vector3 center = (verts[0].position + verts[1].position + verts[2].position) / 3.0f;
    Math::Vector3 bc = tri.compute_barycentric(center, verts);
    float sum = bc.x + bc.y + bc.z;
    EXPECT_NEAR(sum, 1.0f, 1e-5f);
    EXPECT_GE(bc.x, 0.0f);
    EXPECT_GE(bc.y, 0.0f);
    EXPECT_GE(bc.z, 0.0f);

    Math::Vector3 col = tri.interpolate_color(bc, verts);
    // Цвет должен быть усреднением вершин
    Math::Vector3 expected = (verts[0].color + verts[1].color + verts[2].color) / 3.0f;
    EXPECT_NEAR(col.x, expected.x, 1e-5f);
    EXPECT_NEAR(col.y, expected.y, 1e-5f);
    EXPECT_NEAR(col.z, expected.z, 1e-5f);
}


