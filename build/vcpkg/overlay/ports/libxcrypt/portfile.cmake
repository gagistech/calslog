# Overlay port: wrap system libcrypt instead of building from source.
# The upstream vcpkg port fails with CMAKE_MAKE_PROGRAM not set.
vcpkg_find_program(PKG_CONFIG pkg-config)

execute_process(
    COMMAND ${PKG_CONFIG} --cflags libcrypt
    OUTPUT_VARIABLE _cflags
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _result
)
if(NOT _result EQUAL 0)
    # fallback: no special cflags needed
    set(_cflags "")
endif()

execute_process(
    COMMAND ${PKG_CONFIG} --libs libcrypt
    OUTPUT_VARIABLE _libs
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _result
)
if(NOT _result EQUAL 0)
    set(_libs "-lcrypt")
endif()

file(WRITE "${CURRENT_PACKAGES_DIR}/include/crypt.h" "#include <crypt.h>\n")
# Just create a stub include dir; the real header comes from system libcrypt-dev
file(COPY DESTINATION "${CURRENT_PACKAGES_DIR}/include" FILES "")

file(WRITE "${CURRENT_PACKAGES_DIR}/share/libxcrypt/libxcryptConfig.cmake" "[[set(LIBXCRYPT_FOUND TRUE)
add_library(xcrypt::libxcrypt INTERFACE IMPORTED)
set_target_properties(xcrypt::libxcrypt PROPERTIES
    INTERFACE_LINK_LIBRARIES \"${_libs}\"
)
set(libxcrypt_FOUND TRUE)
]]")

file(WRITE "${CURRENT_PACKAGES_DIR}/share/libxcrypt/copyright" "System package wrapper\n")
