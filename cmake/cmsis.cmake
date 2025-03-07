if (NOT EXISTS "${CMAKE_SOURCE_DIR}/Drivers/CMSIS")
  message(FATAL_ERROR "CMSIS files not found.")
endif()

include_directories("Drivers/CMSIS/include")
include_directories("Drivers/CMSIS/Device/ST/STM32F1xx/Include")