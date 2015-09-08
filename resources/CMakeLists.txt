ADD_CUSTOM_TARGET(tethering_images.edj
                COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/images
		        ${CMAKE_CURRENT_SOURCE_DIR}/tethering_images.edc
			${CMAKE_CURRENT_SOURCE_DIR}/tethering_images.edj
		DEPENDS
			${CMAKE_CURRENT_SOURCE_DIR}/tethering_images.edc
					       )
ADD_DEPENDENCIES(${PROJECT_NAME} tethering_images.edj)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tethering_images.edj DESTINATION ${EDJDIR})

ADD_CUSTOM_TARGET(tethering.edj
                COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/
                        ${CMAKE_CURRENT_SOURCE_DIR}/tethering.edc
                        ${CMAKE_CURRENT_SOURCE_DIR}/tethering.edj
                DEPENDS
                        ${CMAKE_CURRENT_SOURCE_DIR}/tethering.edc
                                               )
ADD_DEPENDENCIES(${PROJECT_NAME} tethering.edj)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tethering.edj DESTINATION ${EDJDIR})

