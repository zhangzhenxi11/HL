cmake_minimum_required(VERSION 3.0)
project(HL_VTM)
############################工程设置############################
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG}  ${PROJECT_BINARY_DIR}/output/${OUTPUTCONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG}  ${PROJECT_BINARY_DIR}/output/${OUTPUTCONFIG})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG}  ${PROJECT_BINARY_DIR}/output/${OUTPUTCONFIG})
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )


# Add a d postfix to the debug libraries
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Set Debug library postfix" FORCE)
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "Set Release library postfix" FORCE)
SET(CMAKE_MINSIZEREL_POSTFIX "" CACHE STRING "Set MinSizeRel library postfix" FORCE)
SET(CMAKE_RELWITHDEBINFO_POSTFIX "" CACHE STRING "Set RelWithDebInfo library postfix" FORCE)
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

############################依赖设置############################
find_package(xVersionCommon REQUIRED)
find_package(Poco REQUIRED Foundation Util)
find_package(Kernel REQUIRED  Core Utils EditorUI CoreUI Fortrend FortrendUI)
find_package(QtUI REQUIRED) 
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport WebEngineWidgets AxContainer)
include_directories(
${CMAKE_BINARY_DIR}  # 包含编译目标顶级目录，为了能找到版本号头文件
${Qt5AxContainer_INCLUDE_DIRS}
${Qt5Core_INCLUDE_DIRS}
${Qt5Widgets_INCLUDE_DIRS}
${Qt5WebEngineWidgets_INCLUDE_DIRS}
${QtUI_INCLUDE_DIRS}
${libkernel_INCLUDE_DIRS}
)



############################CPPCHECK############################
# https://blog.csdn.net/FakeTaoZero/article/details/127274010
find_program(CPPCHECK_PROC NAMES cppcheck)
if (CPPCHECK_PROC)
  list(
    APPEND CPPCHECK_CONFIG
    "--enable=warning,performance,information"
    "--suppress=missingIncludeSystem"
    "--language=c++"
    "--std=c++11"
    "--max-ctu-depth=10"
    "-j 4"
    "--output-file=${CMAKE_BINARY_DIR}/cppcheck_report.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}"
  )

  add_custom_target(CPPCHECK_TASK)

  add_custom_command(TARGET CPPCHECK_TASK
    COMMAND ${CPPCHECK_PROC} ${CPPCHECK_CONFIG})
endif ()


############################编译设置############################
add_subdirectory(app)
add_subdirectory(device)
add_subdirectory(MyUnit)