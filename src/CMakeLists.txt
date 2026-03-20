target_sources(cadsh PUBLIC
	cadsh.cpp manifoldIO.cpp miniz.c #meshIO.cpp 
)

target_include_directories(cadsh PRIVATE ".")
