# Sigma – Measurement Uncertainty Toolkit
# Copyright (c) 2025–2026 Ramon Obdam
# Licensed under the MIT License. See LICENSE file for details.

# Sigma Qt Test based test suite containing unit tests and end-to-end regression
# tests.

find_package( Qt6 REQUIRED COMPONENTS Test Concurrent QmlIntegration )

set( SIGMA_SRC_DIR ${CMAKE_SOURCE_DIR}/src )

# Static library with the Core and Orchestration layer sources, shared by all
# unit test executables. The GUI-only (windowscaptionhelper) and CLI-only
# (commandlineinterface) sources, as well as main.cpp, are intentionally
# excluded since they are outside the scope of these unit tests.
add_library( SigmaCore STATIC
    ${SIGMA_SRC_DIR}/applicationsettings.cpp
    ${SIGMA_SRC_DIR}/applicationsettings.h
    ${SIGMA_SRC_DIR}/budgetmodel.cpp
    ${SIGMA_SRC_DIR}/budgetmodel.h
    ${SIGMA_SRC_DIR}/correlation.cpp
    ${SIGMA_SRC_DIR}/correlation.h
    ${SIGMA_SRC_DIR}/data.cpp
    ${SIGMA_SRC_DIR}/data.h
    ${SIGMA_SRC_DIR}/datatype.h
    ${SIGMA_SRC_DIR}/diffutil.cpp
    ${SIGMA_SRC_DIR}/diffutil.h
    ${SIGMA_SRC_DIR}/distribution.cpp
    ${SIGMA_SRC_DIR}/distribution.h
    ${SIGMA_SRC_DIR}/inputparameter.cpp
    ${SIGMA_SRC_DIR}/inputparameter.h
    ${SIGMA_SRC_DIR}/jsondiff.h
    ${SIGMA_SRC_DIR}/lockableitemselectionmodel.cpp
    ${SIGMA_SRC_DIR}/lockableitemselectionmodel.h
    ${SIGMA_SRC_DIR}/mathconstants.h
    ${SIGMA_SRC_DIR}/mixedcopulasampler.hpp
    ${SIGMA_SRC_DIR}/model.hpp
    ${SIGMA_SRC_DIR}/modelcontrol.hpp
    ${SIGMA_SRC_DIR}/montecarlo.cpp
    ${SIGMA_SRC_DIR}/montecarlo.h
    ${SIGMA_SRC_DIR}/outputmodel.cpp
    ${SIGMA_SRC_DIR}/outputmodel.h
    ${SIGMA_SRC_DIR}/outputparameter.cpp
    ${SIGMA_SRC_DIR}/outputparameter.h
    ${SIGMA_SRC_DIR}/parameter.cpp
    ${SIGMA_SRC_DIR}/parameter.h
    ${SIGMA_SRC_DIR}/record.h
    ${SIGMA_SRC_DIR}/resultsmodel.cpp
    ${SIGMA_SRC_DIR}/resultsmodel.h
    ${SIGMA_SRC_DIR}/samplebatch.cpp
    ${SIGMA_SRC_DIR}/samplebatch.h
    ${SIGMA_SRC_DIR}/settings.cpp
    ${SIGMA_SRC_DIR}/settings.h
    ${SIGMA_SRC_DIR}/statistics.cpp
    ${SIGMA_SRC_DIR}/statistics.h
    ${SIGMA_SRC_DIR}/stringutils.cpp
    ${SIGMA_SRC_DIR}/stringutils.h
    ${SIGMA_SRC_DIR}/transaction.cpp
    ${SIGMA_SRC_DIR}/transaction.h
    ${SIGMA_SRC_DIR}/uncertaintycalculation.cpp
    ${SIGMA_SRC_DIR}/uncertaintycalculation.h
    ${SIGMA_SRC_DIR}/uncertaintycomponent.cpp
    ${SIGMA_SRC_DIR}/uncertaintycomponent.h
    ${SIGMA_SRC_DIR}/undohistorymodel.cpp
    ${SIGMA_SRC_DIR}/undohistorymodel.h
    ${SIGMA_SRC_DIR}/undostack.cpp
    ${SIGMA_SRC_DIR}/undostack.h
)

# CMAKE_SOURCE_DIR is added so that the project-root-relative includes used
# throughout src/ (e.g. "third_party/exprtk/exprtk.hpp") resolve correctly,
# just like they do for the Sigma target itself.
target_include_directories( SigmaCore PUBLIC
    ${CMAKE_SOURCE_DIR}
    ${SIGMA_SRC_DIR}
)

target_link_libraries( SigmaCore PUBLIC
    Qt6::Core
    Qt6::Concurrent
    Qt6::QmlIntegration
    ThirdParty
)

if ( MSVC )
    target_compile_options( SigmaCore PRIVATE /bigobj )
endif()

# Helper function that declares one Qt Test executable per tst_*.cpp file and
# registers it with CTest.
function( sigma_add_test test_name )
    add_executable( ${test_name} ${test_name}.cpp )
    target_link_libraries( ${test_name} PRIVATE SigmaCore Qt6::Test )
    add_test( NAME ${test_name} COMMAND ${test_name} )
    # Keep test runs hermetic: redirect QStandardPaths/QSettings locations
    # used by ApplicationSettings/UncertaintyCalculation to a private,
    # per-test directory instead of the real user configuration.
    set_tests_properties( ${test_name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endfunction()

sigma_add_test( tst_distribution )
sigma_add_test( tst_statistics )
sigma_add_test( tst_stringutils )
sigma_add_test( tst_inputparameter )
sigma_add_test( tst_correlation )
sigma_add_test( tst_uncertaintycomponent )
sigma_add_test( tst_outputparameter )
sigma_add_test( tst_samplebatch )
sigma_add_test( tst_montecarlo )
sigma_add_test( tst_diffutil )
sigma_add_test( tst_uncertaintycalculation )

# The end-to-end regression tests on all demo projects
add_executable( tst_demoprojects
    tst_demoprojects.cpp
    regressionconfig.cpp
    regressionconfig.h
    resultcomparator.cpp
    resultcomparator.h
)

target_link_libraries( tst_demoprojects PRIVATE Qt6::Test )

# The regression tests run the Sigma executable as a separate process
add_dependencies( tst_demoprojects Sigma )

target_compile_definitions( tst_demoprojects PRIVATE
    SIGMA_EXECUTABLE="$<TARGET_FILE:Sigma>"
    SIGMA_REGRESSION_CONFIG="${CMAKE_SOURCE_DIR}/tests/demo_projects.json"
)

add_test( NAME demo_projects COMMAND tst_demoprojects )
set_tests_properties( demo_projects PROPERTIES TIMEOUT 3600 )
