cmake_minimum_required(VERSION 3.0.0)
project(motoilet_whisper VERSION 3.0.0)
project(motoilet_whisper C)

set(CMAKE_C_STANDARD 90)

add_library(motoilet_whisper STATIC
                src/main/app_layer.c
                src/main/app_layer.h
                src/main/crc.c
                src/main/crc.h
                src/main/data_layer.c
                src/main/motoilet_whisper.c
                src/main/motoilet_whisper_message.c
                src/main/transmission_layer.c
)
target_include_directories(motoilet_whisper PUBLIC include)

include(CTest)
enable_testing()

add_library(unity STATIC components/Unity/src/unity.c)
target_include_directories(unity PUBLIC components/Unity/src)

# Unit tests for the data layer
add_executable(data_layer_test 
    src/main/crc.c
    src/test/data_layer_test.c
    src/test/driver_mock.c
)
target_include_directories(data_layer_test PRIVATE src/main include)
target_link_libraries(data_layer_test PUBLIC unity)
add_test(data_layer_test, data_layer_test)

# Unit tests for the tranmission layer
add_executable(transmission_layer_test 
    src/test/transmission_layer_test.c
    src/test/data_layer_mock.c
    src/test/driver_mock.c
)
target_include_directories(transmission_layer_test PRIVATE src/main include)
target_link_libraries(transmission_layer_test PUBLIC unity)
add_test(transmission_layer_test, transmission_layer_test)

# Unit tets for the app layer
add_executable(app_layer_test 
    src/main/app_layer.c
    src/test/app_layer_test.c
)
target_include_directories(app_layer_test PRIVATE src/main include)
target_link_libraries(app_layer_test PUBLIC unity)
add_test(app_layer_test, app_layer_test)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
