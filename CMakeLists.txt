cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(DivaFTouchEmu)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wl,-q -fno-rtti -fno-exceptions -fpermissive")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

if (NOT ${RELEASE})
  add_definitions(-DENABLE_LOGGING)
endif()

add_executable(divaf-touch-emu
  divaf-touch-emu.c
)

target_link_libraries(divaf-touch-emu
  taihen_stub
  SceLibKernel_stub
  SceIofilemgr_stub
  SceCtrl_stub
)

set_target_properties(divaf-touch-emu
  PROPERTIES LINK_FLAGS "-nostartfiles"
)

vita_create_self(divaf-touch-emu.suprx divaf-touch-emu
  CONFIG ${CMAKE_SOURCE_DIR}/divaf-touch-emu.yml
)
