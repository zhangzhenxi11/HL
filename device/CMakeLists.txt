SET(LIBNAME "libdevice_plugin")

############################文件提取############################
# Sources
# file(GLOB_RECURSE SRCS_G 
# "src/*.cpp" 
# )
# COMMON_SOURCES_AUTO( SRCS ${SRCS_G})

file(GLOB_RECURSE IMPL_SRCS  src/*.c src/*.cpp)

COMMON_SOURCES_AUTO( SRCS ${IMPL_SRCS})

#message(STATUS "IMPL_SRCS = ${IMPL_SRCS}")  # 输出 IMPL_SRCS 的内容
# Headers
file(GLOB_RECURSE HDRS_G 
"include/*.h"
)
COMMON_HEADERS_AUTO( SRCS ${HDRS_G})
#message(STATUS "HDRS_G = ${HDRS_G}")  # 输出 HDRS_G 的内容

 # QT ui files
file(GLOB_RECURSE UI_SOURCE 
"src/*.ui"
)
file(GLOB qrc_files "resource/*.qrc")



############################工程设置############################
# 强制在 VS 中显示所有文件（即使未被编译）
#source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${IMPL_SRCS} ${HDRS_G})

QT5_WRAP_CPP(MOC ${HDRS_G})
QT5_WRAP_UI(MOC_UI ${UI_SOURCE})
QT5_ADD_RESOURCES (QRC_RCC  ${qrc_files})


SOURCE_GROUP(Mocs FILES ${MOC} ${MOC_UI})
SOURCE_GROUP(Ui FILES ${UI_SOURCE})

# set output directories for all builds (Debug, Release, etc.)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG}  ${PROJECT_BINARY_DIR}/output/${OUTPUTCONFIG}/modules)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG}  ${PROJECT_BINARY_DIR}/output/${OUTPUTCONFIG}/modules)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG}  ${PROJECT_BINARY_DIR}/output/${OUTPUTCONFIG}/modules)
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )



############################编译设置 GUI插件############################

add_library( "${LIBNAME}" SHARED ${SRCS} ${MOC} ${MOC_UI})


find_library(STANDARD_MODBUS_API_LIBRARY NAMES StandardModbusApi PATHS ${CMAKE_CURRENT_SOURCE_DIR}/InovancePLCExternal)#设置StandardModbusApi路径



target_link_libraries( "${LIBNAME}" 
${SYSLIBS} 
Kernel::Core
Kernel::EditorUI
Kernel::CoreUI
Kernel::Fortrend
Kernel::FortrendUI
QtUI::core  
QtUI::common 
QtUI::coreplugin 
QtUI::guiplugin 
QtUI::xuiplugin 
Qt5::AxContainer
Qt5::Core 
Qt5::Widgets 
Qt5::PrintSupport
Qt5::WebEngineWidgets
myunit
)
target_link_libraries("${LIBNAME}" ${STANDARD_MODBUS_API_LIBRARY})#链接StandardModbusApi库


target_include_directories( "${LIBNAME}"
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
	)
target_compile_definitions("${LIBNAME}" PUBLIC ${LIB_MODE_DEFINITIONS})

USE_GIT_VERSION(${LIBNAME})

############################拷贝插件描述文件############################

add_custom_command(TARGET ${LIBNAME} 
   POST_BUILD 
   COMMAND ${CMAKE_COMMAND} -E 
		copy_if_different  
        "$<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/src/${LIBNAME}.pluginspec>"  
        "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/src/${LIBNAME}.pluginspec>"  
        "${PROJECT_BINARY_DIR}/output/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>/modules/"
) 


#以下脚本为拷贝设备框架libkernel依赖
#参考：https://www.cnblogs.com/JoyPoint/p/11629521.html
FILE(GLOB FRAMEWORK_DLL_FILES  "${Kernel_DIR}/../../../bin/*.dll")
add_custom_command(TARGET ${LIBNAME} 
   POST_BUILD 
   VERBATIM 
   COMMAND_EXPAND_LISTS 
   COMMAND ${CMAKE_COMMAND} -E 
		copy_if_different  
        "${FRAMEWORK_DLL_FILES}"   
        "${PROJECT_BINARY_DIR}/output/$(Configuration)"	
		
) 



# 拷贝xml & app.json
FILE(GLOB APP_CONFIG_FILES  
"${CMAKE_CURRENT_SOURCE_DIR}/../app.json"
"${CMAKE_CURRENT_SOURCE_DIR}/../kernel.xml"
"${CMAKE_CURRENT_SOURCE_DIR}/../data.data"
)
add_custom_command(TARGET ${LIBNAME} 
   POST_BUILD 
   VERBATIM 
   COMMAND_EXPAND_LISTS 
   COMMAND ${CMAKE_COMMAND} -E 
		copy_if_different  
        "${APP_CONFIG_FILES}"   
        "${PROJECT_BINARY_DIR}/output/$(Configuration)"	
		
) 

# 拷贝配置文件
add_custom_command(TARGET ${LIBNAME} 
   POST_BUILD 
   VERBATIM 
   COMMAND_EXPAND_LISTS 
   COMMAND ${CMAKE_COMMAND} -E 
		copy_directory   
        "${CMAKE_CURRENT_SOURCE_DIR}/../config"   
        "${PROJECT_BINARY_DIR}/output/$(Configuration)/config"	
		
) 

# 拷贝图标
add_custom_command(TARGET ${LIBNAME} 
   POST_BUILD 
   VERBATIM 
   COMMAND_EXPAND_LISTS 
   COMMAND ${CMAKE_COMMAND} -E 
		copy_directory   
        "${CMAKE_CURRENT_SOURCE_DIR}/../images"   
        "${PROJECT_BINARY_DIR}/output/$(Configuration)/images"	
		
) 

# 拷贝Echarts
add_custom_command(TARGET ${LIBNAME} 
   POST_BUILD 
   VERBATIM 
   COMMAND_EXPAND_LISTS 
   COMMAND ${CMAKE_COMMAND} -E 
		copy_directory   
        "${CMAKE_CURRENT_SOURCE_DIR}/../Echarts"   
        "${PROJECT_BINARY_DIR}/output/$(Configuration)/Echarts"	
		
) 

# 拷贝错误代码
add_custom_command(TARGET ${LIBNAME} 
   POST_BUILD 
   VERBATIM 
   COMMAND_EXPAND_LISTS 
   COMMAND ${CMAKE_COMMAND} -E 
		copy_directory   
        "${CMAKE_CURRENT_SOURCE_DIR}/../Error Code Definition"   
        "${PROJECT_BINARY_DIR}/output/$(Configuration)/Error Code Definition"	
		
) 

# 拷贝汇川PLC库
FILE(GLOB APP_CONFIG_FILES  
"${CMAKE_CURRENT_SOURCE_DIR}/InovancePLCExternal/ModbusTcpAPI.dll"
"${CMAKE_CURRENT_SOURCE_DIR}/InovancePLCExternal/StandardModbusApi.dll"
)

add_custom_command(TARGET ${LIBNAME} 
   POST_BUILD 
   VERBATIM 
   COMMAND_EXPAND_LISTS 
   COMMAND ${CMAKE_COMMAND} -E 
		copy_if_different  
        "${APP_CONFIG_FILES}"   
        "${PROJECT_BINARY_DIR}/output/$(Configuration)"	
		
)