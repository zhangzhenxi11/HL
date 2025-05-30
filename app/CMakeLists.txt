SET(EXE_NAME app)

############################列出所需文件############################
file(GLOB APP_SOURCES  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

############################编译设置############################
add_executable(${EXE_NAME}   ${APP_SOURCES})
add_executable(${EXE_NAME}_console   ${APP_SOURCES})

target_link_libraries(${EXE_NAME}
		QtUI::core_runner
		Qt5::Core
		Qt5::Widgets)
target_link_libraries(${EXE_NAME}_console
		QtUI::core_runner
		Qt5::Core
		Qt5::Widgets)

IF(WIN32)
	#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:\"mainCRTStartup\"")
	set_property(
			TARGET
			${EXE_NAME}
			PROPERTY LINK_FLAGS
			"${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\""
	)
ENDIF(WIN32)

USE_GIT_VERSION(${EXE_NAME})

if(USE_CLION)
	set_target_properties(${EXE_NAME} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/output/${CMAKE_BUILD_TYPE}")
	set_target_properties(${EXE_NAME}_console PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/output/${CMAKE_BUILD_TYPE}")
else()
	set_target_properties(${EXE_NAME} PROPERTIES
			VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/output/$(Configuration)")
	set_target_properties(${EXE_NAME}_console PROPERTIES
			VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/output/$(Configuration)")
endif()
	
	

#以下脚本为拷贝UI框架依赖
#参考：https://www.cnblogs.com/JoyPoint/p/11629521.html
FILE(GLOB FRAMEWORK_DLL_FILES  ${QtUI_DIR}/../*.dll  ${QtUI_DIR}/../app.exe)
FILE(GLOB FRAMEWORK_MODULES_FILES  ${QtUI_DIR}/../modules/*.*)

if(USE_CLION)
	add_custom_command(TARGET ${EXE_NAME}
			POST_BUILD
			VERBATIM
			COMMAND_EXPAND_LISTS
			COMMAND ${CMAKE_COMMAND} -E
			copy_if_different
			"${FRAMEWORK_DLL_FILES}"
			"${PROJECT_BINARY_DIR}/output/${CMAKE_BUILD_TYPE}"

			COMMAND ${CMAKE_COMMAND} -E
			make_directory "${PROJECT_BINARY_DIR}/output/${CMAKE_BUILD_TYPE}/modules"

			COMMAND ${CMAKE_COMMAND} -E
			copy_if_different
			"${FRAMEWORK_MODULES_FILES}"
			"${PROJECT_BINARY_DIR}/output/${CMAKE_BUILD_TYPE}/modules"
			)
else()
	add_custom_command(TARGET ${EXE_NAME}
	   POST_BUILD
	   VERBATIM
	   COMMAND_EXPAND_LISTS
	   COMMAND ${CMAKE_COMMAND} -E
			copy_if_different
			"${FRAMEWORK_DLL_FILES}"
			"${PROJECT_BINARY_DIR}/output/$(Configuration)"

	   COMMAND ${CMAKE_COMMAND} -E
			make_directory "${PROJECT_BINARY_DIR}/output/$(Configuration)/modules"

	   COMMAND ${CMAKE_COMMAND} -E
			copy_if_different
			"${FRAMEWORK_MODULES_FILES}"
			"${PROJECT_BINARY_DIR}/output/$(Configuration)/modules"

	)
endif()