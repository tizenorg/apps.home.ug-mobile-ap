CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ug-setting-mobileap-efl C)

SET(PACKAGE ${PROJECT_NAME})
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(RESDIR "${PREFIX}/res")
SET(LOCALEDIR "/usr/ug/res/locale")
SET(EDJDIR "${RESDIR}/edje/${PROJECT_NAME}")
SET(IMGDIR "/usr/ug/res/images/${PROJECT_NAME}")
SET(LIBDIR "${PREFIX}/lib")
SET(TABLEDIR "/usr/apps/ug-setting-mobileap-efl/shared/res/tables")

SET(SRCS
	src/mh_main_ug.c
	src/mh_view_main.c
	src/mh_view_wifi_setup.c
	src/mh_func_onoff.c
	src/mh_common_utility.c
	src/mh_popup.c
#	src/mh_device_rename.c
	src/mh_connected_client.c
)

IF (TIZEN_DUALSIM_ENABLE)
	ADD_DEFINITIONS(-DTIZEN_DUALSIM_ENABLE)
ENDIF (TIZEN_DUALSIM_ENABLE)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

SET(dependents "elementary utilX evas ui-gadget-1 capi-network-tethering capi-network-connection capi-network-wifi notification efl-extension")

SET(CMAKE_BUILD_TYPE "Release")

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

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION /usr/apps/ug-setting-mobileap-efl/lib/ug)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ug-setting-mobileap-efl.efl DESTINATION /etc/smack/accesses.d)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ug-setting-mobileap-efl.xml DESTINATION /usr/share/packages/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/help_setting_tethering.xml DESTINATION /usr/apps/ug-setting-mobileap-efl/res/help/ug-setting-mobileap-efl/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/images/org.tizen.tethering.png DESTINATION ${IMGDIR})
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/images/ DESTINATION ${IMGDIR}
			FILES_MATCHING
			PATTERN "noti_tethering_*.png"
			)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tables/ DESTINATION ${TABLEDIR} FILES_MATCHING PATTERN "*.xml")

ADD_SUBDIRECTORY(po)
ADD_SUBDIRECTORY(resources)
