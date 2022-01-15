#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "FastNoise2::FastNoise" for configuration "Release"
set_property(TARGET FastNoise2::FastNoise APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(FastNoise2::FastNoise PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/FastNoise.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/FastNoise.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS FastNoise2::FastNoise )
list(APPEND _IMPORT_CHECK_FILES_FOR_FastNoise2::FastNoise "${_IMPORT_PREFIX}/lib/FastNoise.lib" "${_IMPORT_PREFIX}/bin/FastNoise.dll" )

# Import target "FastNoise2::NoiseTool" for configuration "Release"
set_property(TARGET FastNoise2::NoiseTool APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(FastNoise2::NoiseTool PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/NoiseTool.exe"
  )

list(APPEND _IMPORT_CHECK_TARGETS FastNoise2::NoiseTool )
list(APPEND _IMPORT_CHECK_FILES_FOR_FastNoise2::NoiseTool "${_IMPORT_PREFIX}/bin/NoiseTool.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
