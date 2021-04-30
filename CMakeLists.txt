cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(OPTIMIZE "Optimize the code at the cost of fewer safety checks" ON)

project(Planets VERSION 1.0.0)

add_executable(${PROJECT_NAME})

add_subdirectory(Planets/Source)

target_sources(${PROJECT_NAME} PRIVATE "Icon/Planets.rc")

configure_file(Configure.h.in Configure.h)


# vvv Preprocessor definitions vvv

# Tell GLEW that we want to link statically
add_compile_definitions(GLEW_STATIC)

if(${OPTIMIZE})
  add_compile_definitions(RELEASE NDEBUG)
else()
  add_compile_definitions(DEBUG)
endif()

# ^^^ Preprocessor definitions ^^^


# vvv Add the libraries vvv

target_include_directories(
${PROJECT_NAME} PRIVATE
"Dependencies/GLEW/Include"
"Dependencies/GLFW/Include"
"Planets"
"${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_directories(
${PROJECT_NAME} PRIVATE
"Dependencies/GLEW/Library"
"Dependencies/GLFW/Library"
)

target_link_libraries(
${PROJECT_NAME} PRIVATE
glew32s.lib
glfw3.lib
OpenGL32.lib
)

# ^^^ Add the libraries ^^^

# vvv Make installation vvv
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY "Planets/Source/Shaders" DESTINATION Source)
install(DIRECTORY "Planets/Source/Textures" DESTINATION Source)
install(DIRECTORY "Planets/Source/DynamicVariableFiles" DESTINATION Source)
install(DIRECTORY "Planets/Benchmarks" DESTINATION "./")
# ^^^ Make installation ^^^

# vvv CPack Installation vvv

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Celestial body generation")
set(CPACK_PACKAGE_VENDOR "Flompey")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/InstallerData/Description.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYRIGHT.txt")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

if(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full UNIX paths properly.
  # Make sure there is at least one set of four backlashes.
  set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/InstallerData\\\\InstallImage.bmp")
  set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/Icon\\\\Icon.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/InstallerData\\\\UninstallIcon.ico")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${PROJECT_NAME}.exe")
  set(CPACK_NSIS_MODIFY_PATH ON)
endif()

set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME} ${PROJECT_NAME})
set(CPACK_CREATE_DESKTOP_LINKS ${PROJECT_NAME})
include(CPack)