file(GLOB EXAMPLE_FILES *.cpp)

foreach(file ${EXAMPLE_FILES})
    get_filename_component(example_name ${file} NAME_WE)
    add_executable(${example_name} ${file})
    target_include_directories(${example_name} PRIVATE ${CMAKE_SOURCE_DIR})
endforeach()
