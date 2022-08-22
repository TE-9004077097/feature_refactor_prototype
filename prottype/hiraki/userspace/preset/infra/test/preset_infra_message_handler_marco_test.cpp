/*
 * preset_infra_message_handler_marco_test.cpp
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
#include "preset_infra_message_handler_marco.h"
#include "preset/preset_manager_message.h"
#include "preset/preset_common_message.h"
#include "ptp/driver/ptp_driver_command_if_mock.h"
#include "ptp/ptp_device_property.h"
#include "ptp/ptp_error.h"
#include "ptzf/ptzf_status_if_mock.h"
#include "ptzf/ptzf_config_if_mock.h"
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

// + recallPresetSuccess
//   - RecallPreset成功
// + recallPresetZoomPositionFailure
//   - ZoomPositionSettingエラー後FocusPositionSetting
// + recallPresetFocusCommandFailure
//   - Focus関連コマンドすべてエラー後ZoomPositionSetting
// + recallPresetAllCommandFailure
//   - コマンドすべてエラー後CompleteFcbPresetRecallResultを返却
// + cancelCameraRecallSuccess
//   - RecallPresetCancel成功
// + cancelCameraRecallAllCommandFailure
//   - RecallPresetCancelでコマンドがエラー
// + recallPresetMultipleRequest
//   - RecallPreset → RecallPresetCancel → 後段のRecallPreset
// + recallPresetMultipleRequestCancelCommandFailure
//   - RecallPreset → RecallPresetCancelでエラー → 後段のRecallPreset
// + setPanTiltSlowModeRequestSuccess
// + setPanTiltSlowModeRequestError
// + setPanTiltSlowModeRequestMultipleRequest
// + setPanTiltSlowModeWithSeqIdRequestSuccess
// + setPanTiltSlowModeWithSeqIdRequestError
// + setPanTiltSlowModeWithSeqIdRequestMultipleRequest
// + setRecallFreezeModeSuccess
// + getPanTiltTimeRequestSuccess

class PresetInfraMessageHandlerTest : public ::testing::Test
{
protected:
    PresetInfraMessageHandlerTest()
        : ptp_driver_if_mock_holder_(),
          ptp_driver_if_mock_(ptp_driver_if_mock_holder_.getMock()),
          ptzf_mock_holder_object_(),
          ptzf_status_if_mock_(ptzf_mock_holder_object_.getMock()),
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

    void getPtzfStatusParameters(u32_t preset_id)
    {
        // GetFocusMode
        EXPECT_CALL(ptzf_status_if_mock_, getFocusMode(preset_id, _)).Times(1).WillOnce(Return());

        // GetAfTransitionSpeed
        EXPECT_CALL(ptzf_status_if_mock_, getAfTransitionSpeed(preset_id, _)).Times(1).WillOnce(Return());

        // GetAfSubjShiftSens
        EXPECT_CALL(ptzf_status_if_mock_, getAfSubjShiftSens(preset_id, _)).Times(1).WillOnce(Return());

        // GetFocusFaceEyedetection
        EXPECT_CALL(ptzf_status_if_mock_, getFocusFaceEyedetection(preset_id, _)).Times(1).WillOnce(Return());

        // GetFocusArea
        EXPECT_CALL(ptzf_status_if_mock_, getFocusArea(preset_id, _)).Times(1).WillOnce(Return());

        // GetAFAreaPositionAFC
        EXPECT_CALL(ptzf_status_if_mock_, getAFAreaPositionAFC(preset_id, _, _)).Times(1).WillOnce(Return());

        // GetAFAreaPositionAFS
        EXPECT_CALL(ptzf_status_if_mock_, getAFAreaPositionAFS(preset_id, _, _)).Times(1).WillOnce(Return());

        // GetZoomPosition
        EXPECT_CALL(ptzf_status_if_mock_, getZoomPosition(preset_id, _)).Times(1).WillOnce(Return());

        // GetFocusPosition
        EXPECT_CALL(ptzf_status_if_mock_, getFocusPosition(preset_id, _)).Times(1).WillOnce(Return());
    }

protected:
    MockHolderObject<ptp::driver::PtpDriverCommandIfMock> ptp_driver_if_mock_holder_;
    ptp::driver::PtpDriverCommandIfMock& ptp_driver_if_mock_;
    MockHolderObject<ptzf::PtzfStatusIfMock> ptzf_mock_holder_object_;
    ptzf::PtzfStatusIfMock& ptzf_status_if_mock_;
    MockHolderObject<visca::ViscaServerPtzfIfMock> visca_ptzf_if_mock_holder_object;
    visca::ViscaServerPtzfIfMock& visca_ptzf_if_mock_;
    MockHolderObject<visca::ViscaServerMessageIfMock> visca_if_mock_holder_object;
    visca::ViscaServerMessageIfMock& visca_if_mock_;
    common::MessageQueue reply_;
    PresetInfraMessageHandler handler_;
};

TEST_F(PresetInfraMessageHandlerTest, recallPresetSuccess)
{
    // PtzfStatusIf
    getPtzfStatusParameters(DEFAULT_PRESET_ID);

    // SetFocusMode
    u32_t dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_MODE_SETTING;
    u8_t value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAfTransitionSpeed
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_TRANSITION_SPEED;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAfSubjShiftSens
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_SUBJ_SHIFT_SENS;
    value = U8_T(5);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetFocusFaceEyedetection
    dp_code = ptp::CR_DEVICE_PROPERTY_FACE_EYE_DETECTIONAF;
    value = ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_ONLYAF;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetFocusArea
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_AREA;
    value = ptp::CR_FOCUS_AREA_WIDE;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAFAreaPositionAFC
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_C;
    uint32_t af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAFAreaPositionAFS
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_S;
    af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetZoomPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_ZOOM_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    uint32_t zoom_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(zoom_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    RecallPresetRequest msg(DEFAULT_PRESET_ID, reply_.getName());
    handler_.handleRequest(msg);

    // SetFocusPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    uint32_t focus_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(focus_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    int32_t event_code = ptp::CR_NOTIFY_ZOOM_POSITION_RESULT;
    ptp::driver::PtpDriverEventMessage zoom_result(event_code);
    handler_.handleRequest(zoom_result);

    event_code = ptp::CR_NOTIFY_FOCUS_POSITION_RESULT;
    ptp::driver::PtpDriverEventMessage focus_result(event_code);
    handler_.handleRequest(focus_result);

    preset::CompleteFcbPresetRecallResult result;
    reply_.pend(result);
    EXPECT_EQ(result.err, ERRORCODE_SUCCESS);
}

TEST_F(PresetInfraMessageHandlerTest, recallPresetZoomPositionFailure)
{
    // PtzfStatusIf
    getPtzfStatusParameters(DEFAULT_PRESET_ID);

    // SetFocusMode
    u32_t dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_MODE_SETTING;
    u8_t value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAfTransitionSpeed
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_TRANSITION_SPEED;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAfSubjShiftSens
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_SUBJ_SHIFT_SENS;
    value = U8_T(5);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetFocusFaceEyedetection
    dp_code = ptp::CR_DEVICE_PROPERTY_FACE_EYE_DETECTIONAF;
    value = ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_ONLYAF;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetFocusArea
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_AREA;
    value = ptp::CR_FOCUS_AREA_WIDE;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAFAreaPositionAFC
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_C;
    uint32_t af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAFAreaPositionAFS
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_S;
    af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetZoomPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_ZOOM_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    uint32_t zoom_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(zoom_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetFocusPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    uint32_t focus_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(focus_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    RecallPresetRequest msg(DEFAULT_PRESET_ID, reply_.getName());
    handler_.handleRequest(msg);

    int32_t event_code = ptp::CR_NOTIFY_FOCUS_POSITION_RESULT;
    ptp::driver::PtpDriverEventMessage focus_result(event_code);
    handler_.handleRequest(focus_result);

    preset::CompleteFcbPresetRecallResult result;
    reply_.pend(result);
    EXPECT_EQ(result.err, ERRORCODE_SUCCESS);
}

TEST_F(PresetInfraMessageHandlerTest, recallPresetFocusCommandFailure)
{
    u32_t preset_id = U32_T(1234);
    // PtzfStatusIf
    getPtzfStatusParameters(preset_id);

    // SetFocusMode
    u32_t dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_MODE_SETTING;
    u8_t value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetAfTransitionSpeed
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_TRANSITION_SPEED;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetAfSubjShiftSens
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_SUBJ_SHIFT_SENS;
    value = U8_T(5);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetFocusFaceEyedetection
    dp_code = ptp::CR_DEVICE_PROPERTY_FACE_EYE_DETECTIONAF;
    value = ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_ONLYAF;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetFocusArea
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_AREA;
    value = ptp::CR_FOCUS_AREA_WIDE;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetAFAreaPositionAFC
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_C;
    uint32_t af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetAFAreaPositionAFS
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_S;
    af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetZoomPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_ZOOM_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    uint32_t zoom_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(zoom_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    RecallPresetRequest msg(preset_id, reply_.getName());
    handler_.handleRequest(msg);

    // SetFocusPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    uint32_t focus_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(focus_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    int32_t event_code = ptp::CR_NOTIFY_ZOOM_POSITION_RESULT;
    ptp::driver::PtpDriverEventMessage zoom_result(event_code);
    handler_.handleRequest(zoom_result);

    preset::CompleteFcbPresetRecallResult result;
    reply_.pend(result);
    EXPECT_EQ(result.err, ERRORCODE_SUCCESS);
}

TEST_F(PresetInfraMessageHandlerTest, recallPresetAllCommandFailure)
{
    // PtzfStatusIf
    getPtzfStatusParameters(DEFAULT_PRESET_ID);

    // SetFocusMode
    u32_t dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_MODE_SETTING;
    u8_t value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetAfTransitionSpeed
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_TRANSITION_SPEED;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetAfSubjShiftSens
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_SUBJ_SHIFT_SENS;
    value = U8_T(5);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetFocusFaceEyedetection
    dp_code = ptp::CR_DEVICE_PROPERTY_FACE_EYE_DETECTIONAF;
    value = ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_ONLYAF;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetFocusArea
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_AREA;
    value = ptp::CR_FOCUS_AREA_WIDE;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetAFAreaPositionAFC
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_C;
    uint32_t af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetAFAreaPositionAFS
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_S;
    af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetZoomPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_ZOOM_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    uint32_t zoom_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(zoom_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // SetFocusPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    uint32_t focus_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(focus_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));
    RecallPresetRequest msg(DEFAULT_PRESET_ID, reply_.getName());
    handler_.handleRequest(msg);

    preset::CompleteFcbPresetRecallResult result;
    reply_.pend(result);
    EXPECT_EQ(result.err, ERRORCODE_SUCCESS);
}

TEST_F(PresetInfraMessageHandlerTest, cancelCameraRecallSuccess)
{
    // CancelZoomPosition
    uint32_t control_code = ptp::CR_COMMAND_ID_CANCEL_ZOOM_POSITION;
    uint64_t zoom_cancel_down_value = U64_T(0x0002);
    uint64_t zoom_cancel_up_value = U64_T(0x0001);
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(zoom_cancel_down_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(zoom_cancel_up_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // CancelFocusPosition
    control_code = ptp::CR_COMMAND_ID_CANCEL_FOCUS_POSITION;
    uint64_t focus_cancel_down_value = U64_T(0x0002);
    uint64_t focus_cancel_up_value = U64_T(0x0001);
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(focus_cancel_down_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(focus_cancel_up_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    CancelCameraRecallRequest msg(reply_.getName());
    handler_.handleRequest(msg);

    preset::CompleteFcbPresetRecallResult result;
    reply_.pend(result);
    EXPECT_EQ(result.err, ERRORCODE_SUCCESS);
}

TEST_F(PresetInfraMessageHandlerTest, cancelCameraRecallAllCommandFailure)
{
    // CancelZoomPosition
    uint32_t control_code = ptp::CR_COMMAND_ID_CANCEL_ZOOM_POSITION;
    uint64_t zoom_cancel_down_value = U64_T(0x0002);
    uint64_t zoom_cancel_up_value = U64_T(0x0001);
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(zoom_cancel_down_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(zoom_cancel_up_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // CancelFocusPosition
    control_code = ptp::CR_COMMAND_ID_CANCEL_FOCUS_POSITION;
    uint64_t focus_cancel_down_value = U64_T(0x0002);
    uint64_t focus_cancel_up_value = U64_T(0x0001);
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(focus_cancel_down_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(focus_cancel_up_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));
    CancelCameraRecallRequest msg(reply_.getName());
    handler_.handleRequest(msg);

    preset::CompleteFcbPresetRecallResult result;
    reply_.pend(result);
    EXPECT_EQ(result.err, ERRORCODE_SUCCESS);
}

TEST_F(PresetInfraMessageHandlerTest, recallPresetMultipleRequest)
{
    // 1Way
    // SetFocusMode
    EXPECT_CALL(ptzf_status_if_mock_, getFocusMode(DEFAULT_PRESET_ID, _)).Times(1).WillOnce(Return());
    u32_t dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_MODE_SETTING;
    u8_t value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAfTransitionSpeed
    EXPECT_CALL(ptzf_status_if_mock_, getAfTransitionSpeed(DEFAULT_PRESET_ID, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_TRANSITION_SPEED;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAfSubjShiftSens
    EXPECT_CALL(ptzf_status_if_mock_, getAfSubjShiftSens(DEFAULT_PRESET_ID, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_SUBJ_SHIFT_SENS;
    value = U8_T(5);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetFocusFaceEyedetection
    EXPECT_CALL(ptzf_status_if_mock_, getFocusFaceEyedetection(DEFAULT_PRESET_ID, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_FACE_EYE_DETECTIONAF;
    value = ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_ONLYAF;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetFocusArea
    EXPECT_CALL(ptzf_status_if_mock_, getFocusArea(DEFAULT_PRESET_ID, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_AREA;
    value = ptp::CR_FOCUS_AREA_WIDE;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAFAreaPositionAFC
    EXPECT_CALL(ptzf_status_if_mock_, getAFAreaPositionAFC(DEFAULT_PRESET_ID, _, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_C;
    uint32_t af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAFAreaPositionAFS
    EXPECT_CALL(ptzf_status_if_mock_, getAFAreaPositionAFS(DEFAULT_PRESET_ID, _, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_S;
    af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetZoomPosition
    EXPECT_CALL(ptzf_status_if_mock_, getZoomPosition(DEFAULT_PRESET_ID, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_ZOOM_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    uint32_t zoom_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(zoom_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    RecallPresetRequest msg(DEFAULT_PRESET_ID, reply_.getName());
    handler_.handleRequest(msg);

    // CancelRecall
    // CancelZoomPosition
    uint32_t control_code = ptp::CR_COMMAND_ID_CANCEL_ZOOM_POSITION;
    uint64_t zoom_cancel_down_value = U64_T(0x0002);
    uint64_t zoom_cancel_up_value = U64_T(0x0001);
    int32_t event_code = ptp::CR_NOTIFY_ZOOM_POSITION_RESULT;
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(zoom_cancel_down_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(zoom_cancel_up_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // CancelFocusPosition
    control_code = ptp::CR_COMMAND_ID_CANCEL_FOCUS_POSITION;
    uint64_t focus_cancel_down_value = U64_T(0x0002);
    uint64_t focus_cancel_up_value = U64_T(0x0001);
    event_code = ptp::CR_NOTIFY_FOCUS_POSITION_RESULT;
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(focus_cancel_down_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(focus_cancel_up_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    CancelCameraRecallRequest cancel_request(reply_.getName());
    handler_.handleRequest(cancel_request);

    preset::CompleteFcbPresetRecallResult result;
    reply_.pend(result);
    EXPECT_EQ(result.err, ERRORCODE_SUCCESS);

    // 2Way
    u32_t preset_id = U32_T(1234);
    // PtzfStatusIf
    getPtzfStatusParameters(preset_id);

    // SetFocusMode
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_MODE_SETTING;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAfTransitionSpeed
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_TRANSITION_SPEED;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAfSubjShiftSens
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_SUBJ_SHIFT_SENS;
    value = U8_T(5);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetFocusFaceEyedetection
    dp_code = ptp::CR_DEVICE_PROPERTY_FACE_EYE_DETECTIONAF;
    value = ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_ONLYAF;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetFocusArea
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_AREA;
    value = ptp::CR_FOCUS_AREA_WIDE;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAFAreaPositionAFC
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_C;
    af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAFAreaPositionAFS
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_S;
    af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetZoomPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_ZOOM_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    zoom_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(zoom_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    RecallPresetRequest msg2(preset_id, reply_.getName());
    handler_.handleRequest(msg2);

    // SetFocusPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    uint32_t focus_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(focus_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    event_code = ptp::CR_NOTIFY_ZOOM_POSITION_RESULT;
    ptp::driver::PtpDriverEventMessage zoom_result(event_code);
    handler_.handleRequest(zoom_result); // as a result of 1st zoom request
    handler_.handleRequest(zoom_result);

    event_code = ptp::CR_NOTIFY_FOCUS_POSITION_RESULT;
    ptp::driver::PtpDriverEventMessage focus_result(event_code);
    handler_.handleRequest(focus_result);

    reply_.pend(result);
    EXPECT_EQ(result.err, ERRORCODE_SUCCESS);
}

TEST_F(PresetInfraMessageHandlerTest, recallPresetMultipleRequestCancelCommandFailure)
{
    // 1Way
    // SetFocusMode
    EXPECT_CALL(ptzf_status_if_mock_, getFocusMode(DEFAULT_PRESET_ID, _)).Times(1).WillOnce(Return());
    u32_t dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_MODE_SETTING;
    u8_t value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAfTransitionSpeed
    EXPECT_CALL(ptzf_status_if_mock_, getAfTransitionSpeed(DEFAULT_PRESET_ID, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_TRANSITION_SPEED;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAfSubjShiftSens
    EXPECT_CALL(ptzf_status_if_mock_, getAfSubjShiftSens(DEFAULT_PRESET_ID, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_SUBJ_SHIFT_SENS;
    value = U8_T(5);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetFocusFaceEyedetection
    EXPECT_CALL(ptzf_status_if_mock_, getFocusFaceEyedetection(DEFAULT_PRESET_ID, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_FACE_EYE_DETECTIONAF;
    value = ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_ONLYAF;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetFocusArea
    EXPECT_CALL(ptzf_status_if_mock_, getFocusArea(DEFAULT_PRESET_ID, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_AREA;
    value = ptp::CR_FOCUS_AREA_WIDE;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAFAreaPositionAFC
    EXPECT_CALL(ptzf_status_if_mock_, getAFAreaPositionAFC(DEFAULT_PRESET_ID, _, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_C;
    uint32_t af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAFAreaPositionAFS
    EXPECT_CALL(ptzf_status_if_mock_, getAFAreaPositionAFS(DEFAULT_PRESET_ID, _, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_S;
    af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetZoomPosition
    EXPECT_CALL(ptzf_status_if_mock_, getZoomPosition(DEFAULT_PRESET_ID, _)).Times(1).WillOnce(Return());
    dp_code = ptp::CR_DEVICE_PROPERTY_ZOOM_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    uint32_t zoom_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(zoom_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    RecallPresetRequest msg(DEFAULT_PRESET_ID, reply_.getName());
    handler_.handleRequest(msg);

    // CancelRecall
    // CancelZoomPosition
    uint32_t control_code = ptp::CR_COMMAND_ID_CANCEL_ZOOM_POSITION;
    uint64_t zoom_cancel_down_value = U64_T(0x0002);
    uint64_t zoom_cancel_up_value = U64_T(0x0001);
    int32_t event_code = ptp::CR_NOTIFY_ZOOM_POSITION_RESULT;
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(zoom_cancel_down_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(zoom_cancel_up_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));

    // CancelFocusPosition
    control_code = ptp::CR_COMMAND_ID_CANCEL_FOCUS_POSITION;
    uint64_t focus_cancel_down_value = U64_T(0x0002);
    uint64_t focus_cancel_up_value = U64_T(0x0001);
    event_code = ptp::CR_NOTIFY_FOCUS_POSITION_RESULT;
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(focus_cancel_down_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));
    EXPECT_CALL(ptp_driver_if_mock_, sendCommand(control_code, Matcher<uint64_t>(focus_cancel_up_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_GENERIC));
    CancelCameraRecallRequest cancel_request(reply_.getName());
    handler_.handleRequest(cancel_request);

    preset::CompleteFcbPresetRecallResult result;
    reply_.pend(result);
    EXPECT_EQ(result.err, ERRORCODE_SUCCESS);

    // 2Way
    u32_t preset_id = U32_T(1234);
    // PtzfStatusIf
    getPtzfStatusParameters(preset_id);

    // SetFocusMode
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_MODE_SETTING;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAfTransitionSpeed
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_TRANSITION_SPEED;
    value = U8_T(1);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAfSubjShiftSens
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_SUBJ_SHIFT_SENS;
    value = U8_T(5);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetFocusFaceEyedetection
    dp_code = ptp::CR_DEVICE_PROPERTY_FACE_EYE_DETECTIONAF;
    value = ptp::CR_FACE_EYE_DETECTIONAF_FACE_EYE_ONLYAF;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetFocusArea
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_AREA;
    value = ptp::CR_FOCUS_AREA_WIDE;
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint8_t>(value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAFAreaPositionAFC
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_C;
    af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetAFAreaPositionAFS
    dp_code = ptp::CR_DEVICE_PROPERTY_AF_AREA_POSITION_AF_S;
    af_area_position_value = U32_T(0x00000000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(af_area_position_value)))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));

    // SetZoomPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_ZOOM_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    zoom_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(zoom_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    RecallPresetRequest msg2(preset_id, reply_.getName());
    handler_.handleRequest(msg2);

    // SetFocusPosition
    dp_code = ptp::CR_DEVICE_PROPERTY_FOCUS_POSITION_SETTING;
    EXPECT_CALL(ptp_driver_if_mock_, getDevicePropertyEnableFlag(dp_code, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(ptp::CR_ENABLE_VALUE_TRUE), Return(ptp::CR_ERROR_NONE)));
    uint32_t focus_value = U32_T(0x0000);
    EXPECT_CALL(ptp_driver_if_mock_, setDeviceProperty(dp_code, Matcher<uint32_t>(focus_value), _))
        .Times(1)
        .WillOnce(Return(ptp::CR_ERROR_NONE));
    event_code = ptp::CR_NOTIFY_ZOOM_POSITION_RESULT;
    ptp::driver::PtpDriverEventMessage zoom_result(event_code);
    handler_.handleRequest(zoom_result); // as a result of 1st zoom request
    handler_.handleRequest(zoom_result);

    event_code = ptp::CR_NOTIFY_FOCUS_POSITION_RESULT;
    ptp::driver::PtpDriverEventMessage focus_result(event_code);
    handler_.handleRequest(focus_result);

    reply_.pend(result);
    EXPECT_EQ(result.err, ERRORCODE_SUCCESS);
}

TEST_F(PresetInfraMessageHandlerTest, setPanTiltSlowModeRequestSuccess)
{
    uint32_t seq_id;
    visca::CompReply visca_reply;
    visca_reply.status = ERRORCODE_SUCCESS;
    const u32_t UNUSED_SEQ_ID = U32_T(1);

    // 1way
    common::MessageQueueName blank_name;
    gtl::copyString(blank_name.name, "");
    SetPanTiltSlowModeRequest msg(false, blank_name);
    EXPECT_CALL(visca_ptzf_if_mock_, sendExecutablePanTiltSlowModeRequest(false, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<2>(&seq_id), Return()));
    handler_.handleRequest(msg);
    visca_reply.seq_id = seq_id;
    handler_.handleRequest(visca_reply);

    // 2way
    msg.slow = true;
    msg.reply_name = reply_.getName();
    EXPECT_CALL(visca_ptzf_if_mock_, sendExecutablePanTiltSlowModeRequest(true, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<2>(&seq_id), Return()));
    handler_.handleRequest(msg);
    visca_reply.seq_id = seq_id;
    handler_.handleRequest(visca_reply);

    preset::infra::CompReply result;
    reply_.pend(result);
    EXPECT_EQ(UNUSED_SEQ_ID + U8_T(2), result.seq_id);
    EXPECT_EQ(ERRORCODE_SUCCESS, result.status);
}

TEST_F(PresetInfraMessageHandlerTest, setPanTiltSlowModeRequestError)
{
    uint32_t seq_id;
    common::MessageQueue mq;
    const u32_t UNUSED_SEQ_ID = U32_T(1);

    SetPanTiltSlowModeRequest msg(true, reply_.getName());
    EXPECT_CALL(visca_ptzf_if_mock_, sendExecutablePanTiltSlowModeRequest(true, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<2>(&seq_id), Return()));
    handler_.handleRequest(msg);

    visca::CompReply visca_reply;
    visca_reply.status = ERRORCODE_EXEC;
    visca_reply.seq_id = seq_id;
    handler_.handleRequest(visca_reply);

    preset::infra::CompReply result;
    reply_.pend(result);
    EXPECT_EQ(UNUSED_SEQ_ID + U8_T(1), result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.status);
}

TEST_F(PresetInfraMessageHandlerTest, setPanTiltSlowModeRequestMultipleRequest)
{
    u32_t seq_id1;
    u32_t seq_id2;
    common::MessageQueue mq;
    const u32_t UNUSED_SEQ_ID = U32_T(1);

    SetPanTiltSlowModeRequest msg1(false, reply_.getName());
    EXPECT_CALL(visca_ptzf_if_mock_, sendExecutablePanTiltSlowModeRequest(false, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<2>(&seq_id1), Return()));
    handler_.handleRequest(msg1);

    SetPanTiltSlowModeRequest msg2(true, reply_.getName());
    EXPECT_CALL(visca_ptzf_if_mock_, sendExecutablePanTiltSlowModeRequest(true, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<2>(&seq_id2), Return()));
    handler_.handleRequest(msg2);

    visca::CompReply visca_reply;
    visca_reply.status = ERRORCODE_EXEC;
    visca_reply.seq_id = seq_id2;
    handler_.handleRequest(visca_reply);

    preset::infra::CompReply result;
    reply_.pend(result);
    EXPECT_EQ(UNUSED_SEQ_ID + U8_T(2), result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.status);

    visca_reply.status = ERRORCODE_SUCCESS;
    visca_reply.seq_id = seq_id1;
    handler_.handleRequest(visca_reply);

    reply_.pend(result);
    EXPECT_EQ(UNUSED_SEQ_ID + U8_T(1), result.seq_id);
    EXPECT_EQ(ERRORCODE_SUCCESS, result.status);
}

TEST_F(PresetInfraMessageHandlerTest, setPanTiltSlowModeWithSeqIdRequestSuccess)
{
    u32_t biz_seq_id = U32_T(1234);
    u32_t infra_seq_id;
    visca::CompReply visca_reply;
    visca_reply.status = ERRORCODE_SUCCESS;

    // 1way
    common::MessageQueueName blank_name;
    gtl::copyString(blank_name.name, "");
    SetPanTiltSlowModeWithSeqIdRequest msg(false, blank_name, biz_seq_id);
    EXPECT_CALL(visca_ptzf_if_mock_, sendExecutablePanTiltSlowModeRequest(false, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<2>(&infra_seq_id), Return()));
    handler_.handleRequest(msg);
    visca_reply.seq_id = infra_seq_id;
    handler_.handleRequest(visca_reply);

    // 2way
    msg.slow = true;
    msg.reply_name = reply_.getName();
    EXPECT_CALL(visca_ptzf_if_mock_, sendExecutablePanTiltSlowModeRequest(true, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<2>(&infra_seq_id), Return()));
    handler_.handleRequest(msg);
    visca_reply.seq_id = infra_seq_id;
    handler_.handleRequest(visca_reply);

    preset::infra::CompReply result;
    reply_.pend(result);
    EXPECT_EQ(biz_seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_SUCCESS, result.status);
}

TEST_F(PresetInfraMessageHandlerTest, setPanTiltSlowModeWithSeqIdRequestError)
{
    u32_t biz_seq_id = U32_T(1);
    u32_t infra_seq_id;
    common::MessageQueue mq;

    SetPanTiltSlowModeWithSeqIdRequest msg(true, reply_.getName(), biz_seq_id);
    EXPECT_CALL(visca_ptzf_if_mock_, sendExecutablePanTiltSlowModeRequest(true, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<2>(&infra_seq_id), Return()));
    handler_.handleRequest(msg);

    visca::CompReply visca_reply;
    visca_reply.status = ERRORCODE_EXEC;
    visca_reply.seq_id = infra_seq_id;
    handler_.handleRequest(visca_reply);

    preset::infra::CompReply result;
    reply_.pend(result);
    EXPECT_EQ(biz_seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.status);
}

TEST_F(PresetInfraMessageHandlerTest, setPanTiltSlowModeWithSeqIdRequestMultipleRequest)
{
    u32_t biz_seq_id1 = U32_T(123);
    u32_t biz_seq_id2 = U32_T(456);
    u32_t infra_seq_id1;
    u32_t infra_seq_id2;
    common::MessageQueue mq;

    SetPanTiltSlowModeWithSeqIdRequest msg1(false, reply_.getName(), biz_seq_id1);
    EXPECT_CALL(visca_ptzf_if_mock_, sendExecutablePanTiltSlowModeRequest(false, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<2>(&infra_seq_id1), Return()));
    handler_.handleRequest(msg1);

    SetPanTiltSlowModeWithSeqIdRequest msg2(true, reply_.getName(), biz_seq_id2);
    EXPECT_CALL(visca_ptzf_if_mock_, sendExecutablePanTiltSlowModeRequest(true, _, _))
        .Times(1)
        .WillOnce(DoAll(SaveArg<2>(&infra_seq_id2), Return()));
    handler_.handleRequest(msg2);

    visca::CompReply visca_reply;
    visca_reply.status = ERRORCODE_EXEC;
    visca_reply.seq_id = infra_seq_id2;
    handler_.handleRequest(visca_reply);

    preset::infra::CompReply result;
    reply_.pend(result);
    EXPECT_EQ(biz_seq_id2, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.status);

    visca_reply.status = ERRORCODE_SUCCESS;
    visca_reply.seq_id = infra_seq_id1;
    handler_.handleRequest(visca_reply);

    reply_.pend(result);
    EXPECT_EQ(biz_seq_id1, result.seq_id);
    EXPECT_EQ(ERRORCODE_SUCCESS, result.status);
}

TEST_F(PresetInfraMessageHandlerTest, setRecallFreezeModeSuccess)
{
    u32_t seq_id = U32_T(123);

    EXPECT_CALL(visca_if_mock_, sendCamFreezeRequest(Eq(false), _, Eq(seq_id))).Times(1);

    SetRecallFreezeModeRequest msg(false, reply_, seq_id);
    handler_.handleRequest(msg);
}

TEST_F(PresetInfraMessageHandlerTest, getPanTiltTimeRequestSuccess)
{
    u8_t speed = U8_T(0x18);
    u32_t pan = U32_T(0x12);
    u32_t tilt = U32_T(0x34);
    u32_t seq_id = U32_T(123);
    preset::infra::PanTiltTimeReply result;

    GetPanTiltTimeRequest msg(speed, pan, tilt, reply_.getName(), seq_id);
    EXPECT_CALL(visca_if_mock_, sendPanTiltTimeInqRequest(speed, pan, tilt, _, seq_id))
        .Times(2)
        .WillRepeatedly(Return());

    // normal
    handler_.handleRequest(msg);
    visca::PanTiltTimeInqReply inq_reply;
    inq_reply.seq_id = seq_id;
    inq_reply.time = U8_T(0x80);
    handler_.handleRequest(inq_reply);
    reply_.pend(result);
    EXPECT_EQ(U8_T(0x80), result.time);
    EXPECT_EQ(seq_id, result.seq_id);

    // time out
    handler_.handleRequest(msg);
    visca::CompReply comp_reply(ERRORCODE_TIMEOUT, seq_id);
    handler_.handleRequest(comp_reply);
}

} // namespace infra
} // namespace preset
