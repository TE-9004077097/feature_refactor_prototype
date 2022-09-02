include_parent_directories()

list(APPEND ptz_trace_backup_infra_if_libs common_core)
list(APPEND ptz_trace_backup_infra_if_libs common_crypt)
list(APPEND ptz_trace_backup_infra_if_libs ptz_trace_data)
list(APPEND ptz_trace_backup_infra_if_sources ptz_trace_backup_infra_if.cpp)
list(APPEND ptz_trace_backup_infra_if_sources ptz_trace_backup_infra_meta.cpp)
list(APPEND ptz_trace_backup_infra_if_sources ptz_trace_backup_infra_item.cpp)

cxx_gmock_executable(ptz_trace_backup_infra_if_test
  "${ptz_trace_backup_infra_if_libs}"
  ${ptz_trace_backup_infra_if_sources}
  ptz_trace_backup_infra_meta_impl_mock.cpp
  ptz_trace_backup_infra_item_impl_mock.cpp
  test/ptz_trace_backup_infra_if_test.cpp
)
add_library_tests(ptz_trace_backup_infra_if ptz_trace_backup_infra_if_test)

if(CMAKE_CROSSCOMPILING)
  cxx_static_library(ptz_trace_status_backup_infra_if "visca_config_core" ptz_trace_status_backup_infra_if.cpp)
  if(DIADEM_BUILD)
    cxx_static_library(pt_micon_power_infra_if
      "upm_wrapper;common_diadem_uipc_if;visca_server_message_if"
      pt_micon_power_infra_if.cpp
      pt_micon_power_infra_if_uipc_receiver.cpp)
    cxx_static_library(ptz_trace_backup_infra_if
      "${ptz_trace_backup_infra_if_libs};common_file_operation;common_diadem_uipc_if"
      ${ptz_trace_backup_infra_if_sources}
      ptz_trace_backup_infra_meta_impl_backup.cpp
      ptz_trace_backup_infra_item_impl_file.cpp)
    cxx_static_library(ptz_trace_status_infra_if
      "model_info_rc;visca_config_core"
      ptz_trace_status_infra_if.cpp
      ptz_trace_status_infra_if_status_holder.cpp)
    cxx_static_library(ptzf_debug_info_infra_if "config_diadem_backup_if;visca_config_core;common_core" ptzf_debug_info_infra_if.cpp)
    list(APPEND ptzf_status_infra_if_sources ptzf_status_infra_if.cpp)
    list(APPEND ptzf_config_infra_if_sources ptzf_config_infra_if.cpp)
    list(APPEND ptzf_status_infra_if_sources ptzf_status_infra_if_no_save_last_position.cpp)
    cxx_static_library(ptzf_status_infra_if "model_info_rc;visca_config_core;ptz_trace_status_backup_infra_if" ${ptzf_status_infra_if_sources})
    cxx_static_library(ptzf_config_infra_if "model_info_rc;visca_config_core;ptz_trace_status_backup_infra_if" ${ptzf_config_infra_if_sources})
    cxx_static_library(ptzf_capability_infra_if "ptzf_status_infra_if;ptzf_config_infra_if" ptzf_capability_infra_if.cpp)
    cxx_static_library(ptzf_biz_message_infra_if
      "visca_config_core;common_core;visca_server_message_if"
      ptzf_biz_message_infra_if.cpp)
  elseif(G8_BUILD)
    cxx_static_library(ptz_trace_backup_infra_if
      "${ptz_trace_backup_infra_if_libs};common_file_operation;visca_config_core;visca_config_ptz_trace_database_backup"
      ${ptz_trace_backup_infra_if_sources}
      ptz_trace_backup_infra_meta_impl_config.cpp
      ptz_trace_backup_infra_item_impl_file.cpp
    )
    cxx_static_library(ptz_trace_status_infra_if
      "model_info_rc;visca_config_core;ptz_trace_status_backup_infra_if"
      ptz_trace_status_infra_if.cpp
      ptz_trace_status_infra_if_status_holder.cpp)
      list(APPEND ptzf_status_infra_if_sources ptzf_status_infra_if.cpp)
      list(APPEND ptzf_status_infra_if_sources ptzf_config_infra_if.cpp)
      if(APPRC_PANTILTT_INITIAL_POSITION_LAST_STOPPED)
      list(APPEND ptzf_config_infra_if_sources ptzf_status_infra_if_save_last_position.cpp)
    else(APPRC_PANTILTT_INITIAL_POSITION_LAST_STOPPED)
      list(APPEND ptzf_status_infra_if_sources ptzf_status_infra_if_no_save_last_position.cpp)
    endif(APPRC_PANTILTT_INITIAL_POSITION_LAST_STOPPED)
    cxx_static_library(ptzf_status_infra_if "model_info_rc;visca_config_core;ptz_trace_status_backup_infra_if;ptzf_zoom_infra_if" ${ptzf_status_infra_if_sources})
    cxx_static_library(ptzf_config_infra_if "model_info_rc;visca_config_core;ptz_trace_status_backup_infra_if;ptzf_zoom_infra_if" ${ptzf_config_infra_if_sources})
    cxx_static_library(ptzf_biz_message_infra_if
      "visca_config_core;common_core;visca_server_message_if"
      ptzf_biz_message_infra_if.cpp)
    cxx_static_library(ptzf_backup_infra_if "visca_config_core" ptzf_backup_infra_if.cpp)
    if(APPRC_CAMERA_PROTOCOL_VISCA)
      cxx_static_library(ptzf_capability_infra_if "ptzf_status_infra_if;ptzf_config_infra_if;visca_config_core" ptzf_capability_infra_if_g8.cpp)
      cxx_static_library(ptzf_input_infra_if "" ptzf_input_infra_if_fake.cpp)
      cxx_static_library(pt_micon_power_infra_if "visca_server_message_if;bizglobal" pt_micon_power_infra_if_g8.cpp)
      cxx_static_library(ptzf_finalize_infra_if "ptzf_finalize_infra_message_handler" ptzf_finalize_infra_if_g8.cpp)
      cxx_static_library(ptzf_initialize_infra_if "ptzf_initialize_infra_message_handler" ptzf_initialize_infra_if_g8.cpp)
      list(APPEND ptzf_pan_tilt_infra_if_dependencies ptzf_pan_tilt_infra_message_handler)
      list(APPEND ptzf_pan_tilt_infra_if_dependencies ptzf_status_if)
      list(APPEND ptzf_pan_tilt_infra_if_dependencies ptz_updater)
      cxx_static_library(ptzf_pan_tilt_infra_if "${ptzf_pan_tilt_infra_if_dependencies}" ptzf_pan_tilt_infra_if_g8.cpp)
      cxx_static_library(ptzf_zoom_infra_if "ptzf_zoom_infra_message_handler" ptzf_zoom_infra_if_g8.cpp)
      cxx_static_library(ptzf_focus_infra_if "ptzf_focus_infra_message_handler" ptzf_focus_infra_if_g8.cpp)
      cxx_static_library(ptzf_if_clear_infra_if "ptzf_if_clear_infra_message_handler" ptzf_if_clear_infra_if_g8.cpp)
      cxx_static_library(ptzf_pan_tilt_lock_infra_if "" ptzf_pan_tilt_lock_infra_if_g8.cpp)
      cxx_static_library(ptzf_initialize_infra_message_handler
          "visca_server_message_if;event_router_if;ptzf_status_if;ptzf_status;ptzf_config;error_notifier_message_if"
          ptzf_initialize_infra_message_handler_g8.cpp
          sequence_id_controller.cpp)
      cxx_static_library(ptzf_finalize_infra_message_handler
          "common_core;common_timer;visca_server_message_if;ptzf_backup_infra_if;ptz_updater"
          ptzf_finalize_infra_message_handler_g8.cpp
          sequence_id_controller.cpp)
      cxx_static_library(ptzf_pan_tilt_infra_message_handler
          "visca_server_message_if"
          ptzf_pan_tilt_infra_message_handler_g8.cpp
          sequence_id_controller.cpp)
      cxx_static_library(ptzf_zoom_infra_message_handler
          "visca_server_message_if"
          ptzf_zoom_infra_message_handler_g8.cpp
          sequence_id_controller.cpp)
      cxx_static_library(ptzf_focus_infra_message_handler
          "visca_server_message_if"
          ptzf_focus_infra_message_handler_g8.cpp
          sequence_id_controller.cpp)
      cxx_static_library(ptzf_if_clear_infra_message_handler
          "visca_server_message_if"
          ptzf_if_clear_infra_message_handler_g8.cpp
          sequence_id_controller.cpp)
      cxx_static_library(ptzf_debug_info_infra_if "common_core" ptzf_debug_info_infra_if_g8.cpp)
    else(APPRC_CAMERA_PROTOCOL_PTP)
      cxx_static_library(ptzf_capability_infra_if "ptzf_status_infra_if;ptzf_config_infra_if;visca_config_core" ptzf_capability_infra_if_marco.cpp)
      if(VSS_INTEGRATION_BUILD OR TARGET gpio_if)
        cxx_static_library(ptzf_input_infra_if "gpio_if" ptzf_input_infra_if_marco.cpp)
      else(VSS_INTEGRATION_BUILD OR TARGET gpio_if)
        cxx_static_library(ptzf_input_infra_if "" ptzf_input_infra_if_fake.cpp)
      endif(VSS_INTEGRATION_BUILD OR TARGET gpio_if)
      cxx_static_library(pt_micon_power_infra_if "ptzf_message_if;bizglobal;ptzf_status_infra_if;ptzf_config_infra_if;ptzf_input_infra_if" pt_micon_power_infra_if_marco.cpp)
      cxx_static_library(ptzf_initialize_infra_if "ptzf_initialize_infra_message_handler" ptzf_initialize_infra_if_marco.cpp)
      cxx_static_library(ptzf_finalize_infra_if "ptzf_finalize_infra_message_handler" ptzf_finalize_infra_if_marco.cpp)
      list(APPEND ptzf_pan_tilt_infra_if_dependencies ptzf_pan_tilt_infra_message_handler)
      list(APPEND ptzf_pan_tilt_infra_if_dependencies ptzf_status_if)
      list(APPEND ptzf_pan_tilt_infra_if_dependencies ptzf_input_infra_if)
      cxx_static_library(ptzf_pan_tilt_infra_if "${ptzf_pan_tilt_infra_if_dependencies}" ptzf_pan_tilt_infra_if_marco.cpp)
      cxx_static_library(ptzf_zoom_infra_if "ptzf_zoom_infra_message_handler" ptzf_zoom_infra_if_marco.cpp)
      cxx_static_library(ptzf_focus_infra_if "ptzf_focus_infra_message_handler" ptzf_focus_infra_if_marco.cpp)
      cxx_static_library(ptzf_if_clear_infra_if "ptzf_if_clear_infra_message_handler" ptzf_if_clear_infra_if_marco.cpp)
      list(APPEND ptzf_pan_tilt_lock_infra_if_dependencies ptzf_pan_tilt_lock_infra_message_handler)
      cxx_static_library(ptzf_pan_tilt_lock_infra_if
        "${ptzf_pan_tilt_lock_infra_if_dependencies}"
        ptzf_pan_tilt_lock_infra_if_marco.cpp)
      cxx_static_library(ptzf_ptp_accessor
        "bizglobal;ptp_driver_command_if"
        ptzf_ptp_accessor.cpp)
      list(APPEND pan_tilt_initialize_infra_message_handler_dependencies visca_server_message_if)
      list(APPEND pan_tilt_initialize_infra_message_handler_dependencies event_router_if)
      list(APPEND pan_tilt_initialize_infra_message_handler_dependencies ptzf_status_if)
      list(APPEND pan_tilt_initialize_infra_message_handler_dependencies ptzf_status)
      list(APPEND pan_tilt_initialize_infra_message_handler_dependencies ptzf_config)
      list(APPEND pan_tilt_initialize_infra_message_handler_dependencies error_notifier_message_if)
      list(APPEND pan_tilt_initialize_infra_message_handler_dependencies ptzf_ptp_accessor)
      list(APPEND pan_tilt_initialize_infra_message_handler_dependencies ptzf_pan_tilt_lock_infra_if)
      cxx_static_library(ptzf_initialize_infra_message_handler
          "${pan_tilt_initialize_infra_message_handler_dependencies}"
          ptzf_initialize_infra_message_handler_marco.cpp
          sequence_id_controller.cpp)
      list(APPEND pan_tilt_finalize_infra_message_handler_dependencies common_core)
      list(APPEND pan_tilt_finalize_infra_message_handler_dependencies common_timer)
      list(APPEND pan_tilt_finalize_infra_message_handler_dependencies visca_server_message_if)
      list(APPEND pan_tilt_finalize_infra_message_handler_dependencies ptzf_backup_infra_if)
      list(APPEND pan_tilt_finalize_infra_message_handler_dependencies ptz_updater)
      list(APPEND pan_tilt_finalize_infra_message_handler_dependencies ptzf_status_infra_if)
      list(APPEND pan_tilt_finalize_infra_message_handler_dependencies ptzf_config_infra_if)
      cxx_static_library(ptzf_finalize_infra_message_handler
          "${pan_tilt_finalize_infra_message_handler_dependencies}"
          ptzf_finalize_infra_message_handler_marco.cpp
          sequence_id_controller.cpp)
      # FIXME: tentative use fake debug info to skip power on sequence
      list(APPEND pan_tilt_infra_message_handler_dependencies common_core)
      list(APPEND pan_tilt_infra_message_handler_dependencies visca_server_message_if)
      list(APPEND pan_tilt_infra_message_handler_dependencies ptp_error_checker)
      list(APPEND pan_tilt_infra_message_handler_dependencies ptp_driver_command_if)
      list(APPEND pan_tilt_infra_message_handler_dependencies ptzf_status_infra_if)
      list(APPEND pan_tilt_infra_message_handler_dependencies ptzf_config_infra_if)
      cxx_static_library(ptzf_pan_tilt_infra_message_handler
        "${pan_tilt_infra_message_handler_dependencies}"
        ptzf_pan_tilt_infra_message_handler_marco.cpp
        sequence_id_controller.cpp
        )
      list(APPEND ptzf_focus_infra_message_handler_dependencies common_core)
      list(APPEND ptzf_focus_infra_message_handler_dependencies ptp_error_checker)
      list(APPEND ptzf_focus_infra_message_handler_dependencies ptp_driver_command_if)
      list(APPEND ptzf_focus_infra_message_handler_dependencies visca_status_if)
      cxx_static_library(ptzf_focus_infra_message_handler
        "${ptzf_focus_infra_message_handler_dependencies}"
        ptzf_focus_infra_message_handler_marco.cpp
        )
      list(APPEND ptzf_zoom_infra_message_handler_dependencies common_core)
      list(APPEND ptzf_zoom_infra_message_handler_dependencies ptp_error_checker)
      list(APPEND ptzf_zoom_infra_message_handler_dependencies ptp_driver_command_if)
      list(APPEND ptzf_zoom_infra_message_handler_dependencies visca_status_if)
      cxx_static_library(ptzf_zoom_infra_message_handler
        "${ptzf_zoom_infra_message_handler_dependencies}"
        ptzf_zoom_infra_message_handler_marco.cpp
        )
      cxx_static_library(ptzf_if_clear_infra_message_handler
        "visca_server_message_if"
        ptzf_if_clear_infra_message_handler_marco.cpp
        sequence_id_controller.cpp)
      message(WARNING "using fake ptzf debug info")
      cxx_static_library(ptzf_debug_info_infra_if "" ../ptzf_debug_info_infra_if_fake.cpp)

      # Pan-Tilt Lock
      list(APPEND ptzf_pan_tilt_lock_infra_message_handler_dependencies common_core)
      list(APPEND ptzf_pan_tilt_lock_infra_message_handler_dependencies bizglobal)
      cxx_static_library(ptzf_pan_tilt_lock_infra_message_handler
        "${ptzf_pan_tilt_lock_infra_message_handler_dependencies}"
        ptzf_pan_tilt_lock_infra_message_handler_marco.cpp
        )
    endif(APPRC_CAMERA_PROTOCOL_VISCA)
  endif(DIADEM_BUILD)
else(CMAKE_CROSSCOMPILING)
  cxx_static_library(ptz_trace_status_backup_infra_if "" ptz_trace_status_backup_infra_if_fake.cpp)
  if(APPRC_CAMERA_PROTOCOL_VISCA)
    cxx_static_library(ptzf_initialize_infra_message_handler
        "visca_server_message_if;event_router_if;ptzf_status_if;ptzf_status;ptzf_config;error_notifier_message_if"
        ptzf_initialize_infra_message_handler_g8.cpp)
    cxx_static_library(ptzf_finalize_infra_message_handler
        "visca_server_message_if;event_router_if;ptzf_status_if;ptzf_status;ptzf_config;error_notifier_message_if"
        ptzf_finalize_infra_message_handler_g8.cpp)
    cxx_static_library(ptzf_pan_tilt_infra_message_handler
        "visca_server_message_if"
        ptzf_pan_tilt_infra_message_handler_g8_fake.cpp
        sequence_id_controller.cpp)
    cxx_static_library(ptzf_zoom_infra_message_handler
        "visca_server_message_if"
        ptzf_zoom_infra_message_handler_g8_fake.cpp
        sequence_id_controller.cpp)
    cxx_static_library(ptzf_focus_infra_message_handler
        "visca_server_message_if"
        ptzf_focus_infra_message_handler_g8_fake.cpp
        sequence_id_controller.cpp)
    cxx_static_library(ptzf_if_clear_infra_message_handler
        "visca_server_message_if"
        ptzf_if_clear_infra_message_handler_g8_fake.cpp
        sequence_id_controller.cpp)
    cxx_static_library(ptzf_input_infra_if "" ptzf_input_infra_if_fake.cpp)
#    cxx_static_library(ptzf_capability_infra_if_g8
#        "ptzf_status_infra_if_mock;visca_config_fake"
#        ptzf_capability_infra_if_g8.cpp)
  else(APPRC_CAMERA_PROTOCOL_PTP)
    cxx_static_library(ptzf_initialize_infra_message_handler
      "visca_server_message_if;event_router_if;ptzf_status_if;ptzf_status;ptzf_config;error_notifier_message_if"
      ptzf_initialize_infra_message_handler_marco.cpp)
    cxx_static_library(ptzf_finalize_infra_message_handler
      "visca_server_message_if;event_router_if;ptzf_status_if;ptzf_status;ptzf_config;error_notifier_message_if;ptzf_status_infra_if;ptzf_config_infra_if"
      ptzf_finalize_infra_message_handler_marco.cpp)
    list(APPEND ptzf_pan_tilt_infra_message_handler_dependencies visca_server_message_if)
    cxx_static_library(ptzf_pan_tilt_infra_message_handler
      "${ptzf_pan_tilt_infra_message_handler_dependencies}"
      ptzf_pan_tilt_infra_message_handler_marco_fake.cpp
      sequence_id_controller.cpp)
    cxx_static_library(ptzf_focus_infra_message_handler
      ""
      ptzf_focus_infra_message_handler_marco_fake.cpp
      )
    cxx_static_library(ptzf_zoom_infra_message_handler
      ""
      ptzf_zoom_infra_message_handler_marco_fake.cpp
      )
    cxx_static_library(ptzf_if_clear_infra_message_handler
      "visca_server_message_if"
      ptzf_if_clear_infra_message_handler_marco_fake.cpp
      sequence_id_controller.cpp)
    cxx_static_library(ptzf_input_infra_if "" ptzf_input_infra_if_fake.cpp)

    # Pan-Tilt Lock
    list(APPEND ptzf_pan_tilt_lock_infra_message_handler_dependencies common_core)
    list(APPEND ptzf_pan_tilt_lock_infra_message_handler_dependencies bizglobal)
    cxx_static_library(ptzf_pan_tilt_lock_infra_message_handler
      "${ptzf_pan_tilt_lock_infra_message_handler_dependencies}"
      ptzf_pan_tilt_lock_infra_message_handler_marco.cpp
      )
  endif(APPRC_CAMERA_PROTOCOL_VISCA)
endif(CMAKE_CROSSCOMPILING)

cxx_static_library(ptzf_inq_infra_message_handler
  "common_core;visca_server_message_if"
  inq_controller_message_handler.cpp
  sequence_id_controller.cpp)

cxx_gmock_executable(ptzf_biz_message_infra_if_test
  "common_core;visca_server_message_if_mock;ptzf_initialize_infra_if;ptzf_finalize_infra_if;ptzf_zoom_infra_if"
  test/ptzf_biz_message_infra_if_test.cpp
  ptzf_biz_message_infra_if.cpp
  inq_controller_message_handler_fake.cpp
  sequence_id_controller.cpp)

cxx_gmock_executable(ptzf_inq_infra_message_handler_test
  "common_core;visca_server_message_if_mock"
  test/inq_controller_message_handler_test.cpp
  inq_controller_message_handler.cpp
  sequence_id_controller.cpp
)

cxx_gmock_executable(ptzf_status_infra_if_test
  "model_info_rc;common_core;visca_config_core;ptzf_zoom_infra_if_mock"
  test/ptzf_status_infra_if_test.cpp
  ptzf_status_infra_if.cpp
)

#cxx_gmock_executable(ptzf_config_infra_if_test
#  "model_info_rc;common_core;visca_config_core;ptzf_zoom_infra_if_mock"
#  test/ptzf_status_infra_if_test.cpp
#  ptzf_status_infra_if.cpp
#)

cxx_gmock_executable(ptzf_status_infra_if_new_test
  "model_info_rc;common_core;visca_config_fake;ptzf_zoom_infra_if_mock"
  test/ptzf_status_infra_if_new_test.cpp
  ptzf_status_infra_if.cpp
)

#cxx_gmock_executable(ptzf_config_infra_if_new_test
#  "model_info_rc;common_core;visca_config_fake;ptzf_zoom_infra_if_mock"
#  test/ptzf_status_infra_if_new_test.cpp
#  ptzf_status_infra_if.cpp
#)

if(APPRC_PANTILTT_INITIAL_POSITION_LAST_STOPPED)
  cxx_gmock_executable(ptzf_status_infra_if_save_last_position_test
    "model_info_rc;common_core;visca_config_core;ptzf_zoom_infra_if_mock"
    test/ptzf_status_infra_if_save_last_position_test.cpp
    ptzf_status_infra_if.cpp
    ptzf_status_infra_if_save_last_position.cpp
  )
else(APPRC_PANTILTT_INITIAL_POSITION_LAST_STOPPED)
  cxx_gmock_executable(ptzf_status_infra_if_no_save_last_position_test
    "model_info_rc;common_core;visca_config_core;ptzf_zoom_infra_if_mock"
    test/ptzf_status_infra_if_no_save_last_position_test.cpp
    ptzf_status_infra_if.cpp
    ptzf_status_infra_if_no_save_last_position.cpp
  )
endif(APPRC_PANTILTT_INITIAL_POSITION_LAST_STOPPED)

add_library_tests(ptzf_status_infra_if ptzf_status_infra_if_test)
#add_library_tests(ptzf_config_infra_if ptzf_config_infra_if_test)
add_library_tests(ptzf_status_infra_if ptzf_status_infra_if_new_test)
#add_library_tests(ptzf_config_infra_if ptzf_config_infra_if_new_test)


if(DIADEM_BUILD)
  cxx_gmock_executable(ptz_trace_backup_infra_meta_impl_test
    "common_core;common_diadem_uipc_if;config_diadem_backup_if_mock"
    test/ptz_trace_backup_infra_meta_impl_backup_test.cpp
    ptz_trace_backup_infra_meta_impl_backup.cpp
  )
  cxx_gmock_executable(ptz_trace_backup_infra_item_impl_test
    "common_core;common_crypt;common_file_operation"
    test/ptz_trace_backup_infra_item_impl_file_test.cpp
    ptz_trace_backup_infra_item_impl_file.cpp
  )

  cxx_gmock_executable(ptz_trace_status_infra_if_test
    "model_info_rc;common_core;visca_config_core"
    test/ptz_trace_status_infra_if_test.cpp
    ptz_trace_status_infra_if.cpp
    )

  cxx_gmock_executable(ptzf_debug_info_infra_if_test
    "common_diadem_uipc_if;common_core;visca_config_core;config_diadem_backup_if_mock"
    test/ptzf_debug_info_infra_if_test.cpp
    ptzf_debug_info_infra_if.cpp
    )

  cxx_gmock_executable(ptzf_capability_infra_if_test
    "ptzf_capability_infra_if;ptzf_status_infra_if_mock;ptzf_config_infra_if_mock"
    test/ptzf_capability_infra_if_test.cpp)

  add_library_tests(ptz_trace_backup_infra_if ptz_trace_backup_infra_meta_impl_test)
  add_library_tests(ptz_trace_backup_infra_if ptz_trace_backup_infra_item_impl_test)
  add_library_tests(ptz_trace_status_infra_if ptz_trace_status_infra_if_test)
  add_library_tests(ptzf_debug_info_infra_if ptzf_debug_info_infra_if_test)
  add_library_tests(ptzf_biz_message_infra_if ptzf_biz_message_infra_if_test)
  add_library_tests(ptzf_inq_infra_message_handler ptzf_inq_infra_message_handler_test)
  add_library_tests(ptzf_capability_infra_if ptzf_capability_infra_if_test)
elseif(G8_BUILD)
  cxx_gmock_executable(ptz_trace_backup_infra_meta_impl_test
    "common_core;visca_config_fake;visca_config_ptz_trace_database_backup"
    test/ptz_trace_backup_infra_meta_impl_config_test.cpp
    ptz_trace_backup_infra_meta_impl_config.cpp
  )
  cxx_gmock_executable(ptz_trace_backup_infra_item_impl_test
    "common_core;common_crypt;common_file_operation"
    test/ptz_trace_backup_infra_item_impl_file_test.cpp
    ptz_trace_backup_infra_item_impl_file.cpp
  )
  cxx_gmock_executable(ptz_trace_status_infra_if_test
    "model_info_rc;common_core;visca_config_core"
    test/ptz_trace_status_infra_if_test.cpp
    ptz_trace_status_infra_if.cpp
    ptz_trace_status_infra_if_status_holder.cpp
  )
  cxx_static_library(ptzf_initializer_infra_if
    "common_core;bizglobal;event_router_if"
    ptzf_initializer_infra_if.cpp
  )
  cxx_gmock_executable(ptzf_initializer_infra_if_test
    "ptzf_initializer_infra_if;event_router_if_mock"
    test/ptzf_initializer_infra_if_test.cpp
  )
  cxx_static_library(ptzf_finalizer_infra_if
    "common_core;bizglobal;event_router_if"
    ptzf_finalizer_infra_if.cpp
  )
  cxx_gmock_executable(ptzf_finalizer_infra_if_test
    "ptzf_finalizer_infra_if;event_router_if_mock"
    test/ptzf_finalizer_infra_if_test.cpp
  )
  add_library_tests(ptz_trace_backup_infra_if ptz_trace_backup_infra_meta_impl_test)
  add_library_tests(ptz_trace_backup_infra_if ptz_trace_backup_infra_item_impl_test)
  add_library_tests(ptz_trace_status_infra_if ptz_trace_status_infra_if_test)
  add_library_tests(ptzf_initializer_infra_if ptzf_initializer_infra_if_test)
  add_library_tests(ptzf_finalizer_infra_if ptzf_finalizer_infra_if_test)

  cxx_gmock_executable(ptzf_backup_infra_if_test
    "visca_config_fake"
    ptzf_backup_infra_if.cpp
    test/ptzf_backup_infra_if_test.cpp
  )
  add_library_tests(ptzf_backup_infra_if ptzf_backup_infra_if_test)

  if(APPRC_CAMERA_PROTOCOL_VISCA)
    list(APPEND ptzf_infra_entry_dependencies common_core)
    list(APPEND ptzf_infra_entry_dependencies ptzf_initialize_infra_message_handler)
    list(APPEND ptzf_infra_entry_dependencies ptzf_finalize_infra_message_handler)
    list(APPEND ptzf_infra_entry_dependencies ptzf_pan_tilt_infra_message_handler)
    list(APPEND ptzf_infra_entry_dependencies ptzf_zoom_infra_message_handler)
    list(APPEND ptzf_infra_entry_dependencies ptzf_focus_infra_message_handler)
    list(APPEND ptzf_infra_entry_dependencies ptzf_if_clear_infra_message_handler)
    cxx_static_library(ptzf_infra_entry
      "${ptzf_infra_entry_dependencies}"
      ptzf_infra_controller_entry_g8.cpp)

    cxx_gmock_executable(ptzf_initialize_infra_if_test
      "common_core;visca_server_message_if;ptzf_status;ptzf_config;ptzf_status_if;error_notifier_message_if"
      test/ptzf_initialize_infra_if_g8_test.cpp
      ptzf_initialize_infra_if_g8.cpp
      ptzf_initialize_infra_message_handler_g8_fake.cpp
      sequence_id_controller.cpp)
    add_library_tests(ptzf_initialize_infra_if ptzf_initialize_infra_if_test)

    cxx_gmock_executable(ptzf_finalize_infra_if_test
      "common_core;visca_server_message_if;ptzf_status;ptzf_config;ptzf_status_if;error_notifier_message_if"
      test/ptzf_finalize_infra_if_g8_test.cpp
      ptzf_finalize_infra_if_g8.cpp
      ptzf_finalize_infra_message_handler_g8_fake.cpp
      sequence_id_controller.cpp)
    add_library_tests(ptzf_finalize_infra_if ptzf_finalize_infra_if_test)

    list(APPEND ptzf_pan_tilt_infra_if_test_dependencies ptzf_status_if_mock)
    list(APPEND ptzf_pan_tilt_infra_if_test_dependencies ptzf_config_if_mock)
    list(APPEND ptzf_pan_tilt_infra_if_test_dependencies ptzf_pan_tilt_infra_message_handler)
    cxx_gmock_executable(ptzf_pan_tilt_infra_if_test
        "${ptzf_pan_tilt_infra_if_test_dependencies}"
      test/ptzf_pan_tilt_infra_if_g8_test.cpp
      ptzf_pan_tilt_infra_if_g8.cpp
      test/ptz_updater_mock.cpp)
    add_library_tests(ptzf_pan_tilt_infra_if ptzf_pan_tilt_infra_if_test)

    cxx_gmock_executable(ptzf_zoom_infra_if_test
      "ptzf_zoom_infra_message_handler"
      test/ptzf_zoom_infra_if_g8_test.cpp
      ptzf_zoom_infra_if_g8.cpp)
    add_library_tests(ptzf_zoom_infra_if ptzf_zoom_infra_if_test)

    cxx_gmock_executable(ptzf_focus_infra_if_test
      "ptzf_focus_infra_message_handler"
      test/ptzf_focus_infra_if_g8_test.cpp
      ptzf_focus_infra_if_g8.cpp)
    add_library_tests(ptzf_focus_infra_if ptzf_focus_infra_if_test)

    cxx_gmock_executable(ptzf_if_clear_infra_if_test
      "ptzf_if_clear_infra_message_handler"
      test/ptzf_if_clear_infra_if_g8_test.cpp
      ptzf_if_clear_infra_if_g8.cpp)
    add_library_tests(ptzf_if_clear_infra_if ptzf_if_clear_infra_if_test)

    list(APPEND ptzf_initialize_infra_message_handler_libs bizglobal)
    list(APPEND ptzf_initialize_infra_message_handler_libs common_core)
    list(APPEND ptzf_initialize_infra_message_handler_libs common_timer)
    list(APPEND ptzf_initialize_infra_message_handler_libs ptz_trace_if_mock)
    list(APPEND ptzf_initialize_infra_message_handler_libs ptz_trace_status_if)
    list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_status)
    list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_config)
    if(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_status_if_g8)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_config_if_g8)
    else(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_status_if)
    endif(NOT CMAKE_CROSSCOMPILING)
    list(APPEND ptzf_initialize_infra_message_handler_libs video_status_if_mock)
    list(APPEND ptzf_initialize_infra_message_handler_libs error_notifier_message_if)
    list(APPEND ptzf_initialize_infra_message_handler_libs pan_tilt_limit_position)
    list(APPEND ptzf_initialize_infra_message_handler_libs visca_server_message_if_mock)
    if(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs metadata_control_if_mock)
    else(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs metadata_control_if)
    endif(NOT CMAKE_CROSSCOMPILING)
    if(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_pan_tilt_lock_infra_if_mock)
    else(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_pan_tilt_lock_infra_if)
    endif(NOT CMAKE_CROSSCOMPILING)
    cxx_gmock_executable(ptzf_initialize_infra_message_handler_test
      "${ptzf_initialize_infra_message_handler_libs}"
      test/ptzf_initialize_infra_message_handler_g8_test.cpp
      ptzf_initialize_infra_message_handler_g8.cpp
      sequence_id_controller.cpp)
    add_library_tests(ptzf_initialize_infra_message_handler ptzf_initialize_infra_message_handler_test)

    list(APPEND ptzf_finalize_infra_message_handler_libs ptzf_backup_infra_if_mock)
    list(APPEND ptzf_finalize_infra_message_handler_libs ptzf_status_if_mock)
    list(APPEND ptzf_finalize_infra_message_handler_libs ptzf_config_if_mock)
    list(APPEND ptzf_finalize_infra_message_handler_libs visca_server_message_if_mock)
    cxx_gmock_executable(ptzf_finalize_infra_message_handler_test
      "${ptzf_finalize_infra_message_handler_libs}"
      test/ptzf_finalize_infra_message_handler_g8_test.cpp
      ptzf_finalize_infra_message_handler_g8.cpp
      sequence_id_controller.cpp)
    add_library_tests(ptzf_finalize_infra_message_handler ptzf_finalize_infra_message_handler_test)

    cxx_gmock_executable(ptzf_pan_tilt_infra_message_handler_test
      "visca_server_message_if_mock"
      test/ptzf_pan_tilt_infra_message_handler_g8_test.cpp
      ptzf_pan_tilt_infra_message_handler_g8.cpp
      sequence_id_controller.cpp)
    add_library_tests(ptzf_pan_tilt_infra_message_handler ptzf_pan_tilt_infra_message_handler_test)

    cxx_gmock_executable(ptzf_zoom_infra_message_handler_test
      "visca_server_message_if_mock"
      test/ptzf_zoom_infra_message_handler_g8_test.cpp
      ptzf_zoom_infra_message_handler_g8.cpp
      sequence_id_controller.cpp)
    add_library_tests(ptzf_zoom_infra_message_handler ptzf_zoom_infra_message_handler_test)

    cxx_gmock_executable(ptzf_focus_infra_message_handler_test
      "visca_server_message_if_mock"
      test/ptzf_focus_infra_message_handler_g8_test.cpp
      ptzf_focus_infra_message_handler_g8.cpp
      sequence_id_controller.cpp)
    add_library_tests(ptzf_focus_infra_message_handler ptzf_focus_infra_message_handler_test)

    cxx_gmock_executable(ptzf_if_clear_infra_message_handler_test
      "visca_server_message_if_mock"
      test/ptzf_if_clear_infra_message_handler_g8_test.cpp
      ptzf_if_clear_infra_message_handler_g8.cpp
      sequence_id_controller.cpp)
    add_library_tests(ptzf_if_clear_infra_message_handler ptzf_if_clear_infra_message_handler_test)

    cxx_gmock_executable(ptzf_capability_infra_if_test
      "ptzf_status_infra_if_mock;ptzf_config_infra_if_mock;visca_config_fake"
      ptzf_capability_infra_if_g8.cpp
      test/ptzf_capability_infra_if_g8_test.cpp)
    add_library_tests(ptzf_capability_infra_if ptzf_capability_infra_if_test)

  elseif(APPRC_CAMERA_PROTOCOL_PTP)
    list(APPEND ptzf_infra_entry_dependencies common_core)
    list(APPEND ptzf_infra_entry_dependencies ptzf_inq_infra_message_handler)
    list(APPEND ptzf_infra_entry_dependencies ptzf_initialize_infra_message_handler)
    list(APPEND ptzf_infra_entry_dependencies ptzf_finalize_infra_message_handler)
    list(APPEND ptzf_infra_entry_dependencies ptzf_focus_infra_message_handler)
    list(APPEND ptzf_infra_entry_dependencies ptzf_zoom_infra_message_handler)
    list(APPEND ptzf_infra_entry_dependencies ptzf_pan_tilt_infra_message_handler)
    list(APPEND ptzf_infra_entry_dependencies ptzf_pan_tilt_lock_infra_message_handler)
    list(APPEND ptzf_infra_entry_dependencies ptzf_if_clear_infra_message_handler)
    cxx_static_library(ptzf_infra_entry
      "common_core;${ptzf_infra_entry_dependencies}"
      ptzf_infra_controller_entry_marco.cpp)

    cxx_gmock_executable(ptzf_initialize_infra_if_test
      "common_core;visca_server_message_if;ptzf_status;ptzf_config;ptzf_status_if;error_notifier_message_if;ptzf_pan_tilt_lock_infra_if"
      test/ptzf_initialize_infra_if_marco_test.cpp
      ptzf_initialize_infra_if_marco.cpp
      ptzf_initialize_infra_message_handler_marco_fake.cpp
      ptzf_ptp_accessor_fake.cpp
      sequence_id_controller.cpp)
    add_library_tests(ptzf_initialize_infra_if ptzf_initialize_infra_if_test)

    cxx_gmock_executable(ptzf_finalize_infra_if_test
      "common_core;visca_server_message_if;ptzf_status;ptzf_config;ptzf_status_if;error_notifier_message_if"
      test/ptzf_finalize_infra_if_marco_test.cpp
      ptzf_finalize_infra_if_marco.cpp
      ptzf_finalize_infra_message_handler_marco_fake.cpp
      sequence_id_controller.cpp)
    add_library_tests(ptzf_finalize_infra_if ptzf_finalize_infra_if_test)

    # PanTilt domain
    list(APPEND pan_tilt_infra_if_test_dependencies common_core)
    list(APPEND pan_tilt_infra_if_test_dependencies visca_server_message_if_mock)
    list(APPEND pan_tilt_infra_if_test_dependencies ptp_driver_command_if)
    list(APPEND pan_tilt_infra_if_test_dependencies ptzf_status_if_mock)
    list(APPEND pan_tilt_infra_if_test_dependencies ptzf_config_if_mock)
    list(APPEND pan_tilt_infra_if_test_dependencies ptzf_input_infra_if_mock)
    cxx_gmock_executable(ptzf_pan_tilt_infra_if_test
      "${pan_tilt_infra_if_test_dependencies}"
      test/ptzf_pan_tilt_infra_if_marco_test.cpp
      ptzf_pan_tilt_infra_if_marco.cpp
      ptzf_pan_tilt_infra_message_handler_marco_fake.cpp
      sequence_id_controller.cpp
      )
    add_library_tests(ptzf_pan_tilt_infra_if ptzf_pan_tilt_infra_if_test)

    list(APPEND pan_tilt_infra_message_handler_test_dependencies common_core)
    list(APPEND pan_tilt_infra_message_handler_test_dependencies visca_server_message_if_mock)
    list(APPEND pan_tilt_infra_message_handler_test_dependencies ptp_error_checker)
    list(APPEND pan_tilt_infra_message_handler_test_dependencies ptzf_status_infra_if_mock)
    list(APPEND pan_tilt_infra_message_handler_test_dependencies ptzf_config_infra_if_mock)
    cxx_gmock_executable(ptzf_pan_tilt_infra_message_handler_test
      "${pan_tilt_infra_message_handler_test_dependencies}"
      test/ptzf_pan_tilt_infra_message_handler_marco_test.cpp
      ptzf_pan_tilt_infra_message_handler_marco.cpp
      sequence_id_controller.cpp
      $<TARGET_OBJECTS:ptp_drv_cmd_if_fake_obj>
      )
    add_library_tests(ptzf_pan_tilt_infra_message_handler ptzf_pan_tilt_infra_message_handler_test)

    # Zoom domain
    list(APPEND ptzf_zoom_infra_if_test_dependencies common_core)
    list(APPEND ptzf_zoom_infra_if_test_dependencies ptp_driver_command_if)
    list(APPEND ptzf_zoom_infra_if_test_dependencies visca_status_if_mock)
    cxx_gmock_executable(ptzf_zoom_infra_if_test
      "${ptzf_zoom_infra_if_test_dependencies}"
      test/ptzf_zoom_infra_if_marco_test.cpp
      ptzf_zoom_infra_if_marco.cpp
      ptzf_zoom_infra_message_handler_marco_fake.cpp
      )
    add_library_tests(ptzf_zoom_infra_if ptzf_zoom_infra_if_test)

    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies common_core)
    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies ptp_error_checker)
    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies visca_status_if_mock)
    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies visca_server_internal_mode_manager)
    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies visca_capability_infra_if_mock)
    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies power_status_if_mock)
    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies preset_status_if_mock)
    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies video_status_if_mock)
    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies menu_status_mock)
    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies ptzf_status_if_mock)
    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies ptzf_config_if_mock)
    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies ptz_trace_status_if_mock)
    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies ptz_trace_if_mock)
    list(APPEND ptzf_zoom_infra_message_handler_test_dependencies camera_osd_status_if_mock)
    cxx_gmock_executable(ptzf_zoom_infra_message_handler_test
      "${ptzf_zoom_infra_message_handler_test_dependencies}"
      test/ptzf_zoom_infra_message_handler_marco_test.cpp
      ptzf_zoom_infra_message_handler_marco.cpp
      $<TARGET_OBJECTS:ptp_drv_cmd_if_fake_obj>
    )
    add_library_tests(ptzf_zoom_infra_message_handler ptzf_zoom_infra_message_handler_test)

    # Focus domain
    list(APPEND ptzf_focus_infra_if_test_dependencies common_core)
    list(APPEND ptzf_focus_infra_if_test_dependencies ptp_driver_command_if)
    list(APPEND ptzf_focus_infra_if_test_dependencies visca_status_if_mock)
    cxx_gmock_executable(ptzf_focus_infra_if_test
      "${ptzf_focus_infra_if_test_dependencies}"
      test/ptzf_focus_infra_if_marco_test.cpp
      ptzf_focus_infra_if_marco.cpp
      ptzf_focus_infra_message_handler_marco_fake.cpp
      )
    add_library_tests(ptzf_focus_infra_if ptzf_focus_infra_if_test)

    list(APPEND ptzf_focus_infra_message_handler_test_dependencies common_core)
    list(APPEND ptzf_focus_infra_message_handler_test_dependencies ptp_error_checker)
    list(APPEND ptzf_focus_infra_message_handler_test_dependencies visca_status_if_mock)
    list(APPEND ptzf_focus_infra_message_handler_test_dependencies visca_server_internal_mode_manager)
    list(APPEND ptzf_focus_infra_message_handler_test_dependencies visca_capability_infra_if_mock)
    list(APPEND ptzf_focus_infra_message_handler_test_dependencies power_status_if_mock)
    list(APPEND ptzf_focus_infra_message_handler_test_dependencies preset_status_if_mock)
    list(APPEND ptzf_focus_infra_message_handler_test_dependencies video_status_if_mock)
    list(APPEND ptzf_focus_infra_message_handler_test_dependencies menu_status_mock)
    list(APPEND ptzf_focus_infra_message_handler_test_dependencies ptzf_status_if_mock)
    list(APPEND ptzf_focus_infra_message_handler_test_dependencies ptzf_config_if_mock)
    list(APPEND ptzf_focus_infra_message_handler_test_dependencies ptz_trace_status_if_mock)
    list(APPEND ptzf_focus_infra_message_handler_test_dependencies ptz_trace_if_mock)
    list(APPEND ptzf_focus_infra_message_handler_test_dependencies camera_osd_status_if_mock)
    cxx_gmock_executable(ptzf_focus_infra_message_handler_test
      "${ptzf_focus_infra_message_handler_test_dependencies}"
      test/ptzf_focus_infra_message_handler_marco_test.cpp
      ptzf_focus_infra_message_handler_marco.cpp
      $<TARGET_OBJECTS:ptp_drv_cmd_if_fake_obj>
      )
    add_library_tests(ptzf_focus_infra_message_handler ptzf_focus_infra_message_handler_test)

    cxx_gmock_executable(ptzf_if_clear_infra_if_test
      "ptzf_if_clear_infra_message_handler"
      test/ptzf_if_clear_infra_if_marco_test.cpp
      ptzf_if_clear_infra_if_marco.cpp)
    add_library_tests(ptzf_if_clear_infra_if ptzf_if_clear_infra_if_test)

    list(APPEND pan_tilt_lock_infra_if_test_dependencies common_core)
    cxx_gmock_executable(ptzf_pan_tilt_lock_infra_if_test
      "${pan_tilt_lock_infra_if_test_dependencies}"
      test/ptzf_pan_tilt_lock_infra_if_marco_test.cpp
      ptzf_pan_tilt_lock_infra_if_marco.cpp
      ptzf_pan_tilt_lock_infra_message_handler_marco_fake.cpp
      sequence_id_controller.cpp
      )
    add_library_tests(ptzf_pan_tilt_lock_infra_if ptzf_pan_tilt_lock_infra_if_test)

    list(APPEND ptzf_initialize_infra_message_handler_libs bizglobal)
    list(APPEND ptzf_initialize_infra_message_handler_libs common_core)
    list(APPEND ptzf_initialize_infra_message_handler_libs common_timer)
    list(APPEND ptzf_initialize_infra_message_handler_libs ptz_trace_if_mock)
    list(APPEND ptzf_initialize_infra_message_handler_libs ptz_trace_status_if)
    list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_status)
    list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_config)
    if(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_status_if_marco)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_config_if_marco)
    else(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_status_if)
    endif(NOT CMAKE_CROSSCOMPILING)
    if(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_status_infra_if_mock)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_config_infra_if_mock)
    else(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_status_infra_if)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_config_infra_if)
    endif(NOT CMAKE_CROSSCOMPILING)
    list(APPEND ptzf_initialize_infra_message_handler_libs video_status_if_mock)
    list(APPEND ptzf_initialize_infra_message_handler_libs error_notifier_message_if)
    list(APPEND ptzf_initialize_infra_message_handler_libs pan_tilt_limit_position)
    list(APPEND ptzf_initialize_infra_message_handler_libs visca_server_message_if_mock)
    if(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs metadata_control_if_mock)
    else(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs metadata_control_if)
    endif(NOT CMAKE_CROSSCOMPILING)
    if(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_pan_tilt_lock_infra_if_mock)
    else(NOT CMAKE_CROSSCOMPILING)
      list(APPEND ptzf_initialize_infra_message_handler_libs ptzf_pan_tilt_lock_infra_if)
    endif(NOT CMAKE_CROSSCOMPILING)
    cxx_gmock_executable(ptzf_initialize_infra_message_handler_test
      "${ptzf_initialize_infra_message_handler_libs}"
      test/ptzf_initialize_infra_message_handler_marco_test.cpp
      ptzf_initialize_infra_message_handler_marco.cpp
      ptzf_ptp_accessor_mock.cpp
      sequence_id_controller.cpp)
    add_library_tests(ptzf_initialize_infra_message_handler ptzf_initialize_infra_message_handler_test)

    list(APPEND ptzf_finalize_infra_message_handler_libs ptzf_backup_infra_if_mock)
    list(APPEND ptzf_finalize_infra_message_handler_libs ptzf_status_if_mock)
    list(APPEND ptzf_finalize_infra_message_handler_libs ptzf_config_if_mock)
    list(APPEND ptzf_finalize_infra_message_handler_libs visca_server_message_if_mock)
    list(APPEND ptzf_finalize_infra_message_handler_libs ptzf_status_infra_if_mock)
    list(APPEND ptzf_finalize_infra_message_handler_libs ptzf_config_infra_if_mock)
    cxx_gmock_executable(ptzf_finalize_infra_message_handler_test
      "${ptzf_finalize_infra_message_handler_libs}"
      test/ptzf_finalize_infra_message_handler_marco_test.cpp
      ptzf_finalize_infra_message_handler_marco.cpp
      sequence_id_controller.cpp)
    add_library_tests(ptzf_finalize_infra_message_handler ptzf_finalize_infra_message_handler_test)

    cxx_gmock_executable(ptzf_if_clear_infra_message_handler_test
      "visca_server_message_if_mock"
      test/ptzf_if_clear_infra_message_handler_marco_test.cpp
      ptzf_if_clear_infra_message_handler_marco.cpp
      sequence_id_controller.cpp)
    add_library_tests(ptzf_if_clear_infra_message_handler ptzf_if_clear_infra_message_handler_test)

    cxx_gmock_executable(ptzf_capability_infra_if_test
      "ptzf_status_infra_if_mock;ptzf_config_infra_if_mock;visca_config_fake"
      ptzf_capability_infra_if_marco.cpp
      test/ptzf_capability_infra_if_marco_test.cpp)
    add_library_tests(ptzf_capability_infra_if ptzf_capability_infra_if_test)

    list(APPEND pan_tilt_lock_infra_message_handler_test_dependencies common_core)
    list(APPEND pan_tilt_lock_infra_message_handler_test_dependencies bizglobal)
    list(APPEND pan_tilt_lock_infra_message_handler_test_dependencies ptzf_input_infra_if_mock)
    list(APPEND pan_tilt_lock_infra_message_handler_test_dependencies ptzf_status_infra_if_mock)
    list(APPEND pan_tilt_lock_infra_message_handler_test_dependencies ptzf_config_infra_if_mock)
    cxx_gmock_executable(ptzf_pan_tilt_lock_infra_message_handler_test
      "${pan_tilt_lock_infra_message_handler_test_dependencies}"
      test/ptzf_pan_tilt_lock_infra_message_handler_marco_test.cpp
      ptzf_pan_tilt_lock_infra_message_handler_marco.cpp
      sequence_id_controller.cpp
      )
    add_library_tests(ptzf_pan_tilt_lock_infra_message_handler ptzf_pan_tilt_lock_infra_message_handler_test)
  endif(APPRC_CAMERA_PROTOCOL_VISCA)
endif(DIADEM_BUILD)

#
# PtMiconVsyncNotifierInfraIf
#
if(CMAKE_CROSSCOMPILING)
  cxx_static_library(pt_micon_vsync_notifier_infra_if
    "bizglobal;vsync_notification_if"
    pt_micon_vsync_notifier_infra_if.cpp)
endif(CMAKE_CROSSCOMPILING)

cxx_gmock_executable(pt_micon_vsync_notifier_infra_if_test
  "bizglobal;vsync_notification_if_mock"
  pt_micon_vsync_notifier_infra_if.cpp
  test/pt_micon_vsync_notifier_infra_if_test.cpp)
add_library_tests(pt_micon_vsync_notifier_infra_if pt_micon_vsync_notifier_infra_if_test)
