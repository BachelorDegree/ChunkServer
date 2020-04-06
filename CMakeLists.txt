CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(chunkserver)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "-Wall")

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(/usr/local/include)
LINK_LIBRARIES(pthread protobuf gpr grpc++ grpc++_reflection colib)

ADD_SUBDIRECTORY(ChunkMasterServiceClient)

SET(APP_SOURCES "./dylib.cpp")
FILE(GLOB APP_SOURCES ${APP_SOURCES} "./*.cpp")
FILE(GLOB APP_SOURCES ${APP_SOURCES} "./Proto/*.cc")
FILE(GLOB APP_SOURCES ${APP_SOURCES} "./Logic/*.cpp")
FILE(GLOB APP_SOURCES ${APP_SOURCES} "./Common/*.cpp")
FILE(GLOB APP_SOURCES ${APP_SOURCES} "./Handler/*.cpp")
ADD_LIBRARY(chunkserver SHARED ${APP_SOURCES})
TARGET_LINK_LIBRARIES(chunkserver coredeps)
TARGET_LINK_LIBRARIES(chunkserver chunkmasterclient)