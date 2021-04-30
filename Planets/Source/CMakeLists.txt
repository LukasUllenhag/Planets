add_subdirectory(Benchmark)
add_subdirectory(CelestialBody)
add_subdirectory(Console)
add_subdirectory(Iterator)
add_subdirectory(Mathematics)
add_subdirectory(Noise)
add_subdirectory(Rendering)
add_subdirectory(Window)

target_precompile_headers(${PROJECT_NAME} PRIVATE PrecompiledHeader.h) 

target_sources(
${PROJECT_NAME} PRIVATE
CustomConcepts.h
CustomException.cpp
CustomException.h
DynamicVariableGroup.h
DynamicVariableManager.h
Game.cpp
Game.h
Keyboard.cpp
Keyboard.h
Main.cpp
Timer.h
)