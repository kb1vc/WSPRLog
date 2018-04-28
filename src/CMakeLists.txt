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



set(WSPRLogLineFilter_SRCS
    WSPRLog.cxx
    WSPRLogLineFilter.cxx
    )

add_executable(WSPRLogLineFilter ${WSPRLogLineFilter_SRCS})

target_link_libraries(WSPRLogLineFilter  ${ZLIB_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS WSPRLogLineFilter DESTINATION bin)



set(WSPRLogSplitter_SRCS
    WSPRLog.cxx
    WSPRLogSplitter.cxx
    )

add_executable(WSPRLogSplitter ${WSPRLogSplitter_SRCS})

target_link_libraries(WSPRLogSplitter  ${ZLIB_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS WSPRLogSplitter DESTINATION bin)




set(WSPRLogHisto_SRCS
    WSPRLog.cxx
    WSPRLogHisto.cxx
    )

add_executable(WSPRLogHisto ${WSPRLogHisto_SRCS})

target_link_libraries(WSPRLogHisto  ${ZLIB_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS WSPRLogHisto DESTINATION bin)


set(WSPRLogXY_SRCS
    WSPRLog.cxx
    WSPRLogXY.cxx
    )

add_executable(WSPRLogXY ${WSPRLogXY_SRCS})

target_link_libraries(WSPRLogXY  ${ZLIB_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS WSPRLogXY DESTINATION bin)

