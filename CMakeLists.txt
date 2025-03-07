cmake_minimum_required(VERSION 3.16)

set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)
set(CMAKE_C_LINK_FLAGS "")
set(CMAKE_CXX_LINK_FLAGS "")

include(cmake/os.cmake)
include(cmake/cmsis.cmake)
include(cmake/arm-none-eabi.cmake)

project(firmware C CXX ASM)

add_definitions(-DSTM32F103xB)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 컴파일러 플래그 설정
set(MCU "cortex-m3")
set(CFCOMMON "-mcpu=${MCU} -mthumb --specs=nano.specs --specs=nosys.specs -Wall -fmessage-length=0 -ffunction-sections -fdata-sections")
set(CFDEBUG "-O0 -g")

##
set(LIB_DIR "/Applications/ArmGNUToolchain/14.2.rel1/arm-none-eabi")
set(GCC_LIB_DIR "/Applications/ArmGNUToolchain/14.2.rel1/lib/gcc/arm-none-eabi/14.2.1")

# EXTRA
set(CFLAGS_EXTRA "-fno-builtin -funsigned-char -fno-strict-aliasing -fno-common -pipe")
# set(LDFLAGS_EXTRA " -Bstatic -nostdlib -p -EL")
set(LDFLAGS_EXTRA "-Wl,--gc-sections -Wl,--start-group -lc -lm -lgcc -lrdimon -Wl,--end-group")
set(LIBRARY_PATHS "-L${LIB_DIR}/lib/thumb/v7-m/nofp -L${GCC_LIB_DIR}/thumb/v7-m/nofp")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFDEBUG} ${CFCOMMON} ${CFLAGS_EXTRA} -fno-exceptions")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFDEBUG} ${CFCOMMON} ${CFLAGS_EXTRA}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${CFCOMMON} -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_SOURCE_DIR}/STM32F103VBHX_FLASH.ld ${LIBRARY_PATHS} ${LDFLAGS_EXTRA}")


# 소스 파일 찾기
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "src/*.c" "src/*.cpp" "*.s")

# 실행 파일 생성
add_executable(${PROJECT_NAME}.elf ${SOURCE_FILES})

# 추가 빌드 명령어
add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
    COMMAND ${OBJCOPY} -Obinary "${PROJECT_NAME}.elf" "${PROJECT_NAME}.bin"
    COMMENT "Generating binary file ${PROJECT_NAME}.bin"
)

# add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
#     COMMAND ${OBJCOPY} -Oihex "${PROJECT_NAME}.elf" "${PROJECT_NAME}.hex"
#     COMMENT "Generating hex file ${PROJECT_NAME}.hex"
# )

add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
    COMMAND ${SIZE} --format=berkeley "${PROJECT_NAME}.elf"
    COMMENT "Generating size information for ${PROJECT_NAME}"
)

add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
    COMMAND ${CMAKE_OBJDUMP} -h -S "${PROJECT_NAME}.elf" > "${PROJECT_NAME}.map"
    COMMENT "Generating map file ${PROJECT_NAME}.map"
)