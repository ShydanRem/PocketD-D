# Same disabled Atomics guard as ESP; no OS bindings or untrusted script loader.
set(QJS "${CMAKE_CURRENT_LIST_DIR}/../../.deps/quickjs")
include(${CMAKE_CURRENT_LIST_DIR}/prepare.cmake)
pocket_prepare_quickjs("${QJS}" "${CMAKE_CURRENT_BINARY_DIR}/quickjs-pocket.c")
add_library(pocket_quickjs STATIC "${CMAKE_CURRENT_BINARY_DIR}/quickjs-pocket.c" "${QJS}/dtoa.c" "${QJS}/libregexp.c" "${QJS}/libunicode.c" "${QJS}/cutils.c")
target_include_directories(pocket_quickjs PUBLIC "${QJS}")
target_compile_definitions(pocket_quickjs PRIVATE _GNU_SOURCE CONFIG_VERSION="2026-06-04-pocket")
target_compile_options(pocket_quickjs PRIVATE -Os -fwrapv -Wno-array-bounds)
target_link_libraries(pocket_quickjs m)
