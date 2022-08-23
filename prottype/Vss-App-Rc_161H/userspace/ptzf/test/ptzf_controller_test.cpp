/*
 * ptzf_controller_test.cpp
 *
 * Copyright 2016,2018 Sony Imaging Products & Solutions Inc.
 */

#include "types.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "common_gmock_util.h"
#include "gtl_array.h"
#include "common_thread_object.h"

#include "ptzf_controller.h"
#include "ptzf/ptzf_parameter.h"
#include "visca/visca_server_ptzf_if_mock.h"
#include "ptzf/ptzf_biz_message_if_mock.h"
#include "ptzf/ptzf_status_if_mock.h"
#include "ptzf/ptzf_config_if_mock.h"
#include "ptzf_capability_infra_if_mock.h"
#include "ptzf_pan_tilt_infra_if_mock.h"
#include "ptzf_zoom_infra_if_mock.h"
#include "ptzf_focus_infra_if_mock.h"
#include "ptz_updater_mock.h"
#include "ptzf_controller_message_handler.h"
#include "ptzf_status_infra_if_mock.h"
#include "ptzf_config_infra_if_mock.h"
#include "ptzf/ptzf_initialize_infra_if_mock.h"
#include "ptzf/ptzf_common_message.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnArg;
using ::testing::Field;
using ::testing::DoAll;
using ::testing::SaveArg;
using ::testing::SetArgReferee;

namespace ptzf {

namespace {

PanTiltDirection pan_tilt_direction_table[] = {
    PAN_TILT_DIRECTION_STOP,     PAN_TILT_DIRECTION_UP,        PAN_TILT_DIRECTION_DOWN,
    PAN_TILT_DIRECTION_LEFT,     PAN_TILT_DIRECTION_RIGHT,     PAN_TILT_DIRECTION_UP_LEFT,
    PAN_TILT_DIRECTION_UP_RIGHT, PAN_TILT_DIRECTION_DOWN_LEFT, PAN_TILT_DIRECTION_DOWN_RIGHT,
};

const struct ZoomMoveParamTable
{
    u8_t speed;
    ZoomDirection direction;
    ZoomMove move;
    u8_t visca_speed;
} zoom_move_param_table[] = {
    { U8_T(1), ZOOM_DIRECTION_TELE, ZOOM_MOVE_TELE, U8_T(1) },
    { U8_T(1), ZOOM_DIRECTION_WIDE, ZOOM_MOVE_WIDE, U8_T(1) },
    { U8_T(7), ZOOM_DIRECTION_TELE, ZOOM_MOVE_TELE, U8_T(7) },
    { U8_T(7), ZOOM_DIRECTION_WIDE, ZOOM_MOVE_WIDE, U8_T(7) },
};

const struct ZoomMoveSircsParamTable
{
    u8_t speed;
    ZoomDirection direction;
    ZoomMove move;
    u8_t visca_speed;
} zoom_move_sircs_param_table[] = {
    { U8_T(1), ZOOM_DIRECTION_TELE, ZOOM_MOVE_TELE, U8_T(2) },
    { U8_T(1), ZOOM_DIRECTION_WIDE, ZOOM_MOVE_WIDE, U8_T(2) },
    { U8_T(2), ZOOM_DIRECTION_TELE, ZOOM_MOVE_TELE, U8_T(7) },
    { U8_T(2), ZOOM_DIRECTION_WIDE, ZOOM_MOVE_WIDE, U8_T(7) },
};

const FocusMode focus_mode_param_table[] = {
    FOCUS_MODE_AUTO,
    FOCUS_MODE_MANUAL,
};

const FocusDirection focus_direction_param_table[] = {
    FOCUS_DIRECTION_STOP,
    FOCUS_DIRECTION_FAR,
    FOCUS_DIRECTION_NEAR,
};

struct ZoomRelativeMoveTeleTable
{
    PTZRelativeAmount amount;
    DZoom zoom_mode;
    u16_t position;
} zoom_relative_move_tele_table[] = {
    { PTZ_RELATIVE_AMOUNT_1, DZOOM_FULL, U16_T(0xc46) },
    { PTZ_RELATIVE_AMOUNT_2, DZOOM_FULL, U16_T(0x1269) },
    { PTZ_RELATIVE_AMOUNT_3, DZOOM_FULL, U16_T(0x188c) },
    { PTZ_RELATIVE_AMOUNT_4, DZOOM_FULL, U16_T(0x1eb0) },
    { PTZ_RELATIVE_AMOUNT_5, DZOOM_FULL, U16_T(0x24d3) },
    { PTZ_RELATIVE_AMOUNT_6, DZOOM_FULL, U16_T(0x3119) },
    { PTZ_RELATIVE_AMOUNT_7, DZOOM_FULL, U16_T(0x3d60) },
    { PTZ_RELATIVE_AMOUNT_8, DZOOM_FULL, U16_T(0x51df) },
    { PTZ_RELATIVE_AMOUNT_9, DZOOM_FULL, U16_T(0x6640) },
    { PTZ_RELATIVE_AMOUNT_10, DZOOM_FULL, U16_T(0x7ac0) },
    { PTZ_RELATIVE_AMOUNT_1, DZOOM_OPTICAL, U16_T(0xc46) },
    { PTZ_RELATIVE_AMOUNT_2, DZOOM_OPTICAL, U16_T(0x1269) },
    { PTZ_RELATIVE_AMOUNT_3, DZOOM_OPTICAL, U16_T(0x188c) },
    { PTZ_RELATIVE_AMOUNT_4, DZOOM_OPTICAL, U16_T(0x1eb0) },
    { PTZ_RELATIVE_AMOUNT_5, DZOOM_OPTICAL, U16_T(0x24d3) },
    { PTZ_RELATIVE_AMOUNT_6, DZOOM_OPTICAL, U16_T(0x3119) },
    { PTZ_RELATIVE_AMOUNT_7, DZOOM_OPTICAL, U16_T(0x3d60) },
    { PTZ_RELATIVE_AMOUNT_8, DZOOM_OPTICAL, U16_T(0x4000) },
    { PTZ_RELATIVE_AMOUNT_9, DZOOM_OPTICAL, U16_T(0x4000) },
    { PTZ_RELATIVE_AMOUNT_10, DZOOM_OPTICAL, U16_T(0x4000) },
    { PTZ_RELATIVE_AMOUNT_1, DZOOM_CLEAR_IMAGE, U16_T(0xc46) },
    { PTZ_RELATIVE_AMOUNT_2, DZOOM_CLEAR_IMAGE, U16_T(0x1269) },
    { PTZ_RELATIVE_AMOUNT_3, DZOOM_CLEAR_IMAGE, U16_T(0x188c) },
    { PTZ_RELATIVE_AMOUNT_4, DZOOM_CLEAR_IMAGE, U16_T(0x1eb0) },
    { PTZ_RELATIVE_AMOUNT_5, DZOOM_CLEAR_IMAGE, U16_T(0x24d3) },
    { PTZ_RELATIVE_AMOUNT_6, DZOOM_CLEAR_IMAGE, U16_T(0x3119) },
    { PTZ_RELATIVE_AMOUNT_7, DZOOM_CLEAR_IMAGE, U16_T(0x3d60) },
    { PTZ_RELATIVE_AMOUNT_8, DZOOM_CLEAR_IMAGE, U16_T(0x51df) },
    { PTZ_RELATIVE_AMOUNT_9, DZOOM_CLEAR_IMAGE, U16_T(0x6000) },
    { PTZ_RELATIVE_AMOUNT_10, DZOOM_CLEAR_IMAGE, U16_T(0x6000) },
};

struct ZoomRelativeMoveWideTable
{
    PTZRelativeAmount amount;
    DZoom zoom_mode;
    u16_t position;
} zoom_relative_move_wide_table[] = {
    { PTZ_RELATIVE_AMOUNT_1, DZOOM_FULL, U16_T(0x6e7a) },
    { PTZ_RELATIVE_AMOUNT_2, DZOOM_FULL, U16_T(0x6857) },
    { PTZ_RELATIVE_AMOUNT_3, DZOOM_FULL, U16_T(0x6234) },
    { PTZ_RELATIVE_AMOUNT_4, DZOOM_FULL, U16_T(0x5c10) },
    { PTZ_RELATIVE_AMOUNT_5, DZOOM_FULL, U16_T(0x55ed) },
    { PTZ_RELATIVE_AMOUNT_6, DZOOM_FULL, U16_T(0x49a7) },
    { PTZ_RELATIVE_AMOUNT_7, DZOOM_FULL, U16_T(0x3d60) },
    { PTZ_RELATIVE_AMOUNT_8, DZOOM_FULL, U16_T(0x28e1) },
    { PTZ_RELATIVE_AMOUNT_9, DZOOM_FULL, U16_T(0x1480) },
    { PTZ_RELATIVE_AMOUNT_10, DZOOM_FULL, U16_T(0x0) },
    { PTZ_RELATIVE_AMOUNT_1, DZOOM_OPTICAL, U16_T(0x4000) },
    { PTZ_RELATIVE_AMOUNT_2, DZOOM_OPTICAL, U16_T(0x4000) },
    { PTZ_RELATIVE_AMOUNT_3, DZOOM_OPTICAL, U16_T(0x4000) },
    { PTZ_RELATIVE_AMOUNT_4, DZOOM_OPTICAL, U16_T(0x4000) },
    { PTZ_RELATIVE_AMOUNT_5, DZOOM_OPTICAL, U16_T(0x4000) },
    { PTZ_RELATIVE_AMOUNT_6, DZOOM_OPTICAL, U16_T(0x4000) },
    { PTZ_RELATIVE_AMOUNT_7, DZOOM_OPTICAL, U16_T(0x3d60) },
    { PTZ_RELATIVE_AMOUNT_8, DZOOM_OPTICAL, U16_T(0x28e1) },
    { PTZ_RELATIVE_AMOUNT_9, DZOOM_OPTICAL, U16_T(0x1480) },
    { PTZ_RELATIVE_AMOUNT_10, DZOOM_OPTICAL, U16_T(0x0) },
    { PTZ_RELATIVE_AMOUNT_1, DZOOM_CLEAR_IMAGE, U16_T(0x6000) },
    { PTZ_RELATIVE_AMOUNT_2, DZOOM_CLEAR_IMAGE, U16_T(0x6000) },
    { PTZ_RELATIVE_AMOUNT_3, DZOOM_CLEAR_IMAGE, U16_T(0x6000) },
    { PTZ_RELATIVE_AMOUNT_4, DZOOM_CLEAR_IMAGE, U16_T(0x5c10) },
    { PTZ_RELATIVE_AMOUNT_5, DZOOM_CLEAR_IMAGE, U16_T(0x55ed) },
    { PTZ_RELATIVE_AMOUNT_6, DZOOM_CLEAR_IMAGE, U16_T(0x49a7) },
    { PTZ_RELATIVE_AMOUNT_7, DZOOM_CLEAR_IMAGE, U16_T(0x3d60) },
    { PTZ_RELATIVE_AMOUNT_8, DZOOM_CLEAR_IMAGE, U16_T(0x28e1) },
    { PTZ_RELATIVE_AMOUNT_9, DZOOM_CLEAR_IMAGE, U16_T(0x1480) },
    { PTZ_RELATIVE_AMOUNT_10, DZOOM_CLEAR_IMAGE, U16_T(0x0) },
};

const struct PTZPanTiltRelativeParamTable
{
    PanTiltDirection direction;
    PTZRelativeAmount amount;
    u16_t zoom_position;
    u32_t pan_position;
    u32_t tilt_position;
} ptz_pan_tilt_relative_param_table[] = {
    { PAN_TILT_DIRECTION_UP, PTZ_RELATIVE_AMOUNT_1, U16_T(0x0000), U32_T(0x0), U32_T(0xd6) },
    { PAN_TILT_DIRECTION_DOWN, PTZ_RELATIVE_AMOUNT_2, U16_T(0x0DC1), U32_T(0x0), U32_T(0xffffff54) },
    { PAN_TILT_DIRECTION_LEFT, PTZ_RELATIVE_AMOUNT_3, U16_T(0x186C), U32_T(0xfffffeee), U32_T(0x0) },
    { PAN_TILT_DIRECTION_RIGHT, PTZ_RELATIVE_AMOUNT_4, U16_T(0x2015), U32_T(0x104), U32_T(0x0) },
    { PAN_TILT_DIRECTION_UP_LEFT, PTZ_RELATIVE_AMOUNT_5, U16_T(0x2594), U32_T(0xffffff01), U32_T(0x93) },
    { PAN_TILT_DIRECTION_UP_RIGHT, PTZ_RELATIVE_AMOUNT_6, U16_T(0x29B7), U32_T(0x120), U32_T(0xa8) },
    { PAN_TILT_DIRECTION_DOWN_LEFT, PTZ_RELATIVE_AMOUNT_7, U16_T(0x2CFB), U32_T(0xfffffeca), U32_T(0xffffff4c) },
    { PAN_TILT_DIRECTION_DOWN_RIGHT, PTZ_RELATIVE_AMOUNT_8, U16_T(0x2FB0), U32_T(0x16e), U32_T(0xffffff32) },
    { PAN_TILT_DIRECTION_UP_LEFT, PTZ_RELATIVE_AMOUNT_9, U16_T(0x320C), U32_T(0xfffffe68), U32_T(0xe9) },
    { PAN_TILT_DIRECTION_UP_RIGHT, PTZ_RELATIVE_AMOUNT_10, U16_T(0x342D), U32_T(0x1c2), U32_T(0x104) },
    { PAN_TILT_DIRECTION_UP, PTZ_RELATIVE_AMOUNT_1, U16_T(0x3608), U32_T(0x0), U32_T(0x17) },
    { PAN_TILT_DIRECTION_DOWN, PTZ_RELATIVE_AMOUNT_2, U16_T(0x37AA), U32_T(0x0), U32_T(0xffffffdf) },
    { PAN_TILT_DIRECTION_LEFT, PTZ_RELATIVE_AMOUNT_3, U16_T(0x391C), U32_T(0xffffffb8), U32_T(0x0) },
    { PAN_TILT_DIRECTION_RIGHT, PTZ_RELATIVE_AMOUNT_4, U16_T(0x3A66), U32_T(0x55), U32_T(0x0) },
    { PAN_TILT_DIRECTION_UP_LEFT, PTZ_RELATIVE_AMOUNT_5, U16_T(0x3B90), U32_T(0xffffffa0), U32_T(0x39) },
    { PAN_TILT_DIRECTION_UP_RIGHT, PTZ_RELATIVE_AMOUNT_6, U16_T(0x3C9C), U32_T(0x78), U32_T(0x48) },
    { PAN_TILT_DIRECTION_DOWN_LEFT, PTZ_RELATIVE_AMOUNT_7, U16_T(0x3D91), U32_T(0xffffff6f), U32_T(0xffffffab) },
    { PAN_TILT_DIRECTION_DOWN_RIGHT, PTZ_RELATIVE_AMOUNT_8, U16_T(0x3E72), U32_T(0xba), U32_T(0xffffff96) },
    { PAN_TILT_DIRECTION_UP_LEFT, PTZ_RELATIVE_AMOUNT_9, U16_T(0x3F40), U32_T(0xffffff28), U32_T(0x7c) },
    { PAN_TILT_DIRECTION_UP_RIGHT, PTZ_RELATIVE_AMOUNT_10, U16_T(0x4000), U32_T(0xfa), U32_T(0x8c) },
    { PAN_TILT_DIRECTION_UP, PTZ_RELATIVE_AMOUNT_1, U16_T(0x5556), U32_T(0x0), U32_T(0x9) },
    { PAN_TILT_DIRECTION_DOWN, PTZ_RELATIVE_AMOUNT_2, U16_T(0x6000), U32_T(0x0), U32_T(0xfffffff6) },
    { PAN_TILT_DIRECTION_LEFT, PTZ_RELATIVE_AMOUNT_3, U16_T(0x6AAB), U32_T(0xffffffee), U32_T(0x0) },
    { PAN_TILT_DIRECTION_RIGHT, PTZ_RELATIVE_AMOUNT_4, U16_T(0x7000), U32_T(0x11), U32_T(0x0) },
    { PAN_TILT_DIRECTION_UP_LEFT, PTZ_RELATIVE_AMOUNT_5, U16_T(0x7334), U32_T(0xffffffee), U32_T(0xc) },
    { PAN_TILT_DIRECTION_UP_RIGHT, PTZ_RELATIVE_AMOUNT_6, U16_T(0x7556), U32_T(0x14), U32_T(0x10) },
    { PAN_TILT_DIRECTION_DOWN_LEFT, PTZ_RELATIVE_AMOUNT_7, U16_T(0x76DC), U32_T(0xffffffec), U32_T(0xfffffff1) },
    { PAN_TILT_DIRECTION_DOWN_RIGHT, PTZ_RELATIVE_AMOUNT_8, U16_T(0x7800), U32_T(0x1a), U32_T(0xffffffec) },
    { PAN_TILT_DIRECTION_UP_LEFT, PTZ_RELATIVE_AMOUNT_9, U16_T(0x799A), U32_T(0xffffffe8), U32_T(0x10) },
    { PAN_TILT_DIRECTION_UP_RIGHT, PTZ_RELATIVE_AMOUNT_10, U16_T(0x7AC0), U32_T(0x14), U32_T(0xa) },
};

const struct PanTiltResetParamTable
{
    PanTiltLockControlStatus status;
    bool initialize_pan_tilt;
    bool need_ack;
} pan_tilt_reset_param_table[] = {
    {PAN_TILT_LOCK_STATUS_NONE, false, false},
    {PAN_TILT_LOCK_STATUS_UNLOCKED, false, false},
    {PAN_TILT_LOCK_STATUS_LOCKED, false, false},
    {PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING, true, false},
    {PAN_TILT_LOCK_STATUS_NONE, false, true},
    {PAN_TILT_LOCK_STATUS_UNLOCKED, false, true},
    {PAN_TILT_LOCK_STATUS_LOCKED, false, true},
    {PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING, true, true},
};
} // namespace

// ○テストリスト
// + 各メソッドは適切なViscaServerMessageIfクラスのメソッドを呼び出すこと
//  + 引数に応じた適切なPanTiltDirectionをmovePanTiltDirectionで送信すること
//  + 取得したZoom値、SlowMode、DigitalZoom On/Off、zoom倍率 X20/X12に応じた適切なPanTiltSpeedを
//    movePanTiltDirectionで送信すること
//  + 取得したZoom値が異常(>0x6000)の場合は従来のPT速度初期値(0x0C)を
//    movePanTiltDirectionMoveRequestで送信すること
//  + 引数に応じた適切なZoomSpeedとZoomDirectionをmoveZoomDirectionで送信すること
//  + 引数に応じた適切なFocusModeをsendFocusModeで送信すること
//  + 引数に応じた適切なFocusDirectionをsendFocusMoveで送信すること
//  + 引数にMessageQueueが指定された場合も、適切なViscaServerMessageIfクラスのメソッドを呼び出すこと
// + 各メソッドは適切なViscaServerPtzfIfクラスのメソッドを呼び出すこと
//  + movePTZPanTiltRelative
//  + 引数に応じた適切なpositionをmovePanTiltRelativeで送信すること
//  + 不適切なパラメータはERRORCODE_EXECを返すこと
//  + movePTZPanTiltRelative
//  + 引数に応じた適切なpositionをmoveZoomAbsoluteで送信すること
//  + 不適切なパラメータはERRORCODE_EXECを返すこと
// + PanTiltResetInPTLockStatusAfterBooting
//   + PanTiltLockControlStatusが通電アンロック状態の場合はPowerOnSequenceStatusをtrueに設定した上、 
//     initializePanTiltInUnlockedAfterBoot()を、それ以外の場合はresetPanTilt()を呼び出す事

class PtzfControllerTest : public ::testing::Test
{
protected:
    PtzfControllerTest()
        : capability_if_mock_holder_(),
          capability_if_mock_(capability_if_mock_holder_.getMock()),
          visca_ptzf_if_mock_holder_(),
          visca_ptzf_if_mock_(visca_ptzf_if_mock_holder_.getMock()),
          ptzf_status_if_mock_holder_(),
          ptzf_status_if_mock_(ptzf_status_if_mock_holder_.getMock()),
          ptzf_config_if_mock_holder_(),
          ptzf_config_if_mock_(ptzf_config_if_mock_holder_.getMock()),
          ptzf_biz_message_if_mock_holder_(),
          ptzf_biz_message_if_mock_(ptzf_biz_message_if_mock_holder_.getMock()),
          ptz_updater_mock_holder_(),
          ptz_updater_mock_(ptz_updater_mock_holder_.getMock()),
          ptzf_pan_tilt_if_mock_holder_(),
          ptzf_pan_tilt_if_mock_(ptzf_pan_tilt_if_mock_holder_.getMock()),
          ptzf_zoom_if_mock_holder_(),
          ptzf_zoom_if_mock_(ptzf_zoom_if_mock_holder_.getMock()),
          ptzf_focus_if_mock_holder_(),
          ptzf_focus_if_mock_(ptzf_focus_if_mock_holder_.getMock()),
          ptzf_status_infra_if_mock_holder_(),
          ptzf_status_infra_if_mock_(ptzf_status_infra_if_mock_holder_.getMock()),
          ptzf_config_infra_if_mock_holder_(),
          ptzf_config_infra_if_mock_(ptzf_config_infra_if_mock_holder_.getMock()),
          ptzf_initialize_if_mock_holder_(),
          ptzf_initialize_if_mock_(ptzf_initialize_if_mock_holder_.getMock())
    {}

    virtual void SetUp()
    {}

    virtual void TearDown()
    {}

    void waitQueueEmpty(common::MessageQueue& mq)
    {
        common::MessageQueueAttribute attr;
        for (;;) {
            mq.getAttribute(attr);
            if (0 < attr.message_size_current) {
                common::Task::msleep(U32_T(1));
            }
            else {
                break;
            }
        }
    }

protected:
    MockHolderObject<infra::CapabilityInfraIfMock> capability_if_mock_holder_;
    infra::CapabilityInfraIfMock& capability_if_mock_;
    MockHolderObject<visca::ViscaServerPtzfIfMock> visca_ptzf_if_mock_holder_;
    visca::ViscaServerPtzfIfMock& visca_ptzf_if_mock_;
    MockHolderObject<PtzfStatusIfMock> ptzf_status_if_mock_holder_;
    PtzfStatusIfMock& ptzf_status_if_mock_;
    MockHolderObject<PtzfConfigIfMock> ptzf_config_if_mock_holder_;
    PtzfConfigIfMock& ptzf_config_if_mock_;
    MockHolderObject<ptzf::PtzfBizMessageIfMock> ptzf_biz_message_if_mock_holder_;
    ptzf::PtzfBizMessageIfMock& ptzf_biz_message_if_mock_;
    MockHolderObject<PtzUpdaterImpl> ptz_updater_mock_holder_;
    PtzUpdaterImpl& ptz_updater_mock_;
    MockHolderObject<infra::PtzfPanTiltInfraIfMock> ptzf_pan_tilt_if_mock_holder_;
    infra::PtzfPanTiltInfraIfMock& ptzf_pan_tilt_if_mock_;
    MockHolderObject<infra::PtzfZoomInfraIfMock> ptzf_zoom_if_mock_holder_;
    infra::PtzfZoomInfraIfMock& ptzf_zoom_if_mock_;
    MockHolderObject<infra::PtzfFocusInfraIfMock> ptzf_focus_if_mock_holder_;
    infra::PtzfFocusInfraIfMock& ptzf_focus_if_mock_;
    MockHolderObject<infra::PtzfStatusInfraIfMock> ptzf_status_infra_if_mock_holder_;
    infra::PtzfStatusInfraIfMock& ptzf_status_infra_if_mock_;
    MockHolderObject<infra::PtzfConfigInfraIfMock> ptzf_config_infra_if_mock_holder_;
    infra::PtzfConfigInfraIfMock& ptzf_config_infra_if_mock_;
    MockHolderObject<infra::PtzfInitializeInfraIfMock> ptzf_initialize_if_mock_holder_;
    infra::PtzfInitializeInfraIfMock& ptzf_initialize_if_mock_;

};

MATCHER_P2(EqBizPanTiltDirectionMoveRequest, left_right, up_down, "")
{
    const visca::PanTiltDirectionMoveRequest& payload = arg();
    if (left_right == payload.left_right && up_down == payload.up_down) {
        return true;
    }
    return false;
}

TEST_F(PtzfControllerTest, PanTiltDirectionMoveSircsWithRightDirection)
{
    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_)).WillOnce(DoAll(SetArgReferee<0>(true), Return(true)));

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .WillOnce(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    PtzfController controller_;
    ARRAY_FOREACH (pan_tilt_direction_table, i) {
        if (PAN_TILT_DIRECTION_STOP == pan_tilt_direction_table[i]) {
            EXPECT_CALL(ptzf_pan_tilt_if_mock_, stopPanTiltDirection()).Times(1).WillOnce(Return());
        }
        else {
            EXPECT_CALL(ptzf_pan_tilt_if_mock_, moveDynamicPanTiltDirection(pan_tilt_direction_table[i], _, _, _))
                .Times(1)
                .WillOnce(Return());
        }
        controller_.moveSircsPanTilt(pan_tilt_direction_table[i]);
    }
}

TEST_F(PtzfControllerTest, PanTiltDirectionMoveWithRightDirection)
{
    common::MessageQueue reply;
    u8_t pan_speed = U8_T(24);
    u8_t tilt_speed = U8_T(23);

    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_)).WillOnce(DoAll(SetArgReferee<0>(true), Return(true)));

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .WillOnce(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X12_0), Return(true)));

    PtzfController controller_;
    ARRAY_FOREACH (pan_tilt_direction_table, i) {
        if (PAN_TILT_DIRECTION_STOP == pan_tilt_direction_table[i]) {
            EXPECT_CALL(ptzf_pan_tilt_if_mock_, stopPanTiltDirection()).Times(1).WillOnce(Return());
        }
        else {
            EXPECT_CALL(ptzf_pan_tilt_if_mock_,
                        moveDynamicPanTiltDirection(pan_tilt_direction_table[i], pan_speed, tilt_speed, _))
                .Times(1)
                .WillOnce(Return());
        }

        controller_.movePanTilt(pan_tilt_direction_table[i], pan_speed, tilt_speed);

        if (PAN_TILT_DIRECTION_STOP == pan_tilt_direction_table[i]) {
            EXPECT_CALL(ptzf_pan_tilt_if_mock_, stopPanTiltDirection(_, _)).Times(1).WillOnce(Return());
        }
        else {
            EXPECT_CALL(ptzf_pan_tilt_if_mock_,
                        moveDynamicPanTiltDirection(pan_tilt_direction_table[i], pan_speed, tilt_speed, _, _, _))
                .Times(1)
                .WillOnce(Return());
        }

        controller_.movePanTilt(pan_tilt_direction_table[i], pan_speed, tilt_speed, &reply, U32_T(0x123));
    }
}

TEST_F(PtzfControllerTest, ZoomMove)
{
    common::MessageQueue reply;

    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_)).WillOnce(DoAll(SetArgReferee<0>(true), Return(true)));

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .WillOnce(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    PtzfController controller_;
    ARRAY_FOREACH (zoom_move_sircs_param_table, i) {
        EXPECT_CALL(
            ptzf_zoom_if_mock_,
            moveZoomDirection(zoom_move_sircs_param_table[i].move, true, zoom_move_sircs_param_table[i].visca_speed))
            .Times(1)
            .WillOnce(Return());

        controller_.moveSircsZoom(zoom_move_sircs_param_table[i].speed, zoom_move_sircs_param_table[i].direction);
    }
    ARRAY_FOREACH (zoom_move_param_table, i) {
        EXPECT_CALL(ptzf_zoom_if_mock_,
                    moveZoomDirection(zoom_move_param_table[i].move, true, zoom_move_param_table[i].visca_speed, _, _))
            .Times(1)
            .WillOnce(Return());

        controller_.moveZoom(zoom_move_param_table[i].speed, zoom_move_param_table[i].direction, &reply);
    }

    EXPECT_CALL(ptzf_zoom_if_mock_, stopZoomDirection(_, _)).Times(1).WillOnce(Return());
    controller_.moveZoom(U8_T(1), ZOOM_DIRECTION_STOP, &reply);

    EXPECT_CALL(ptzf_zoom_if_mock_, stopZoomDirection()).Times(1).WillOnce(Return());
    controller_.moveSircsZoom(U8_T(1), ZOOM_DIRECTION_STOP);
}

TEST_F(PtzfControllerTest, FocusMode)
{
    common::MessageQueue reply;
    u32_t seq_id = U32_T(0x123);

    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_)).WillOnce(DoAll(SetArgReferee<0>(true), Return(true)));

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .WillOnce(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    PtzfController controller_;
    ARRAY_FOREACH (focus_mode_param_table, i) {
        EXPECT_CALL(ptzf_focus_if_mock_, setFocusMode(focus_mode_param_table[i])).Times(1).WillOnce(Return());

        controller_.setFocusMode(focus_mode_param_table[i]);

        EXPECT_CALL(ptzf_focus_if_mock_, setFocusMode(focus_mode_param_table[i], _, _)).Times(1).WillOnce(Return());

        controller_.setFocusMode(focus_mode_param_table[i], &reply, seq_id);
    }
}

TEST_F(PtzfControllerTest, FocusMove)
{
    common::MessageQueue reply;
    u32_t seq_id = U32_T(0x123);

    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_)).WillOnce(DoAll(SetArgReferee<0>(true), Return(true)));

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .WillOnce(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    PtzfController controller_;
    u8_t speed = U8_T(8);
    ARRAY_FOREACH (focus_direction_param_table, i) {
        if (FOCUS_DIRECTION_STOP == focus_direction_param_table[i]) {
            EXPECT_CALL(ptzf_focus_if_mock_, stopFocusDirection()).Times(1).WillOnce(Return());

            controller_.moveFocus(focus_direction_param_table[i], speed);

            EXPECT_CALL(ptzf_focus_if_mock_, stopFocusDirection(_, _)).Times(1).WillOnce(Return());

            controller_.moveFocus(focus_direction_param_table[i], speed, &reply, seq_id);
        }
        else {
            EXPECT_CALL(ptzf_focus_if_mock_, moveFocusDirection(focus_direction_param_table[i], true, speed))
                .Times(1)
                .WillOnce(Return());

            controller_.moveFocus(focus_direction_param_table[i], speed);

            EXPECT_CALL(ptzf_focus_if_mock_, moveFocusDirection(focus_direction_param_table[i], true, speed, _, _))
                .Times(1)
                .WillOnce(Return());

            controller_.moveFocus(focus_direction_param_table[i], speed, &reply, seq_id);
        }
    }
}

TEST_F(PtzfControllerTest, HomePosition)
{
    EXPECT_CALL(ptzf_pan_tilt_if_mock_, movePanTiltHomePosition()).Times(1).WillOnce(Return());

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .WillOnce(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_)).WillOnce(DoAll(SetArgReferee<0>(true), Return(true)));

    PtzfController controller_;
    controller_.moveToHomePosition();

    EXPECT_CALL(ptzf_pan_tilt_if_mock_, movePanTiltHomePosition(_, _)).Times(1).WillOnce(Return());

    common::MessageQueue reply;
    controller_.moveToHomePosition(&reply);
}

TEST_F(PtzfControllerTest, PanTiltResetInPTLockStatusAfterBooting)
{

    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_))
        .WillOnce(DoAll(SetArgReferee<0>(true), Return(true)));
    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .WillOnce(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    PtzfController controller_;
    common::MessageQueue reply;

    ARRAY_FOREACH (pan_tilt_reset_param_table, i) {
        EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
            .Times(1).WillOnce(SetArgReferee<0>(pan_tilt_reset_param_table[i].status));

        if (pan_tilt_reset_param_table[i].initialize_pan_tilt) {
            EXPECT_CALL(ptzf_initialize_if_mock_, setPowerOnSequenceStatus(true,_))
                .Times(1)
                .WillRepeatedly(testing::Invoke([](testing::Unused, const common::MessageQueueName& reply_name) {
                    common::MessageQueue local_mq(reply_name.name);
                    ptzf::message::PtzfExecComp result;
                    local_mq.post(result);
                    return true;
                }));

            EXPECT_CALL(ptzf_pan_tilt_if_mock_, resetPanTilt(_, pan_tilt_reset_param_table[i].need_ack, _)).Times(0);
            EXPECT_CALL(ptzf_initialize_if_mock_,
                       initializePanTiltInUnlockedAfterBoot(_, pan_tilt_reset_param_table[i].need_ack))
                .Times(1).WillOnce(Return());
        }
        else {
            EXPECT_CALL(ptzf_pan_tilt_if_mock_, resetPanTilt(_, pan_tilt_reset_param_table[i].need_ack, _))
                .Times(1).WillOnce(Return());
            EXPECT_CALL(ptzf_initialize_if_mock_, initializePanTiltInUnlockedAfterBoot(_)).Times(0);
        }

        controller_.resetPanTiltPosition(reply, pan_tilt_reset_param_table[i].need_ack, INVALID_SEQ_ID);
    }
}

TEST_F(PtzfControllerTest, moveZoomRelative)
{
    common::MessageQueue thread_mq_(PtzfControllerMQ::getUipcName());

    common::MessageQueueName reply_name;
    u32_t seq_id = U32_T(0x123);

    EXPECT_CALL(ptz_updater_mock_, getZoomPosition()).Times(1).WillOnce(Return(uint_t(4096)));
    EXPECT_CALL(ptzf_biz_message_if_mock_, getDZoomMode(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptzf_zoom_if_mock_, moveZoomAbsolute(_, _, _, _)).Times(1).WillOnce(Return());

    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(true), Return(true)));

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    PtzfController controller_;
    common::ThreadObject<PtzfControllerMessageHandler> message_handler_;
    EXPECT_EQ(ERRORCODE_SUCCESS, controller_.moveZoomRelative(S32_T(4096), reply_name, seq_id));

    DZoomModeInquiryResult comp_req;
    comp_req.d_zoom = DZOOM_FULL;
    thread_mq_.post(comp_req);
    ZoomAbsolutePositionInquiryResult zoom_pos;
    zoom_pos.position = S32_T(4096);
    thread_mq_.post(zoom_pos);

    waitQueueEmpty(thread_mq_);
}

TEST_F(PtzfControllerTest, moveZoomRelativeError)
{
    common::MessageQueue thread_mq_(PtzfControllerMQ::getUipcName());

    common::MessageQueueName reply_name;
    u32_t seq_id = U32_T(0x123);

    EXPECT_CALL(ptz_updater_mock_, getZoomPosition()).Times(1).WillOnce(Return(uint_t(4096)));

    EXPECT_CALL(ptzf_biz_message_if_mock_, getDZoomMode(_)).Times(1).WillOnce(Return(false));

    EXPECT_CALL(ptzf_zoom_if_mock_, moveZoomAbsolute(_, _, _, _)).Times(1).WillOnce(Return());

    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(true), Return(true)));

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    PtzfController controller_;
    common::ThreadObject<PtzfControllerMessageHandler> message_handler_;
    EXPECT_EQ(ERRORCODE_EXEC, controller_.moveZoomRelative(S32_T(4096), reply_name, seq_id));

    DZoomModeInquiryResult comp_req;
    comp_req.d_zoom = DZOOM_FULL;
    thread_mq_.post(comp_req);

    waitQueueEmpty(thread_mq_);
}

TEST_F(PtzfControllerTest, moveFocusRelative)
{
    common::MessageQueue thread_mq_(PtzfControllerMQ::getUipcName());

    common::MessageQueueName reply;
    u32_t seq_id = U32_T(0x123);

    EXPECT_CALL(ptzf_biz_message_if_mock_, getFocusAbsolutePosition(_))
        .Times(2)
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    EXPECT_CALL(ptzf_focus_if_mock_, moveFocusAbsolute(_, _, _, _)).Times(2).WillOnce(Return()).WillOnce(Return());

    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(true), Return(true)));

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    PtzfController controller_;
    common::ThreadObject<PtzfControllerMessageHandler> message_handler_;
    EXPECT_EQ(ERRORCODE_SUCCESS, controller_.moveFocusRelative(S32_T(-16384), reply, seq_id));
    EXPECT_EQ(ERRORCODE_SUCCESS, controller_.moveFocusRelative(S32_T(45056), reply, seq_id));

    FocusAbsolutePositionInquiryResult focus_pos;
    focus_pos.position = S32_T(-16384);
    thread_mq_.post(focus_pos);
    FocusAbsolutePositionInquiryResult focus_pos2;
    focus_pos2.position = S32_T(45056);
    thread_mq_.post(focus_pos2);

    waitQueueEmpty(thread_mq_);
}

TEST_F(PtzfControllerTest, moveFocusRelativeError)
{
    common::MessageQueueName reply;
    u32_t seq_id = U32_T(0x123);

    EXPECT_CALL(ptzf_biz_message_if_mock_, getFocusAbsolutePosition(_)).Times(1).WillRepeatedly(Return(false));

    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_)).WillOnce(DoAll(SetArgReferee<0>(true), Return(true)));

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .WillOnce(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    PtzfController controller_;
    EXPECT_EQ(ERRORCODE_EXEC, controller_.moveFocusRelative(S32_T(4096), reply, seq_id));
}

TEST_F(PtzfControllerTest, movePTZPanTiltRelative)
{
    common::MessageQueueName reply_name;

    u32_t pan_position = U32_T(0);
    u32_t tilt_position = U32_T(0);

    EXPECT_CALL(ptzf_status_if_mock_, roundPTZPanRelativeMoveRange(_)).WillRepeatedly(ReturnArg<0>());
    EXPECT_CALL(ptzf_status_if_mock_, roundPTZTiltRelativeMoveRange(_)).WillRepeatedly(ReturnArg<0>());
    EXPECT_CALL(ptzf_status_if_mock_, panSinDataToViscaData(_)).WillRepeatedly(ReturnArg<0>());
    EXPECT_CALL(ptzf_status_if_mock_, tiltSinDataToViscaData(_)).WillRepeatedly(ReturnArg<0>());
    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_)).WillOnce(DoAll(SetArgReferee<0>(true), Return(true)));

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .WillOnce(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    PtzfController controller_;
    ARRAY_FOREACH (ptz_pan_tilt_relative_param_table, i) {
        EXPECT_CALL(ptz_updater_mock_, getZoomPosition())
            .Times(1)
            .WillOnce(Return(static_cast<uint_t>(ptz_pan_tilt_relative_param_table[i].zoom_position)));
        EXPECT_CALL(ptzf_pan_tilt_if_mock_, movePanTiltRelative(_, _, _, _, _, _))
            .WillOnce(DoAll(SaveArg<2>(&pan_position), SaveArg<3>(&tilt_position), Return()));

        EXPECT_EQ(ERRORCODE_SUCCESS,
                  controller_.movePTZPanTiltRelative(ptz_pan_tilt_relative_param_table[i].direction,
                                                     ptz_pan_tilt_relative_param_table[i].amount,
                                                     reply_name,
                                                     U32_T(0x123)));

        EXPECT_EQ(ptz_pan_tilt_relative_param_table[i].pan_position, pan_position);
        EXPECT_EQ(ptz_pan_tilt_relative_param_table[i].tilt_position, tilt_position);
    }
}

TEST_F(PtzfControllerTest, movePTZPanTiltRelativeError)
{
    common::MessageQueueName reply_name;

    EXPECT_CALL(ptz_updater_mock_, getZoomPosition()).Times(1).WillOnce(Return(uint_t(0x00)));

    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_)).WillOnce(DoAll(SetArgReferee<0>(true), Return(true)));

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .WillOnce(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    PtzfController controller_;
    EXPECT_EQ(
        ERRORCODE_EXEC,
        controller_.movePTZPanTiltRelative(PAN_TILT_DIRECTION_STOP, PTZ_RELATIVE_AMOUNT_1, reply_name, U32_T(0x123)));
}

TEST_F(PtzfControllerTest, movePTZZoomRelative)
{
    common::MessageQueue thread_mq_(PtzfControllerMQ::getUipcName());

    common::MessageQueueName reply_name;
    u32_t seq_id = U32_T(0x123);
    u16_t ab_position = U16_T(0);

    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(true), Return(true)));

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    PtzfController controller_;
    common::ThreadObject<PtzfControllerMessageHandler> message_handler_;
    ARRAY_FOREACH (zoom_relative_move_tele_table, i) {
        EXPECT_CALL(ptz_updater_mock_, getZoomPosition()).Times(1).WillOnce(Return(uint_t(0x0000)));
        EXPECT_CALL(ptzf_biz_message_if_mock_, getDZoomMode(_)).Times(1).WillOnce(Return(true));

        if (zoom_relative_move_tele_table[i].zoom_mode == DZOOM_CLEAR_IMAGE) {
            EXPECT_CALL(ptzf_status_if_mock_, getMaxZoomPosition()).Times(1).WillOnce(Return(U16_T(0x6000)));
        }
        EXPECT_CALL(ptzf_zoom_if_mock_, moveZoomAbsolute(_, _, _, _))
            .Times(1)
            .WillOnce(DoAll(SaveArg<0>(&ab_position), Return()));

        EXPECT_EQ(ERRORCODE_SUCCESS,
                  controller_.movePTZZoomRelative(
                      ZOOM_DIRECTION_TELE, zoom_relative_move_tele_table[i].amount, reply_name, seq_id));

        DZoomModeInquiryResult comp_req;
        comp_req.d_zoom = zoom_relative_move_tele_table[i].zoom_mode;
        thread_mq_.post(comp_req);
        ZoomAbsolutePositionInquiryResult comp_req2;
        comp_req2.position = zoom_relative_move_tele_table[i].position;
        thread_mq_.post(comp_req2);
        common::Task::msleep(100);

        EXPECT_EQ(zoom_relative_move_tele_table[i].position, ab_position);

        waitQueueEmpty(thread_mq_);
    }

    ARRAY_FOREACH (zoom_relative_move_wide_table, j) {
        EXPECT_CALL(ptz_updater_mock_, getZoomPosition()).Times(1).WillOnce(Return(uint_t(0x7AC0)));
        EXPECT_CALL(ptzf_biz_message_if_mock_, getDZoomMode(_)).Times(1).WillOnce(Return(true));

        if (zoom_relative_move_wide_table[j].zoom_mode == DZOOM_CLEAR_IMAGE) {
            EXPECT_CALL(ptzf_status_if_mock_, getMaxZoomPosition()).Times(1).WillOnce(Return(U16_T(0x6000)));
        }
        EXPECT_CALL(ptzf_zoom_if_mock_, moveZoomAbsolute(_, _, _, _))
            .Times(1)
            .WillOnce(DoAll(SaveArg<0>(&ab_position), Return()));

        EXPECT_EQ(ERRORCODE_SUCCESS,
                  controller_.movePTZZoomRelative(
                      ZOOM_DIRECTION_WIDE, zoom_relative_move_wide_table[j].amount, reply_name, seq_id));

        DZoomModeInquiryResult comp_req;
        comp_req.d_zoom = zoom_relative_move_wide_table[j].zoom_mode;
        thread_mq_.post(comp_req);
        ZoomAbsolutePositionInquiryResult comp_req2;
        comp_req2.position = zoom_relative_move_wide_table[j].position;
        thread_mq_.post(comp_req2);
        common::Task::msleep(100);

        EXPECT_EQ(zoom_relative_move_wide_table[j].position, ab_position);

        waitQueueEmpty(thread_mq_);
    }
}

TEST_F(PtzfControllerTest, movePTZZoomRelativeError)
{
    common::MessageQueue thread_mq_(PtzfControllerMQ::getUipcName());

    common::MessageQueueName reply_name;
    u32_t seq_id = U32_T(0x123);

    EXPECT_CALL(capability_if_mock_, getSupportDigitalZoom(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(true), Return(true)));

    EXPECT_CALL(capability_if_mock_, getMaxOpticalZoomRatio(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(infra::ZOOM_RATIO_X20_0), Return(true)));

    PtzfController controller_;
    common::ThreadObject<PtzfControllerMessageHandler> message_handler_;

    EXPECT_EQ(ERRORCODE_EXEC,
              controller_.movePTZZoomRelative(ZOOM_DIRECTION_STOP, PTZ_RELATIVE_AMOUNT_1, reply_name, seq_id));
}

} // namespace ptzf
