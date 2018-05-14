include_directories(SYSTEM ${Boost_INCLUDE_DIRS} )

set(WSPRLogLib_SRCS 
  WSPRLog.cxx
  TimeCorr.cxx
  ChiSquared.cxx
  KolmogorovSmirnov.cxx
  SolarTime.cxx
  )

add_library(WSPRLogLib ${WSPRLogLib_SRCS})


set(WSPRLogTest_SRCS
    WSPRLogTest.cxx
)
add_executable(WSPRLogTest ${WSPRLogTest_SRCS})
target_link_libraries(WSPRLogTest WSPRLogLib ${Boost_LIBRARIES})


set(WSPRLogBandFilter_SRCS
    WSPRLogBandFilter.cxx
)
add_executable(WSPRLogBandFilter ${WSPRLogBandFilter_SRCS})
target_link_libraries(WSPRLogBandFilter WSPRLogLib 
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS WSPRLogBandFilter DESTINATION bin)


set(WSPRLogLineFilter_SRCS
    WSPRLogLineFilter.cxx
    )
add_executable(WSPRLogLineFilter ${WSPRLogLineFilter_SRCS})
target_link_libraries(WSPRLogLineFilter  WSPRLogLib 
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS WSPRLogLineFilter DESTINATION bin)



set(WSPRLogSplitter_SRCS
    WSPRLogSplitter.cxx
    )
add_executable(WSPRLogSplitter ${WSPRLogSplitter_SRCS})
target_link_libraries(WSPRLogSplitter  WSPRLogLib 
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})
install(TARGETS WSPRLogSplitter DESTINATION bin)


set(WSPRLogHisto_SRCS
    WSPRLogHisto.cxx
    )

add_executable(WSPRLogHisto ${WSPRLogHisto_SRCS})
target_link_libraries(WSPRLogHisto  WSPRLogLib 
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS WSPRLogHisto DESTINATION bin)


set(WSPRLogXY_SRCS
    WSPRLogXY.cxx
    )

add_executable(WSPRLogXY ${WSPRLogXY_SRCS})

target_link_libraries(WSPRLogXY  WSPRLogLib 
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS WSPRLogXY DESTINATION bin)

set(tctest_SRCS
    tctest.cxx
    )

add_executable(tctest ${tctest_SRCS})

target_link_libraries(tctest WSPRLogLib
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})


set(WSPRLogDiffTime_SRCS
    WSPRLogDiffTime.cxx
    )

add_executable(WSPRLogDiffTime ${WSPRLogDiffTime_SRCS})

target_link_libraries(WSPRLogDiffTime  WSPRLogLib 
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS WSPRLogDiffTime DESTINATION bin)


set(ChiSquaredTest_SRCS
  ChiSquaredTest.cxx
  )

add_executable(ChiSquaredTest ${ChiSquaredTest_SRCS})
 
target_link_libraries(ChiSquaredTest WSPRLogLib
	 ${Boost_LIBRARIES})

install(TARGETS ChiSquaredTest DESTINATION bin)


set(WSPRLogTimeHisto_SRCS
    WSPRLogTimeHisto.cxx
    )

add_executable(WSPRLogTimeHisto ${WSPRLogTimeHisto_SRCS})

target_link_libraries(WSPRLogTimeHisto  WSPRLogLib 
	${ZLIB_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS WSPRLogTimeHisto DESTINATION bin)


set(KSTest_SRCS
  KSTest.cxx
  )

add_executable(KSTest ${KSTest_SRCS})
 
target_link_libraries(KSTest WSPRLogLib
	 ${Boost_LIBRARIES})

install(TARGETS KSTest DESTINATION bin)


set(HistoStats_SRCS
  HistoStats.cxx
  )

add_executable(HistoStats ${HistoStats_SRCS})
 
target_link_libraries(HistoStats 
	 ${Boost_LIBRARIES})

install(TARGETS HistoStats DESTINATION bin)

set(RiskRatio_SRCS
  RiskRatio.cxx
  )

add_executable(RiskRatio ${RiskRatio_SRCS})
 
target_link_libraries(RiskRatio WSPRLogLib
	 ${Boost_LIBRARIES})

install(TARGETS RiskRatio DESTINATION bin)


set(CallRR_SRCS
  CallRR.cxx
  )

add_executable(CallRR ${CallRR_SRCS})
 
target_link_libraries(CallRR WSPRLogLib
	 ${Boost_LIBRARIES})

install(TARGETS CallRR DESTINATION bin)


set(WSPRLogSolTimeOR_SRCS
  WSPRLogSolTimeOR.cxx
  )

add_executable(WSPRLogSolTimeOR ${WSPRLogSolTimeOR_SRCS})
 
target_link_libraries(WSPRLogSolTimeOR WSPRLogLib
	 ${Boost_LIBRARIES})

install(TARGETS WSPRLogSolTimeOR DESTINATION bin)


set(WSPRLogToMidSol_SRCS
  WSPRLogToMidSol.cxx
  )

add_executable(WSPRLogToMidSol ${WSPRLogToMidSol_SRCS})
 
target_link_libraries(WSPRLogToMidSol WSPRLogLib
	 ${Boost_LIBRARIES})

install(TARGETS WSPRLogToMidSol DESTINATION bin)
