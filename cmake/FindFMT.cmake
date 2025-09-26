include(FindPackageHandleStandardArgs)

set(FMT_BUNDLED OFF)

if(NOT PREFER_BUNDLED_LIBS)
  set(CMAKE_MODULE_PATH ${ORIGINAL_CMAKE_MODULE_PATH})
  find_package(fmt QUIET)
  set(CMAKE_MODULE_PATH ${OWN_CMAKE_MODULE_PATH})

  if(TARGET fmt::fmt)
    set(FMT_LIBRARY fmt::fmt)
    set(FMT_BUNDLED OFF)
    set(FMT_FOUND ON)
  endif()
endif()

if(NOT FMT_FOUND)
  set(FMT_BUNDLED ON)

  include(FetchContent)

  FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        11.2.0
    SOURCE_DIR     ${CMAKE_CURRENT_BINARY_DIR}/_deps/fmt-src
    BINARY_DIR     ${CMAKE_CURRENT_BINARY_DIR}/_deps/fmt-build
    CONFIGURE_COMMAND ""
    BUILD_COMMAND     ""
    INSTALL_COMMAND   ""
    TEST_COMMAND      ""
  )

  FetchContent_GetProperties(fmt)
  if(NOT fmt_POPULATED)
    message(STATUS "Fetching and configuring fmt (11.2.0)...")
    FetchContent_Populate(fmt)
    add_subdirectory(
      ${fmt_SOURCE_DIR}
      ${fmt_BINARY_DIR}
      EXCLUDE_FROM_ALL
    )
  endif()

  set(FMT_LIBRARY fmt::fmt)
  set(FMT_FOUND ON)
endif()

find_package_handle_standard_args(
  FMT
  REQUIRED_VARS FMT_LIBRARY
  VERSION_VAR   FMT_VERSION
)

if(FMT_FOUND)
  set(FMT_LIBRARY ${FMT_LIBRARY})
  set(FMT_BUNDLED ${FMT_BUNDLED})
endif()
