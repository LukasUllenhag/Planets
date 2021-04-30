add_subdirectory(PostProcessing)
add_subdirectory(Vertex)

target_sources(
${PROJECT_NAME} PRIVATE
Camera.cpp
Camera.h
GlMacro.h
PngLoader.cpp
PngLoader.h
Program.cpp
Program.h
Shader.cpp
Shader.h
Texture.cpp
Texture.h
)