cmake_minimum_required(VERSION 3.26)
project(lab3sda)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_PREFIX_PATH "C:/Qt/6.11.0/mingw_64")

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

add_executable(lab3sda
        main.cpp
        mainwindow.cpp
        mainwindow.h
        services.cpp
        sqlite3.c
        domain.h
        repository.h
        exceptions.h
        file_repo.h
        services.h
        validator.h
        commands.h
        tutorialtablemodel.h
        watchlistwindow.h
)

target_link_libraries(lab3sda PRIVATE Qt6::Widgets)