# Copy manifest
configure_file(deploy/webos/appinfo.json.in ./appinfo.json @ONLY)

# Copy all files under deploy/webos/ to package root
install(DIRECTORY deploy/webos/ DESTINATION . PATTERN ".*" EXCLUDE PATTERN "*.in" EXCLUDE)
install(FILES "${CMAKE_BINARY_DIR}/appinfo.json" DESTINATION .)
install(CODE "execute_process(COMMAND npm run webos-gen-i18n -- -o \"\${CMAKE_INSTALL_PREFIX}/resources\" ${I18N_LOCALES})")

add_custom_target(webos-generate-gamecontrollerdb
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/webos/gen_gamecontrollerdb.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )

set(WEBOS_PACKAGE_FILENAME ${WEBOS_APPINFO_ID}_${PROJECT_VERSION}_$ENV{ARCH}.ipk)

set(CPACK_PACKAGE_NAME "${WEBOS_APPINFO_ID}")
set(CPACK_GENERATOR "External")
set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${CMAKE_SOURCE_DIR}/cmake/AresPackage.cmake")
set(CPACK_EXTERNAL_ENABLE_STAGING TRUE)
set(CPACK_MONOLITHIC_INSTALL TRUE)
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_SOURCE_DIR}/dist)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${PROJECT_VERSION}_$ENV{ARCH}")
# Will use all cores on CMake 3.20+
set(CPACK_THREADS 0)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(CPACK_STRIP_FILES TRUE)
endif()

add_custom_target(webos-package-moonlight COMMAND cpack)

include(CPack)