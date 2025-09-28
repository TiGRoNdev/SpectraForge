/**
 * @name Утечки графических ресурсов
 * @description Обнаружение потенциальных утечек графических ресурсов (Vulkan, OpenGL)
 * @kind path-problem
 * @problem.severity error
 * @security-severity 6.0
 * @precision medium
 * @id hyperengine/resource-leaks
 * @tags reliability
 *       performance
 *       external/cwe/cwe-404
 */

import cpp
import semmle.code.cpp.dataflow.TaintTracking

/**
 * Функции создания графических ресурсов
 */
class ResourceCreateFunction extends Function {
  ResourceCreateFunction() {
    this.getName().regexpMatch("(vk|gl|cuda).*[Cc]reate.*") or
    this.getName().regexpMatch("(vk|gl|cuda).*[Aa]llocate.*") or
    this.getName() in [
      "glGenBuffers", "glGenTextures", "glGenVertexArrays",
      "glCreateProgram", "glCreateShader",
      "cudaMalloc", "cudaMallocManaged"
    ]
  }
}

/**
 * Функции освобождения графических ресурсов
 */
class ResourceDestroyFunction extends Function {
  ResourceDestroyFunction() {
    this.getName().regexpMatch("(vk|gl|cuda).*[Dd]estroy.*") or
    this.getName().regexpMatch("(vk|gl|cuda).*[Ff]ree.*") or
    this.getName() in [
      "glDeleteBuffers", "glDeleteTextures", "glDeleteVertexArrays",
      "glDeleteProgram", "glDeleteShader",
      "cudaFree"
    ]
  }
}

/**
 * Класс для отслеживания потока данных от создания до освобождения ресурсов
 */
class ResourceFlow extends TaintTracking::Configuration {
  ResourceFlow() { this = "ResourceFlow" }

  override predicate isSource(DataFlow::Node source) {
    exists(FunctionCall call | 
      call.getTarget() instanceof ResourceCreateFunction and
      source.asExpr() = call
    )
  }

  override predicate isSink(DataFlow::Node sink) {
    exists(FunctionCall call |
      call.getTarget() instanceof ResourceDestroyFunction and
      sink.asExpr() = call.getAnArgument()
    )
  }
}

from 
  FunctionCall create, 
  ResourceCreateFunction createFunc,
  Function containingFunc
where
  create.getTarget() = createFunc and
  containingFunc = create.getEnclosingFunction() and
  // Проверяем, что нет соответствующего вызова освобождения
  not exists(ResourceFlow flow, DataFlow::Node source, DataFlow::Node sink |
    flow.hasFlow(source, sink) and
    source.asExpr() = create
  ) and
  // Исключаем деструкторы (там освобождение может быть автоматическим)
  not containingFunc instanceof Destructor and
  // Исключаем тестовые файлы
  not create.getFile().getAbsolutePath().matches("%test%")
select create, create, create,
  "Потенциальная утечка графического ресурса: " + createFunc.getName() + 
  " без соответствующего освобождения."
