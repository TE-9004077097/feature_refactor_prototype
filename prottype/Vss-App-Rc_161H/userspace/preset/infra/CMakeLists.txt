include_parent_directories()

if(CMAKE_CROSSCOMPILING)
  if(DIADEM_BUILD)
    cxx_static_library(preset_capability_infra_if "config_diadem_backup_if"
      preset_capability_infra_if.cpp
      )
    list(APPEND preset_database_backup_infra_if_dependencies visca_config_core)
    list(APPEND preset_database_backup_infra_if_dependencies preset_database_backup)
    list(APPEND preset_database_backup_infra_if_dependencies common_diadem_uipc_if)
    list(APPEND preset_database_backup_infra_if_dependencies common_core)
    cxx_static_library(preset_database_backup_infra_if "${preset_database_backup_infra_if_dependencies}"
      preset_database_backup_infra_if.cpp
      )
  elseif(G8_BUILD)
    cxx_static_library(preset_capability_infra_if "visca_config_core"
      preset_capability_infra_if_g8.cpp
      )
    if(APPRC_CAMERA_PROTOCOL_VISCA)
      list(APPEND database_initialize_infra_if_dependencies visca_config_core)
      cxx_static_library(database_initialize_infra_if
        "${database_initialize_infra_if_dependencies}"
        database_initialize_infra_if.cpp
        )
      list(APPEND preset_infra_if_dependencies visca_server_message_if)
      list(APPEND preset_infra_if_dependencies visca_config_core)
      cxx_static_library(preset_infra_if "${preset_infra_if_dependencies}"
        preset_infra_if_g8.cpp
        )
      cxx_static_library(preset_status_config_infra_if "model_info_rc;visca_config_core;common_core" preset_status_config_infra_if.cpp)
      list(APPEND preset_database_backup_infra_if_dependencies visca_config_core)
      list(APPEND preset_database_backup_infra_if_dependencies visca_config_preset_database_backup)
      cxx_static_library(preset_database_backup_infra_if "${preset_database_backup_infra_if_dependencies}"
        preset_database_backup_infra_if_g8.cpp
        )
    elseif(APPRC_CAMERA_PROTOCOL_PTP)
      list(APPEND database_initialize_infra_if_dependencies visca_config_core)
      cxx_static_library(database_initialize_infra_if
        "${database_initialize_infra_if_dependencies}"
        database_initialize_infra_if_marco.cpp
        )
      list(APPEND preset_infra_if_dependencies visca_server_message_if)
      list(APPEND preset_infra_if_dependencies visca_config_core)
      cxx_static_library(preset_infra_if "${preset_infra_if_dependencies}"
        preset_infra_if_marco.cpp
        )
      cxx_static_library(preset_status_config_infra_if "model_info_rc;visca_config_core;common_core" preset_status_config_infra_if_marco.cpp)
      list(APPEND preset_database_backup_infra_if_dependencies visca_config_core)
      list(APPEND preset_database_backup_infra_if_dependencies visca_config_preset_database_backup)
      cxx_static_library(preset_database_backup_infra_if "${preset_database_backup_infra_if_dependencies}"
        preset_database_backup_infra_if_marco.cpp
        )
    endif(APPRC_CAMERA_PROTOCOL_VISCA)
  endif(DIADEM_BUILD)
endif(CMAKE_CROSSCOMPILING)

if(DIADEM_BUILD)
  list(APPEND preset_database_backup_infra_if_test_dependencies preset_database_backup)
  list(APPEND preset_database_backup_infra_if_test_dependencies common_core)
  list(APPEND preset_database_backup_infra_if_test_dependencies common_diadem_uipc_if)
  list(APPEND preset_database_backup_infra_if_test_dependencies model_info_rc)
  list(APPEND preset_database_backup_infra_if_test_dependencies config_db_mock)
  list(APPEND preset_database_backup_infra_if_test_dependencies config_diadem_backup_if_mock)

  cxx_gmock_executable(preset_capability_infra_if_test "config_diadem_backup_if_mock"
    test/preset_capability_infra_if_test.cpp
    preset_capability_infra_if.cpp
    )
  cxx_gmock_executable(preset_database_backup_infra_if_test "${preset_database_backup_infra_if_test_dependencies}"
    test/preset_database_backup_infra_if_test.cpp
    preset_database_backup_infra_if.cpp)

  add_library_tests(preset_capability_infra_if preset_capability_infra_if_test)
  add_library_tests(preset_database_backup_infra_if preset_database_backup_infra_if_test)
  cxx_gmock_executable(preset_status_config_infra_if_test "model_info_rc;visca_config_core;common_core"
    test/preset_status_config_infra_if_test.cpp
    preset_status_config_infra_if.cpp)
  cxx_gmock_executable(database_initialize_infra_if_test "visca_config_fake"
    test/database_initialize_infra_if_test.cpp
    database_initialize_infra_if.cpp)
  add_library_tests(preset_status_config_infra_if preset_status_config_infra_if_test)
  add_library_tests(database_initialize_infra_if database_initialize_infra_if_test)

  add_custom_target(preset_test_rc_data
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/dbfile
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/config/dbfile/num_BRC1000BASE.db ${CMAKE_CURRENT_BINARY_DIR}/dbfile
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/config/dbfile/enum_BRC1000BASE.db ${CMAKE_CURRENT_BINARY_DIR}/dbfile
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/config/dbfile/bool_BRC1000BASE.db ${CMAKE_CURRENT_BINARY_DIR}/dbfile
    )
  add_dependencies(preset_database_backup_infra_if_test preset_test_rc_data)
elseif(G8_BUILD)
  if(APPRC_CAMERA_PROTOCOL_VISCA)
    list(APPEND database_initialize_infra_if_test_dependencies visca_config_fake)
    cxx_gmock_executable(database_initialize_infra_if_test
      "${database_initialize_infra_if_test_dependencies}"
      test/database_initialize_infra_if_test.cpp
      database_initialize_infra_if.cpp)
    add_library_tests(database_initialize_infra_if database_initialize_infra_if_test)

    list(APPEND preset_database_backup_infra_if_test_dependencies common_core)
    list(APPEND preset_database_backup_infra_if_test_dependencies model_info_rc)
    list(APPEND preset_database_backup_infra_if_test_dependencies visca_config_preset_database_backup)
    list(APPEND preset_database_backup_infra_if_test_dependencies visca_config_db_mock)
    list(APPEND preset_database_backup_infra_if_test_dependencies visca_config_fake)
    list(APPEND preset_database_backup_infra_if_test_dependencies visca_config_database_manager_mock)
    cxx_gmock_executable(preset_database_backup_infra_if_test "${preset_database_backup_infra_if_test_dependencies}"
      test/preset_database_backup_infra_if_g8_test.cpp
      preset_database_backup_infra_if_g8.cpp)
    add_library_tests(preset_database_backup_infra_if preset_database_backup_infra_if_test)

    cxx_gmock_executable(preset_status_config_infra_if_test "model_info_rc;visca_config_fake;common_core"
      test/preset_status_config_infra_if_test.cpp
      preset_status_config_infra_if.cpp)
    add_library_tests(preset_status_config_infra_if preset_status_config_infra_if_test)

    list(APPEND preset_infra_if_test_dependencies visca_server_message_if_mock)
    list(APPEND preset_infra_if_test_dependencies visca_config_fake)
    cxx_gmock_executable(preset_infra_if_test "${preset_infra_if_test_dependencies}"
      test/preset_infra_if_g8_test.cpp
      preset_infra_if_g8.cpp)
    add_library_tests(preset_infra_if preset_infra_if_test)
    cxx_static_library(preset_infra_entry
      "common_core"
      preset_infra_controller_entry_g8.cpp)
  elseif(APPRC_CAMERA_PROTOCOL_PTP)
    list(APPEND database_initialize_infra_if_test_dependencies visca_config_fake)
    cxx_gmock_executable(database_initialize_infra_if_test
      "${database_initialize_infra_if_test_dependencies}"
      test/database_initialize_infra_if_marco_test.cpp
      database_initialize_infra_if_marco.cpp)
    add_library_tests(database_initialize_infra_if database_initialize_infra_if_test)

    list(APPEND preset_database_backup_infra_if_test_dependencies common_core)
    list(APPEND preset_database_backup_infra_if_test_dependencies model_info_rc)
    list(APPEND preset_database_backup_infra_if_test_dependencies visca_config_preset_database_backup)
    list(APPEND preset_database_backup_infra_if_test_dependencies visca_config_db_mock)
    list(APPEND preset_database_backup_infra_if_test_dependencies visca_config_fake)
    list(APPEND preset_database_backup_infra_if_test_dependencies visca_config_database_manager_mock)
    list(APPEND preset_database_backup_infra_if_test_dependencies ptp_driver_command_if)
    list(APPEND preset_database_backup_infra_if_test_dependencies biz_ptzf_if)
    cxx_gmock_executable(preset_database_backup_infra_if_test "${preset_database_backup_infra_if_test_dependencies}"
      test/preset_database_backup_infra_if_marco_test.cpp
      preset_database_backup_infra_message_handler_marco_fake.cpp
      preset_database_backup_infra_if_marco.cpp)
    add_library_tests(preset_database_backup_infra_if preset_database_backup_infra_if_test)

    list(APPEND preset_database_backup_infra_message_handler_dependencies common_core)
    list(APPEND preset_database_backup_infra_message_handler_dependencies ptp_error_checker)
    list(APPEND preset_database_backup_infra_message_handler_dependencies ptp_driver_command_if)
    cxx_static_library(preset_database_backup_infra_message_handler
      "${preset_database_backup_infra_message_handler_dependencies}"
      preset_database_backup_infra_message_handler_marco.cpp
      )

    list(APPEND preset_database_backup_infra_message_handler_test_dependencies common_core)
    list(APPEND preset_database_backup_infra_message_handler_test_dependencies ptp_error_checker)
    list(APPEND preset_database_backup_infra_message_handler_test_dependencies visca_server_message_if_mock)
    list(APPEND preset_database_backup_infra_message_handler_test_dependencies biz_ptzf_if)
    cxx_gmock_executable(preset_database_backup_infra_message_handler_test
      "${preset_database_backup_infra_message_handler_test_dependencies}"
      test/preset_database_backup_infra_message_handler_marco_test.cpp
      preset_database_backup_infra_message_handler_marco.cpp
      $<TARGET_OBJECTS:ptp_drv_cmd_if_fake_obj>
      )
    add_library_tests(preset_database_backup_infra_message_handler preset_database_backup_infra_message_handler_test)

    cxx_gmock_executable(preset_status_config_infra_if_test "model_info_rc;visca_config_fake;common_core"
      test/preset_status_config_infra_if_marco_test.cpp
      preset_status_config_infra_if_marco.cpp)
    add_library_tests(preset_status_config_infra_if preset_status_config_infra_if_test)

    list(APPEND preset_infra_if_test_dependencies visca_server_message_if_mock)
    list(APPEND preset_infra_if_test_dependencies visca_config_fake)
    list(APPEND preset_infra_if_test_dependencies biz_ptzf_if)
    list(APPEND preset_infra_if_test_dependencies ptp_driver_command_if)
    cxx_gmock_executable(preset_infra_if_test "${preset_infra_if_test_dependencies}"
      test/preset_infra_if_marco_test.cpp
      preset_infra_message_handler_marco_fake.cpp
      preset_infra_if_marco.cpp
      sequence_id_controller.cpp
      )
    add_library_tests(preset_infra_if preset_infra_if_test)

    list(APPEND preset_infra_message_handler_dependencies common_core)
    list(APPEND preset_infra_message_handler_dependencies ptp_error_checker)
    list(APPEND preset_infra_message_handler_dependencies ptp_driver_command_if)
    list(APPEND preset_infra_message_handler_dependencies visca_server_message_if)
    list(APPEND preset_infra_message_handler_dependencies biz_ptzf_if)
    cxx_static_library(preset_infra_message_handler
      "${preset_infra_message_handler_dependencies}"
      preset_infra_message_handler_marco.cpp
      sequence_id_controller.cpp
      )

    list(APPEND preset_infra_message_handler_test_dependencies common_core)
    list(APPEND preset_infra_message_handler_test_dependencies ptp_error_checker)
    list(APPEND preset_infra_message_handler_test_dependencies visca_server_message_if_mock)
    list(APPEND preset_infra_message_handler_test_dependencies biz_ptzf_if)
    cxx_gmock_executable(preset_infra_message_handler_test
      "${preset_infra_message_handler_test_dependencies}"
      test/preset_infra_message_handler_marco_test.cpp
      preset_infra_message_handler_marco.cpp
      sequence_id_controller.cpp
      $<TARGET_OBJECTS:ptp_drv_cmd_if_fake_obj>
      )
    add_library_tests(preset_infra_message_handler preset_infra_message_handler_test)

    list(APPEND preset_infra_entry_dependencies common_core)
    list(APPEND preset_infra_entry_dependencies preset_database_backup_infra_message_handler)
    list(APPEND preset_infra_entry_dependencies preset_infra_message_handler)
    cxx_static_library(preset_infra_entry
      "${preset_infra_entry_dependencies}"
      preset_infra_controller_entry_marco.cpp)
  endif(APPRC_CAMERA_PROTOCOL_VISCA)

  add_library_tests(preset_status_config_infra_if preset_status_config_infra_if_test)

  cxx_gmock_executable(preset_capability_infra_if_test "visca_config_fake"
    test/preset_capability_infra_if_g8_test.cpp
    preset_capability_infra_if_g8.cpp
    )
  add_library_tests(preset_capability_infra_if preset_capability_infra_if_test)

  add_custom_target(preset_test_rc_data
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/dbfile
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/config/dbfile/num_BRC1000BASE.db ${CMAKE_CURRENT_BINARY_DIR}/dbfile
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/config/dbfile/enum_BRC1000BASE.db ${CMAKE_CURRENT_BINARY_DIR}/dbfile
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/config/dbfile/bool_BRC1000BASE.db ${CMAKE_CURRENT_BINARY_DIR}/dbfile
    )
  add_dependencies(preset_database_backup_infra_if_test preset_test_rc_data)
endif(DIADEM_BUILD)
