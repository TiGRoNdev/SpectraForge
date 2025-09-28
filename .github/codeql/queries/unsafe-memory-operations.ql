/**
 * @name Небезопасные операции с памятью
 * @description Обнаружение потенциально небезопасных операций с памятью в HyperEngine
 * @kind problem
 * @problem.severity warning
 * @security-severity 7.0
 * @precision medium
 * @id hyperengine/unsafe-memory-operations
 * @tags security
 *       reliability
 *       external/cwe/cwe-120
 *       external/cwe/cwe-787
 */

import cpp

/**
 * Функции, которые считаются небезопасными для работы с памятью
 */
class UnsafeMemoryFunction extends Function {
  UnsafeMemoryFunction() {
    this.getName() in [
      "strcpy", "strcat", "sprintf", "vsprintf",
      "gets", "scanf", "fscanf", "sscanf",
      "memcpy", // без проверки границ
      "memmove", // без проверки границ
      "strncpy" // может не добавить null terminator
    ]
  }
}

/**
 * Проверяем использование небезопасных функций памяти
 */
from FunctionCall call, UnsafeMemoryFunction func
where 
  call.getTarget() = func and
  // Исключаем тестовые файлы
  not call.getFile().getAbsolutePath().matches("%test%") and
  not call.getFile().getAbsolutePath().matches("%example%")
select call, 
  "Использование небезопасной функции '" + func.getName() + 
  "'. Рассмотрите использование безопасных альтернатив."
