
cmake_minimum_required(VERSION 3.5)

set(CMAKE_BUILD_TYPE Debug)

set(DOGBOT_VERSION 0.1)
set(CMAKE_INSTALL_LIBDIR "/usr/local/lib")
set(CMAKE_INSTALL_INCLUDEDIR "/usr/local/include")

project(dogbot VERSION ${DOGBOT_VERSION} LANGUAGES CXX)# DESCRIPTION "API for React AI DogBot firmware interaction")
FIND_PACKAGE ( Threads REQUIRED )

include (FindPkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(JSONCPP jsoncpp)
  pkg_check_modules(ZMQ libzmq)
  pkg_check_modules(LIBUSB libusb-1.0)
endif()

include_directories(${JSONCPP_INCLUDE_DIRS})
link_directories(${JSONCPP_LIBRARY_DIRS})

include_directories(${ZMQ_INCLUDE_DIRS})
link_directories(${ZMQ_LIBRARY_DIRS})

include_directories(${LIBUSB_INCLUDE_DIRS})
link_directories(${LIBUSB_LIBRARY_DIRS})

add_definitions(-std=c++11)

ADD_LIBRARY (DogBotAPI STATIC
        Coms.cc 
        ComsZMQServer.cc 
        ComsZMQClient.cc
        ComsSerial.cc 
        ComsProxy.cc 
        ComsUSB.cc 
        DogBotAPI.cc 
        Servo.cc 
        LegKinematics.cc 
        Joint.cc 
        JointRelative.cc 
        Joint4BarLinkage.cc 
        ZMQContext.cc
        CallbackArray.cc
        FirmwareUpdate.cc
)

set_target_properties(DogBotAPI PROPERTIES VERSION ${PROJECT_VERSION})

target_link_libraries (DogBotAPI LINK_PUBLIC ${JSONCPP_LIBRARIES} ${ZMQ_LIBRARIES} ${LIBUSB_LIBRARIES})

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(DogBotAPI PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
    /usr/local/include
    PRIVATE src)

add_executable (exampleClient exampleClient.cc)

target_link_libraries (exampleClient LINK_PUBLIC DogBotAPI ${CMAKE_THREAD_LIBS_INIT} ${LIBUSB_LIBRARIES})

add_executable (dumpServoState dumpServoState.cc)

target_link_libraries (dumpServoState LINK_PUBLIC DogBotAPI ${CMAKE_THREAD_LIBS_INIT})

add_executable (testKinematics testKinematics.cc)

target_link_libraries (testKinematics LINK_PUBLIC DogBotAPI ${CMAKE_THREAD_LIBS_INIT})

add_executable (dogBotServer dogBotServer.cc)

target_link_libraries (dogBotServer LINK_PUBLIC DogBotAPI ${CMAKE_THREAD_LIBS_INIT})

add_executable (dogBotFirmwareUpdate dogBotFirmwareUpdate.cc)

target_link_libraries (dogBotFirmwareUpdate LINK_PUBLIC DogBotAPI ${CMAKE_THREAD_LIBS_INIT})


# 'make install' to the correct locations (provided by GNUInstallDirs).

install(TARGETS DogBotAPI EXPORT DogBotAPIConfig
   ARCHIVE  DESTINATION lib
   LIBRARY  DESTINATION lib
   RUNTIME  DESTINATION bin)  # This is for Windows

install(DIRECTORY ../include/dogbot DESTINATION include)

INSTALL(TARGETS DogBotAPI
        DESTINATION lib
        )
# INSTALL(FILES ../include/dogbot/DogBotAPI.hh
#         DESTINATION include/dogbot
#         )

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT DogBotAPIConfig DESTINATION share/dogbot/cmake)

# This makes the project importable from the build directory
export(TARGETS DogBotAPI FILE DogBotAPIConfig.cmake)
