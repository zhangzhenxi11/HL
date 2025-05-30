SET(LIBNAME "myunit")

############################文件提取############################

file(GLOB_RECURSE IMPL_SRCS  src/*.c src/*.cpp)
COMMON_SOURCES_AUTO( SRCS ${IMPL_SRCS}) #分类源文件

# Headers
file(GLOB_RECURSE HDRS_G "include/*.h")

COMMON_HEADERS_AUTO( SRCS ${HDRS_G})  #分类头文件

 # QT ui files
file(GLOB_RECURSE UI_SOURCE 
"src/ui/*.ui"
)


############################工程设置############################
QT5_WRAP_CPP(MOC ${HDRS_G})
QT5_WRAP_UI(MOC_UI ${UI_SOURCE})

SOURCE_GROUP(Mocs FILES ${MOC} ${MOC_UI})
SOURCE_GROUP(Ui FILES ${UI_SOURCE})

# set output directories for all builds (Debug, Release, etc.)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG}  ${PROJECT_BINARY_DIR}/output/${OUTPUTCONFIG})#/modules
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG}  ${PROJECT_BINARY_DIR}/output/${OUTPUTCONFIG})#/modules
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG}  ${PROJECT_BINARY_DIR}/output/${OUTPUTCONFIG})#/modules
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )



############################编译设置 GUI插件############################

add_library( "${LIBNAME}" SHARED ${SRCS} ${MOC} ${MOC_UI})
set_target_properties( "${LIBNAME}"
	PROPERTIES
	OUTPUT_NAME ${LIBNAME}
	DEFINE_SYMBOL MYCUSTOMLIB_EXPORTS
	)	


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
Qt5::Core 
Qt5::Widgets 
Qt5::PrintSupport
)


target_include_directories( "${LIBNAME}"
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
	)
target_compile_definitions("${LIBNAME}" PUBLIC ${LIB_MODE_DEFINITIONS})

USE_GIT_VERSION(${LIBNAME})

# 拷贝Echarts
add_custom_command(TARGET ${LIBNAME} 
   POST_BUILD 
   VERBATIM 
   COMMAND_EXPAND_LISTS 
   COMMAND ${CMAKE_COMMAND} -E 
		copy_directory   
        "${CMAKE_CURRENT_SOURCE_DIR}/image"   
        "${PROJECT_BINARY_DIR}/output/$(Configuration)/image"
		
) 