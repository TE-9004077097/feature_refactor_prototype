/*
 * biz_ptzf_if_test.cpp
 *
 * Copyright 2018 Sony Imaging Products & Solutions Inc.
 */

#include <string>
#include "types.h"
#include "gtl_string.h"
#include "gtl_memory.h"
#include "gtl_array.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "common_gmock_util.h"
#include "common_message_queue.h"
#include "biz_ptzf_if.h"
#include "ptzf/ptzf_message.h"
#include "event_router/event_router_if_mock.h"
#include "event_router/event_router_target_type.h"
#include "ptzf/ptzf_status_if.h"
#include "../ptzf/ptzf_status.h"
#include "visca/dboutputs/enum.h"
#include "application/error_notifier/error_notifier_message_if.h"
#include "ptzf/ptzf_biz_message_if_mock.h"
#include "visca/visca_status_if_mock.h"
#include "ptzf/ptz_trace_status_if_mock.h"
#include "ptzf/ptzf_common_message.h"
#include "bizglobal.h"
#include "inbound/general/model_name.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Eq;
using ::testing::SetArgReferee;
using ::testing::Field;
using ::testing::StrCaseEq;
using ::testing::Invoke;

// ○テストリスト
// + sendPanTiltMoveRequest()
// + sendZoomMoveRequest()
// + sendFocusModeRequest()
// + sendFocusMoveRequest()
// + sendPanTiltResetRequest()
//   - 無名MQ・INVALID_SEQ_IDを指定した場合はPanTiltResetRequestを送信しない事
//   - ReplyMQ・SEQ_IDのみを指定した場合、mode_checkedがfalseのPanTiltResetRequestを送信する事
//   - ReplyMQ・SEQ_ID・checkedを指定した場合、指定した内容のPanTiltResetRequestを送信する事
// + sendIfClearRequest()
// + sendZoomFineMoveRequest()
// + setPanTiltRampCurve()
// + setPanTiltMotorPower()
// + setPanTiltSlowMode()
// + setPanTiltImageFlipMode()
// + setPanTiltLimit()
// + setPanTiltPanLimitOn()
// + setPanTiltPanLimitOff()
// + setPanTiltTiltLimitOn()
// + setPanTiltTiltLimitOff()
// + setIRCorrection()
// + setTeleShiftMode()
// + setDZoomMode()
// + setZoomAbsolutePosition()
// + setZoomRelativePosition
// + setFocusAbsolutePosition()
// + setFocusRelativePosition
// + setFocusOnePushTrigger()
// + setFocusAfSensitivity()
// + setFocusNearLimit()
// + setFocusAFMode()
// + setFocusFaceEyedetection()
// + setAfAssist()
// + setFocusTrackingPosition()
// + setTouchFunctionInMf()
// + setFocusAFTimer()
// + setPanTiltLimitClear()
// + setPTZMode()
// + setPTZPanTiltMove()
// + setPTZZoomMove()
// + setPanTiltAbsolutePosition()
// + setPanTiltRelativePosition()
// + setPanTiltRelativeMove()
// + setZoomRelativeMove()
// + setHomePosition()
// + setStandbyMode()
// + exeFocusHoldButton()
// + exePushFocusButton()
// + setFocusTrackingCancel()
// + setAfSubjShiftSens()
// + setAfTransitionSpeed()
// + setPanTiltSpeedMode()
// + setPanTiltSpeedStep()
// + setSettingPosition()
// + setPanDirection()
// + setTiltDirection()
// + setZoomSpeedScale()
// + setPushAFMode()
// + exeCancelZoomPosition
// + exeCancelFocusPosition
// + setFocusMode
// + setAfTransitionSpeedValue
// + setAfSubjShiftSensValue
// + setFocusFaceEyedetectionValue
// + setFocusArea
// + setAFAreaPositionAFC
// + setAFAreaPositionAFS
// + setZoomPosition
// + setFocusPosition
//    上記メソッドそれぞれについて、以下の観点を確認
//    + PtzControllerMessageHandler向けに送信しているメッセージが正しいこと(1Way/2Wayそれぞれ)
//    + enum未定義の設定値である場合、PtzControllerMessageHandler向けにメッセージを送信しないこと(1Way/2Wayそれぞれ)
//    + 引数のシーケンスIDがINVALID_SEQ_ID(0)の場合、PtzControllerMessageHandler向けにメッセージを送信しないこと
//
// + getPanTiltPosition()
// + getPanTiltStatus()
// + getPanTiltSlowMode()
// + getPanTiltImageFlipMode()
// + getPanTiltImageFlipModePreset()
// + getPanLimitLeft()
// + getPanLimitRight()
// + getTiltLimitUp()
// + getTiltLimitDown()
// + getIRCorrection()
// + getPanLimitMode()
// + getTiltLimitMode()
// + getDZoomMode()
// + getAbsoluteZoomPosition()
// + getFocusMode()
// + getFocusAbsolutePosition()
// + getFocusAFMode()
// + getFocusFaceEyedetection()
// + getAfAssist()
// + getTouchFunctionInMf()
// + getFocusAFTimer()
// + getFocusAfSensitivity()
// + getFocusNearLimit()
// + getPTZMode()
// + getPTZPanTiltMove()
// + getPTZZoomMove()
// + getPanTiltMaxSpeeds()
// + getPanMovementRange()
// + getTiltMovementRange()
// + getOpticalZoomMaxMagnification()
// + getZoomMovementRange()
// + getZoomMaxVelocity()
// + getZoomStatus()
// + getFocusStatus()
// + getPanTiltMoveStatus()
// + getAfSubjShiftSens()
// + getAfTransitionSpeed()
// + getFocusTrackingStatus()
//    上記メソッドそれぞれについて、以下の観点を確認
//    + PtzControllerMessageHandler向けの送信しているメッセージが正しいこと
//    + enum未定義の取得値である場合、、PtzControllerMessageHandler向けにメッセージを送信しないこと
//    + 結果を通知するメッセージキュー名が未設定の場合、PtzControllerMessageHandler向けにメッセージを送信しないこと
// + getStandbyMode()
// + getCamOp()
// + getPanTiltSpeedScale()
// + getPanTiltSpeedMode()
// + getPanTiltSpeedStep()
// + getSettingPosition()
// + getPanDirection()
// + getTiltDirection()
// + getZoomSpeedScale()
//   - PtzfBizMessageIfがtrueを返す場合
// + getFocusTrackingPositionPmt()
//   - PtzfBizMessageIfがtrueを返す場合
// + getFocusTrackingCancelPmt()
//   - PtzfBizMessageIfがtrueを返す場合
// + getFocusModePmt()
//   - PtzfBizMessageIfがtrueを返す場合
// + getFocusAbsolutePositionPmt()
//   - PtzfBizMessageIfがtrueを返す場合
// + getPushFocusButtonPmt()
//   - PtzfBizMessageIfがtrueを返す場合
// + getFocusHoldButtonPmt()
//   - PtzfBizMessageIfがtrueを返す場合
// + getFocusFaceEyedetectionPmt()
//   - PtzfBizMessageIfがtrueを返す場合
// + getAFTransitionSpeedPmt()
//   - PtzfBizMessageIfがtrueを返す場合
// + getAfSubjShitSensPmt()
//   - PtzfBizMessageIfがtrueを返す場合
// + getTouchFunctionInMfPmt()
//   - PtzfBizMessageIfがtrueを返す場合
// + getPushAFModePmt()
//   - PtzfBizMessageIfがtrueを返す場合
// + getAfAssistPmt()
//   - PtzfBizMessageIfがtrueを返す場合
// + getZoomFineMoveRequestPmt()
//   - PtzfBizMessageIfがtrueを返す場合
// + getIndicatorFocusModeState()
// + getIndicatorFaceEyeAFState()
// + getIndicatorRegisteredTrackingFaceState()
// + getIndicatorTrackingAFStopState()
// + getIndicatorFocusPositionMeterState()
// + getIndicatorFocusPositionFeetState()
// + getIndicatorFocusPositionUnitState()
// + getIndicatorFocusPositionPmt()
// + getIndicatorZoomPositionState()
// + getIndicatorZoomPositionRateState()
// + getIndicatorZoomPositionUnitState()
// + getIndicatorZoomPositionPmt()
// + getIndicatorCizIconState()
// + getIndicatorCizRatioState()
// + getIndicatorCizRatioPmt()
// + getPanTiltLockStatus()
// + getPanTiltEnabledState()
//   - ENABLE / DISABLEの各状態をdomainの定義からbizの定義に変換して返せていることを確認
//   - domainからErrorが通知された場合、それを呼び出し元に返せていることを確認

namespace config {

struct ConfigReadyNotification
{};

} // namespace config

namespace biz_ptzf {

namespace {

const DZoom zoom_mode_values[] = { DZOOM_FULL, DZOOM_OPTICAL, DZOOM_CLEAR_IMAGE };
const AFMode focus_af_mode_values[] = { AUTO_FOCUS_NORMAL, AUTO_FOCUS_INTERVAL, AUTO_FOCUS_ZOOM_TRIGGER };
const AFSensitivityMode af_sensitivity_value[] = { AF_SENSITIVITY_MODE_NORMAL, AF_SENSITIVITY_MODE_LOW };
const uint8_t af_subj_shift_sens_values[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
const uint8_t af_transition_speed_values[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
const FocusMode focus_mode_values[] = { FOCUS_MODE_AUTO, FOCUS_MODE_MANUAL, FOCUS_MODE_TOGGLE };
const FocusArea focus_area_values[] = { FOCUS_AREA_WIDE, FOCUS_AREA_ZONE, FOCUS_AREA_FLEXIBLE_SPOT };
const u16_t u16_t_case_list[] = { U32_T(0), U32_T(1), U32_T(10), U32_T(65534), U32_T(65535) };
const u32_t u32_t_case_list[] = { U32_T(0), U32_T(1), U32_T(10), U32_T(4294967294), U32_T(4294967295) };

} // namespace

class BizPtzfIfTest : public ::testing::Test
{
protected:
    BizPtzfIfTest()
        : event_router_mock_holder_(),
          er_mock_(event_router_mock_holder_.getMock()),
          reply_(),
          ptzf_biz_message_if_mock_holder_(),
          ptzf_biz_message_if_mock_(ptzf_biz_message_if_mock_holder_.getMock()),
          visca_status_mock_holder_(),
          visca_status_if_mock_(visca_status_mock_holder_.getMock()),
          ptz_trace_status_if_mock_holder_(),
          ptz_trace_status_if_mock_(ptz_trace_status_if_mock_holder_.getMock())
    {}

    virtual void SetUp()
    {
        bizglobal::ModelName model;
        gtl::copyString(model.model_name_, "BRC-X400");
        bizglobal::BizGlobal::instance().update(model);

        common::MessageQueue mq("ConfigReadyMQ");
        config::ConfigReadyNotification message;
        mq.post(message);
    }

    virtual void TearDown()
    {
        common::MessageQueue er_mq("EventRouterMQ");
        er_mq.unlink();
        common::MessageQueue config_mq("ConfigReadyMQ");
        config_mq.unlink();
        reply_.unlink();
    }

protected:
    MockHolderObject<event_router::EventRouterIfMock> event_router_mock_holder_;
    event_router::EventRouterIfMock& er_mock_;
    common::MessageQueue reply_;
    MockHolderObject<ptzf::PtzfBizMessageIfMock> ptzf_biz_message_if_mock_holder_;
    ptzf::PtzfBizMessageIfMock& ptzf_biz_message_if_mock_;
    MockHolderObject<visca::ViscaStatusIfMock> visca_status_mock_holder_;
    visca::ViscaStatusIfMock& visca_status_if_mock_;
    MockHolderObject<ptzf::PtzTraceStatusIfMock> ptz_trace_status_if_mock_holder_;
    ptzf::PtzTraceStatusIfMock& ptz_trace_status_if_mock_;
};

namespace {

struct MQResponder
{
    void returnResultZoomAckOn(const event_router::EventRouterTargetType,
                               const u32_t,
                               const u32_t,
                               const char_t* arg_msg)
    {
        ptzf::SetZoomAbsolutePositionRequest* msg = (ptzf::SetZoomAbsolutePositionRequest*)arg_msg;
        common::MessageQueue mq(msg->biz_mq_name.name);
        ptzf::message::PtzfZoomAbsoluteAck result(DEFAULT_SEQ_ID, true);
        mq.post(result);
    }
    void returnResultZoomAckOff(const event_router::EventRouterTargetType,
                                const u32_t,
                                const u32_t,
                                const char_t* arg_msg)
    {
        ptzf::SetZoomAbsolutePositionRequest* msg = (ptzf::SetZoomAbsolutePositionRequest*)arg_msg;
        common::MessageQueue mq(msg->biz_mq_name.name);
        ptzf::message::PtzfZoomAbsoluteAck result(DEFAULT_SEQ_ID, false);
        mq.post(result);
    }
    void returnResultFocusAckOn(const event_router::EventRouterTargetType,
                                const u32_t,
                                const u32_t,
                                const char_t* arg_msg)
    {
        ptzf::SetFocusAbsolutePositionRequest* msg = (ptzf::SetFocusAbsolutePositionRequest*)arg_msg;
        common::MessageQueue mq(msg->biz_mq_name.name);
        ptzf::message::PtzfFocusAbsoluteAck result(DEFAULT_SEQ_ID, true);
        mq.post(result);
    }
    void returnResultFocusAckOff(const event_router::EventRouterTargetType,
                                 const u32_t,
                                 const u32_t,
                                 const char_t* arg_msg)
    {
        ptzf::SetFocusAbsolutePositionRequest* msg = (ptzf::SetFocusAbsolutePositionRequest*)arg_msg;
        common::MessageQueue mq(msg->biz_mq_name.name);
        ptzf::message::PtzfFocusAbsoluteAck result(DEFAULT_SEQ_ID, false);
        mq.post(result);
    }
};

const struct PanTiltMoveTestValueTable
{
    PanTiltDirection in_dir;
    ptzf::PanTiltDirection out_dir;
} pan_tilt_move_test_value_table[] = {
    { PAN_TILT_DIRECTION_STOP, ptzf::PAN_TILT_DIRECTION_STOP },
    { PAN_TILT_DIRECTION_UP, ptzf::PAN_TILT_DIRECTION_UP },
    { PAN_TILT_DIRECTION_DOWN, ptzf::PAN_TILT_DIRECTION_DOWN },
    { PAN_TILT_DIRECTION_LEFT, ptzf::PAN_TILT_DIRECTION_LEFT },
    { PAN_TILT_DIRECTION_RIGHT, ptzf::PAN_TILT_DIRECTION_RIGHT },
    { PAN_TILT_DIRECTION_UP_LEFT, ptzf::PAN_TILT_DIRECTION_UP_LEFT },
    { PAN_TILT_DIRECTION_UP_RIGHT, ptzf::PAN_TILT_DIRECTION_UP_RIGHT },
    { PAN_TILT_DIRECTION_DOWN_LEFT, ptzf::PAN_TILT_DIRECTION_DOWN_LEFT },
    { PAN_TILT_DIRECTION_DOWN_RIGHT, ptzf::PAN_TILT_DIRECTION_DOWN_RIGHT },
};

const struct ZoomMoveTestValueTable
{
    u8_t in_speed;
    ZoomDirection in_dir;
    u8_t out_speed;
    ptzf::ZoomDirection out_dir;
} zoom_move_test_value_table[] = {
    { U8_T(1), ZOOM_DIRECTION_STOP, U8_T(1), ptzf::ZOOM_DIRECTION_STOP },
    { U8_T(1), ZOOM_DIRECTION_TELE, U8_T(1), ptzf::ZOOM_DIRECTION_TELE },
    { U8_T(1), ZOOM_DIRECTION_WIDE, U8_T(1), ptzf::ZOOM_DIRECTION_WIDE },
    { U8_T(24), ZOOM_DIRECTION_STOP, U8_T(24), ptzf::ZOOM_DIRECTION_STOP },
    { U8_T(24), ZOOM_DIRECTION_TELE, U8_T(24), ptzf::ZOOM_DIRECTION_TELE },
    { U8_T(24), ZOOM_DIRECTION_WIDE, U8_T(24), ptzf::ZOOM_DIRECTION_WIDE },
};

const struct FocusModeTestValueTable
{
    FocusMode in_mode;
    ptzf::FocusMode out_mode;
} focus_mode_test_value_table[] = {
    { FOCUS_MODE_AUTO, ptzf::FOCUS_MODE_AUTO },
    { FOCUS_MODE_MANUAL, ptzf::FOCUS_MODE_MANUAL },
    { FOCUS_MODE_TOGGLE, ptzf::FOCUS_MODE_TOGGLE },
};

const struct FocusDirectionTestValueTable
{
    FocusDirection in_dir;
    ptzf::FocusDirection out_dir;
} focus_direction_test_value_table[] = {
    { FOCUS_DIRECTION_STOP, ptzf::FOCUS_DIRECTION_STOP },
    { FOCUS_DIRECTION_FAR, ptzf::FOCUS_DIRECTION_FAR },
    { FOCUS_DIRECTION_NEAR, ptzf::FOCUS_DIRECTION_NEAR },
};

const struct PanTiltLimitTypeTestValueTable
{
    PanTiltLimitType in_type;
    ptzf::PanTiltLimitType out_type;
} pan_tilt_limit_type_test_value_table[] = {
    { PAN_TILT_LIMIT_TYPE_DOWN_LEFT, ptzf::PAN_TILT_LIMIT_TYPE_DOWN_LEFT },
    { PAN_TILT_LIMIT_TYPE_UP_RIGHT, ptzf::PAN_TILT_LIMIT_TYPE_UP_RIGHT },
};

const struct IRCorrectionTestValueTable
{
    IRCorrection in_ir_correction;
    ptzf::IRCorrection out_ir_correction;
} ir_correction_test_value_table[] = { { IR_CORRECTION_STANDARD, ptzf::IR_CORRECTION_STANDARD },
                                       { IR_CORRECTION_IRLIGHT, ptzf::IR_CORRECTION_IRLIGHT } };

const struct ZoomModeTestValueTable
{
    DZoom in_zoom;
    ptzf::DZoom out_zoom;
} zoom_mode_test_value_table[] = {
    { DZOOM_FULL, ptzf::DZOOM_FULL },
    { DZOOM_OPTICAL, ptzf::DZOOM_OPTICAL },
    { DZOOM_CLEAR_IMAGE, ptzf::DZOOM_CLEAR_IMAGE },
};

const struct AfSensitivityTestValueTable
{
    AFSensitivityMode in_mode;
    ptzf::AFSensitivityMode out_mode;
} af_sensitivity_test_value_table[] = {
    { AF_SENSITIVITY_MODE_NORMAL, ptzf::AF_SENSITIVITY_MODE_NORMAL },
    { AF_SENSITIVITY_MODE_LOW, ptzf::AF_SENSITIVITY_MODE_LOW },
};

const struct AfModeTestValueTable
{
    AFMode in_mode;
    ptzf::AutoFocusMode out_mode;
} af_mode_test_value_table[] = {
    { AUTO_FOCUS_NORMAL, ptzf::AUTO_FOCUS_MODE_NORMAL },
    { AUTO_FOCUS_INTERVAL, ptzf::AUTO_FOCUS_MODE_INTERVAL },
    { AUTO_FOCUS_ZOOM_TRIGGER, ptzf::AUTO_FOCUS_MODE_ZOOMTRIGGER },
};

const struct FocusFaceEyeDetectionModeTestValueTable
{
    FocusFaceEyeDetectionMode in_mode;
    ptzf::FocusFaceEyeDetectionMode out_mode;
} focus_face_eye_detection_mode_test_value_table[] = {
    { FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY, ptzf::FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY },
    { FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_PRIORITY, ptzf::FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_PRIORITY },
    { FOCUS_FACE_EYE_DETECTION_MODE_OFF, ptzf::FOCUS_FACE_EYE_DETECTION_MODE_OFF },
};

const struct AfAssistTestValueTable
{
    bool in_mode;
    bool out_mode;
} af_assist_test_value_table[] = {
    { true, true },
    { false, false },
};

const struct FocusTrackingPositionTestValueTable
{
    uint16_t in_mode_x;
    uint16_t in_mode_y;
    uint16_t out_mode_x;
    uint16_t out_mode_y;
} focus_tracking_position_test_value_table[] = {
    { 0x00, 0x00, 0x00, 0x00 },
    { 0x12, 0x34, 0x12, 0x34 },
    { 0xcd, 0xef, 0xcd, 0xef },
};

const struct TouchFunctionInMfTestValueTable
{
    TouchFunctionInMf in_mode;
    ptzf::TouchFunctionInMf out_mode;
} touch_function_in_mf_test_value_table[] = {
    { TOUCH_FUNCTION_IN_MF_TRACKING_AF, ptzf::TOUCH_FUNCTION_IN_MF_TRACKING_AF },
    { TOUCH_FUNCTION_IN_MF_SPOT_FOCUS, ptzf::TOUCH_FUNCTION_IN_MF_SPOT_FOCUS },
};

const struct PTZRelativeAmountTestValueTable
{
    PTZRelativeAmount in_dir;
    ptzf::PTZRelativeAmount out_dir;
} ptz_relative_amount_test_value_table[] = {
    { PTZ_RELATIVE_AMOUNT_1, ptzf::PTZ_RELATIVE_AMOUNT_1 }, { PTZ_RELATIVE_AMOUNT_2, ptzf::PTZ_RELATIVE_AMOUNT_2 },
    { PTZ_RELATIVE_AMOUNT_3, ptzf::PTZ_RELATIVE_AMOUNT_3 }, { PTZ_RELATIVE_AMOUNT_4, ptzf::PTZ_RELATIVE_AMOUNT_4 },
    { PTZ_RELATIVE_AMOUNT_5, ptzf::PTZ_RELATIVE_AMOUNT_5 }, { PTZ_RELATIVE_AMOUNT_6, ptzf::PTZ_RELATIVE_AMOUNT_6 },
    { PTZ_RELATIVE_AMOUNT_7, ptzf::PTZ_RELATIVE_AMOUNT_7 }, { PTZ_RELATIVE_AMOUNT_8, ptzf::PTZ_RELATIVE_AMOUNT_8 },
    { PTZ_RELATIVE_AMOUNT_9, ptzf::PTZ_RELATIVE_AMOUNT_9 }, { PTZ_RELATIVE_AMOUNT_10, ptzf::PTZ_RELATIVE_AMOUNT_10 },
};

const struct ZoomRlativeMoveTestValueTable
{
    ZoomDirection in_dir;
    ptzf::ZoomDirection out_dir;
} zoom_move_relative_test_value_table[] = {
    { ZOOM_DIRECTION_STOP, ptzf::ZOOM_DIRECTION_STOP },
    { ZOOM_DIRECTION_TELE, ptzf::ZOOM_DIRECTION_TELE },
    { ZOOM_DIRECTION_WIDE, ptzf::ZOOM_DIRECTION_WIDE },
};

const struct PTZModeTestValueTable
{
    PTZMode in_mode;
    ptzf::PTZMode out_mode;
} ptz_mode_test_value_table[] = {
    { PTZ_MODE_NORMAL, ptzf::PTZ_MODE_NORMAL },
    { PTZ_MODE_STEP, ptzf::PTZ_MODE_STEP },
};

const struct PushAFModeTestValueTable
{
    PushAfMode in_mode;
    ptzf::PushAfMode out_mode;
} push_af_mode_value_table[] = {
    { PUSH_AF_MODE_AF, ptzf::PUSH_AF_MODE_AF },
    { PUSH_AF_MODE_AF_SINGLE_SHOT, ptzf::PUSH_AF_MODE_AF_SINGLE_SHOT },
};

static const struct StandbyModeTestTable
{
    ptzf::StandbyMode ptzf_value;
    biz_ptzf::StandbyMode biz_ptzf_value;
} standby_mode_test_table[] = { { ptzf::StandbyMode::NEUTRAL, StandbyMode::NEUTRAL },
                                { ptzf::StandbyMode::SIDE, StandbyMode::SIDE } };

static const struct ZoomFineMoveRequest
{
    ZoomDirection in_dir;
    ptzf::ZoomDirection out_dir;
} zoom_fine_move_test_value_table[] = {
    { ZOOM_DIRECTION_STOP, ptzf::ZOOM_DIRECTION_STOP },
    { ZOOM_DIRECTION_TELE, ptzf::ZOOM_DIRECTION_TELE },
    { ZOOM_DIRECTION_WIDE, ptzf::ZOOM_DIRECTION_WIDE },
};

MATCHER_P5(EqPanTiltMoveRequest, direction, pan_speed, tilt_speed, seq_id, mq_name, "")
{
    const ptzf::PanTiltMoveRequest* req = reinterpret_cast<const ptzf::PanTiltMoveRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->direction == direction && req->pan_speed == pan_speed && req->tilt_speed == tilt_speed
        && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P4(EqZoomMoveRequest, speed, direction, seq_id, mq_name, "")
{
    const ptzf::ZoomMoveRequest* req = reinterpret_cast<const ptzf::ZoomMoveRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->speed == speed && req->direction == direction && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqFocusModeRequest, mode, seq_id, mq_name, "")
{
    const ptzf::FocusModeRequest* req = reinterpret_cast<const ptzf::FocusModeRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->mode == mode && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P4(EqFocusMoveRequest, direction, speed, seq_id, mq_name, "")
{
    const ptzf::FocusMoveRequest* req = reinterpret_cast<const ptzf::FocusMoveRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->direction == direction && req->speed == speed && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqPanTiltResetRequest, seq_id, need_ack, checked, "")
{
    const ptzf::PanTiltResetRequest* req = reinterpret_cast<const ptzf::PanTiltResetRequest*>(arg);
    if (req->seq_id == seq_id && req->need_ack == need_ack && req->mode_checked == checked) {
        return true;
    }
    return false;
}

MATCHER_P2(EqIfClearRequest, seq_id, mq_name, "")
{
    const ptzf::IfClearRequest* req = reinterpret_cast<const ptzf::IfClearRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P4(EqZoomFineMoveRequest, direction, fine_move, seq_id, mq_name, "")
{
    const ptzf::ZoomFineMoveRequest* req = reinterpret_cast<const ptzf::ZoomFineMoveRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->direction == direction && req->fine_move == fine_move && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetRampCurveRequest, mode, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetRampCurveRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetRampCurveRequest>*>(arg);
    const ptzf::SetRampCurveRequest& payload = (*req)();
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str) && mode == payload.mode) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetPanTiltMotorPowerRequest, motor_power, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetPanTiltMotorPowerRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetPanTiltMotorPowerRequest>*>(arg);
    const ptzf::SetPanTiltMotorPowerRequest& payload = (*req)();
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)
        && motor_power == payload.motor_power) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetPanTiltSlowModeRequest, enable, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetPanTiltSlowModeRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetPanTiltSlowModeRequest>*>(arg);
    const ptzf::SetPanTiltSlowModeRequest& payload = (*req)();
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str) && enable == payload.enable) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetImageFlipRequest, enable, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetImageFlipRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetImageFlipRequest>*>(arg);
    const ptzf::SetImageFlipRequest& payload = (*req)();
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str) && enable == payload.enable) {
        return true;
    }
    return false;
}

MATCHER_P5(EqSetPanTiltLimitRequestForBiz, type, pan, tilt, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetPanTiltLimitRequestForBiz>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetPanTiltLimitRequestForBiz>*>(arg);
    const ptzf::SetPanTiltLimitRequestForBiz& payload = (*req)();
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str) && type == payload.type
        && pan == payload.tilt && tilt == payload.tilt) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetPanTiltLimitClearRequestForBiz, type, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetPanTiltLimitClearRequestForBiz>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetPanTiltLimitClearRequestForBiz>*>(arg);
    const ptzf::SetPanTiltLimitClearRequestForBiz& payload = (*req)();
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str) && type == payload.type) {
        return true;
    }
    return false;
}

MATCHER_P2(EqSetPanLimitOnRequest, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetPanLimitOnRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetPanLimitOnRequest>*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P2(EqSetPanLimitOffRequest, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetPanLimitOffRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetPanLimitOffRequest>*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P2(EqSetTiltLimitOnRequest, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetTiltLimitOnRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetTiltLimitOnRequest>*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P2(EqSetTiltLimitOffRequest, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetTiltLimitOffRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetTiltLimitOffRequest>*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetIRCorrectionRequest, ir_correction, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetIRCorrectionRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetIRCorrectionRequest>*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->payload.ir_correction == ir_correction && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetTeleShiftModeRequest, enable, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetTeleShiftModeRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetTeleShiftModeRequest>*>(arg);
    const ptzf::SetTeleShiftModeRequest& payload = (*req)();
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str) && enable == payload.enable) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetDZoomModeRequest, d_zoom, seq_id, mq_name, "")
{
    const ptzf::SetDZoomModeRequest* req = reinterpret_cast<const ptzf::SetDZoomModeRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->d_zoom == d_zoom && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetZoomAbsolutePositionRequest, position, seq_id, mq_name, "")
{
    const ptzf::SetZoomAbsolutePositionRequest* req =
        reinterpret_cast<const ptzf::SetZoomAbsolutePositionRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->position == position && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetZoomRelativePositionRequest, position, seq_id, mq_name, "")
{
    const ptzf::SetZoomRelativePositionRequest* req =
        reinterpret_cast<const ptzf::SetZoomRelativePositionRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->position == position && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetFocusAbsolutePositionRequest, position, seq_id, mq_name, "")
{
    const ptzf::SetFocusAbsolutePositionRequest* req =
        reinterpret_cast<const ptzf::SetFocusAbsolutePositionRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->position == position && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetFocusRelativePositionRequest, position, seq_id, mq_name, "")
{
    const ptzf::SetFocusRelativePositionRequest* req =
        reinterpret_cast<const ptzf::SetFocusRelativePositionRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->position == position && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P2(EqSetFocusOnePushTriggerRequest, seq_id, mq_name, "")
{
    const ptzf::SetFocusOnePushTriggerRequest* req = reinterpret_cast<const ptzf::SetFocusOnePushTriggerRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetAFSensitivityModeRequest, af_mode, seq_id, mq_name, "")
{
    const ptzf::SetAFSensitivityModeRequest* req = reinterpret_cast<const ptzf::SetAFSensitivityModeRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->af_mode == af_mode && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetFocusNearLimitRequest, position, seq_id, mq_name, "")
{
    const ptzf::SetFocusNearLimitRequest* req = reinterpret_cast<const ptzf::SetFocusNearLimitRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->position == position && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetFocusAFModeRequest, mode, seq_id, mq_name, "")
{
    const ptzf::SetFocusAFModeRequest* req = reinterpret_cast<const ptzf::SetFocusAFModeRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->mode == mode && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetFocusFaceEyeDetectionModeRequest, focus_face_eye_detection_mode, seq_id, mq_name, "")
{
    const ptzf::SetFocusFaceEyeDetectionModeRequest* req =
        reinterpret_cast<const ptzf::SetFocusFaceEyeDetectionModeRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->focus_face_eye_detection_mode == focus_face_eye_detection_mode && req->seq_id == seq_id
        && gtl::isStringEqual(req->reply_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetFocusModeRequest, mode, seq_id, mq_name, "")
{
    const ptzf::FocusModeValueRequest* req =
        reinterpret_cast<const ptzf::FocusModeValueRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->mode == mode && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetAfTransitionSpeedValueRequest, af_transition_speed, seq_id, mq_name, "")
{
    const ptzf::SetAfTransitionSpeedValueRequest* req =
        reinterpret_cast<const ptzf::SetAfTransitionSpeedValueRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->af_transition_speed == af_transition_speed && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetAfSubjShiftSensValueRequest, af_subj_shift_sens, seq_id, mq_name, "")
{
    const ptzf::SetAfSubjShiftSensValueRequest* req =
        reinterpret_cast<const ptzf::SetAfSubjShiftSensValueRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->af_subj_shift_sens == af_subj_shift_sens && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetFocusFaceEyeDetectionValueModeRequest, focus_face_eye_detection_mode, seq_id, mq_name, "")
{
    const ptzf::SetFocusFaceEyeDetectionValueModeRequest* req =
        reinterpret_cast<const ptzf::SetFocusFaceEyeDetectionValueModeRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->focus_face_eye_detection_mode == focus_face_eye_detection_mode && req->seq_id == seq_id
        && gtl::isStringEqual(req->reply_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetFocusAreaRequest, focusarea, seq_id, mq_name, "")
{
    const ptzf::FocusAreaRequest* req =
        reinterpret_cast<const ptzf::FocusAreaRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->focusarea == focusarea && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P4(EqSetAFAreaPositionAFCRequest, position_x, position_y, seq_id, mq_name, "")
{
    const ptzf::AFAreaPositionAFCRequest* req =
        reinterpret_cast<const ptzf::AFAreaPositionAFCRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->positionx == position_x && req->positiony == position_y && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P4(EqSetAFAreaPositionAFSRequest, position_x, position_y, seq_id, mq_name, "")
{
    const ptzf::AFAreaPositionAFSRequest* req =
        reinterpret_cast<const ptzf::AFAreaPositionAFSRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->positionx == position_x && req->positiony == position_y && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetZoomPositionRequest, position, seq_id, mq_name, "")
{
    const ptzf::ZoomPositionRequest* req =
        reinterpret_cast<const ptzf::ZoomPositionRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->pos == position && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetFocusPositionRequest, position, seq_id, mq_name, "")
{
    const ptzf::FocusPositionRequest* req =
        reinterpret_cast<const ptzf::FocusPositionRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->pos == position && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetAfAssistRequest, on_off, seq_id, mq_name, "")
{
    const ptzf::SetAfAssistRequest* req = reinterpret_cast<const ptzf::SetAfAssistRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->on_off == on_off && req->seq_id == seq_id && gtl::isStringEqual(req->reply_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P4(EqSetFocusTrackingPositionRequest, pos_x, pos_y, seq_id, mq_name, "")
{
    const ptzf::SetFocusTrackingPositionRequest* req =
        reinterpret_cast<const ptzf::SetFocusTrackingPositionRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->pos_x == pos_x && req->pos_y == pos_y && req->seq_id == seq_id
        && gtl::isStringEqual(req->reply_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetTouchFunctionInMfRequest, touch_function_in_mf, seq_id, mq_name, "")
{
    const ptzf::SetTouchFunctionInMfRequest* req = reinterpret_cast<const ptzf::SetTouchFunctionInMfRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->touch_function_in_mf == touch_function_in_mf && req->seq_id == seq_id
        && gtl::isStringEqual(req->reply_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P4(EqSetFocusAFTimerRequest, action_time, stop_time, seq_id, mq_name, "")
{
    const ptzf::SetFocusAFTimerRequest* req = reinterpret_cast<const ptzf::SetFocusAFTimerRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->action_time == action_time && req->stop_time == stop_time && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetPTZModeRequest, mode, seq_id, mq_name, "")
{
    const ptzf::SetPTZModeRequest* req = reinterpret_cast<const ptzf::SetPTZModeRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->mode == mode && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetPTZPanTiltMoveRequest, step, seq_id, mq_name, "")
{
    const ptzf::SetPTZPanTiltMoveRequest* req = reinterpret_cast<const ptzf::SetPTZPanTiltMoveRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->step == step && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetPTZZoomMoveRequest, step, seq_id, mq_name, "")
{
    const ptzf::SetPTZZoomMoveRequest* req = reinterpret_cast<const ptzf::SetPTZZoomMoveRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->step == step && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P6(EqSetPanTiltAbsolutePositionRequest, pan_speed, tilt_speed, pan_position, tilt_position, seq_id, mq_name, "")
{
    const ptzf::SetPanTiltRelativePositionRequest* req =
        reinterpret_cast<const ptzf::SetPanTiltRelativePositionRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->pan_speed == pan_speed && req->tilt_speed == tilt_speed && req->pan_position == pan_position
        && req->tilt_position == tilt_position && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P6(EqSetPanTiltRelativePositionRequest, pan_speed, tilt_speed, pan_position, tilt_position, seq_id, mq_name, "")
{
    const ptzf::SetPanTiltRelativePositionRequest* req =
        reinterpret_cast<const ptzf::SetPanTiltRelativePositionRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->pan_speed == pan_speed && req->tilt_speed == tilt_speed && req->pan_position == pan_position
        && req->tilt_position == tilt_position && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P4(EqSetPanTiltRelativeMoveRequest, direction, amount, seq_id, mq_name, "")
{
    const ptzf::SetPanTiltRelativeMoveRequest* req = reinterpret_cast<const ptzf::SetPanTiltRelativeMoveRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->direction == direction && req->amount == amount && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P4(EqSetZoomRelativeMoveRequest, direction, amount, seq_id, mq_name, "")
{
    const ptzf::SetZoomRelativeMoveRequest* req = reinterpret_cast<const ptzf::SetZoomRelativeMoveRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->direction == direction && req->amount == amount && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P2(EqHomePositionRequest, seq_id, mq_name, "")
{
    const ptzf::HomePositionRequest* req = reinterpret_cast<const ptzf::HomePositionRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqsetStandbyModeRequest, mode, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetStandbyModeRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetStandbyModeRequest>*>(arg);
    const ptzf::SetStandbyModeRequest& payload = (*req)();
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str) && mode == payload.mode_) {
        return true;
    }

    return false;
}

MATCHER_P3(EqsetFocusHoldRequest, focus_hold, seq_id, mq_name, "")
{
    const ptzf::SetFocusHoldRequest* req = reinterpret_cast<const ptzf::SetFocusHoldRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->focus_hold == focus_hold && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqsetFocusTrackingCancelRequest, focus_tracking_cancel, seq_id, mq_name, "")
{
    const ptzf::SetFocusTrackingCancelRequest* req = reinterpret_cast<const ptzf::SetFocusTrackingCancelRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->focus_tracking_cancel == focus_tracking_cancel && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqsetPushFocusRequest, push_focus, seq_id, mq_name, "")
{
    const ptzf::SetPushFocusRequest* req = reinterpret_cast<const ptzf::SetPushFocusRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->push_focus == push_focus && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqPanTiltSpeedModeRequest, speed_mode, seq_id, mq_name, "")
{
    const ptzf::SetPanTiltSpeedModeRequest* req = reinterpret_cast<const ptzf::SetPanTiltSpeedModeRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->speed_mode == speed_mode && req->seq_id == seq_id
        && gtl::isStringEqual(req->reply_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqPanTiltSpeedStepRequest, speed_step, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetPanTiltSpeedStepRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetPanTiltSpeedStepRequest>*>(arg);
    const ptzf::SetPanTiltSpeedStepRequest& payload = (*req)();
    std::string mq_name_str = mq_name.name;
    if (payload.speed_step == speed_step && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSettingPositionRequest, setting_position, seq_id, mq_name, "")
{
    const ptzf::SetSettingPositionRequest* req = reinterpret_cast<const ptzf::SetSettingPositionRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->setting_position == setting_position && req->seq_id == seq_id
        && gtl::isStringEqual(req->reply_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqPanDirectionRequest, pan_direction, seq_id, mq_name, "")
{
    const ptzf::SetPanDirectionRequest* req = reinterpret_cast<const ptzf::SetPanDirectionRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->pan_direction == pan_direction && req->seq_id == seq_id
        && gtl::isStringEqual(req->reply_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqTiltDirectionRequest, tilt_direction, seq_id, mq_name, "")
{
    const ptzf::SetTiltDirectionRequest* req = reinterpret_cast<const ptzf::SetTiltDirectionRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->tilt_direction == tilt_direction && req->seq_id == seq_id
        && gtl::isStringEqual(req->reply_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqPanReverseRequest, enable, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetPanReverseRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetPanReverseRequest>*>(arg);
    const ptzf::SetPanReverseRequest& payload = (*req)();
    std::string mq_name_str = mq_name.name;
    if (payload.enable == enable && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqTiltReverseRequest, enable, seq_id, mq_name, "")
{
    const ptzf::BizMessage<ptzf::SetTiltReverseRequest>* req =
        reinterpret_cast<const ptzf::BizMessage<ptzf::SetTiltReverseRequest>*>(arg);
    const ptzf::SetTiltReverseRequest& payload = (*req)();
    std::string mq_name_str = mq_name.name;
    if (payload.enable == enable && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqSetZoomSpeedScaleRequest, zoom_speed_scale, seq_id, mq_name, "")
{
    const ptzf::SetZoomSpeedScaleRequest* req = reinterpret_cast<const ptzf::SetZoomSpeedScaleRequest*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->zoom_speed_scale == zoom_speed_scale && req->seq_id == seq_id
        && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P3(EqsetPushAFModeRequest, mode, seq_id, mq_name, "")
{
    const ptzf::SetPushAFModeRequestForBiz* req = reinterpret_cast<const ptzf::SetPushAFModeRequestForBiz*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->mode == mode && req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P2(EqExeCancelZoomPosition, seq_id, mq_name, "")
{
    const ptzf::ExeCancelZoomPositionRequestForBiz* req =
        reinterpret_cast<const ptzf::ExeCancelZoomPositionRequestForBiz*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

MATCHER_P2(EqExeCancelFocusPositionRequest, seq_id, mq_name, "")
{
    const ptzf::ExeCancelFocusPositionRequestForBiz* req =
        reinterpret_cast<const ptzf::ExeCancelFocusPositionRequestForBiz*>(arg);
    std::string mq_name_str = mq_name.name;
    if (req->seq_id == seq_id && gtl::isStringEqual(req->mq_name.name, mq_name_str)) {
        return true;
    }
    return false;
}

TEST_F(BizPtzfIfTest, sendPanTiltMoveRequest1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;
    u8_t pan_speed = U8_T(24);
    u8_t tilt_speed = U8_T(23);

    ARRAY_FOREACH (pan_tilt_move_test_value_table, i) {
        EXPECT_CALL(
            er_mock_,
            post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                 _,
                 sizeof(ptzf::PanTiltMoveRequest),
                 EqPanTiltMoveRequest(
                     pan_tilt_move_test_value_table[i].out_dir, pan_speed, tilt_speed, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.sendPanTiltMoveRequest(pan_tilt_move_test_value_table[i].in_dir, pan_speed, tilt_speed);
        EXPECT_TRUE(result);
    }

    result = biz_ptzf_if.sendPanTiltMoveRequest(
        static_cast<PanTiltDirection>(PAN_TILT_DIRECTION_DOWN_RIGHT + U8_T(0x01)), pan_speed, tilt_speed);
    EXPECT_FALSE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.sendPanTiltMoveRequest(
        pan_tilt_move_test_value_table[0].in_dir, pan_speed, tilt_speed, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, sendPanTiltMoveRequest2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    u8_t pan_speed = U8_T(24);
    u8_t tilt_speed = U8_T(23);
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (pan_tilt_move_test_value_table, i) {
        EXPECT_CALL(
            er_mock_,
            post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                 _,
                 sizeof(ptzf::PanTiltMoveRequest),
                 EqPanTiltMoveRequest(
                     pan_tilt_move_test_value_table[i].out_dir, pan_speed, tilt_speed, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result =
            biz_ptzf_if.sendPanTiltMoveRequest(pan_tilt_move_test_value_table[i].in_dir, pan_speed, tilt_speed, seq_id);
        EXPECT_TRUE(result);
    }

    result = biz_ptzf_if.sendPanTiltMoveRequest(
        static_cast<PanTiltDirection>(PAN_TILT_DIRECTION_DOWN_RIGHT + U8_T(0x01)), pan_speed, tilt_speed, seq_id);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, sendZoomMoveRequest1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (zoom_move_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::ZoomMoveRequest),
                         EqZoomMoveRequest(zoom_move_test_value_table[i].out_speed,
                                           zoom_move_test_value_table[i].out_dir,
                                           DEFAULT_SEQ_ID,
                                           blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.sendZoomMoveRequest(zoom_move_test_value_table[i].in_dir,
                                                 zoom_move_test_value_table[i].in_speed);
        EXPECT_TRUE(result);
    }

    result = biz_ptzf_if.sendZoomMoveRequest(static_cast<ZoomDirection>(ZOOM_DIRECTION_WIDE + U8_T(0x01)), U8_T(25));
    EXPECT_FALSE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.sendZoomMoveRequest(
        zoom_move_test_value_table[0].in_dir, zoom_move_test_value_table[0].in_speed, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, sendZoomMoveRequest2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (zoom_move_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::ZoomMoveRequest),
                         EqZoomMoveRequest(zoom_move_test_value_table[i].out_speed,
                                           zoom_move_test_value_table[i].out_dir,
                                           seq_id,
                                           reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.sendZoomMoveRequest(
            zoom_move_test_value_table[i].in_dir, zoom_move_test_value_table[i].in_speed, seq_id);
        EXPECT_TRUE(result);
    }

    result =
        biz_ptzf_if.sendZoomMoveRequest(static_cast<ZoomDirection>(ZOOM_DIRECTION_WIDE + U8_T(0x01)), U8_T(25), seq_id);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, sendFocusModeRequest1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (focus_mode_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::FocusModeRequest),
                         EqFocusModeRequest(focus_mode_test_value_table[i].out_mode, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.sendFocusModeRequest(focus_mode_test_value_table[i].in_mode);
        EXPECT_TRUE(result);
    }

    result = biz_ptzf_if.sendFocusModeRequest(static_cast<FocusMode>(FOCUS_MODE_TOGGLE + U8_T(0x01)));
    EXPECT_FALSE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.sendFocusModeRequest(focus_mode_test_value_table[0].in_mode, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, sendFocusModeRequest2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (focus_mode_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::FocusModeRequest),
                         EqFocusModeRequest(focus_mode_test_value_table[i].out_mode, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.sendFocusModeRequest(focus_mode_test_value_table[i].in_mode, seq_id);
        EXPECT_TRUE(result);
    }

    result = biz_ptzf_if.sendFocusModeRequest(static_cast<FocusMode>(FOCUS_MODE_TOGGLE + U8_T(0x01)), seq_id);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, sendFocusMoveRequest1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;
    u8_t speed = U8_T(24);

    ARRAY_FOREACH (focus_direction_test_value_table, i) {
        EXPECT_CALL(
            er_mock_,
            post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                 _,
                 sizeof(ptzf::FocusMoveRequest),
                 EqFocusMoveRequest(focus_direction_test_value_table[i].out_dir, speed, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.sendFocusMoveRequest(focus_direction_test_value_table[i].in_dir, speed);
        EXPECT_TRUE(result);
    }

    result = biz_ptzf_if.sendFocusMoveRequest(static_cast<FocusDirection>(FOCUS_DIRECTION_NEAR + U8_T(0x01)), speed);
    EXPECT_FALSE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.sendFocusMoveRequest(focus_direction_test_value_table[0].in_dir, speed, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, sendFocusMoveRequest2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    u8_t speed = U8_T(24);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (focus_direction_test_value_table, i) {
        EXPECT_CALL(
            er_mock_,
            post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                 _,
                 sizeof(ptzf::FocusMoveRequest),
                 EqFocusMoveRequest(focus_direction_test_value_table[i].out_dir, speed, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.sendFocusMoveRequest(focus_direction_test_value_table[i].in_dir, speed, seq_id);
        EXPECT_TRUE(result);
    }

    result =
        biz_ptzf_if.sendFocusMoveRequest(static_cast<FocusDirection>(FOCUS_DIRECTION_NEAR + U8_T(0x01)), speed, seq_id);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, sendPanTiltResetRequest1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::PanTiltResetRequest),
                     EqPanTiltResetRequest(DEFAULT_SEQ_ID, false, false)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.sendPanTiltResetRequest();
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.sendPanTiltResetRequest(INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, sendPanTiltResetRequest2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::PanTiltResetRequest),
                     EqPanTiltResetRequest(seq_id, false, false)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.sendPanTiltResetRequest(seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, sendPanTiltResetRequest3Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool checked = true;
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::PanTiltResetRequest),
                     EqPanTiltResetRequest(seq_id, false, checked)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.sendPanTiltResetRequest(seq_id, checked);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, sendPanTiltResetRequest4Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool checked = true;
    bool need_ack = true;
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::PanTiltResetRequest),
                     EqPanTiltResetRequest(seq_id, need_ack, checked)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.sendPanTiltResetRequest(seq_id, checked, need_ack);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, sendIfClearRequest1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::IfClearRequest),
                     EqIfClearRequest(DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.sendIfClearRequest();
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.sendIfClearRequest(INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, sendIfClearRequest2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::IfClearRequest),
                     EqIfClearRequest(seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.sendIfClearRequest(seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, sendZoomFineMoveReques1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;
    u16_t speed = U8_T(24);

    ARRAY_FOREACH (zoom_fine_move_test_value_table, i) {
        EXPECT_CALL(
            er_mock_,
            post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                 _,
                 sizeof(ptzf::FocusMoveRequest),
                 EqZoomFineMoveRequest(zoom_fine_move_test_value_table[i].out_dir, speed, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.sendZoomFineMoveRequest(zoom_fine_move_test_value_table[i].in_dir, speed);
        EXPECT_TRUE(result);
    }

    result = biz_ptzf_if.sendZoomFineMoveRequest(static_cast<ZoomDirection>(ZOOM_DIRECTION_WIDE + U8_T(0x01)), speed);
    EXPECT_FALSE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.sendZoomFineMoveRequest(zoom_fine_move_test_value_table[0].in_dir, speed, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, sendZoomFineMoveReques2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    u16_t speed = U8_T(24);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (zoom_fine_move_test_value_table, i) {
        EXPECT_CALL(
            er_mock_,
            post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                 _,
                 sizeof(ptzf::FocusMoveRequest),
                 EqZoomFineMoveRequest(zoom_fine_move_test_value_table[i].out_dir, speed, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.sendZoomFineMoveRequest(zoom_fine_move_test_value_table[i].in_dir, speed, seq_id);
        EXPECT_TRUE(result);
    }

    result = biz_ptzf_if.sendZoomFineMoveRequest(
        static_cast<ZoomDirection>(ZOOM_DIRECTION_WIDE + U8_T(0x01)), speed, seq_id);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltRampCurve1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    u32_t mode = U32_T(1);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetRampCurveRequest>),
                     EqSetRampCurveRequest(mode, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltRampCurve(mode);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltRampCurve(mode, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltRampCurve2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    u32_t mode = U32_T(2);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetRampCurveRequest>),
                     EqSetRampCurveRequest(mode, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltRampCurve(mode, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltMotorPower)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    PanTiltMotorPower motor_power = PAN_TILT_MOTOR_POWER_NORMAL;
    ptzf::PanTiltMotorPower ptzf_motor_power = ptzf::PAN_TILT_MOTOR_POWER_NORMAL;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetPanTiltMotorPowerRequest>),
                     EqSetPanTiltMotorPowerRequest(ptzf_motor_power, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltMotorPower(motor_power, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltSlowMode1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    bool enable = true;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetPanTiltSlowModeRequest>),
                     EqSetPanTiltSlowModeRequest(enable, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltSlowMode(enable);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltSlowMode(enable, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltSlowMode2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    bool enable = false;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetPanTiltSlowModeRequest>),
                     EqSetPanTiltSlowModeRequest(enable, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltSlowMode(enable, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltImageFlipMode1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    bool enable = true;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetImageFlipRequest>),
                     EqSetImageFlipRequest(enable, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltImageFlipMode(enable);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltImageFlipMode(enable, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltImageFlipMode2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    bool enable = false;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetImageFlipRequest>),
                     EqSetImageFlipRequest(enable, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltImageFlipMode(enable, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltLimit1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    u32_t pan = U32_T(1);
    u32_t tilt = U32_T(1);
    bool result;

    ARRAY_FOREACH (pan_tilt_limit_type_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::BizMessage<ptzf::SetPanTiltLimitRequestForBiz>),
                         EqSetPanTiltLimitRequestForBiz(
                             pan_tilt_limit_type_test_value_table[i].out_type, pan, tilt, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setPanTiltLimit(pan_tilt_limit_type_test_value_table[i].in_type, pan, tilt);
        EXPECT_TRUE(result);
    }

    result = biz_ptzf_if.setPanTiltLimit(
        static_cast<PanTiltLimitType>(PAN_TILT_LIMIT_TYPE_UP_RIGHT + U8_T(0x01)), pan, tilt);
    EXPECT_FALSE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltLimit(pan_tilt_limit_type_test_value_table[0].in_type, pan, tilt, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltLimit2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    u32_t pan = U32_T(1);
    u32_t tilt = U32_T(1);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (pan_tilt_limit_type_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::BizMessage<ptzf::SetPanTiltLimitRequestForBiz>),
                         EqSetPanTiltLimitRequestForBiz(
                             pan_tilt_limit_type_test_value_table[i].out_type, pan, tilt, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setPanTiltLimit(pan_tilt_limit_type_test_value_table[i].in_type, pan, tilt, seq_id);
        EXPECT_TRUE(result);
    }

    result = biz_ptzf_if.setPanTiltLimit(
        static_cast<PanTiltLimitType>(PAN_TILT_LIMIT_TYPE_UP_RIGHT + U8_T(0x01)), pan, tilt, seq_id);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltPanLimitOn1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetPanLimitOnRequest>),
                     EqSetPanLimitOnRequest(DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltPanLimitOn();
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltPanLimitOn(INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltPanLimitOn2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetPanLimitOnRequest>),
                     EqSetPanLimitOnRequest(seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltPanLimitOn(seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltPanLimitOff1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetPanLimitOffRequest>),
                     EqSetPanLimitOffRequest(DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltPanLimitOff();
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltPanLimitOff(INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltPanLimitOff2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetPanLimitOffRequest>),
                     EqSetPanLimitOffRequest(seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltPanLimitOff(seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltTiltLimitOn1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetTiltLimitOnRequest>),
                     EqSetTiltLimitOnRequest(DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltTiltLimitOn();
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltTiltLimitOn(INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltTiltLimitOn2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetTiltLimitOnRequest>),
                     EqSetTiltLimitOnRequest(seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltTiltLimitOn(seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltTiltLimitOff1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetTiltLimitOffRequest>),
                     EqSetTiltLimitOffRequest(DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltTiltLimitOff();
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltTiltLimitOff(INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltTiltLimitOff2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetTiltLimitOffRequest>),
                     EqSetTiltLimitOffRequest(seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltTiltLimitOff(seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setIRCorrection1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (ir_correction_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::BizMessage<ptzf::SetIRCorrectionRequest>),
                         EqSetIRCorrectionRequest(
                             ir_correction_test_value_table[i].out_ir_correction, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setIRCorrection(ir_correction_test_value_table[i].in_ir_correction);
        EXPECT_TRUE(result);
    }

    result = biz_ptzf_if.setIRCorrection(static_cast<IRCorrection>(IR_CORRECTION_IRLIGHT + U8_T(0x01)));
    EXPECT_FALSE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setIRCorrection(ir_correction_test_value_table[0].in_ir_correction, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setIRCorrection2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (ir_correction_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::BizMessage<ptzf::SetIRCorrectionRequest>),
                         EqSetIRCorrectionRequest(
                             ir_correction_test_value_table[i].out_ir_correction, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setIRCorrection(ir_correction_test_value_table[i].in_ir_correction, seq_id);
        EXPECT_TRUE(result);
    }

    result = biz_ptzf_if.setIRCorrection(static_cast<IRCorrection>(IR_CORRECTION_IRLIGHT + U8_T(0x01)), seq_id);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setTeleShiftMode1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    bool enable = true;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetTeleShiftModeRequest>),
                     EqSetTeleShiftModeRequest(enable, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setTeleShiftMode(enable);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setTeleShiftMode(enable, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setTeleShiftMode2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    bool enable = true;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetTeleShiftModeRequest>),
                     EqSetTeleShiftModeRequest(enable, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setTeleShiftMode(enable, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltSpeedMode1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    biz_ptzf::PanTiltSpeedMode speed_mode = biz_ptzf::PAN_TILT_SPEED_MODE_NORMAL;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetPanTiltSpeedModeRequest),
                     EqPanTiltSpeedModeRequest(speed_mode, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltSpeedMode(speed_mode);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltSpeedMode(speed_mode, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltSpeedMode2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    biz_ptzf::PanTiltSpeedMode speed_mode = biz_ptzf::PAN_TILT_SPEED_MODE_NORMAL;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetPanTiltSpeedModeRequest),
                     EqPanTiltSpeedModeRequest(speed_mode, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltSpeedMode(speed_mode, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltSpeedStep1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    biz_ptzf::PanTiltSpeedStep speed_step = biz_ptzf::PAN_TILT_SPEED_STEP_NORMAL;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetPanTiltSpeedStepRequest>),
                     EqPanTiltSpeedStepRequest(speed_step, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltSpeedStep(speed_step);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltSpeedStep(speed_step, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltSpeedStep2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    biz_ptzf::PanTiltSpeedStep speed_step = biz_ptzf::PAN_TILT_SPEED_STEP_NORMAL;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetPanTiltSpeedStepRequest>),
                     EqPanTiltSpeedStepRequest(speed_step, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltSpeedStep(speed_step, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setSettingPosition1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    biz_ptzf::SettingPosition setting_position = biz_ptzf::SETTING_POSITION_CEILING;
    bool enable = true;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetImageFlipRequest>),
                     EqSetImageFlipRequest(enable, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setSettingPosition(setting_position);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setSettingPosition(setting_position, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setSettingPosition2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    biz_ptzf::SettingPosition setting_position = biz_ptzf::SETTING_POSITION_DESKTOP;
    bool enable = false;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetImageFlipRequest>),
                     EqSetImageFlipRequest(enable, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setSettingPosition(setting_position, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setPanDirection1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    biz_ptzf::PanDirection pan_direction = biz_ptzf::PAN_DIRECTION_NORMAL;
    bool enable = false;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetPanReverseRequest>),
                     EqPanReverseRequest(enable, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanDirection(pan_direction);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanDirection(pan_direction, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanDirection2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    biz_ptzf::PanDirection pan_direction = biz_ptzf::PAN_DIRECTION_NORMAL;
    bool enable = false;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetPanReverseRequest>),
                     EqPanReverseRequest(enable, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanDirection(pan_direction, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setTiltDirection1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    biz_ptzf::TiltDirection tilt_direction = biz_ptzf::TILT_DIRECTION_NORMAL;
    bool enable = false;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetTiltReverseRequest>),
                     EqTiltReverseRequest(enable, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setTiltDirection(tilt_direction);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setTiltDirection(tilt_direction, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setTiltDirection2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    biz_ptzf::TiltDirection tilt_direction = biz_ptzf::TILT_DIRECTION_NORMAL;
    bool enable = false;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::BizMessage<ptzf::SetTiltReverseRequest>),
                     EqTiltReverseRequest(enable, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setTiltDirection(tilt_direction, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setZoomSpeedScale1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    u8_t zoom_speed_scale = U8_T(10);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetZoomSpeedScaleRequest),
                     EqSetZoomSpeedScaleRequest(zoom_speed_scale, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setZoomSpeedScale(zoom_speed_scale);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setZoomSpeedScale(zoom_speed_scale, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setZoomSpeedScale2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    u8_t zoom_speed_scale = U8_T(10);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetZoomSpeedScaleRequest),
                     EqSetZoomSpeedScaleRequest(zoom_speed_scale, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setZoomSpeedScale(zoom_speed_scale, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setPushAfMode1way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (push_af_mode_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetPushAFModeRequestForBiz),
                         EqsetPushAFModeRequest(push_af_mode_value_table[i].out_mode, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setPushAFMode(push_af_mode_value_table[i].in_mode);
        EXPECT_TRUE(result);
    }

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPushAFMode(push_af_mode_value_table[0].in_mode, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPushAfMode2way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (push_af_mode_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetPushAFModeRequestForBiz),
                         EqsetPushAFModeRequest(push_af_mode_value_table[i].out_mode, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setPushAFMode(push_af_mode_value_table[i].in_mode, seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, getPanTiltPositionSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    u32_t pan = U32_T(0);
    u32_t tilt = U32_T(0);

    EXPECT_CALL(ptzf_biz_message_if_mock_, getPanTiltAbsolutePosition(pan, tilt))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(U32_T(0xABCDE)), SetArgReferee<1>(U32_T(0x12345)), Return(ERRORCODE_SUCCESS)));
    EXPECT_EQ(ERRORCODE_SUCCESS, biz_ptzf_if.getPanTiltPosition(pan, tilt));
    EXPECT_EQ(U32_T(0xABCDE), pan);
    EXPECT_EQ(U32_T(0x12345), tilt);
}

TEST_F(BizPtzfIfTest, getPanTiltStatusSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    u32_t status_expect = U32_T(0);
    u32_t status_result = 0;
    ErrorCode err;

    EXPECT_CALL(ptzf_biz_message_if_mock_, getPanTiltStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(status_expect), Return(ERRORCODE_SUCCESS)));
    err = biz_ptzf_if.getPanTiltStatus(status_result);
    EXPECT_EQ(status_expect, status_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    status_expect = U32_T(0x2800);
    EXPECT_CALL(ptzf_biz_message_if_mock_, getPanTiltStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(status_expect), Return(ERRORCODE_SUCCESS)));
    err = biz_ptzf_if.getPanTiltStatus(status_result);
    EXPECT_EQ(status_expect, status_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getPanTiltSlowModeSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    bool enable = true;
    bool enable_result = false;
    ErrorCode err;

    ptzf_st.setSlowMode(enable);
    err = biz_ptzf_if.getPanTiltSlowMode(enable_result);
    EXPECT_TRUE(enable_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    enable = false;
    ptzf_st.setSlowMode(enable);
    err = biz_ptzf_if.getPanTiltSlowMode(enable_result);
    EXPECT_FALSE(enable_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getPanTiltImageFlipModeSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    visca::PictureFlipMode mode = visca::PICTURE_FLIP_MODE_ON;
    biz_ptzf::PictureFlipMode mode_result = biz_ptzf::PICTURE_FLIP_MODE_OFF;
    ErrorCode err;

    ptzf_st.setImageFlipStatusOnBoot(mode);
    err = biz_ptzf_if.getPanTiltImageFlipMode(mode_result);
    EXPECT_EQ(biz_ptzf::PICTURE_FLIP_MODE_ON, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    mode = visca::PICTURE_FLIP_MODE_OFF;
    ptzf_st.setImageFlipStatusOnBoot(mode);
    err = biz_ptzf_if.getPanTiltImageFlipMode(mode_result);
    EXPECT_EQ(biz_ptzf::PICTURE_FLIP_MODE_OFF, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    mode = static_cast<visca::PictureFlipMode>(U32_T(0x04));
    ptzf_st.setImageFlipStatusOnBoot(mode);
    err = biz_ptzf_if.getPanTiltImageFlipMode(mode_result);
    EXPECT_NE(static_cast<PictureFlipMode>(U32_T(0x04)), mode_result);
    EXPECT_EQ(ERRORCODE_VAL, err);
}

TEST_F(BizPtzfIfTest, getPanTiltImageFlipModePresetSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    visca::PictureFlipMode mode = visca::PICTURE_FLIP_MODE_ON;
    biz_ptzf::PictureFlipMode mode_result = biz_ptzf::PICTURE_FLIP_MODE_OFF;
    ErrorCode err;

    ptzf_st.setImageFlipStatusOnBoot(mode);
    ptzf_st.setImageFlipStatus(mode);
    err = biz_ptzf_if.getPanTiltImageFlipMode(mode_result);
    EXPECT_EQ(biz_ptzf::PICTURE_FLIP_MODE_ON, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    mode = visca::PICTURE_FLIP_MODE_OFF;
    ptzf_st.setImageFlipStatus(mode);
    err = biz_ptzf_if.getPanTiltImageFlipMode(mode_result);
    EXPECT_EQ(biz_ptzf::PICTURE_FLIP_MODE_ON, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
    err = biz_ptzf_if.getPanTiltImageFlipModePreset(mode_result);
    EXPECT_EQ(biz_ptzf::PICTURE_FLIP_MODE_OFF, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    mode = static_cast<visca::PictureFlipMode>(U32_T(0x04));
    ptzf_st.setImageFlipStatus(mode);
    err = biz_ptzf_if.getPanTiltImageFlipMode(mode_result);
    EXPECT_EQ(biz_ptzf::PICTURE_FLIP_MODE_ON, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
    err = biz_ptzf_if.getPanTiltImageFlipModePreset(mode_result);
    EXPECT_NE(static_cast<PictureFlipMode>(U32_T(0x04)), mode_result);
    EXPECT_EQ(ERRORCODE_VAL, err);
}

TEST_F(BizPtzfIfTest, getPanLimitDownLeftSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    u32_t pan = U32_T(0x210);
    u32_t tilt = U32_T(0x123);
    u32_t pan_result = 0;
    u32_t tilt_result = 0;
    ErrorCode err;

    ptzf_st.setPanTiltLimitDownLeft(pan, tilt);
    err = biz_ptzf_if.getPanLimitLeft(pan_result);
    EXPECT_EQ(pan, pan_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
    err = biz_ptzf_if.getTiltLimitDown(tilt_result);
    EXPECT_EQ(tilt, tilt_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getPanLimitRightUpSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    u32_t pan = U32_T(0x321);
    u32_t tilt = U32_T(0x234);
    u32_t pan_result = 0;
    u32_t tilt_result = 0;
    ErrorCode err;

    ptzf_st.setPanTiltLimitUpRight(pan, tilt);
    err = biz_ptzf_if.getPanLimitRight(pan_result);
    EXPECT_EQ(pan, pan_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
    err = biz_ptzf_if.getTiltLimitUp(tilt_result);
    EXPECT_EQ(tilt, tilt_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getIRCorrectionSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    visca::IRCorrection ir_correction[2] = { visca::IR_CORRECTION_STANDARD, visca::IR_CORRECTION_IRLIGHT };
    biz_ptzf::IRCorrection ir_correction_result;
    ErrorCode err;

    ARRAY_FOREACH (ir_correction_test_value_table, i) {
        ptzf_st.setIRCorrection(ir_correction[i]);
        err = biz_ptzf_if.getIRCorrection(ir_correction_result);
        EXPECT_EQ(ir_correction[i], ir_correction_result);
        EXPECT_EQ(ERRORCODE_SUCCESS, err);
    }

    ptzf_st.setIRCorrection(static_cast<visca::IRCorrection>(U32_T(0x02)));
    err = biz_ptzf_if.getIRCorrection(ir_correction_result);
    EXPECT_NE(static_cast<IRCorrection>(U32_T(0x02)), ir_correction_result);
    EXPECT_EQ(ERRORCODE_VAL, err);
}

TEST_F(BizPtzfIfTest, getPanTiltLimitModeSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    bool pan_limit_mode = false;
    bool tilt_limit_mode = false;
    ErrorCode err;

    ptzf_st.setPanLimitMode(pan_limit_mode);
    err = biz_ptzf_if.getPanLimitMode(pan_limit_mode);
    EXPECT_FALSE(pan_limit_mode);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
    ptzf_st.setTiltLimitMode(tilt_limit_mode);
    err = biz_ptzf_if.getTiltLimitMode(tilt_limit_mode);
    EXPECT_FALSE(tilt_limit_mode);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    pan_limit_mode = true;
    tilt_limit_mode = true;

    ptzf_st.setPanLimitMode(pan_limit_mode);
    err = biz_ptzf_if.getPanLimitMode(pan_limit_mode);
    EXPECT_TRUE(pan_limit_mode);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
    ptzf_st.setTiltLimitMode(tilt_limit_mode);
    err = biz_ptzf_if.getTiltLimitMode(tilt_limit_mode);
    EXPECT_TRUE(tilt_limit_mode);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getTeleShiftModeSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    bool enable = true;
    bool enable_result = false;
    ErrorCode err;

    ptzf_st.setTeleShiftMode(enable);
    err = biz_ptzf_if.getTeleShiftMode(enable_result);
    EXPECT_TRUE(enable_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    enable = false;
    ptzf_st.setTeleShiftMode(enable);
    err = biz_ptzf_if.getTeleShiftMode(enable_result);
    EXPECT_FALSE(enable_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getPanTiltRampCurveSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    u8_t mode = U8_T(0);
    u8_t mode_result = U8_T(1);
    ErrorCode err;

    ptzf_st.setRampCurve(mode);
    err = biz_ptzf_if.getPanTiltRampCurve(mode_result);
    EXPECT_EQ(mode, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    mode = U8_T(1);
    ptzf_st.setRampCurve(mode);
    err = biz_ptzf_if.getPanTiltRampCurve(mode_result);
    EXPECT_EQ(mode, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getPanTiltMotorPowerSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    ptzf::PanTiltMotorPower ptzf_motor_power = ptzf::PAN_TILT_MOTOR_POWER_NORMAL;
    PanTiltMotorPower motor_power_result = PAN_TILT_MOTOR_POWER_NORMAL;
    PanTiltMotorPower motor_power = PAN_TILT_MOTOR_POWER_LOW;
    ErrorCode err;

    ptzf_st.setPanTiltMotorPower(ptzf_motor_power);
    err = biz_ptzf_if.getPanTiltMotorPower(motor_power);
    EXPECT_EQ(motor_power, motor_power_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    ptzf_motor_power = ptzf::PAN_TILT_MOTOR_POWER_LOW;
    motor_power = PAN_TILT_MOTOR_POWER_LOW;
    ptzf_st.setPanTiltMotorPower(ptzf_motor_power);
    err = biz_ptzf_if.getPanTiltMotorPower(motor_power_result);
    EXPECT_EQ(motor_power, motor_power_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, setDZoomMode1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (zoom_mode_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetDZoomModeRequest),
                         EqSetDZoomModeRequest(zoom_mode_test_value_table[i].out_zoom, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setDZoomMode(zoom_mode_test_value_table[i].in_zoom);
        EXPECT_TRUE(result);
    }

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setDZoomMode(zoom_mode_test_value_table[0].in_zoom, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setDZoomMode2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (zoom_mode_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetDZoomModeRequest),
                         EqSetDZoomModeRequest(zoom_mode_test_value_table[i].out_zoom, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setDZoomMode(zoom_mode_test_value_table[i].in_zoom, seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setZoomAbsolutePosition1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;
    MQResponder responder;

    u16_t position = U16_T(0);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetZoomAbsolutePositionRequest),
                     EqSetZoomAbsolutePositionRequest(position, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(DoAll(Invoke(&responder, &MQResponder::returnResultZoomAckOn), Return()));

    result = biz_ptzf_if.setZoomAbsolutePosition(position);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setZoomAbsolutePosition(position, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setZoomAbsolutePosition2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    MQResponder responder;
    biz_ptzf_if.registNotification(reply_.getName());

    u16_t position = U16_T(0x4000);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetZoomAbsolutePositionRequest),
                     EqSetZoomAbsolutePositionRequest(position, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(DoAll(Invoke(&responder, &MQResponder::returnResultZoomAckOn), Return()));

    result = biz_ptzf_if.setZoomAbsolutePosition(position, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setZoomRelativePosition1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    s32_t position = S32_T(0);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetZoomRelativePositionRequest),
                     EqSetZoomRelativePositionRequest(position, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setZoomRelativePosition(position);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setZoomRelativePosition(position, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setZoomRelativePosition2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    s32_t position = S32_T(0x4000);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetZoomRelativePositionRequest),
                     EqSetZoomRelativePositionRequest(position, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setZoomRelativePosition(position, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setFocusAbsolutePosition1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;
    MQResponder responder;

    u16_t position = U16_T(0);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusAbsolutePositionRequest),
                     EqSetFocusAbsolutePositionRequest(position, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(DoAll(Invoke(&responder, &MQResponder::returnResultFocusAckOn), Return()));

    result = biz_ptzf_if.setFocusAbsolutePosition(position);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setFocusAbsolutePosition(position, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusAbsolutePosition2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    MQResponder responder;
    biz_ptzf_if.registNotification(reply_.getName());

    u16_t position = U16_T(0x1000);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusAbsolutePositionRequest),
                     EqSetFocusAbsolutePositionRequest(position, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(DoAll(Invoke(&responder, &MQResponder::returnResultFocusAckOn), Return()));

    result = biz_ptzf_if.setFocusAbsolutePosition(position, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setFocusRelativePosition1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    s32_t position = S32_T(0);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusRelativePositionRequest),
                     EqSetFocusRelativePositionRequest(position, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setFocusRelativePosition(position);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setFocusRelativePosition(position, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusRelativePosition2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    s32_t position = S32_T(0x1000);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusRelativePositionRequest),
                     EqSetFocusRelativePositionRequest(position, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setFocusRelativePosition(position, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setFocusOnePushTrigger1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusOnePushTriggerRequest),
                     EqSetFocusOnePushTriggerRequest(DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setFocusOnePushTrigger();
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setFocusOnePushTrigger(INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusOnePushTrigger2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusOnePushTriggerRequest),
                     EqSetFocusOnePushTriggerRequest(seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setFocusOnePushTrigger(seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setFocusAfSensitivity1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (af_sensitivity_test_value_table, i) {
        EXPECT_CALL(
            er_mock_,
            post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                 _,
                 sizeof(ptzf::SetAFSensitivityModeRequest),
                 EqSetAFSensitivityModeRequest(af_sensitivity_test_value_table[i].out_mode, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setFocusAfSensitivity(af_sensitivity_test_value_table[i].in_mode);
        EXPECT_TRUE(result);
    }

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setFocusAfSensitivity(af_sensitivity_test_value_table[0].in_mode, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusAfSensitivity2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (af_sensitivity_test_value_table, i) {
        EXPECT_CALL(
            er_mock_,
            post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                 _,
                 sizeof(ptzf::SetAFSensitivityModeRequest),
                 EqSetAFSensitivityModeRequest(af_sensitivity_test_value_table[i].out_mode, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setFocusAfSensitivity(af_sensitivity_test_value_table[i].in_mode, seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setFocusNearLimit1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    u16_t position = U16_T(0);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusNearLimitRequest),
                     EqSetFocusNearLimitRequest(position, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setFocusNearLimit(position);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setFocusNearLimit(position, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusNearLimit2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    u16_t position = U16_T(0x1000);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusNearLimitRequest),
                     EqSetFocusNearLimitRequest(position, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setFocusNearLimit(position, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setFocusAFMode1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (af_mode_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetFocusAFModeRequest),
                         EqSetFocusAFModeRequest(af_mode_test_value_table[i].out_mode, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setFocusAFMode(af_mode_test_value_table[i].in_mode);
        EXPECT_TRUE(result);
    }

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setFocusAFMode(af_mode_test_value_table[0].in_mode, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusAFMode2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (af_mode_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetFocusAFModeRequest),
                         EqSetFocusAFModeRequest(af_mode_test_value_table[i].out_mode, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setFocusAFMode(af_mode_test_value_table[i].in_mode, seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setFocusFaceEyedetection1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (focus_face_eye_detection_mode_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetFocusFaceEyeDetectionModeRequest),
                         EqSetFocusFaceEyeDetectionModeRequest(
                             focus_face_eye_detection_mode_test_value_table[i].out_mode, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setFocusFaceEyedetection(focus_face_eye_detection_mode_test_value_table[i].in_mode);
        EXPECT_TRUE(result);
    }

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result =
        biz_ptzf_if.setFocusFaceEyedetection(focus_face_eye_detection_mode_test_value_table[0].in_mode, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusFaceEyedetection2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (focus_face_eye_detection_mode_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetFocusFaceEyeDetectionModeRequest),
                         EqSetFocusFaceEyeDetectionModeRequest(
                             focus_face_eye_detection_mode_test_value_table[i].out_mode, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result =
            biz_ptzf_if.setFocusFaceEyedetection(focus_face_eye_detection_mode_test_value_table[i].in_mode, seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setAfAssist1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (af_assist_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetAfAssistRequest),
                         EqSetAfAssistRequest(af_assist_test_value_table[i].out_mode, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setAfAssist(af_assist_test_value_table[i].in_mode);
        EXPECT_TRUE(result);
    }

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setAfAssist(af_assist_test_value_table[0].in_mode, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setAfAssist2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (af_assist_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetAfAssistRequest),
                         EqSetAfAssistRequest(af_assist_test_value_table[i].out_mode, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setAfAssist(af_assist_test_value_table[i].in_mode, seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setFocusTrackingPosition1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (focus_tracking_position_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetFocusTrackingPositionRequest),
                         EqSetFocusTrackingPositionRequest(focus_tracking_position_test_value_table[i].out_mode_x,
                                                           focus_tracking_position_test_value_table[i].out_mode_y,
                                                           DEFAULT_SEQ_ID,
                                                           blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setFocusTrackingPosition(focus_tracking_position_test_value_table[i].in_mode_x,
                                                      focus_tracking_position_test_value_table[i].in_mode_y);
        EXPECT_TRUE(result);
    }

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setFocusTrackingPosition(focus_tracking_position_test_value_table[0].in_mode_x,
                                                  focus_tracking_position_test_value_table[0].in_mode_y,
                                                  INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusTrackingPosition2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (focus_tracking_position_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetFocusTrackingPositionRequest),
                         EqSetFocusTrackingPositionRequest(focus_tracking_position_test_value_table[i].out_mode_x,
                                                           focus_tracking_position_test_value_table[i].out_mode_y,
                                                           seq_id,
                                                           reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setFocusTrackingPosition(focus_tracking_position_test_value_table[i].in_mode_x,
                                                      focus_tracking_position_test_value_table[i].in_mode_y,
                                                      seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setTouchFunctionInMf1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (touch_function_in_mf_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetTouchFunctionInMfRequest),
                         EqSetTouchFunctionInMfRequest(
                             touch_function_in_mf_test_value_table[i].out_mode, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setTouchFunctionInMf(touch_function_in_mf_test_value_table[i].in_mode);
        EXPECT_TRUE(result);
    }

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setTouchFunctionInMf(touch_function_in_mf_test_value_table[0].in_mode, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setTouchFunctionInMf2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (touch_function_in_mf_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetTouchFunctionInMfRequest),
                         EqSetTouchFunctionInMfRequest(
                             touch_function_in_mf_test_value_table[i].out_mode, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setTouchFunctionInMf(touch_function_in_mf_test_value_table[i].in_mode, seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setPanTiltLimitClear1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (pan_tilt_limit_type_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::BizMessage<ptzf::SetPanTiltLimitClearRequestForBiz>),
                         EqSetPanTiltLimitClearRequestForBiz(
                             pan_tilt_limit_type_test_value_table[i].out_type, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setPanTiltLimitClear(pan_tilt_limit_type_test_value_table[i].in_type);
        EXPECT_TRUE(result);
    }

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltLimitClear(pan_tilt_limit_type_test_value_table[0].in_type, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltLimitClear2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (pan_tilt_limit_type_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::BizMessage<ptzf::SetPanTiltLimitClearRequestForBiz>),
                         EqSetPanTiltLimitClearRequestForBiz(
                             pan_tilt_limit_type_test_value_table[i].out_type, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setPanTiltLimitClear(pan_tilt_limit_type_test_value_table[i].in_type, seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setFocusAFTimer1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    u8_t action_time = U16_T(0x05);
    u8_t stop_time = U16_T(0x05);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusAFTimerRequest),
                     EqSetFocusAFTimerRequest(action_time, stop_time, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setFocusAFTimer(action_time, stop_time);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setFocusAFTimer(action_time, stop_time, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusAFTimer2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    u8_t action_time = U16_T(0xFF);
    u8_t stop_time = U16_T(0xFF);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusAFTimerRequest),
                     EqSetFocusAFTimerRequest(action_time, stop_time, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setFocusAFTimer(action_time, stop_time, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, exeCancelZoomPosition1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::ExeCancelZoomPositionRequestForBiz),
                     EqExeCancelZoomPosition(DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.exeCancelZoomPosition();
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.exeCancelZoomPosition(INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, exeCancelZoomPosition2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    u8_t action_time = U16_T(0xFF);
    u8_t stop_time = U16_T(0xFF);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::ExeCancelZoomPositionRequestForBiz),
                     EqExeCancelZoomPosition(seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.exeCancelZoomPosition(seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, exeCancelFocusPosition1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::ExeCancelFocusPositionRequestForBiz),
                     EqExeCancelFocusPositionRequest(DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.exeCancelFocusPosition();
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.exeCancelFocusPosition(INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, exeCancelFocusPosition2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    u8_t action_time = U16_T(0xFF);
    u8_t stop_time = U16_T(0xFF);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::ExeCancelFocusPositionRequestForBiz),
                     EqExeCancelFocusPositionRequest(seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.exeCancelFocusPosition(seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, getDZoomMode)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getDZoomMode(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getDZoomMode());
}

TEST_F(BizPtzfIfTest, getZoomAbsolutePosition)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getZoomAbsolutePosition(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getZoomAbsolutePosition());
}

TEST_F(BizPtzfIfTest, getFocusMode)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusMode(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusMode());
}

TEST_F(BizPtzfIfTest, getFocusAbsolutePosition)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusAbsolutePosition(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusAbsolutePosition());
}

TEST_F(BizPtzfIfTest, getFocusAFMode)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusAFMode(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusAFMode());
}

TEST_F(BizPtzfIfTest, getFocusFaceEyedetection)
{
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusFaceEyedetection(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusFaceEyedetection());
}

TEST_F(BizPtzfIfTest, getAfAssist)
{
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getAfAssist(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getAfAssist());
}

TEST_F(BizPtzfIfTest, getTouchFunctionInMf)
{
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getTouchFunctionInMf(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getTouchFunctionInMf());
}

TEST_F(BizPtzfIfTest, getFocusAFTimer)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusAFTimer(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusAFTimer());
}

TEST_F(BizPtzfIfTest, getFocusAfSensitivity)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusAfSensitivity(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusAfSensitivity());
}

TEST_F(BizPtzfIfTest, getFocusNearLimit)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusNearLimit(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusNearLimit());
}

TEST_F(BizPtzfIfTest, setPTZMode1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (ptz_mode_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetPTZModeRequest),
                         EqSetPTZModeRequest(ptz_mode_test_value_table[i].out_mode, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setPTZMode(ptz_mode_test_value_table[i].in_mode);
        EXPECT_TRUE(result);
    }

    biz_ptzf::PTZMode mode = biz_ptzf::PTZ_MODE_NORMAL;
    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPTZMode(mode, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPTZMode2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (ptz_mode_test_value_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::SetPTZModeRequest),
                         EqSetPTZModeRequest(ptz_mode_test_value_table[i].out_mode, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setPTZMode(ptz_mode_test_value_table[i].in_mode, seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setPTZPanTiltMove1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    u8_t step = U8_T(1);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetPTZPanTiltMoveRequest),
                     EqSetPTZPanTiltMoveRequest(step, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPTZPanTiltMove(step);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPTZPanTiltMove(step, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPTZPanTiltMove2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    u8_t step = U8_T(1);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetPTZPanTiltMoveRequest),
                     EqSetPTZPanTiltMoveRequest(step, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPTZPanTiltMove(step, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setPTZZoomMove1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    u8_t step = U8_T(1);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetPTZZoomMoveRequest),
                     EqSetPTZZoomMoveRequest(step, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPTZZoomMove(step);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPTZZoomMove(step, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPTZZoomMove2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    u8_t step = U8_T(1);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetPTZZoomMoveRequest),
                     EqSetPTZZoomMoveRequest(step, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPTZZoomMove(step, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, getPTZModeSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    ptzf::PTZMode mode = ptzf::PTZ_MODE_NORMAL;
    PTZMode mode_result = PTZ_MODE_STEP;
    ErrorCode err;

    ptzf_st.setPTZMode(mode);
    err = biz_ptzf_if.getPTZMode(mode_result);
    EXPECT_EQ(mode, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    mode = ptzf::PTZ_MODE_STEP;
    ptzf_st.setPTZMode(mode);
    err = biz_ptzf_if.getPTZMode(mode_result);
    EXPECT_EQ(mode, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getPTZPanTiltMoveSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    u8_t step = U8_T(0);
    u8_t step_result = U8_T(1);
    ErrorCode err;

    ptzf_st.setPTZPanTiltMove(step);
    err = biz_ptzf_if.getPTZPanTiltMove(step_result);
    EXPECT_EQ(step, step_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    step = U8_T(1);
    ptzf_st.setPTZPanTiltMove(step);
    err = biz_ptzf_if.getPTZPanTiltMove(step_result);
    EXPECT_EQ(step, step_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getPTZZoomSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    u8_t step = U8_T(0);
    u8_t step_result = U8_T(1);
    ErrorCode err;

    ptzf_st.setPTZZoomMove(step);
    err = biz_ptzf_if.getPTZZoomMove(step_result);
    EXPECT_EQ(step, step_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    step = U8_T(1);
    ptzf_st.setPTZZoomMove(step);
    err = biz_ptzf_if.getPTZZoomMove(step_result);
    EXPECT_EQ(step, step_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, setPanTiltAbsolutePosition1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    u8_t pan_speed = U8_T(0);
    u8_t tilt_speed = U8_T(0);
    s32_t pan_position = S32_T(0);
    s32_t tilt_position = S32_T(0);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetPanTiltAbsolutePositionRequest),
                     EqSetPanTiltAbsolutePositionRequest(
                         pan_speed, tilt_speed, pan_position, tilt_position, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltAbsolutePosition(pan_speed, tilt_speed, pan_position, tilt_position);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltAbsolutePosition(pan_speed, tilt_speed, pan_position, tilt_position, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltAbsolutePosition2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    u8_t pan_speed = U8_T(1);
    u8_t tilt_speed = U8_T(1);
    s32_t pan_position = S32_T(0x1234);
    s32_t tilt_position = S32_T(0x1234);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetPanTiltAbsolutePositionRequest),
                     EqSetPanTiltAbsolutePositionRequest(
                         pan_speed, tilt_speed, pan_position, tilt_position, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltAbsolutePosition(pan_speed, tilt_speed, pan_position, tilt_position, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltRelativePosition1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    u8_t pan_speed = U8_T(0);
    u8_t tilt_speed = U8_T(0);
    s32_t pan_position = S32_T(0);
    s32_t tilt_position = S32_T(0);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetPanTiltRelativePositionRequest),
                     EqSetPanTiltRelativePositionRequest(
                         pan_speed, tilt_speed, pan_position, tilt_position, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltRelativePosition(pan_speed, tilt_speed, pan_position, tilt_position);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltRelativePosition(pan_speed, tilt_speed, pan_position, tilt_position, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltRelativePosition2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    u8_t pan_speed = U8_T(1);
    u8_t tilt_speed = U8_T(1);
    s32_t pan_position = S32_T(0x1234);
    s32_t tilt_position = S32_T(0x1234);
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetPanTiltRelativePositionRequest),
                     EqSetPanTiltRelativePositionRequest(
                         pan_speed, tilt_speed, pan_position, tilt_position, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setPanTiltRelativePosition(pan_speed, tilt_speed, pan_position, tilt_position, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, getPanTiltMaxSpeedSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    u8_t pan_speed = U8_T(0);
    u8_t tilt_speed = U8_T(0);
    ErrorCode err;

    err = biz_ptzf_if.getPanTiltMaxSpeed(pan_speed, tilt_speed);
    EXPECT_EQ(U8_T(0x18), pan_speed);
    EXPECT_EQ(U8_T(0x17), tilt_speed);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getPanMovementRangeSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    u32_t left = U32_T(0);
    u32_t right = U32_T(0);
    ErrorCode err;

    err = biz_ptzf_if.getPanMovementRange(left, right);
    EXPECT_EQ(U32_T(0xde00), left);
    EXPECT_EQ(U32_T(0x2200), right);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getTiltMovementRangeSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    u32_t down = U32_T(0);
    u32_t up = U32_T(0);
    ErrorCode err;

    ptzf_st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);
    err = biz_ptzf_if.getTiltMovementRange(down, up);
    EXPECT_EQ(U32_T(0xfc00), down);
    EXPECT_EQ(U32_T(0x1200), up);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    ptzf_st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_ON);
    err = biz_ptzf_if.getTiltMovementRange(down, up);
    EXPECT_EQ(U32_T(0xee00), down);
    EXPECT_EQ(U32_T(0x0400), up);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getOpticalZoomMaxMagnificationSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    u8_t Magnification = U8_T(0);
    ErrorCode err;

    err = biz_ptzf_if.getOpticalZoomMaxMagnification(Magnification);
    EXPECT_EQ(U8_T(0x14), Magnification);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getZoomMovementRangeSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    u16_t wide = U16_T(1);
    u16_t optical = U16_T(0);
    u16_t clear_image = U16_T(0);
    u16_t digital = U16_T(0);
    ErrorCode err;

    ptzf_st.setMaxZoomValue(U16_T(0x6000));
    err = biz_ptzf_if.getZoomMovementRange(wide, optical, clear_image, digital);
    EXPECT_EQ(U16_T(0x00), wide);
    EXPECT_EQ(U16_T(0x4000), optical);
    EXPECT_EQ(U16_T(0x6000), clear_image);
    EXPECT_EQ(U16_T(0x7ac0), digital);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    ptzf_st.setMaxZoomValue(U16_T(0x5556));
    err = biz_ptzf_if.getZoomMovementRange(wide, optical, clear_image, digital);
    EXPECT_EQ(U16_T(0x00), wide);
    EXPECT_EQ(U16_T(0x4000), optical);
    EXPECT_EQ(U16_T(0x5556), clear_image);
    EXPECT_EQ(U16_T(0x7ac0), digital);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getZoomMaxVelocitySuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    u8_t velocity = U8_T(0);
    ErrorCode err;

    err = biz_ptzf_if.getZoomMaxVelocity(velocity);
    EXPECT_EQ(U8_T(0x08), velocity);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getZoomStatusSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    bool status = false;
    ErrorCode err;

    EXPECT_CALL(visca_status_if_mock_, isMovingZoom()).Times(1).WillOnce(Return(true));

    err = biz_ptzf_if.getZoomStatus(status);
    EXPECT_TRUE(status);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getFocusStatusSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    bool status = false;
    ErrorCode err;

    EXPECT_CALL(visca_status_if_mock_, isMovingFocus()).Times(1).WillOnce(Return(true));

    err = biz_ptzf_if.getFocusStatus(status);
    EXPECT_TRUE(status);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, setPanTiltRelativeMove1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (pan_tilt_move_test_value_table, i) {
        if (pan_tilt_move_test_value_table[i].in_dir == PAN_TILT_DIRECTION_STOP) {
            EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
            result = biz_ptzf_if.setPanTiltRelativeMove(pan_tilt_move_test_value_table[0].in_dir,
                                                        ptz_relative_amount_test_value_table[0].in_dir,
                                                        DEFAULT_SEQ_ID);
            EXPECT_FALSE(result);
        }
        else {
            EXPECT_CALL(er_mock_,
                        post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                             _,
                             sizeof(ptzf::SetPanTiltRelativeMoveRequest),
                             EqSetPanTiltRelativeMoveRequest(pan_tilt_move_test_value_table[i].out_dir,
                                                             ptz_relative_amount_test_value_table[i].out_dir,
                                                             DEFAULT_SEQ_ID,
                                                             blankName)))
                .Times(1)
                .WillOnce(Return());

            result = biz_ptzf_if.setPanTiltRelativeMove(pan_tilt_move_test_value_table[i].in_dir,
                                                        ptz_relative_amount_test_value_table[i].in_dir);
            EXPECT_TRUE(result);
        }
    }

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setPanTiltRelativeMove(
        pan_tilt_move_test_value_table[0].in_dir, ptz_relative_amount_test_value_table[0].in_dir, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setPanTiltRelativeMove2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (pan_tilt_move_test_value_table, i) {
        if (pan_tilt_move_test_value_table[i].in_dir == PAN_TILT_DIRECTION_STOP) {
            EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
            result = biz_ptzf_if.setPanTiltRelativeMove(
                pan_tilt_move_test_value_table[0].in_dir, ptz_relative_amount_test_value_table[0].in_dir, seq_id);
            EXPECT_FALSE(result);
        }
        else {
            EXPECT_CALL(er_mock_,
                        post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                             _,
                             sizeof(ptzf::SetPanTiltRelativeMoveRequest),
                             EqSetPanTiltRelativeMoveRequest(pan_tilt_move_test_value_table[i].out_dir,
                                                             ptz_relative_amount_test_value_table[i].out_dir,
                                                             seq_id,
                                                             reply_.getName())))
                .Times(1)
                .WillOnce(Return());

            result = biz_ptzf_if.setPanTiltRelativeMove(
                pan_tilt_move_test_value_table[i].in_dir, ptz_relative_amount_test_value_table[i].in_dir, seq_id);
            EXPECT_TRUE(result);
        }
    }
}

TEST_F(BizPtzfIfTest, setZoomRelativeMove1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (zoom_move_relative_test_value_table, i) {
        if (zoom_move_relative_test_value_table[i].in_dir == ZOOM_DIRECTION_STOP) {
            EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
            result = biz_ptzf_if.setZoomRelativeMove(zoom_move_relative_test_value_table[0].in_dir,
                                                     ptz_relative_amount_test_value_table[0].in_dir,
                                                     DEFAULT_SEQ_ID);
            EXPECT_FALSE(result);
        }
        else {
            EXPECT_CALL(er_mock_,
                        post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                             _,
                             sizeof(ptzf::SetZoomRelativeMoveRequest),
                             EqSetZoomRelativeMoveRequest(zoom_move_relative_test_value_table[i].out_dir,
                                                          ptz_relative_amount_test_value_table[i].out_dir,
                                                          DEFAULT_SEQ_ID,
                                                          blankName)))
                .Times(1)
                .WillOnce(Return());

            result = biz_ptzf_if.setZoomRelativeMove(zoom_move_relative_test_value_table[i].in_dir,
                                                     ptz_relative_amount_test_value_table[i].in_dir);
            EXPECT_TRUE(result);
        }
    }

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setZoomRelativeMove(
        zoom_move_relative_test_value_table[0].in_dir, ptz_relative_amount_test_value_table[0].in_dir, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setZoomRelativeMove2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (zoom_move_relative_test_value_table, i) {
        if (zoom_move_relative_test_value_table[i].in_dir == ZOOM_DIRECTION_STOP) {
            EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
            result = biz_ptzf_if.setZoomRelativeMove(
                zoom_move_relative_test_value_table[0].in_dir, ptz_relative_amount_test_value_table[0].in_dir, seq_id);
            EXPECT_FALSE(result);
        }
        else {
            EXPECT_CALL(er_mock_,
                        post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                             _,
                             sizeof(ptzf::SetZoomRelativeMoveRequest),
                             EqSetZoomRelativeMoveRequest(zoom_move_relative_test_value_table[i].out_dir,
                                                          ptz_relative_amount_test_value_table[i].out_dir,
                                                          seq_id,
                                                          reply_.getName())))
                .Times(1)
                .WillOnce(Return());

            result = biz_ptzf_if.setZoomRelativeMove(
                zoom_move_relative_test_value_table[i].in_dir, ptz_relative_amount_test_value_table[i].in_dir, seq_id);
            EXPECT_TRUE(result);
        }
    }
}

TEST_F(BizPtzfIfTest, setHomePosition1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::HomePositionRequest),
                     EqHomePositionRequest(DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setHomePosition();
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setHomePosition(INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setHomePosition2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::HomePositionRequest),
                     EqHomePositionRequest(seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setHomePosition(seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setStandbyMode1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    ARRAY_FOREACH (standby_mode_test_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::BizMessage<ptzf::SetStandbyModeRequest>),
                         EqsetStandbyModeRequest(standby_mode_test_table[i].ptzf_value, DEFAULT_SEQ_ID, blankName)))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setStandbyMode(standby_mode_test_table[i].biz_ptzf_value);
        EXPECT_TRUE(result);
    }

    biz_ptzf::StandbyMode mode = StandbyMode::NEUTRAL;
    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setStandbyMode(mode, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setStandbyMode2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (standby_mode_test_table, i) {
        EXPECT_CALL(er_mock_,
                    post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                         _,
                         sizeof(ptzf::BizMessage<ptzf::SetStandbyModeRequest>),
                         EqsetStandbyModeRequest(standby_mode_test_table[i].ptzf_value, seq_id, reply_.getName())))
            .Times(1)
            .WillOnce(Return());

        result = biz_ptzf_if.setStandbyMode(standby_mode_test_table[i].biz_ptzf_value, seq_id);
        EXPECT_TRUE(result);
    }

    biz_ptzf::StandbyMode mode = StandbyMode::NEUTRAL;
    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setStandbyMode(mode, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, exeFocusHoldButton1way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    FocusHold focus_hold = FOCUS_HOLD_RELEASE;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusHoldRequest),
                     EqsetFocusHoldRequest(focus_hold, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.exeFocusHoldButton(focus_hold);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.exeFocusHoldButton(focus_hold, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, exeFocusHoldButton2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    FocusHold focus_hold = FOCUS_HOLD_RELEASE;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusHoldRequest),
                     EqsetFocusHoldRequest(focus_hold, DEFAULT_SEQ_ID, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.exeFocusHoldButton(focus_hold);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.exeFocusHoldButton(focus_hold, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, exePushFocusButton1way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    PushFocus push_focus = PUSH_FOCUS_RELEASE;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetPushFocusRequest),
                     EqsetPushFocusRequest(push_focus, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.exePushFocusButton(push_focus);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.exePushFocusButton(push_focus, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, exePushFocusButton2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    PushFocus push_focus = PUSH_FOCUS_RELEASE;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetPushFocusRequest),
                     EqsetPushFocusRequest(push_focus, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.exePushFocusButton(push_focus, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setFocusTrackingCancel1way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    FocusTrackingCancel focus_tracking_cancel = FOCUS_TRACKING_CANCEL_RELEASE;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusTrackingCancelRequest),
                     EqsetFocusTrackingCancelRequest(focus_tracking_cancel, DEFAULT_SEQ_ID, blankName)))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setFocusTrackingCancel(focus_tracking_cancel);
    EXPECT_TRUE(result);

    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(0);
    result = biz_ptzf_if.setFocusTrackingCancel(focus_tracking_cancel, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusTrackingCancel2way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;
    biz_ptzf_if.registNotification(reply_.getName());

    FocusTrackingCancel focus_tracking_cancel = FOCUS_TRACKING_CANCEL_RELEASE;
    EXPECT_CALL(er_mock_,
                post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER,
                     _,
                     sizeof(ptzf::SetFocusTrackingCancelRequest),
                     EqsetFocusTrackingCancelRequest(focus_tracking_cancel, seq_id, reply_.getName())))
        .Times(1)
        .WillOnce(Return());

    result = biz_ptzf_if.setFocusTrackingCancel(focus_tracking_cancel, seq_id);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setName1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_, post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, _, _, _))
        .Times(1)
        .WillOnce(Return());

    biz_ptzf::TraceName name;
    name.trace_id = 0;
    gtl::copyString(name.name, "Trace001");

    result = biz_ptzf_if.setName(name);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, setName2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_, post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, _, _, _))
        .Times(1)
        .WillOnce(Return());

    biz_ptzf::TraceName name;
    name.trace_id = 1;
    gtl::copyString(name.name, "Trace001");

    result = biz_ptzf_if.setName(name, 1);
    EXPECT_TRUE(result);
}

TEST_F(BizPtzfIfTest, getPanTiltMoveStatusSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    bool status = false;
    ErrorCode err;

    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand())
        .Times(4)
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .WillOnce(Return(false));

    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand())
        .Times(4)
        .WillOnce(Return(false))
        .WillOnce(Return(true))
        .WillOnce(Return(false))
        .WillOnce(Return(false));

    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand())
        .Times(4)
        .WillOnce(Return(false))
        .WillOnce(Return(false))
        .WillOnce(Return(true))
        .WillOnce(Return(false));

    err = biz_ptzf_if.getPanTiltMoveStatus(status);
    EXPECT_TRUE(status);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    err = biz_ptzf_if.getPanTiltMoveStatus(status);
    EXPECT_TRUE(status);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    err = biz_ptzf_if.getPanTiltMoveStatus(status);
    EXPECT_TRUE(status);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    err = biz_ptzf_if.getPanTiltMoveStatus(status);
    EXPECT_FALSE(status);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getStandbyModeSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    StandbyMode mode = StandbyMode::NEUTRAL;
    StandbyMode mode_result = StandbyMode::SIDE;
    ErrorCode err;

    biz_ptzf_if.setStandbyMode(mode);
    err = biz_ptzf_if.getStandbyMode(mode_result);
    EXPECT_EQ(mode, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    biz_ptzf_if.setStandbyMode(mode);
    err = biz_ptzf_if.getStandbyMode(mode_result);
    EXPECT_EQ(mode, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getName)
{
    BizPtzfIf biz_ptzf_if;

    std::vector<ptzf::PtzTraceId> id_list;
    for (u32_t i = 0; i < ptzf::PTZ_TRACE_ID_MAX_SIZE; ++i) {
        ptzf::PtzTraceId id(i, true);
        id_list.push_back(id);
    }

    EXPECT_CALL(ptz_trace_status_if_mock_, getTraceNumbers(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(id_list), Return()));

    std::vector<TraceName> name_list;
    ErrorCode err = biz_ptzf_if.getNameList(name_list);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, setAfSubjShiftSens)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_, post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, _, _, _))
        .Times(5)
        .WillOnce(Return());

    ARRAY_FOREACH (af_subj_shift_sens_values, i) {
        result = biz_ptzf_if.setAfSubjShiftSens(af_subj_shift_sens_values[i], 1);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setAfTransitionSpeed)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    bool result;

    EXPECT_CALL(er_mock_, post(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER, _, _, _))
        .Times(7)
        .WillOnce(Return());

    ARRAY_FOREACH (af_transition_speed_values, i) {
        result = biz_ptzf_if.setAfTransitionSpeed(af_transition_speed_values[i], 1);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, getAfSubjShiftSens)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    EXPECT_CALL(ptzf_biz_message_if_mock_, getAfSubjShiftSens(_)).Times(1).WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getAfSubjShiftSens());
}

TEST_F(BizPtzfIfTest, getAfTransitionSpeed)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    EXPECT_CALL(ptzf_biz_message_if_mock_, getAfTransitionSpeed(_)).Times(1).WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getAfTransitionSpeed());
}

TEST_F(BizPtzfIfTest, getCamOp)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getCamOp(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getCamOp());
}

TEST_F(BizPtzfIfTest, getPanTiltSpeedScale)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getPanTiltSpeedScale(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getPanTiltSpeedScale());
}

TEST_F(BizPtzfIfTest, getPanTiltSpeedMode)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getPanTiltSpeedMode(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getPanTiltSpeedMode());
}

TEST_F(BizPtzfIfTest, getPanTiltSpeedStep)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    ptzf::message::PanTiltSpeedStepInquiryResult result;

    ptzf::PanTiltSpeedStep speed_step = ptzf::PAN_TILT_SPEED_STEP_NORMAL;
    ptzf_st.setSpeedStep(speed_step);
    EXPECT_TRUE(biz_ptzf_if.getPanTiltSpeedStep());

    reply_.pend(result);
    EXPECT_EQ(speed_step, result.pan_tilt_speed_step);
    EXPECT_EQ(ERRORCODE_SUCCESS, result.error);

    speed_step = ptzf::PAN_TILT_SPEED_STEP_EXTENDED;
    ptzf_st.setSpeedStep(speed_step);
    EXPECT_TRUE(biz_ptzf_if.getPanTiltSpeedStep());

    reply_.pend(result);
    EXPECT_EQ(speed_step, result.pan_tilt_speed_step);
    EXPECT_EQ(ERRORCODE_SUCCESS, result.error);
}

TEST_F(BizPtzfIfTest, getSettingPositionValueSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    ptzf::SettingPosition settingpos = ptzf::SETTING_POSITION_DESKTOP;
    biz_ptzf::SettingPosition mode_result = biz_ptzf::SETTING_POSITION_DESKTOP;
    ErrorCode err;

    ptzf_st.setSettingPositionStatusOnBoot(settingpos);
    err = biz_ptzf_if.getSettingPosition(mode_result);
    EXPECT_EQ(biz_ptzf::SETTING_POSITION_DESKTOP, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    settingpos = ptzf::SETTING_POSITION_CEILING;
    ptzf_st.setSettingPositionStatusOnBoot(settingpos);
    err = biz_ptzf_if.getSettingPosition(mode_result);
    EXPECT_EQ(biz_ptzf::SETTING_POSITION_CEILING, mode_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    settingpos = static_cast<ptzf::SettingPosition>(U32_T(0x04));
    ptzf_st.setSettingPositionStatusOnBoot(settingpos);
    err = biz_ptzf_if.getSettingPosition(mode_result);
    EXPECT_NE(static_cast<ptzf::SettingPosition>(U32_T(0x04)), mode_result);
    EXPECT_EQ(ERRORCODE_VAL, err);
}

TEST_F(BizPtzfIfTest, getPanDirection)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    bool mode = false;
    PanDirection direction_result = biz_ptzf::PAN_DIRECTION_OPPOSITE;
    ErrorCode err = ERRORCODE_SUCCESS;

    ptzf_st.setPanReverse(mode);
    err = biz_ptzf_if.getPanDirection(direction_result);
    EXPECT_EQ(biz_ptzf::PAN_DIRECTION_NORMAL, direction_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    mode = true;
    ptzf_st.setPanReverse(mode);
    err = biz_ptzf_if.getPanDirection(direction_result);
    EXPECT_EQ(biz_ptzf::PAN_DIRECTION_OPPOSITE, direction_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getTiltDirection)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    ptzf::PtzfStatus ptzf_st;

    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    bool mode = false;
    TiltDirection direction_result = biz_ptzf::TILT_DIRECTION_OPPOSITE;
    ErrorCode err = ERRORCODE_SUCCESS;

    ptzf_st.setTiltReverse(mode);
    err = biz_ptzf_if.getTiltDirection(direction_result);
    EXPECT_EQ(biz_ptzf::TILT_DIRECTION_NORMAL, direction_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    mode = true;
    ptzf_st.setTiltReverse(mode);
    err = biz_ptzf_if.getTiltDirection(direction_result);
    EXPECT_EQ(biz_ptzf::TILT_DIRECTION_OPPOSITE, direction_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getZoomSpeedScale)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getZoomSpeedScale(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getZoomSpeedScale());
}

TEST_F(BizPtzfIfTest, getPushAFMode)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getPushAFMode(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getPushAFMode());
}

TEST_F(BizPtzfIfTest, getFocusTrackingStatus)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusTrackingStatus(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusTrackingStatus());
}

TEST_F(BizPtzfIfTest, getFocusTrackingPositionPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusTrackingPositionPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusTrackingPositionPmt());
}

// 現状、未使用I/F
TEST_F(BizPtzfIfTest, getFocusTrackingCancelPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusTrackingCancelPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusTrackingCancelPmt());
}

TEST_F(BizPtzfIfTest, getFocusModePmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusModePmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusModePmt());
}

TEST_F(BizPtzfIfTest, getFocusAbsolutePositionPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusAbsolutePositionPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusAbsolutePositionPmt());
}

TEST_F(BizPtzfIfTest, getPushFocusButtonPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getPushFocusButtonPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getPushFocusButtonPmt());
}

TEST_F(BizPtzfIfTest, getFocusHoldButtonPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusHoldButtonPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusHoldButtonPmt());
}

TEST_F(BizPtzfIfTest, getFocusFaceEyedetectionPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getFocusFaceEyedetectionPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getFocusFaceEyedetectionPmt());
}

TEST_F(BizPtzfIfTest, getAFTransitionSpeedPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getAFTransitionSpeedPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getAFTransitionSpeedPmt());
}

TEST_F(BizPtzfIfTest, getAfSubjShitSensPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getAfSubjShitSensPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getAfSubjShitSensPmt());
}

TEST_F(BizPtzfIfTest, getTouchFunctionInMfPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getTouchFunctionInMfPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getTouchFunctionInMfPmt());
}

TEST_F(BizPtzfIfTest, getPushAFModePmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getPushAFModePmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getPushAFModePmt());
}

TEST_F(BizPtzfIfTest, getAfAssistPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getAfAssistPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getAfAssistPmt());
}

TEST_F(BizPtzfIfTest, getZoomFineMoveRequestPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getZoomFineMoveRequestPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getZoomFineMoveRequestPmt());
}

TEST_F(BizPtzfIfTest, getIndicatorFocusModeState)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getIndicatorFocusModeState(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorFocusModeState());
}

TEST_F(BizPtzfIfTest, getIndicatorFaceEyeAFState)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getIndicatorFaceEyeAFState(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorFaceEyeAFState());
}

TEST_F(BizPtzfIfTest, getIndicatorRegisteredTrackingFaceState)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getIndicatorRegisteredTrackingFaceState(
                    Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorRegisteredTrackingFaceState());
}

TEST_F(BizPtzfIfTest, getIndicatorTrackingAFStopState)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(
        ptzf_biz_message_if_mock_,
        getIndicatorTrackingAFStopState(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorTrackingAFStopState());
}

TEST_F(BizPtzfIfTest, getIndicatorFocusPositionMeterState)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(
        ptzf_biz_message_if_mock_,
        getIndicatorFocusPositionMeterState(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorFocusPositionMeterState());
}

TEST_F(BizPtzfIfTest, getIndicatorFocusPositionFeetState)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(
        ptzf_biz_message_if_mock_,
        getIndicatorFocusPositionFeetState(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorFocusPositionFeetState());
}

TEST_F(BizPtzfIfTest, getIndicatorFocusPositionUnitState)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(
        ptzf_biz_message_if_mock_,
        getIndicatorFocusPositionUnitState(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorFocusPositionUnitState());
}

TEST_F(BizPtzfIfTest, getIndicatorFocusPositionPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getIndicatorFocusPositionPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorFocusPositionPmt());
}

TEST_F(BizPtzfIfTest, getIndicatorZoomPositionState)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getIndicatorZoomPositionState(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorZoomPositionState());
}

TEST_F(BizPtzfIfTest, getIndicatorZoomPositionRateState)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(
        ptzf_biz_message_if_mock_,
        getIndicatorZoomPositionRateState(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorZoomPositionRateState());
}

TEST_F(BizPtzfIfTest, getIndicatorZoomPositionUnitState)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(
        ptzf_biz_message_if_mock_,
        getIndicatorZoomPositionUnitState(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorZoomPositionUnitState());
}

TEST_F(BizPtzfIfTest, getIndicatorZoomPositionPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getIndicatorZoomPositionPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorZoomPositionPmt());
}

TEST_F(BizPtzfIfTest, getIndicatorCizIconState)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getIndicatorCizIconState(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorCizIconState());
}

TEST_F(BizPtzfIfTest, getIndicatorCizRatioState)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getIndicatorCizRatioState(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorCizRatioState());
}

TEST_F(BizPtzfIfTest, getIndicatorCizRatioPmt)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;
    biz_ptzf_if.registNotification(reply_.getName());

    EXPECT_CALL(ptzf_biz_message_if_mock_,
                getIndicatorCizRatioPmt(Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_TRUE(biz_ptzf_if.getIndicatorCizRatioPmt());
}

TEST_F(BizPtzfIfTest, getPanTiltLockStatusSuccess)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());
    BizPtzfIf biz_ptzf_if;

    u32_t status_expect = false;
    bool status_result = false;
    ErrorCode err;

    EXPECT_CALL(ptzf_biz_message_if_mock_, getPanTiltLockStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(status_expect), Return(ERRORCODE_SUCCESS)));
    err = biz_ptzf_if.getPanTiltLockStatus(status_result);
    EXPECT_EQ(status_expect, status_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);

    status_expect = true;
    EXPECT_CALL(ptzf_biz_message_if_mock_, getPanTiltLockStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(status_expect), Return(ERRORCODE_SUCCESS)));
    err = biz_ptzf_if.getPanTiltLockStatus(status_result);
    EXPECT_EQ(status_expect, status_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, err);
}

TEST_F(BizPtzfIfTest, getPanTiltEnabledStateSuccess)
{
    const struct ConvertTable
    {
        PanTiltEnabledState biz_value;
        ptzf::PanTiltEnabledState domain_value;
    } table[] = {
        { PAN_TILT_ENABLED_STATE_DISABLE, ptzf::PAN_TILT_ENABLED_STATE_DISABLE },
        { PAN_TILT_ENABLED_STATE_ENABLE, ptzf::PAN_TILT_ENABLED_STATE_ENABLE },
    };

    ARRAY_FOREACH (table, i) {
        EXPECT_CALL(ptzf_biz_message_if_mock_, getPanTiltEnabledState(_))
            .Times(1)
            .WillOnce(DoAll(SetArgReferee<0>(table[i].domain_value), Return(ERRORCODE_SUCCESS)));

        BizPtzfIf biz_ptzf_if;
        PanTiltEnabledState enabled_state(PAN_TILT_ENABLED_STATE_UNKNOWN);
        const ErrorCode result = biz_ptzf_if.getPanTiltEnabledState(enabled_state);

        EXPECT_EQ(result, ERRORCODE_SUCCESS);
        EXPECT_EQ(enabled_state, table[i].biz_value);
    }
}

TEST_F(BizPtzfIfTest, getPanTiltEnabledStateError)
{
    EXPECT_CALL(ptzf_biz_message_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(ptzf::PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_EXEC)));

    BizPtzfIf biz_ptzf_if;
    PanTiltEnabledState enabled_state(PAN_TILT_ENABLED_STATE_UNKNOWN);
    const ErrorCode result = biz_ptzf_if.getPanTiltEnabledState(enabled_state);

    EXPECT_EQ(result, ERRORCODE_EXEC);
}

TEST_F(BizPtzfIfTest, setFocusMode1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (focus_mode_values, i) {
        result = biz_ptzf_if.setFocusMode(focus_mode_values[i]);
        EXPECT_TRUE(result);
    }

    result =
        biz_ptzf_if.setFocusMode(focus_mode_values[0], INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusMode2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (focus_mode_values, i) {
        result =
            biz_ptzf_if.setFocusMode(focus_mode_values[i], seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setAfTransitionSpeedValue1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());
    ARRAY_FOREACH (af_transition_speed_values, i) {
        result = biz_ptzf_if.setAfTransitionSpeedValue(af_transition_speed_values[i]);
        EXPECT_TRUE(result);
    }
    result =
        biz_ptzf_if.setAfTransitionSpeedValue(af_transition_speed_values[0], INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setAfTransitionSpeedValue2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (af_transition_speed_values, i) {
        result =
            biz_ptzf_if.setAfTransitionSpeedValue(af_transition_speed_values[i], seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setAfSubjShiftSensValue1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());
    ARRAY_FOREACH (af_subj_shift_sens_values, i) {
        result = biz_ptzf_if.setAfSubjShiftSensValue(af_subj_shift_sens_values[i]);
        EXPECT_TRUE(result);
    }
    result =
        biz_ptzf_if.setAfSubjShiftSensValue(af_subj_shift_sens_values[0], INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setAfSubjShiftSensValue2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (af_subj_shift_sens_values, i) {
        result =
            biz_ptzf_if.setAfSubjShiftSensValue(af_subj_shift_sens_values[i], seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setFocusFaceEyedetectionValue1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (focus_face_eye_detection_mode_test_value_table, i) {
        result = biz_ptzf_if.setFocusFaceEyedetectionValue(focus_face_eye_detection_mode_test_value_table[i].in_mode);
        EXPECT_TRUE(result);
    }
    result =
        biz_ptzf_if.setFocusFaceEyedetectionValue(focus_face_eye_detection_mode_test_value_table[0].in_mode, INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusFaceEyedetectionValue2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (focus_face_eye_detection_mode_test_value_table, i) {
        result =
            biz_ptzf_if.setFocusFaceEyedetectionValue(focus_face_eye_detection_mode_test_value_table[i].in_mode, seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setFocusArea1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (focus_area_values, i) {
        result = biz_ptzf_if.setFocusArea(focus_area_values[i]);
        EXPECT_TRUE(result);
    }
    result =
        biz_ptzf_if.setFocusArea(focus_area_values[0], INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusArea2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (focus_area_values, i) {
        result =
            biz_ptzf_if.setFocusArea(focus_area_values[i], seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setAFAreaPositionAFC1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());
    ARRAY_FOREACH (u16_t_case_list, i) {
        result = biz_ptzf_if.setAFAreaPositionAFC(u16_t_case_list[i], u16_t_case_list[i]);
        EXPECT_TRUE(result);
    }
    result =
        biz_ptzf_if.setAFAreaPositionAFC(u16_t_case_list[0], u16_t_case_list[0], INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setAFAreaPositionAFC2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (u16_t_case_list, i) {
        result =
            biz_ptzf_if.setAFAreaPositionAFC(u16_t_case_list[i], u16_t_case_list[i], seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setAFAreaPositionAFS1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());
    ARRAY_FOREACH (u16_t_case_list, i) {
        result = biz_ptzf_if.setAFAreaPositionAFS(u16_t_case_list[i], u16_t_case_list[i]);
        EXPECT_TRUE(result);
    }
    result =
        biz_ptzf_if.setAFAreaPositionAFS(u16_t_case_list[0], u16_t_case_list[0], INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setAFAreaPositionAFS2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (u16_t_case_list, i) {
        result =
            biz_ptzf_if.setAFAreaPositionAFS(u16_t_case_list[i], u16_t_case_list[i], seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setZoomPosition1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());
    ARRAY_FOREACH (u32_t_case_list, i) {
        result = biz_ptzf_if.setZoomPosition(u32_t_case_list[i]);
        EXPECT_TRUE(result);
    }
    result =
        biz_ptzf_if.setZoomPosition(u32_t_case_list[0], INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setZoomPosition2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (u32_t_case_list, i) {
        result =
            biz_ptzf_if.setZoomPosition(u32_t_case_list[i], seq_id);
        EXPECT_TRUE(result);
    }
}

TEST_F(BizPtzfIfTest, setFocusPosition1Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());
    ARRAY_FOREACH (u32_t_case_list, i) {
        result = biz_ptzf_if.setFocusPosition(u32_t_case_list[i]);
        EXPECT_TRUE(result);
    }
    result =
        biz_ptzf_if.setFocusPosition(u32_t_case_list[0], INVALID_SEQ_ID);
    EXPECT_FALSE(result);
}

TEST_F(BizPtzfIfTest, setFocusPosition2Way)
{
    EXPECT_CALL(er_mock_, create(event_router::EVENT_ROUTER_TARGET_TYPE_PTZF_CONTROLLER)).Times(1).WillOnce(Return());

    BizPtzfIf biz_ptzf_if;
    u32_t seq_id = U32_T(123456);
    bool result;

    biz_ptzf_if.registNotification(reply_.getName());

    ARRAY_FOREACH (u32_t_case_list, i) {
        result =
            biz_ptzf_if.setFocusPosition(u32_t_case_list[i], seq_id);
        EXPECT_TRUE(result);
    }
}

} // namespace

} // namespace biz_ptzf
