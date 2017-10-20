
set(ringModulatorSources
	source/ringModulatorController.cpp
	source/ringModulatorController.h
	source/ringModulatorIDs.h
	source/ringModulatorProcessor.cpp
	source/ringModulatorProcessor.h
	source/factory.cpp
	source/version.h
	source/OnePole.cpp
	source/OnePole.h
	source/Oscillator.cpp
	source/Oscillator.h
	source/Ramp.cpp
	source/Ramp.h
)

set(target ringModulator)
smtg_add_vst3plugin(${target} ${SDK_ROOT} ${ringModulatorSources})
target_link_libraries(${target} PRIVATE base sdk)
if(MAC)
	smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/mac/Info.plist" PREPROCESS)
	smtg_set_prefix_header(${target} "${CMAKE_CURRENT_LIST_DIR}/mac/ringModulatorPrefix.pch" "NO")
elseif(WIN)
	target_sources(${target} PRIVATE resource/ringModulator.rc)
endif()
