cmake_minimum_required(VERSION 3.16)

set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)
set(CMAKE_C_LINK_FLAGS "")
set(CMAKE_CXX_LINK_FLAGS "")

set(CMSIS_DSP_PATH ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/DSP)

include(cmake/os.cmake)
include(cmake/cmsis.cmake)
include(cmake/arm-none-eabi.cmake)
include_directories(${CMSIS_DSP_PATH}/Include)

project(firmware C CXX ASM)

add_definitions(-DSTM32F103xB)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 컴파일러 플래그 설정
set(MCU "cortex-m3")
set(CFCOMMON "-mcpu=${MCU} -mthumb -mfloat-abi=soft --specs=nano.specs --specs=nosys.specs --specs=rdimon.specs -Wall -fmessage-length=0 -ffunction-sections -fdata-sections")
set(CFDEBUG "-O0 -g")

set(LIB_DIR "/Applications/ArmGNUToolchain/14.2.rel1/arm-none-eabi")
set(GCC_LIB_DIR "/Applications/ArmGNUToolchain/14.2.rel1/lib/gcc/arm-none-eabi/14.2.1")

# EXTRA
set(CFLAGS_EXTRA "-fno-builtin -funsigned-char -fno-strict-aliasing -fno-common -pipe -mapcs -mno-thumb-interwork ")
set(LDFLAGS_EXTRA "-Wl,--gc-sections -Wl,--start-group -lc -lm -lgcc -lstdc++ -lrdimon -Wl,--end-group")
set(LIBRARY_PATHS "-L${LIB_DIR}/lib/thumb/v7-m/nofp -L${GCC_LIB_DIR}/thumb/v7-m/nofp")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFDEBUG} ${CFCOMMON} ${CFLAGS_EXTRA} -fno-exceptions")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFDEBUG} ${CFCOMMON} ${CFLAGS_EXTRA}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${CFCOMMON} -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_SOURCE_DIR}/STM32F103C8TX_FLASH.ld ${LIBRARY_PATHS} ${LDFLAGS_EXTRA}")


# # CMSIS-DSP 라이브러리 경로 설정

# # CMSIS-DSP의 Include 디렉토리 추가
# include_directories(${CMSIS_DSP_PATH}/Include)

# # CMSIS-DSP의 Source 디렉토리 추가
# add_subdirectory(${CMSIS_DSP_PATH}/Source)

# 소스 파일들 추가
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "src/*.c" "src/*.cpp" "src/source/*.c" "src/source/*.cpp" "*.s")

# Assuming the library is located in the `Drivers/CMSIS/DSP/Source` folder or a subdirectory

# 실행 파일 생성
add_executable(${PROJECT_NAME}.elf ${SOURCE_FILES})

set(CMSISDSP_LIB_DIR "${CMAKE_SOURCE_DIR}/Drivers/CMSIS/DSP/Source")
target_link_libraries(${PROJECT_NAME}.elf PRIVATE ${CMSISDSP_LIB_DIR}/libCMSISDSP.a)

# 추가 빌드 명령어
# Set the path to the bootloader binary file
set(BOOTLOADER_PATH "${CMAKE_SOURCE_DIR}/bootloader/M3_mini_IAP_bootloader.bin")

# Add the bootloader section to the ELF file using the linker script
# add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
#     COMMAND ${OBJCOPY} --update-section .bootloader="${BOOTLOADER_PATH}" "${PROJECT_NAME}.elf"
#     COMMENT "Placing bootloader at 0x08000000 in the ELF file"
# )

# Continue the existing custom commands (generating binary, hex, map, etc.)
add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
    COMMAND ${OBJCOPY} -Obinary "${PROJECT_NAME}.elf" "${PROJECT_NAME}.bin"
    COMMENT "Generating binary file ${PROJECT_NAME}.bin"
)

add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
    COMMAND ${SIZE} --format=berkeley "${PROJECT_NAME}.elf"
    COMMENT "Generating size information for ${PROJECT_NAME}"
)

add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -h -S "${PROJECT_NAME}.elf" > "${PROJECT_NAME}.map"
    COMMENT "Generating map file ${PROJECT_NAME}.map"
)

add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -x -D "${PROJECT_NAME}.elf" > "${PROJECT_NAME}__dump.txt"
    COMMENT "Generating __dump.txt"
)

add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -x -D -S "${PROJECT_NAME}.elf" > "${PROJECT_NAME}__dump_all.txt"
    COMMENT "Generating __dump_all.txt"
)