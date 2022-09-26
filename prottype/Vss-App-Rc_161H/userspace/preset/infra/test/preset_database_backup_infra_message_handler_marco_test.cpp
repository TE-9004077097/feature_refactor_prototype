/*
 * preset_database_backup_infra_message_handler_marco_test.cpp
 *
 * Copyright 2021 Sony Corporation
 */

#include "gtl_memory.h"
#include "gtl_string.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "common_gmock_util.h"
#include "common_message_queue.h"
#include "preset_infra_message.h"
#include "preset_database_backup_infra_message_handler_marco.h"
#include "preset/preset_manager_message.h"
#include "preset/preset_common_message.h"
#include "ptp/driver/ptp_driver_command_if_mock.h"
#include "ptp/ptp_device_property.h"
#include "ptp/ptp_error.h"
#include "biz_ptzf_if_mock.h"
#include "ptp/ptp_command_data.h"
#include "visca/visca_server_message_if_mock.h"
#include "visca/visca_server_ptzf_if_mock.h"
#include "visca/visca_remote_camera2_messages.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Eq;
using ::testing::Matcher;
using ::testing::SaveArg;
using ::testing::SetArgReferee;
using ::testing::SaveArg;
using ::testing::InvokeWithoutArgs;

namespace config {

struct ConfigReadyNotification
{};

} // namespace config

namespace preset {
namespace infra {

// + setPresetSuccess
//   - SetPreset成功
// + setPresetZoomPositionFailure
//   - ZoomPositionSettingエラー後FocusPositionSetting
// + setPresetFocusCommandFailure
//   - Focus関連コマンドすべてエラー後ZoomPositionSetting
// + setPresetAllCommandFailure
//   - コマンドすべてエラー後SetPresetResultを返却

class PresetDatabaseBackupInfraMessageHandlerTest : public ::testing::Test
{
protected:
    PresetDatabaseBackupInfraMessageHandlerTest()
        : ptp_driver_if_mock_holder_(),
          ptp_driver_if_mock_(ptp_driver_if_mock_holder_.getMock()),
          ptzf_mock_holder_object_(),
          biz_ptzf_if_mock_(ptzf_mock_holder_object_.getMock()),
          visca_ptzf_if_mock_holder_object(),
          visca_ptzf_if_mock_(visca_ptzf_if_mock_holder_object.getMock()),
          visca_if_mock_holder_object(),
          visca_if_mock_(visca_if_mock_holder_object.getMock()),
          reply_(),
          handler_()
    {}

    virtual void SetUp()
    {
        common::MessageQueue mq("ConfigReadyMQ");
        config::ConfigReadyNotification message;
        mq.post(message);
    }

    virtual void TearDown()
    {
        common::MessageQueue config_mq("ConfigReadyMQ");
        config_mq.unlink();
        reply_.unlink();
    }

    void setPtzfStatusParameters()
    {
        // SetFocusMode
        EXPECT_CALL(biz_ptzf_if_mock_, setFocusMode(_, _)).Times(1).WillOnce(Return(true));

        // SetAfTransitionSpeed
        EXPECT_CALL(biz_ptzf_if_mock_, setAfTransitionSpeedValue(_, _)).Times(1).WillOnce(Return(true));

        // SetAfSubjShiftSens
        EXPECT_CALL(biz_ptzf_if_mock_, setAfSubjShiftSensValue(_, _)).Times(1).WillOnce(Return(true));

        // SetFocusFaceEyedetection
        EXPECT_CALL(biz_ptzf_if_mock_, setFocusFaceEyedetectionValue(_, _)).Times(1).WillOnce(Return(true));

        // SetFocusArea
        EXPECT_CALL(biz_ptzf_if_mock_, setFocusArea(_, _)).Times(1).WillOnce(Return(true));

        // SetAFAreaPositionAFC
        EXPECT_CALL(biz_ptzf_if_mock_, setAFAreaPositionAFC(_, _, _)).Times(1).WillOnce(Return(true));

        // SetAFAreaPositionAFS
        EXPECT_CALL(biz_ptzf_if_mock_, setAFAreaPositionAFS(_, _, _)).Times(1).WillOnce(Return(true));

        // SetZoomPosition
        EXPECT_CALL(biz_ptzf_if_mock_, setZoomPosition(_, _)).Times(1).WillOnce(Return(true));

        // SetFocusPosition
        EXPECT_CALL(biz_ptzf_if_mock_, setFocusPosition(_, _)).Times(1).WillOnce(Return(true));
    }

protected:
    MockHolderObject<ptp::driver::PtpDriverCommandIfMock> ptp_driver_if_mock_holder_;
    ptp::driver::PtpDriverCommandIfMock& ptp_driver_if_mock_;
    MockHolderObject<biz_ptzf::BizPtzfIfMock> ptzf_mock_holder_object_;
    biz_ptzf::BizPtzfIfMock& biz_ptzf_if_mock_;
    MockHolderObject<visca::ViscaServerPtzfIfMock> visca_ptzf_if_mock_holder_object;
    visca::ViscaServerPtzfIfMock& visca_ptzf_if_mock_;
    MockHolderObject<visca::ViscaServerMessageIfMock> visca_if_mock_holder_object;
    visca::ViscaServerMessageIfMock& visca_if_mock_;
    common::MessageQueue reply_;
    PresetDatabaseBackupInfraMessageHandler handler_;
};

TEST_F(PresetDatabaseBackupInfraMessageHandlerTest, setPresetSuccess)
{
    // PtzfStatusIf
    setPtzfStatusParameters();

    // GetFocusMode
    u32_t dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_MODE_SETTING;
    uint64_t value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetAfTransitionSpeed
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_TRANSITION_SPEED;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetAfSubjShiftSens
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_SUBJ_SHIFT_SENS;
    value = U8_T(5);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetFocusFaceEyedetection
    dp_code = ptp::CR_DEVICE_PROPERTY_FACE_EYE_DETECTIONAF;
    value = ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_ONLYAF;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetFocusArea
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_AREA;
    value = ptp::CR_FOCUS_AREA_WIDE;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetAFAreaPositionAFC
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_C;
    value = U64_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetAFAreaPositionAFS
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_S;
    value = U64_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetZoomPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_ZOOM_POSITION_CURRENT_VALUE;
    value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetFocusPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_POSITION_CURRENT_VALUE;
    value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    SetPresetRequest msg(DEFAULT_PRESET_ID, reply_.getName());
    handler_.handleRequest(msg);

    SetPresetResult result;
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_SUCCESS);
}

TEST_F(PresetDatabaseBackupInfraMessageHandlerTest, setPresetZoomPositionFailure)
{
    // PtzfStatusIf
    setPtzfStatusParameters();

    // GetFocusMode
    u32_t dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_MODE_SETTING;
    uint64_t value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetAfTransitionSpeed
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_TRANSITION_SPEED;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetAfSubjShiftSens
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_SUBJ_SHIFT_SENS;
    value = U8_T(5);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetFocusFaceEyedetection
    dp_code = ptp::CR_DEVICE_PROPERTY_FACE_EYE_DETECTIONAF;
    value = ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_ONLYAF;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetFocusArea
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_AREA;
    value = ptp::CR_FOCUS_AREA_WIDE;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetAFAreaPositionAFC
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_C;
    value = U64_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetAFAreaPositionAFS
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_S;
    value = U64_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetZoomPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_ZOOM_POSITION_CURRENT_VALUE;
    value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetFocusPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_POSITION_CURRENT_VALUE;
    value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    SetPresetRequest msg(DEFAULT_PRESET_ID, reply_.getName());
    handler_.handleRequest(msg);

    SetPresetResult result;
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_SUCCESS);
}

TEST_F(PresetDatabaseBackupInfraMessageHandlerTest, setPresetFocusCommandFailure)
{
    // PtzfStatusIf
    setPtzfStatusParameters();

    // GetFocusMode
    u32_t dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_MODE_SETTING;
    uint64_t value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetAfTransitionSpeed
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_TRANSITION_SPEED;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetAfSubjShiftSens
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_SUBJ_SHIFT_SENS;
    value = U8_T(5);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetFocusFaceEyedetection
    dp_code = ptp::CR_DEVICE_PROPERTY_FACE_EYE_DETECTIONAF;
    value = ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_ONLYAF;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetFocusArea
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_AREA;
    value = ptp::CR_FOCUS_AREA_WIDE;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetAFAreaPositionAFC
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_C;
    value = U64_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetAFAreaPositionAFS
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_S;
    value = U64_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetZoomPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_ZOOM_POSITION_CURRENT_VALUE;
    value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_NONE)));

    // GetFocusPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_POSITION_CURRENT_VALUE;
    value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    SetPresetRequest msg(DEFAULT_PRESET_ID, reply_.getName());
    handler_.handleRequest(msg);

    SetPresetResult result;
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_SUCCESS);
}

TEST_F(PresetDatabaseBackupInfraMessageHandlerTest, setPresetAllCommandFailure)
{
    // PtzfStatusIf
    setPtzfStatusParameters();

    // GetFocusMode
    u32_t dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_MODE_SETTING;
    uint64_t value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetAfTransitionSpeed
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_TRANSITION_SPEED;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetAfSubjShiftSens
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_SUBJ_SHIFT_SENS;
    value = U8_T(5);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetFocusFaceEyedetection
    dp_code = ptp::CR_DEVICE_PROPERTY_FACE_EYE_DETECTIONAF;
    value = ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_ONLYAF;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetFocusArea
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_AREA;
    value = ptp::CR_FOCUS_AREA_WIDE;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetAFAreaPositionAFC
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_C;
    value = U64_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetAFAreaPositionAFS
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_S;
    value = U64_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetZoomPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_ZOOM_POSITION_CURRENT_VALUE;
    value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    // GetFocusPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_POSITION_CURRENT_VALUE;
    value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyValue(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(value), Return(ptp::CR_ERROR_GENERIC)));

    SetPresetRequest msg(DEFAULT_PRESET_ID, reply_.getName());
    handler_.handleRequest(msg);

    SetPresetResult result;
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_SUCCESS);
}

} // namespace infra
} // namespace preset
