cmake_minimum_required(VERSION 2.6)

project(sickle)

if(CMAKE_COMPILER_IS_GNUCXX)
   SET(ENABLE_CXX11 "-std=c++11")

   EXECUTE_PROCESS(COMMAND "${CMAKE_CXX_COMPILER} -dumpversion" OUTPUT_VARIABLE GCC_VERSION)
   if (GCC_VERSION VERSION_LESS 4.7)
      SET(ENABLE_CXX11 "-std=c++0x")
   endif()

   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ENABLE_CXX11}")
endif()

find_package(OpenGL REQUIRED)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIR})

set(SRC_MAPDATA
    mapdata/mapdocument.cpp
    mapdata/maploader.cpp
    mapdata/texturepack.cpp
    mapdata/tokenizer.cpp
    mapdata/texture.cpp
    )

set(SRC_MAPDATA_TESTS
    mapdata/mapdocument-tests.cpp
    glextl.cpp
    )

set(SRC_APP
    program.cpp
    glextl.cpp
    shader.h
    log.h
    font.h
    iconset.h
    )

add_executable(sickle ${SRC_APP} ${SRC_MAPDATA})

target_link_libraries(sickle
    ${SDL2_LIBRARY}
    Ole32 Oleaut32 Imm32 Winmm Version stdc++
    ${OPENGL_LIBRARIES}
)

add_executable(mapdata-tests ${SRC_MAPDATA} ${SRC_MAPDATA_TESTS})

target_link_libraries(mapdata-tests
    Ole32 Oleaut32 Imm32 Winmm Version stdc++
)

enable_testing ()
add_test (NAME MyTest COMMAND mapdata-tests)
