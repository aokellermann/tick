CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(tick)

SET(SOURCES
    main.c
    api.h
    api.c
    portfolio.h
    portfolio.c
)

ADD_EXECUTABLE(tick ${SOURCES})
SET_PROPERTY(TARGET tick PROPERTY C_STANDARD 99)
TARGET_LINK_LIBRARIES(tick curl json-c)

