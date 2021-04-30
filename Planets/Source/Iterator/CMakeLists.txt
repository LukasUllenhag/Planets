add_subdirectory(Container)

target_sources(
${PROJECT_NAME} PRIVATE
ConstRandomAccessIterator.h
ConstRandomAccessIteratorDebugBase.h
ConstRandomAccessIteratorReleaseBase.h
IteratorMacro.h
RandomAccessIterator.h
)