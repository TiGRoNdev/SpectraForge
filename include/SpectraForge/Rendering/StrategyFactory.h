/**
 * @file StrategyFactory.h
 * @brief Фабрика стратегий рендеринга по имени
 */

#pragma once

#include <memory>
#include <string>
#include "SpectraForge/Rendering/ModernRenderer3D.h"

namespace SpectraForge::Rendering {

/**
 * @brief Создать стратегию рендеринга по строковому имени
 * @param name имя стратегии (например, "freqvox")
 * @return shared_ptr стратегии или nullptr, если не найдена
 */
std::shared_ptr<IRenderStrategy> create_strategy_by_name(const std::string& name);

}  // namespace SpectraForge::Rendering


