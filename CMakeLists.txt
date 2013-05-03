CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ug-setting-mobileap-efl C)

SET(PACKAGE ${PROJECT_NAME})
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(RESDIR "${PREFIX}/res")
SET(LOCALEDIR "${RESDIR}/locale")
SET(EDJDIR "${RESDIR}/edje/${PROJECT_NAME}")
SET(IMGDIR "${RESDIR}/images/${PROJECT_NAME}")
SET(LIBDIR "${PREFIX}/lib")

SET(SRCS
	src/mh_main_ug.c
	src/mh_view_main.c
	src/mh_view_wifi_setup.c
	src/mh_func_onoff.c
	src/mh_common_utility.c
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

SET(dependents "elementary evas ui-gadget-1 wifi-direct capi-network-tethering capi-network-connection capi-network-wifi capi-telephony-sim notification")

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED ${dependents})

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_C_FLAGS} -fvisibility=hidden -Wall -fPIC")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_C_FLAGS_RELEASE "-O2")

ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DRESDIR=\"${RESDIR}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DEDJDIR=\"${EDJDIR}\"")

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/ug/lib)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ug-setting-mobileap-efl.xml DESTINATION /usr/share/packages/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/images/tethering.png DESTINATION ${IMGDIR})

ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(resources)
