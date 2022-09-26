cxx_shared_library(biz_ptzf_if
  "event_router_if;common_core;ptzf_biz_message_if;ptzf_status_if;pan_tilt_limit_position;visca_status_if;ptz_trace_status_if"
  biz_ptzf_if.cpp)

cxx_static_library(biz_ptzf_if_mock "" biz_ptzf_if_mock.cpp)

list(APPEND biz_ptzf_if_with_fake_libs event_router_if_mock)
list(APPEND biz_ptzf_if_with_fake_libs common_core)
list(APPEND biz_ptzf_if_with_fake_libs ptzf_biz_message_if_mock)
list(APPEND biz_ptzf_if_with_fake_libs ptzf_status_if_mock)
list(APPEND biz_ptzf_if_with_fake_libs pan_tilt_limit_position)
list(APPEND biz_ptzf_if_with_fake_libs visca_status_if_mock)
list(APPEND biz_ptzf_if_with_fake_libs ptz_trace_status_if_mock)
cxx_shared_library(biz_ptzf_if_with_fake
  "${biz_ptzf_if_with_fake_libs}"
  biz_ptzf_if.cpp
)

if(CMAKE_CROSSCOMPILING)
  if(G8_BUILD)
    cxx_executable_no_install(biz_ptzf_if_target_test
      "biz_ptzf_if;model_info_rc;common_main"
      test/biz_ptzf_if_target_test.cpp)
    cxx_executable_no_install(biz_trace_if_target_test
      "biz_ptzf_if;model_info_rc;common_main"
      test/biz_trace_if_target_test.cpp)
  else(G8_BUILD)
    cxx_executable_no_install(biz_ptzf_if_target_test
      "biz_ptzf_if;common_main"
      test/biz_ptzf_if_target_test.cpp)
  endif(G8_BUILD)
endif(CMAKE_CROSSCOMPILING)

list(APPEND biz_ptzf_if_test_libs event_router_if_mock)
list(APPEND biz_ptzf_if_test_libs common_core)
list(APPEND biz_ptzf_if_test_libs ptzf_status)
list(APPEND biz_ptzf_if_test_libs error_notifier_message_if)
list(APPEND biz_ptzf_if_test_libs ptzf_status_if)
list(APPEND biz_ptzf_if_test_libs pan_tilt_limit_position)
list(APPEND biz_ptzf_if_test_libs ptzf_biz_message_if_mock)
list(APPEND biz_ptzf_if_test_libs visca_status_if_mock)
list(APPEND biz_ptzf_if_test_libs ptz_trace_status_if)
cxx_gmock_executable(biz_ptzf_if_test
 "${biz_ptzf_if_test_libs}"
  biz_ptzf_if.cpp
  test/biz_ptzf_if_test.cpp)

add_library_tests(biz_ptzf_if biz_ptzf_if_test)

cxx_static_library(biz_ptzf_message_handler
  "common_core;visca_server_message_if"
  biz_ptzf_message_handler.cpp
  )

cxx_gmock_executable(biz_ptzf_message_handler_test
  "common_core;visca_server_message_if;bizglobal"
  test/biz_ptzf_message_handler_test.cpp
  biz_ptzf_message_handler.cpp
  )

add_library_tests(biz_ptzf_message_handler biz_ptzf_message_handler_test)

cxx_static_library(biz_ptzf_controller_entry
  "bizglobal;biz_ptzf_message_handler;ptzf_controller_entry"
  biz_ptzf_controller_entry.cpp)

# add subdirectories

enable_coverage_report()
enable_coverity()
enable_verify_coding_rule("./")
