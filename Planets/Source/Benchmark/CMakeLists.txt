add_subdirectory(Data)

target_sources(
${PROJECT_NAME} PRIVATE
BenchmarkEvent.cpp
BenchmarkEvent.h
BenchmarkEventFactory.cpp
BenchmarkEventFactory.h
BenchmarkMacros.h
BenchmarkManager.cpp
BenchmarkManager.h
BenchmarkSession.cpp
BenchmarkSession.h
BenchmarkTimer.h
)