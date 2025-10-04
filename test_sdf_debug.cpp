/**
 * SDF Algorithm Manual Test
 * Проверка halfPlaneDistance и computeTriangleSDF
 */

#include <iostream>
#include <cmath>
#include <algorithm>

struct vec2 {
    float x, y;
    vec2(float x = 0, float y = 0) : x(x), y(y) {}
    
    vec2 operator-(const vec2& o) const { return vec2(x - o.x, y - o.y); }
    vec2 operator+(const vec2& o) const { return vec2(x + o.x, y + o.y); }
    vec2 operator*(float s) const { return vec2(x * s, y * s); }
    vec2 operator/(float s) const { return vec2(x / s, y / s); }
    float dot(const vec2& o) const { return x * o.x + y * o.y; }
    float length() const { return std::sqrt(x * x + y * y); }
    vec2 normalize() const { float len = length(); return (len > 0) ? (*this / len) : vec2(0, 0); }
};

// Реализация halfPlaneDistance из шейдера
float halfPlaneDistance(const vec2& p, const vec2& v0, const vec2& v1) {
    vec2 edge = v1 - v0;
    
    // CRITICAL FIX for SCREEN SPACE (Y grows DOWN):
    // RIGHT perpendicular (clockwise 90°) for screen space CCW triangles
    vec2 normal = vec2(edge.y, -edge.x);
    
    // Normalize to get actual distance
    float edgeLength = edge.length();
    if (edgeLength < 0.0001f) {
        return 10000.0f;  // Degenerate edge
    }
    normal = normal / edgeLength;  // Unit normal
    
    // Li(p) = ni · (p - v0)
    // INSIDE => Li < 0, OUTSIDE => Li > 0
    return normal.dot(p - v0);
}

// Реализация computeTriangleSDF из шейдера
float computeTriangleSDF(const vec2& p, const vec2& v0, const vec2& v1, const vec2& v2) {
    float d0 = halfPlaneDistance(p, v0, v1);
    float d1 = halfPlaneDistance(p, v1, v2);
    float d2 = halfPlaneDistance(p, v2, v0);
    
    // SDF = max of all half-plane distances
    // phi < 0 => inside triangle
    // phi > 0 => outside triangle
    return std::max({d0, d1, d2});
}

// Compute incenter
vec2 computeIncenter(const vec2& v0, const vec2& v1, const vec2& v2) {
    float a = (v1 - v2).length();
    float b = (v2 - v0).length();
    float c = (v0 - v1).length();
    float perimeter = a + b + c;
    
    if (perimeter < 0.0001f) {
        return (v0 + v1 + v2) * (1.0f / 3.0f);  // Degenerate triangle
    }
    
    return (v0 * a + v1 * b + v2 * c) / perimeter;
}

int main() {
    std::cout << "═══════════════════════════════════════════════════════\n";
    std::cout << "🔬 SDF ALGORITHM MANUAL TEST\n";
    std::cout << "═══════════════════════════════════════════════════════\n\n";
    
    // Тестовый треугольник: большой, покрывающий центр экрана 960x540
    // Центр экрана: (480, 270)
    vec2 v0(100, 100);    // Левый нижний угол
    vec2 v1(860, 100);    // Правый нижний угол
    vec2 v2(480, 440);    // Верхняя точка (центрированная)
    
    std::cout << "📐 Треугольник (pixel coordinates):\n";
    std::cout << "  v0: (" << v0.x << ", " << v0.y << ")\n";
    std::cout << "  v1: (" << v1.x << ", " << v1.y << ")\n";
    std::cout << "  v2: (" << v2.x << ", " << v2.y << ")\n\n";
    
    // Проверка CCW порядка вершин
    vec2 e0 = v1 - v0;
    vec2 e1 = v2 - v0;
    float cross = e0.x * e1.y - e0.y * e1.x;
    std::cout << "🔄 Ориентация треугольника: " 
              << (cross > 0 ? "CCW (counter-clockwise) ✅" : "CW (clockwise) ❌") 
              << " (cross product = " << cross << ")\n\n";
    
    // Тест 1: Центр экрана (должен быть внутри!)
    vec2 screenCenter(480, 270);
    float phiCenter = computeTriangleSDF(screenCenter, v0, v1, v2);
    
    std::cout << "🎯 Тест 1: Центр экрана (" << screenCenter.x << ", " << screenCenter.y << ")\n";
    std::cout << "  SDF φ = " << phiCenter << (phiCenter < 0 ? " ✅ INSIDE" : " ❌ OUTSIDE") << "\n";
    
    // Детальные half-plane distances
    float d0_center = halfPlaneDistance(screenCenter, v0, v1);
    float d1_center = halfPlaneDistance(screenCenter, v1, v2);
    float d2_center = halfPlaneDistance(screenCenter, v2, v0);
    std::cout << "  d0 (v0→v1): " << d0_center << (d0_center < 0 ? " inside" : " outside") << "\n";
    std::cout << "  d1 (v1→v2): " << d1_center << (d1_center < 0 ? " inside" : " outside") << "\n";
    std::cout << "  d2 (v2→v0): " << d2_center << (d2_center < 0 ? " inside" : " outside") << "\n\n";
    
    // Тест 2: Центроид треугольника (всегда внутри!)
    vec2 centroid = (v0 + v1 + v2) / 3.0f;
    float phiCentroid = computeTriangleSDF(centroid, v0, v1, v2);
    
    std::cout << "📊 Тест 2: Центроид треугольника (" << centroid.x << ", " << centroid.y << ")\n";
    std::cout << "  SDF φ = " << phiCentroid << (phiCentroid < 0 ? " ✅ INSIDE" : " ❌ OUTSIDE") << "\n\n";
    
    // Тест 3: Incenter (точка с максимальным вписанным кругом)
    vec2 incenter = computeIncenter(v0, v1, v2);
    float phiIncenter = computeTriangleSDF(incenter, v0, v1, v2);
    
    std::cout << "🎯 Тест 3: Incenter (" << incenter.x << ", " << incenter.y << ")\n";
    std::cout << "  SDF φ = " << phiIncenter << " (должен быть наиболее отрицательным)\n\n";
    
    // Тест 4: Точка за пределами треугольника
    vec2 outside(50, 50);
    float phiOutside = computeTriangleSDF(outside, v0, v1, v2);
    
    std::cout << "❌ Тест 4: Точка снаружи (" << outside.x << ", " << outside.y << ")\n";
    std::cout << "  SDF φ = " << phiOutside << (phiOutside > 0 ? " ✅ OUTSIDE" : " ❌ INSIDE?!") << "\n\n";
    
    std::cout << "═══════════════════════════════════════════════════════\n";
    std::cout << "📋 ИТОГОВЫЙ РЕЗУЛЬТАТ:\n";
    std::cout << "  Центр экрана: " << (phiCenter < 0 ? "✅ PASS" : "❌ FAIL") << "\n";
    std::cout << "  Центроид:     " << (phiCentroid < 0 ? "✅ PASS" : "❌ FAIL") << "\n";
    std::cout << "  Incenter:     " << (phiIncenter < 0 ? "✅ PASS" : "❌ FAIL") << "\n";
    std::cout << "  Точка снаружи: " << (phiOutside > 0 ? "✅ PASS" : "❌ FAIL") << "\n";
    std::cout << "═══════════════════════════════════════════════════════\n";
    
    return 0;
}

