target_sources(cadsh PUBLIC
	cadsh.cpp meshIO.cpp miniz.c
)

target_include_directories(cadsh PRIVATE ".")
