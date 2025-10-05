/**
 * @name Неправильное использование графических API
 * @description Обнаружение распространенных ошибок при использовании Vulkan и OpenGL API
 * @kind problem
 * @problem.severity warning
 * @security-severity 5.0
 * @precision high
 * @id hyperengine/graphics-api-misuse
 * @tags reliability
 *       performance
 *       maintainability
 */

import cpp

/**
 * Vulkan функции, требующие проверки результата
 */
class VulkanResultFunction extends Function {
  VulkanResultFunction() {
    this.getName().regexpMatch("vk[A-Z].*") and
    this.getType().(Type).getName() = "VkResult"
  }
}

/**
 * OpenGL функции, после которых нужно проверять ошибки
 */
class OpenGLFunction extends Function {
  OpenGLFunction() {
    this.getName().regexpMatch("gl[A-Z].*") and
    not this.getName() in ["glGetError", "glGetString"]
  }
}

/**
 * Проверка на игнорирование результата Vulkan функций
 */
from FunctionCall call, VulkanResultFunction func
where
  call.getTarget() = func and
  // Результат не проверяется
  not exists(VariableDeclarationEntry vde | vde.getInitializer() = call) and
  not exists(AssignExpr assign | assign.getRValue() = call) and
  not exists(EqualityOperation eq | eq.getAnOperand() = call) and
  not exists(ComparisonOperation comp | comp.getAnOperand() = call) and
  // Исключаем тестовые файлы
  not call.getFile().getAbsolutePath().matches("%test%")
select call, 
  "Результат Vulkan функции '" + func.getName() + 
  "' не проверяется. Это может привести к необнаруженным ошибкам."

/**
 * Проверка на отсутствие glGetError после OpenGL вызовов
 */
from FunctionCall glCall, OpenGLFunction glFunc, BlockStmt block
where
  glCall.getTarget() = glFunc and
  block = glCall.getEnclosingStmt().getParent*() and
  // В том же блоке нет вызова glGetError
  not exists(FunctionCall errorCheck, Function getError |
    getError.getName() = "glGetError" and
    errorCheck.getTarget() = getError and
    errorCheck.getEnclosingStmt().getParent*() = block and
    // glGetError идет после нашего вызова
    errorCheck.getLocation().getStartLine() > glCall.getLocation().getStartLine()
  ) and
  // Исключаем тестовые файлы и функции настройки
  not glCall.getFile().getAbsolutePath().matches("%test%") and
  not glCall.getEnclosingFunction().getName().regexpMatch(".*[Ss]etup.*")
select glCall,
  "OpenGL функция '" + glFunc.getName() + 
  "' вызывается без последующей проверки ошибок через glGetError()."
