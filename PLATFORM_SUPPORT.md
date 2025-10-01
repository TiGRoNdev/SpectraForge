# 🌐 Платформенная поддержка HyperEngine

> **Обновлено:** 1 октября 2025

## 📊 Матрица поддержки SDK по платформам

| SDK | Windows | Linux | macOS | Примечания |
|-----|---------|-------|-------|------------|
| **Vulkan** | ✅ Полная | ✅ Полная | ✅ Полная | Основной рендерер |
| **OpenGL** | ✅ Полная | ✅ Полная | ✅ Полная | Резервный рендерер |
| **CUDA** | ✅ Полная | ✅ Полная | ❌ Не поддерживается | Требует NVIDIA GPU |
| **OptiX** | ✅ Полная | ✅ Полная | ❌ Не поддерживается | Требует NVIDIA GPU |
| **DLSS (Streamline)** | ✅ Полная | ❌ Не поддерживается | ❌ Не поддерживается | **Только Windows!** |
| **FSR** | ✅ Полная | ✅ Полная | ✅ Полная | Универсальная альтернатива DLSS |

## ⚠️ Критически важная информация

### DLSS на Linux

**NVIDIA Streamline SDK (DLSS) официально не поддерживает Linux.**

Причины:
- Streamline SDK разработан только для Windows (DirectX 11/12 и Vulkan на Windows)
- Библиотеки доступны только в формате `.lib` и `.dll` (Windows)
- Нет Linux версии библиотек (`.so`)

**Решение для Linux:** Используйте **FSR (FidelityFX Super Resolution)** от AMD:
- ✅ Полностью кросс-платформенный
- ✅ Работает на любом GPU (NVIDIA, AMD, Intel)
- ✅ Хорошее качество апскейлинга
- ✅ Открытый исходный код

## 🔧 Автоматическое определение платформы

HyperEngine автоматически определяет платформу и отключает неподдерживаемые SDK:

```bash
# Linux - DLSS автоматически отключается
cmake -B build -G Ninja \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=ON \
  -DBUILD_WITH_DLSS=ON \   # Будет отключено с предупреждением
  -DBUILD_WITH_FSR=ON      # Рекомендуется для Linux

# Вывод CMake:
# Warning: DLSS (Streamline SDK) officially supports only Windows.
# Warning: For Linux, consider using FSR (FidelityFX Super Resolution) instead.
# Warning: DLSS support will be disabled.
```

## 📚 Рекомендуемые конфигурации

### Windows с NVIDIA RTX

```bash
# Полная поддержка всех функций
cmake -B build -G "Visual Studio 17 2022" -A x64 \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=ON \
  -DBUILD_WITH_DLSS=ON \
  -DBUILD_WITH_FSR=ON
```

### Linux с NVIDIA GPU

```bash
# Используйте скрипт автоматической сборки
./build_linux.sh Release

# Или вручную с FSR вместо DLSS
cmake -B build -G Ninja \
  -DBUILD_WITH_CUDA=ON \
  -DBUILD_WITH_OPTIX=ON \
  -DBUILD_WITH_DLSS=OFF \
  -DBUILD_WITH_FSR=ON
```

### Linux/Windows с AMD/Intel GPU

```bash
cmake -B build -G Ninja \
  -DBUILD_WITH_CUDA=OFF \
  -DBUILD_WITH_OPTIX=OFF \
  -DBUILD_WITH_DLSS=OFF \
  -DBUILD_WITH_FSR=ON
```

## 🔗 Дополнительные ресурсы

- [BUILD_LINUX.md](BUILD_LINUX.md) - Подробное руководство по сборке на Linux
- [SDK_INSTALLATION_ISSUES.md](SDK_INSTALLATION_ISSUES.md) - Решение проблем с SDK
- [DEPENDENCIES.md](DEPENDENCIES.md) - Полный список зависимостей

## 📝 История изменений

### v1.0.0 (2025-10-01)
- ✅ Добавлено автоматическое определение платформы
- ✅ DLSS автоматически отключается на Linux
- ✅ Улучшена поддержка FSR как универсальной альтернативы
- ✅ Добавлен скрипт автоматической сборки для Linux (`build_linux.sh`)
- ✅ Обновлены CMake модули для лучшей кросс-платформенной поддержки

---

**Поддержка:** Создайте issue на [GitHub](https://github.com/TiGRoNdev/HyperEngine/issues) если у вас возникли проблемы с платформенной поддержкой.

