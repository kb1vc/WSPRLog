include_directories(SYSTEM ${Boost_INCLUDE_DIRS} )


set(WSPRLogTest_SRCS
    WSPRLog.cxx
    WSPRLogTest.cxx
    )

add_executable(WSPRLogTest ${WSPRLogTest_SRCS})

target_link_libraries(WSPRLogTest ${Boost_LIBRARIES})


set(WSPRLogBandFilter_SRCS
    WSPRLog.cxx
    WSPRLogBandFilter.cxx
    )

add_executable(WSPRLogBandFilter ${WSPRLogBandFilter_SRCS})

target_link_libraries(WSPRLogBandFilter  ${ZLIB_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS WSPRLogBandFilter DESTINATION bin)


