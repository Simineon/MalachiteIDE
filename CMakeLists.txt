cmake_minimum_required(VERSION 3.16)
project(Malachite LANGUAGES CXX)

# Настройки компилятора
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Автоматизация для Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Поиск Qt6 с обработкой ошибок
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

if(Qt6_FOUND)
    message(STATUS "Found Qt6 version: ${Qt6_VERSION}")
    message(STATUS "Qt6 include directories: ${Qt6Core_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Qt6 not found. Please install Qt6 development packages.")
endif()

# Добавляем исходные файлы
add_executable(Malachite 
    main.cpp
    scr/app/app.cpp
    scr/parser/parser.cpp
    scr/text/CustomTextEdit.h
    scr/app/execute/executer.h
    scr/app/execute/executer.cpp
    scr/app/tab/tab.h
    scr/app/tab/tab.cpp
)

# Подключаем Qt компоненты
target_link_libraries(Malachite 
    Qt6::Core
    Qt6::Widgets
)

# Включаем директории для заголовочных файлов
target_include_directories(Malachite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Дополнительные флаги компиляции
target_compile_definitions(Malachite PRIVATE
    QT_CORE_LIB
    QT_WIDGETS_LIB
)

# Настройки для релизной сборки
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(Malachite PRIVATE QT_NO_DEBUG_OUTPUT)
endif()