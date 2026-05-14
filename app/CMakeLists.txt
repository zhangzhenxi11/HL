
# 确保 Release 模式也生成 PDB 文件
if(MSVC)
    # 为所有配置类型添加调试符号生成
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
endif()

SET(EXE_NAME app)

############################列出所需文件############################
file(GLOB APP_SOURCES  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

############################编译设置############################
add_executable(${EXE_NAME}   ${APP_SOURCES})
add_executable(${EXE_NAME}_console   ${APP_SOURCES})

# 添加 Dump 生成所需的库链接
if(WIN32)
    target_link_libraries(${EXE_NAME} Dbghelp)
    target_link_libraries(${EXE_NAME}_console Dbghelp)
endif()

target_link_libraries(${EXE_NAME}
		QtUI::core_runner
		Qt5::Core
		Qt5::Widgets
		Qt5::WebEngine)
target_link_libraries(${EXE_NAME}_console
		QtUI::core_runner
		Qt5::Core
		Qt5::Widgets
		Qt5::WebEngine)

IF(WIN32)
	#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:\"mainCRTStartup\"")
	set_property(
			TARGET
			${EXE_NAME}
			PROPERTY LINK_FLAGS
			"${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\""
	)
	# 为控制台版本设置控制台子系统
    set_property(
        TARGET ${EXE_NAME}_console
        PROPERTY LINK_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE"
    )
ENDIF(WIN32)

USE_GIT_VERSION(${EXE_NAME})

if(USE_CLION)
	set_target_properties(${EXE_NAME} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/output/${CMAKE_BUILD_TYPE}"
			# 设置 PDB 输出位置
			PDB_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/output/${CMAKE_BUILD_TYPE}")

	set_target_properties(${EXE_NAME}_console PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/output/${CMAKE_BUILD_TYPE}"
			PDB_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/output/${CMAKE_BUILD_TYPE}")
else()
	set_target_properties(${EXE_NAME} PROPERTIES
			VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/output/$(Configuration)"
			# 设置 PDB 输出位置
			PDB_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/output/$(Configuration)")

	set_target_properties(${EXE_NAME}_console PROPERTIES
			VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/output/$(Configuration)"
			PDB_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/output/$(Configuration)")
endif()
	
message(STATUS "source path: ${PDB_OUTPUT_DIRECTORY}")
	
# 确保 PDB 文件被复制到输出目录
set(PDB_FILES
    $<TARGET_PDB_FILE:${EXE_NAME}>
    $<TARGET_PDB_FILE:${EXE_NAME}_console>
)

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

		# 复制 PDB 文件
        COMMAND ${CMAKE_COMMAND} -E
            copy_if_different
            "${PDB_FILES}"
            "${PROJECT_BINARY_DIR}/output/$(Configuration)"

	)
endif()