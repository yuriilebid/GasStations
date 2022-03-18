cmake_minimum_required(VERSION 3.18.4)
project(PTS2_0)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_VERBOSE_MAKEFILE 0)
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-O1 -g")
#SET(CMAKE_CXX_COMPILER: clang)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.15/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
            TLS_VERIFY ON)
endif()

set(CMAKE_VERBOSE_MAKEFILE 1)

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(REQUIRES
        http_parser/2.9.4
        nlohmann_json/3.9.1
        msgpack/3.3.0
        msgpack-c/4.0.0
        websocketpp/0.8.2
        boost/1.78.0
        libcurl/7.77.0
        simple-websocket-server/2.0.2
        spdlog/1.8.2
        sqlite3/3.36.0
        cxxopts/2.2.1
        openssl/1.1.1l
        BASIC_SETUP
        BUILD missing)

set(SUBDIRS
        ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/inc
        ${PROJECT_SOURCE_DIR}/inc/dataTypes
        ${PROJECT_SOURCE_DIR}/inc/dataTypes/requestDataTypes
        ${PROJECT_SOURCE_DIR}/inc/dataTypes/responseDataTypes
        ${PROJECT_SOURCE_DIR}/inc/Protocols
        ${PROJECT_SOURCE_DIR}/inc/Protocols/ProtocolsMasters
        ${PROJECT_SOURCE_DIR}/inc/Protocols/ProtocolsSlaves
        ${PROJECT_SOURCE_DIR}/inc/Connections
        ${PROJECT_SOURCE_DIR}/inc/Mediator
        ${PROJECT_SOURCE_DIR}/inc/Mediator/wamp
        ${PROJECT_SOURCE_DIR}/inc/Mediator/columnMediator
        ${PROJECT_SOURCE_DIR}/inc/Mediator/jsonMediator
        ${PROJECT_SOURCE_DIR}/inc/Mediator/kassaMediator
        ${PROJECT_SOURCE_DIR}/inc/DataBase
        ${PROJECT_SOURCE_DIR}/inc/utils
        ${PROJECT_SOURCE_DIR}/lib
        ${PROJECT_SOURCE_DIR}/lib/Simple-Web-Server)

add_executable(PTS2_0
        main.cpp
        lib/Simple-Web-Server/server_http.hpp
        src/Protocols/Protocol.cpp
        src/Protocols/ProtocolsMasters/ProtocolMaster.cpp
        src/Protocols/ProtocolsSlaves/ProtocolSlave.cpp
        src/dataTypes/responseDataTypes/UniversalState.cpp
        src/dataTypes/responseDataTypes/UniversalAmount.cpp
        src/dataTypes/responseDataTypes/UniversalProducts.cpp
        src/dataTypes/requestDataTypes/UniversalCmd.cpp
        src/dataTypes/requestDataTypes/UniversalClose.cpp
        src/Protocols/ProtocolsMasters/ProtocolMasterUnipump.cpp
        src/Protocols/ProtocolsMasters/ProtocolMasterAdast.cpp
        src/Protocols/ProtocolsMasters/ProtocolMasterShelf.cpp
        src/Protocols/ProtocolsSlaves/ProtocolSlaveAdast.cpp
        src/Protocols/ProtocolsSlaves/ProtocolSlaveShelf.cpp
        src/Protocols/ProtocolsSlaves/ProtocolSlaveJson.cpp
        src/Protocols/ProtocolsSlaves/ProtocolSlaveUnipump.cpp
        src/Mediator/columnMediator/ColumnWorker.cpp
        src/Mediator/columnMediator/ColumnLine.cpp
        src/Mediator/kassaMediator/KassaWorker.cpp
        src/Mediator/Line.cpp
        src/Mediator/ConcreateMediator.cpp
        src/Connections/Connection.cpp
        src/Connections/TcpConnection.cpp
        src/Connections/serialConnection.cpp
        src/DataBase/TransactionControl.cpp
        src/dataTypes/requestDataTypes/UniversalAuthorize.cpp
        src/dataTypes/requestDataTypes/LogCmd.cpp
        src/dataTypes/requestDataTypes/UniversalSetPrice.cpp
        src/Utils/logger.cpp
        src/Mediator/wamp/wamp.cpp
        src/Mediator/jsonMediator/JsonWorker.cpp
        src/dataTypes/requestDataTypes/TechCmd.cpp
        src/Mediator/DB.cpp
        src/dataTypes/StateMap.cpp
        src/dataTypes/FuelProduct.cpp
        src/Mediator/kassaMediator/Scenario.cpp
        src/dataTypes/nozzleLogicState.cpp
        src/dataTypes/responseDataTypes/UniversalTotals.cpp
        src/dataTypes/responseDataTypes/UniversalPumpsConfig.cpp
        src/dataTypes/responseDataTypes/UniversalTech.cpp)

target_compile_definitions(PTS2_0 PRIVATE PROJECT_NAME="${PROJECT_NAME}" WAMP_TLS SQLITE_ENABLE_UPDATE_DELETE_LIMIT=1 LICENSECODE="1c78b81a21adac14b888d290eef6fac14ba34b8dd63f52ef8bc37c5a9bbabf8d")

#libstdc++fs.a - tmp not used for
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

target_link_libraries(PTS2_0 PRIVATE ${CONAN_LIBS} curl stdc++fs)