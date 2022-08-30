/*
 * ptzf_status_test.cpp
 *
 * Copyright 2016,2018 Sony Imaging Products & Solutions Inc.
 */

#include "types.h"

#include "ptzf/ptzf_status_if.h"
#include "ptzf_status.h"
#include "ptzf_backup_infra_if_mock.h"
#include "common_message_queue.h"

#include "visca/visca_config_if.h"
#include "visca/dboutputs/enum.h"
#include "visca/dboutputs/config_camera_service.h"
#include "preset/preset_manager_message.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "common_gmock_util.h"
#include "application/error_notifier/error_notifier_message_if.h"
#include "application/error_notifier/error_notifier_message_if_mock.h"

#include "bizglobal_modelname_creater.h"
#include "ptzf_capability_infra_if.h"
#include "ptzf_status_infra_if_mock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Eq;
using ::testing::DoAll;
using ::testing::SetArgReferee;

namespace config {

struct ConfigReadyNotification
{};

} // namespace config

namespace {

static const struct PanSpeedTable
{
    bool slow_mode;
    u8_t pan_speed;
    bool result;
} pan_speed_table[] = { { true, U8_T(0x01), true },  { true, U8_T(0x7F), true },  { true, U8_T(0x18), true },
                        { true, U8_T(0x0), false },  { true, U8_T(0x80), false }, { false, U8_T(0x01), true },
                        { false, U8_T(0x18), true }, { false, U8_T(0x0), false }, { false, U8_T(0x19), false },
                        { false, U8_T(0x7F), false } };

static const struct TiltSpeedTable
{
    bool slow_mode;
    u8_t tilt_speed;
    bool result;
} tilt_speed_table[] = { { true, U8_T(0x01), true },  { true, U8_T(0x7F), true },  { true, U8_T(0x17), true },
                         { true, U8_T(0x0), false },  { true, U8_T(0x80), false }, { false, U8_T(0x01), true },
                         { false, U8_T(0x17), true }, { false, U8_T(0x0), false }, { false, U8_T(0x18), true },
                         { false, U8_T(0x7F), false } };

static const struct RoundTiltSpeedTable
{
    bool slow_mode;
    u8_t tilt_speed;
    u8_t result;
} round_tilt_speed_table[] = { { false, U8_T(0x18), U8_T(0x17) },
                               { false, U8_T(0x01), U8_T(0x01) },
                               { true, U8_T(0x18), U8_T(0x18) },
                               { true, U8_T(0x01), U8_T(0x01) } };

static const struct RoundPanSpeedTable
{
    bool slow_mode;
    u8_t pan_speed;
    u8_t result;
} round_pan_speed_table[] = { { false, U8_T(0x18), U8_T(0x18) },
                              { false, U8_T(0x01), U8_T(0x01) },
                              { true, U8_T(0x18), U8_T(0x18) },
                              { true, U8_T(0x01), U8_T(0x01) } };

static const struct PanTiltSpeedWithSpeedStepTable
{
    ptzf::PanTiltSpeedStep speed_step;
    bool slow_mode;
    u8_t pt_speed;
    bool result;
} pan_tilt_speed_with_ss_table[] = { { ptzf::PAN_TILT_SPEED_STEP_NORMAL, false, U8_T(0), false },
                                     { ptzf::PAN_TILT_SPEED_STEP_NORMAL, false, U8_T(1), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_NORMAL, false, U8_T(24), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_NORMAL, false, U8_T(25), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_NORMAL, false, U8_T(50), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_NORMAL, false, U8_T(51), false },
                                     { ptzf::PAN_TILT_SPEED_STEP_NORMAL, true, U8_T(0), false },
                                     { ptzf::PAN_TILT_SPEED_STEP_NORMAL, true, U8_T(1), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_NORMAL, true, U8_T(24), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_NORMAL, true, U8_T(25), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_NORMAL, true, U8_T(50), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_NORMAL, true, U8_T(51), false },
                                     { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, false, U8_T(0), false },
                                     { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, false, U8_T(1), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, false, U8_T(24), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, false, U8_T(25), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, false, U8_T(50), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, false, U8_T(51), false },
                                     { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, true, U8_T(0), false },
                                     { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, true, U8_T(1), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, true, U8_T(24), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, true, U8_T(25), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, true, U8_T(50), true },
                                     { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, true, U8_T(51), false } };

static const struct RoundPanTiltSpeedWithSpeedStepTable
{
    ptzf::PanTiltSpeedStep speed_step;
    bool slow_mode;
    u8_t pt_speed;
    u8_t result;
} round_pan_tilt_speed_with_ss_table[] = {
    { ptzf::PAN_TILT_SPEED_STEP_NORMAL, false, 0, 0 },     { ptzf::PAN_TILT_SPEED_STEP_NORMAL, false, 1, 1 },
    { ptzf::PAN_TILT_SPEED_STEP_NORMAL, false, 24, 24 },   { ptzf::PAN_TILT_SPEED_STEP_NORMAL, false, 25, 24 },
    { ptzf::PAN_TILT_SPEED_STEP_NORMAL, false, 50, 24 },   { ptzf::PAN_TILT_SPEED_STEP_NORMAL, false, 51, 51 },
    { ptzf::PAN_TILT_SPEED_STEP_NORMAL, true, 0, 0 },      { ptzf::PAN_TILT_SPEED_STEP_NORMAL, true, 1, 1 },
    { ptzf::PAN_TILT_SPEED_STEP_NORMAL, true, 24, 24 },    { ptzf::PAN_TILT_SPEED_STEP_NORMAL, true, 25, 24 },
    { ptzf::PAN_TILT_SPEED_STEP_NORMAL, true, 50, 24 },    { ptzf::PAN_TILT_SPEED_STEP_NORMAL, true, 51, 51 },
    { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, false, 0, 0 },   { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, false, 1, 1 },
    { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, false, 24, 24 }, { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, false, 25, 25 },
    { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, false, 50, 50 }, { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, false, 51, 51 },
    { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, true, 0, 0 },    { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, true, 1, 1 },
    { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, true, 24, 24 },  { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, true, 25, 25 },
    { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, true, 50, 50 },  { ptzf::PAN_TILT_SPEED_STEP_EXTENDED, true, 51, 51 }
};

} // namespace

namespace ptzf {

// ○テストリスト
// + PtzfStatus::setImageFlipStatusOnBoot()で設定したPictureFlipModeが
//   PtzfStatusIf::getPanTiltImageFlipMode()で取得できること
// + PtzfStatus::setImageFlipStatus()で設定したPictureFlipModeは
//   PtzfStatusIf::getPanTiltImageFlipMode()で取得できないこと
// + PtzfStatus::setSettingPositionStatusOnBoot()で設定したSettingPositionに対応する値を
//   PtzfStatusIf::getPanTiltImageFlipMode()で取得できること
// + PtzfStatus::SetSettingPositionStatus()で設定したSettingPositionに対応する値を
//   PtzfStatusIf::getPanTiltImageFlipMode()で取得できないこと
// + PtzfStatus::setPanTiltStatus()で設定したStatusがPtzfStatusIf::getPanTiltStatus()で取得できること
// + PtzfStatus::setRampCurve()で設定したStatusがPtzfStatusIf::getPanTiltRampCurve()で取得できること
// + PtzfStatus::setSlowMode()で設定したStatusがPtzfStatusIf::getPanTiltSlowMode()で取得できること
// + PtzfStatus::setPanReverse()で設定したStatusがPtzfStatusIf::getPanReverse()で取得できること
// + PtzfStatus::setTiltReverse()で設定したStatusがPtzfStatusIf::getTiltReverse()で取得できること
// + PtzfStatus::setPanTiltPosition()で設定したStatusがPtzfStatusIf::getPanTiltPosition()で取得できること
// + PtzfStatus::setStatus()で設定したPanTiltステータスが、
//   Pan/Tilt位置エラーの場合に異常通知ジュールにWarning通知を行うこと
//     + PtzfStatus::setStatus()で検知したPan/Tilt位置エラーが解消された場合は、
//       異常通知モジュールにWarning通知解除を行うこと
// + PtzfStatus::setPanTiltLimitDownLeft()で設定したStatusがPtzfStatusIf::getPanLimitLeft(),
//   PtzfStatusIf::getTiltLimitDown()で取得できること
// + PtzfStatus::setPanTiltLimitUpRight()で設定したStatusがPtzfStatusIf::getPanLimitRight(),
//   PtzfStatusIf::getTiltLimitUp()で取得できること
// + PtzfStatus::setPanTiltPosition(), PtzfStatus::setPanTiltLimitUpRight(), PtzfStatus::setPanTiltLimitDownLeft()で
//   設定したStatusによりPtzfStatusIf::isPanTilitPositionInPanTiltLimitArea()でPanTilt位置の有効性が取得できる事
// + PtzfStatus::setIRCorrection()で設定した値がPtzfStatusIf::getIRCorrection()で取得できること
// + PtzfStatusIf::sValidPanLimitLeft(), PtzfStatusIf::sValidPanLimitRight(), PtzfStatusIf::isValidTiltLimitUp(),
//   PtzfStatusIf::isValidTiltLimitDown()で有効値、無効値の判定ができる事
// + PtzfStatus::setPanTiltLimitConfigurationStatus()で設定したStatusが
//   PtzfStatusIf::isConfiguringPanTiltLimit()で取得できること
// + PtzfStatus::setPTZMode()
// + PtzfStatus::setPTZPanTiltMove()
// + PtzfStatus::setPTZZoomMove()
// + isValidSinPanAbsolute
// + isValidSinTiltAbsolute
// + isValidSinPanRelative
// + isValidSinTiltRelative
// + PtzfStatusIf::getPanTiltMaxSpeed()
// + PtzfStatusIf::getPanMovementRange()
// + PtzfStatusIf::getTiltMovementRange()
// + PtzfStatusIf::getOpticalZoomMaxMagnification()
// + PtzfStatusIf::getZoomMovementRange()
// + PtzfStatusIf::getZoomMaxVelocity()
// + PtzfStatusIf::roundPTZPanRelativeMoveRange()
// + PtzfStatusIf::roundPTZTiltRelativeMoveRange()
// + PtzfStatusIf::isValidSinPanRelativeMoveRange()
// + PtzfStatusIf::isValidSinTiltRelativeMoveRange()
// + PtzfStatus::setPanLimitMode()で設定したStatusがPtzfStatusIf::getPanLimitMode()で取得できること
// + PtzfStatus::setTiltLimitMode()で設定したStatusがPtzfStatusIf::getTiltLimitMode()で取得できること
// + PtzfStatusIf::isValidPanSpeed()
// + PtzfStatusIf::isValidTiltSpeed()
// + PtzfStatusIf::roundPanMaxSpeed()
// + PtzfStatusIf::roundTiltMaxSpeed()
// + PtzfStatusIf::getStandbyMode()
//   + PtzfBackupInfraIf::getStandbyMode() で取得した値を返すこと
// + PtzfStatusIf::setPanTiltUnlockErrorStatus()
//   + 通電後アンロックによるエラーの有効・無効がそれぞれ設定できること

class PtzfStatusTest : public ::testing::Test
{
protected:
    PtzfStatusTest()
        : biz_model_(),
          st_(),
          st_if_(),
          capability_infra_if_(),
          mock_holder_object(),
          error_notifier_mock_(mock_holder_object.getMock()),
          backup_infra_if_mock_holder_object(),
          backup_infra_if_mock_(backup_infra_if_mock_holder_object.getMock()),
          status_infra_if_mock_holder_object_(),
          status_infra_if_mock_(status_infra_if_mock_holder_object_.getMock())
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
    }

    bizglobal::BizGlobalModelNameCreater biz_model_;
    PtzfStatus st_;
    PtzfStatusIf st_if_;
    infra::CapabilityInfraIf capability_infra_if_;
    MockHolderObject<application::error_notifier::ErrorNotifierMessageIfMock> mock_holder_object;
    application::error_notifier::ErrorNotifierMessageIfMock& error_notifier_mock_;
    MockHolderObject<infra::PtzfBackupInfraIfMock> backup_infra_if_mock_holder_object;
    infra::PtzfBackupInfraIfMock& backup_infra_if_mock_;
    MockHolderObject<infra::PtzfStatusInfraIfMock> status_infra_if_mock_holder_object_;
    infra::PtzfStatusInfraIfMock& status_infra_if_mock_;
};

TEST_F(PtzfStatusTest, ImageFlipStatusOnBoot)
{
    visca::PictureFlipMode mode = visca::PICTURE_FLIP_MODE_ON;
    st_.setImageFlipStatusOnBoot(mode);
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_ON, st_if_.getPanTiltImageFlipMode());

    mode = visca::PICTURE_FLIP_MODE_OFF;
    st_.setImageFlipStatusOnBoot(mode);
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_OFF, st_if_.getPanTiltImageFlipMode());
}

TEST_F(PtzfStatusTest, ImageFlipStatus)
{
    visca::PictureFlipMode mode = visca::PICTURE_FLIP_MODE_ON;
    st_.setImageFlipStatusOnBoot(mode);
    st_.setImageFlipStatus(mode);
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_ON, st_if_.getPanTiltImageFlipMode());

    mode = visca::PICTURE_FLIP_MODE_OFF;
    st_.setImageFlipStatus(mode);
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_ON, st_if_.getPanTiltImageFlipMode());
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_OFF, st_if_.getPanTiltImageFlipModePreset());
}

TEST_F(PtzfStatusTest, SettingPositionStatusOnBoot)
{
    ptzf::SettingPosition mode = ptzf::SETTING_POSITION_CEILING;
    st_.setSettingPositionStatusOnBoot(mode);
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_ON, st_if_.getPanTiltImageFlipMode());

    mode = ptzf::SETTING_POSITION_DESKTOP;
    st_.setSettingPositionStatusOnBoot(mode);
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_OFF, st_if_.getPanTiltImageFlipMode());
}

TEST_F(PtzfStatusTest, SettingPositionStatus)
{
    ptzf::SettingPosition mode = ptzf::SETTING_POSITION_CEILING;
    st_.setSettingPositionStatusOnBoot(mode);
    st_.setSettingPositionStatus(mode);
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_ON, st_if_.getPanTiltImageFlipMode());
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_ON, st_if_.getPanTiltImageFlipModePreset());

    mode = ptzf::SETTING_POSITION_DESKTOP;
    st_.setSettingPositionStatus(mode);
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_ON, st_if_.getPanTiltImageFlipMode());
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_OFF, st_if_.getPanTiltImageFlipModePreset());
}

TEST_F(PtzfStatusTest, PanTiltStatus)
{
    u32_t status = U32_T(0);
    st_.setPanTiltStatus(status);
    EXPECT_EQ(U32_T(0), st_if_.getPanTiltStatus());

    status = U32_T(0x2800);
    st_.setPanTiltStatus(status);
    EXPECT_EQ(U32_T(0x2800), st_if_.getPanTiltStatus());
}

TEST_F(PtzfStatusTest, RampCurve)
{
    u8_t mode = U8_T(0);
    st_.setRampCurve(mode);
    EXPECT_EQ(U8_T(0), st_if_.getPanTiltRampCurve());

    mode = U8_T(1);
    st_.setRampCurve(mode);
    EXPECT_EQ(U8_T(1), st_if_.getPanTiltRampCurve());
}

TEST_F(PtzfStatusTest, PanTiltMotorPower)
{
    PanTiltMotorPower motor_power = PAN_TILT_MOTOR_POWER_NORMAL;
    st_.setPanTiltMotorPower(motor_power);
    EXPECT_EQ(motor_power, st_if_.getPanTiltMotorPower());

    motor_power = PAN_TILT_MOTOR_POWER_LOW;
    st_.setPanTiltMotorPower(motor_power);
    EXPECT_EQ(motor_power, st_if_.getPanTiltMotorPower());
}

TEST_F(PtzfStatusTest, SlowMode)
{
    bool enable = true;
    st_.setSlowMode(enable);
    EXPECT_TRUE(st_if_.getPanTiltSlowMode());

    enable = false;
    st_.setSlowMode(enable);
    EXPECT_FALSE(st_if_.getPanTiltSlowMode());
}

TEST_F(PtzfStatusTest, PanReverse)
{
    bool enable = true;
    st_.setPanReverse(enable);
    EXPECT_TRUE(st_if_.getPanReverse());

    enable = false;
    st_.setPanReverse(enable);
    EXPECT_FALSE(st_if_.getPanReverse());
}

TEST_F(PtzfStatusTest, TiltReverse)
{
    bool enable = true;
    st_.setTiltReverse(enable);
    EXPECT_TRUE(st_if_.getTiltReverse());

    enable = false;
    st_.setTiltReverse(enable);
    EXPECT_FALSE(st_if_.getTiltReverse());
}

TEST_F(PtzfStatusTest, PanTiltPosition)
{
    u32_t pan_expect = U32_T(0xABCDE), tilt_expect = U16_T(0xFEDC);

    st_.setPanTiltPosition(pan_expect, tilt_expect);

    u32_t pan_result = 0, tilt_result = 0;

    for (u32_t i = preset::DEFAULT_PRESET_ID; i <= preset::MAX_PRESET_ID; ++i) {
        pan_result = 0;
        tilt_result = 0;
        st_if_.getPanTiltPosition(i, pan_result, tilt_result);
        EXPECT_EQ(pan_expect, pan_result);
        EXPECT_EQ(tilt_expect, tilt_result);
    }
    pan_result = 0;
    tilt_result = 0;
    st_if_.getPanTiltPosition(pan_result, tilt_result);
    EXPECT_EQ(pan_expect, pan_result);
    EXPECT_EQ(tilt_expect, tilt_result);
}

TEST_F(PtzfStatusTest, needInitialize)
{
    // DiademBackupIfFake return always true
    EXPECT_TRUE(st_.needInitialize());
}

TEST_F(PtzfStatusTest, notifyPanTiltError)
{
    // 0x10:  Pan position detect error
    // 0x20:  Pan mecha is broken
    // 0x100: Tilt position detect error
    // 0x200: Tilt mecha is broken
    EXPECT_TRUE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2810));
    EXPECT_FALSE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2800));
    EXPECT_TRUE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2820));
    EXPECT_FALSE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2800));
    EXPECT_TRUE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2900));
    EXPECT_FALSE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2800));
    EXPECT_TRUE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2A00));
    EXPECT_FALSE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2800));
    EXPECT_TRUE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2830));
    EXPECT_FALSE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2800));
    EXPECT_TRUE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2B00));
    EXPECT_FALSE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2800));
    EXPECT_TRUE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2B30));
    EXPECT_FALSE(st_if_.isValidPosition());
    st_.setPanTiltStatus(U32_T(0x2800));
    EXPECT_TRUE(st_if_.isValidPosition());
}

TEST_F(PtzfStatusTest, isValidPanAbsolute)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xF6358)));
        EXPECT_TRUE(st_if_.isValidPanAbsolute(U32_T(0xF6359)));
        EXPECT_TRUE(st_if_.isValidPanAbsolute(U32_T(0xFABCD)));
        EXPECT_TRUE(st_if_.isValidPanAbsolute(U32_T(0xFFFFF)));
        EXPECT_TRUE(st_if_.isValidPanAbsolute(U32_T(0x00000)));
        EXPECT_TRUE(st_if_.isValidPanAbsolute(U32_T(0x01234)));
        EXPECT_TRUE(st_if_.isValidPanAbsolute(U32_T(0x09CA7)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x09CA8)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x10000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x20000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x30000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x40000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x50000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x60000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x70000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x80000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x90000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xA0000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xB0000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xC0000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xD0000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xE0000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xF0000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x10000000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x7FFFFFFF)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x80000000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xF0000000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xFFFFFFFF)));
    }
    else {
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xDDFF)));
        EXPECT_TRUE(st_if_.isValidPanAbsolute(U32_T(0xDE00)));
        EXPECT_TRUE(st_if_.isValidPanAbsolute(U32_T(0xFABC)));
        EXPECT_TRUE(st_if_.isValidPanAbsolute(U32_T(0xFFFF)));
        EXPECT_TRUE(st_if_.isValidPanAbsolute(U32_T(0x0000)));
        EXPECT_TRUE(st_if_.isValidPanAbsolute(U32_T(0x0123)));
        EXPECT_TRUE(st_if_.isValidPanAbsolute(U32_T(0x2200)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x2201)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x3000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x4000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x5000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x6000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x7000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x8000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x9000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xa000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xb000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xc000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xd000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x00010000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x10000000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x7FFFFFFF)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0x80000000)));
        EXPECT_FALSE(st_if_.isValidPanAbsolute(U32_T(0xFFFFFFFF)));
    }
}

TEST_F(PtzfStatusTest, isValidTiltAbsolute)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);

    visca::PictureFlipMode mode;
    mode = visca::PICTURE_FLIP_MODE_OFF;
    st_.setImageFlipStatusOnBoot(mode);
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_OFF, st_if_.getPanTiltImageFlipMode());
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xFE45A)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0xFE45B)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0xFFABC)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0xFFFFF)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0x00000)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0x01234)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0x0B3B0)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xB3B1)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x10000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x20000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x30000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x40000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x50000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x60000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x70000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x80000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x90000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xA0000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xB0000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xC0000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xD0000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xE0000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xF0000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x10000000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x7FFFFFFF)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x80000000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xF0000000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xFFFFFFFF)));
    }
    else {
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xFBFF)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0xFC00)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0xFFAB)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0xFFFF)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0x0000)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0x0123)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0x1200)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x1201)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x2000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x3000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x4000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x5000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x6000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x7000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x8000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x9000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xA000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xB000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xC000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xD000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xE000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xF000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x00010000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x10000000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x7FFFFFFF)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x80000000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xFFFFFFFF)));
    }

    mode = visca::PICTURE_FLIP_MODE_ON;
    st_.setImageFlipStatusOnBoot(mode);
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_ON, st_if_.getPanTiltImageFlipMode());
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xF3E7B)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0xF3E7D)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0xFFABC)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0xFFFFF)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0x00000)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0x00123)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0x00DD2)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x00DD3)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x20000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x30000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x40000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x50000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x60000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x70000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x80000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x90000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xA0000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xB0000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xC0000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xD0000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xE0000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xF0000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x10000000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x7FFFFFFF)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x80000000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xF0000000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xFFFFFFFF)));
    }
    else {
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xEDFF)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0xEE00)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0xFABC)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0xFFFF)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0x0000)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0x0123)));
        EXPECT_TRUE(st_if_.isValidTiltAbsolute(U32_T(0x0400)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x0401)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x1000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x2000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x3000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x4000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x5000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x6000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x7000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x8000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x9000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xA000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xB000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xC000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xD000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xE000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x00010000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x10000000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x7FFFFFFF)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0x80000000)));
        EXPECT_FALSE(st_if_.isValidTiltAbsolute(U32_T(0xFFFFFFFF)));
    }
}

TEST_F(PtzfStatusTest, isValidPanRelative)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0xEC6B1)));
        EXPECT_TRUE(st_if_.isValidPanRelative(U32_T(0xEC6B2)));
        EXPECT_TRUE(st_if_.isValidPanRelative(U32_T(0xFABCD)));
        EXPECT_TRUE(st_if_.isValidPanRelative(U32_T(0xFFFFF)));
        EXPECT_TRUE(st_if_.isValidPanRelative(U32_T(0x00000)));
        EXPECT_TRUE(st_if_.isValidPanRelative(U32_T(0x12345)));
        EXPECT_TRUE(st_if_.isValidPanRelative(U32_T(0x1394E)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x1394F)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x20000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x30000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x40000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x50000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x60000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x70000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x80000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x90000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0xA0000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0xB0000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0xC0000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0xD0000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0xE0000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x10000000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x7FFFFFFF)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x80000000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0xF0000000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0xFFFFFFFF)));
    }
    else {
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0xBBFF)));
        EXPECT_TRUE(st_if_.isValidPanRelative(U32_T(0xBC00)));
        EXPECT_TRUE(st_if_.isValidPanRelative(U32_T(0xFABC)));
        EXPECT_TRUE(st_if_.isValidPanRelative(U32_T(0xFFFF)));
        EXPECT_TRUE(st_if_.isValidPanRelative(U32_T(0x0000)));
        EXPECT_TRUE(st_if_.isValidPanRelative(U32_T(0x1234)));
        EXPECT_TRUE(st_if_.isValidPanRelative(U32_T(0x4400)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x4401)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x5000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x6000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x7000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x8000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x9000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0xA000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0xB000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x00010000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x10000000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x7FFFFFFF)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0x80000000)));
        EXPECT_FALSE(st_if_.isValidPanRelative(U32_T(0xFFFFFFFF)));
    }
}

TEST_F(PtzfStatusTest, isValidTiltRelative)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xF30AA)));
        EXPECT_TRUE(st_if_.isValidTiltRelative(U32_T(0xF30AB)));
        EXPECT_TRUE(st_if_.isValidTiltRelative(U32_T(0xFFABC)));
        EXPECT_TRUE(st_if_.isValidTiltRelative(U32_T(0xFFFFF)));
        EXPECT_TRUE(st_if_.isValidTiltRelative(U32_T(0x00000)));
        EXPECT_TRUE(st_if_.isValidTiltRelative(U32_T(0x01234)));
        EXPECT_TRUE(st_if_.isValidTiltRelative(U32_T(0x0CF55)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x0CF56)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x10000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x20000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x30000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x40000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x50000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x60000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x70000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x80000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x90000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xA0000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xB0000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xC0000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xD0000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xE0000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xF0000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x10000000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x7FFFFFFF)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x80000000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xF0000000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xFFFFFFFF)));
    }
    else {
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xE9FF)));
        EXPECT_TRUE(st_if_.isValidTiltRelative(U32_T(0xEA00)));
        EXPECT_TRUE(st_if_.isValidTiltRelative(U32_T(0xFABC)));
        EXPECT_TRUE(st_if_.isValidTiltRelative(U32_T(0xFFFF)));
        EXPECT_TRUE(st_if_.isValidTiltRelative(U32_T(0x0000)));
        EXPECT_TRUE(st_if_.isValidTiltRelative(U32_T(0x1234)));
        EXPECT_TRUE(st_if_.isValidTiltRelative(U32_T(0x1600)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x1601)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x2000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x3000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x4000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x5000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x6000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x7000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x8000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x9000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xA000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xB000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xC000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xD000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xE000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x00010000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x10000000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x7FFFFFFF)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0x80000000)));
        EXPECT_FALSE(st_if_.isValidTiltRelative(U32_T(0xFFFFFFFF)));
    }
}

TEST_F(PtzfStatusTest, isValidPanRelativeMoveRange)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        // Normal Case
        // Current Position(0x00000) home
        st_.setPanTiltPosition(0x00000, 0x00000);
        // Left Move Range (0x00937)(10° )
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0x00937));
        // Left Move Range (0x09CA7)(170°)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0x09CA7));
        // Left Move Range (0x09CA7)(170°<)
        EXPECT_EQ(false, st_if_.isValidPanRelativeMoveRange(0x09CA8));
        // RightMove Range (0xFF6C9)(-10° )
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0xFF6C9));
        // RightMove Range (0xF6359)(-170°)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0xF6359));
        // RightMove Range (0xF6359)(-170°>)
        EXPECT_EQ(false, st_if_.isValidPanRelativeMoveRange(0xF6358));
    }
    else {
        // Normal Case
        // Current Position(0x0000) home
        st_.setPanTiltPosition(0x0000, 0x0000);
        // Left Move Range (0x0200)(10° )
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0x0200));
        // Left Move Range (0x2200)(170°)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0x2200));
        // Left Move Range (0x2200)(170°<)
        EXPECT_EQ(false, st_if_.isValidPanRelativeMoveRange(0x2201));
        // RightMove Range (0xFE00)(-10° )
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0xFE00));
        // RightMove Range (0xDE00)(-170°)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0xDE00));
        // RightMove Range (0xDE00)(-170°>)
        EXPECT_EQ(false, st_if_.isValidPanRelativeMoveRange(0xDDFF));
    }
}

TEST_F(PtzfStatusTest, isValidPanRelativeMoveRangeBoundaryCheck)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);

    // Boundary Value Case //
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        // Current Position(0x00010) Left
        st_.setPanTiltPosition(0x00010, 0x00000);
        // Left Move Range (0x09C97)(170°)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0x09C97));
        // RightMove Range (0xF6349)(-170°)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0xF6369));

        // Current Position(0xFFFFF) Rigth
        st_.setPanTiltPosition(0xFFFFF, 0x00000);
        // Left Move Range (0x09CA6)(170°)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0x09CA6));
        // RightMove Range (0xF635A)(-170°)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0xF635A));

        // Pan Right to Left Case
        // Current Position(0xF6359) Rigth
        st_.setPanTiltPosition(0xF6359, 0x00000);
        // Left Move Range (0x1394E)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0x1394E));

        // Pan Left to Right Case
        // Current Position(0x09CA7) Left
        st_.setPanTiltPosition(0x09CA7, 0x00000);
        // Right Move Range (0xEC6B2)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0xEC6B2));
    }
    else {
        // Current Position(0x0010) Left
        st_.setPanTiltPosition(0x0010, 0x0000);
        // Left Move Range (0xDE00)(170°)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0xDDF0));
        // RightMove Range (0x2200)(-170°)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0x21F0));

        // Current Position(0xFFFF) Rigth
        st_.setPanTiltPosition(0xFFFF, 0x0000);
        // Left Move Range (0x09CA6)(170°)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0xDE01));
        // RightMove Range (0xF635A)(-170°)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0x2201));

        // Pan Right to Left Case
        // Current Position(0x2200) Rigth
        st_.setPanTiltPosition(0x2200, 0x0000);
        // Left Move Range (0xBC00)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0xBC00));

        // Pan Left to Right Case
        // Current Position(0xDE00) Left
        st_.setPanTiltPosition(0xDE00, 0x0000);
        // Right Move Range (0x4400)
        EXPECT_EQ(true, st_if_.isValidPanRelativeMoveRange(0x4400));
    }
}

TEST_F(PtzfStatusTest, isValidTiltRelativeMoveRange)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);

    visca::PictureFlipMode mode;
    // IMG-FLIP(Off) //
    mode = visca::PICTURE_FLIP_MODE_OFF;
    st_.setImageFlipStatusOnBoot(mode);

    // Tilt Flip Off Normal Case //
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        // Current Position(0x00000) home
        st_.setPanTiltPosition(0x00000, 0x00000);
        // Up Move Range (0x00937)(10°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x00937));
        // Up Move Range (0x0B3B0)(195°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x0B3B0));
        // Up Move Range (0x0B3B0<)(195°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x0B3B1));
        // Down Move Range (0x0F6C9)(-10°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xFF6C9));
        // Up Move Range (0xFE45B)(-30°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xFE45B));
        // Up Move Range (0xFE45B>)(-30°>)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xFE45A));
    }
    else {
        // Current Position(0x0000) home
        st_.setPanTiltPosition(0x0000, 0x0000);
        // Up Move Range (0x0200)(10°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x0200));
        // Up Move Range (0x1200)(90°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x1200));
        // Up Move Range (0x1200<)(90°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x1201));
        // Down Move Range (0xFE00)(-10°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xFE00));
        // Up Move Range (0xFC00)(-20°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xFC00));
        // Up Move Range (0xFC00>)(-20°>)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xFBFF));
    }

    // IMG-FLIP(On) //
    mode = visca::PICTURE_FLIP_MODE_ON;
    st_.setImageFlipStatusOnBoot(mode);

    // Tilt Flip On Normal Case //
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        // Current Position(0x00000) home
        st_.setPanTiltPosition(0x00000, 0x00000);
        // Up Move Range (0x00937)(10°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x00937));
        // Up Move Range (0x00DD2)(15°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x00DD2));
        // Up Move Range (0x00DD2<)(15°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x00DD3));
        // Down Move Range (0x0F6C9)(-10°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xFF6C9));
        // Up Move Range (0xF3E7D)(-210°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xF3E7D));
        // Up Move Range (0xF3E7D>)(-210°>)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xF3E7C));
    }
    else {
        // Current Position(0x0000) home
        st_.setPanTiltPosition(0x0000, 0x0000);
        // Up Move Range (0x0200)(10°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x0200));
        // Up Move Range (0x0400)(20°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x0400));
        // Up Move Range (0x0400<)(20°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x0401));
        // Down Move Range (0xFE00)(-10°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xFE00));
        // Up Move Range (0xEE00)(-90°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xEE00));
        // Up Move Range (0xEE00>)(-90°>)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xEDFF));
    }
}

TEST_F(PtzfStatusTest, isValidTiltRelativeMoveRangeBoundaryCheck)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);

    visca::PictureFlipMode mode;

    // IMG-FLIP(Off) //
    mode = visca::PICTURE_FLIP_MODE_OFF;
    st_.setImageFlipStatusOnBoot(mode);

    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        // Boundary Value Case //
        // Current Position(0x00010) Up
        st_.setPanTiltPosition(0x00000, 0x00010);
        // Up Move Range (0x0B3A0)(195°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x0B3A0));
        // Up Move Range (0x0B3A0<)(195°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x0B3A1));
        // Down Move Range (0xFE44B)(-30°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xFE44B));
        // Down Move Range (0xFE44B>)(-30°>)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xFE44A));

        // Current Position(0xFFFFF) Down
        st_.setPanTiltPosition(0x00000, 0xFFFFF);
        // Up Move Range (0x0B3B1)(195°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x0B3B1));
        // Up Move Range (0x0B3B1<)(195°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x0B3B2));
        // Down Move Range (0xFE45C)(-30°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xFE45C));
        // Down Move Range (0x0E45C)(-30°>)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x0E45b));

        // Tilt Down to Up Case
        // Current Position(0xFE45B) Down
        st_.setPanTiltPosition(0x00000, 0xFE45B);
        // Up Move Range (0x0CF55)(195°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x0CF55));
        // Up Move Range (0x0CF55<)(195°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x0CF56));

        // Tilt Up to Down Case
        // Current Position(0x0B3B0) Up
        st_.setPanTiltPosition(0x00000, 0x0B3B0);
        // Down Move Range (0xF30AB)(-30°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xF30AB));
        // Down Move Range (0xF30AB<)(-30°>)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xF30AA));
    }
    else {
        // Boundary Value Case //
        // Current Position(0x00010) Up
        st_.setPanTiltPosition(0x0000, 0x0010);
        // Up Move Range (0x11F0)(90°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x11F0));
        // Up Move Range (0x11F0<)(90°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x11F1));
        // Down Move Range (0xFBF0)(-20°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xFBF0));
        // Down Move Range (0xFBF0>)(-20°>)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xFBEF));

        // Current Position(0xFFFFF) Down
        st_.setPanTiltPosition(0x0000, 0xFFFF);
        // Up Move Range (0x1201)(90°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x1201));
        // Up Move Range (0x1201<)(90°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x1202));
        // Down Move Range (0xFC01)(-20°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xFC01));
        // Down Move Range (0xFC01)(-20°>)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xFC00));

        // Tilt Down to Up Case
        // Current Position(0xFC00) Down
        st_.setPanTiltPosition(0x0000, 0xFC00);
        // Up Move Range (0x1600)(90°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x1600));
        // Up Move Range (0x1600<)(90°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x1601));

        // Tilt Up to Down Case
        // Current Position(0x1200) Up
        st_.setPanTiltPosition(0x0000, 0x12B0);
        // Down Move Range (0xEA00)(-20°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xEA00));
        // Down Move Range (0xEA00<)(-20°>)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xE9FF));
    }

    // IMG-FLIP(On) //
    mode = visca::PICTURE_FLIP_MODE_ON;
    st_.setImageFlipStatusOnBoot(mode);

    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        // Boundary Value Case //
        // Current Position(0x00010) Up
        st_.setPanTiltPosition(0x00000, 0x00010);
        // Up Move Range (0x00DC2)(15°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x00DC2));
        // Up Move Range (0x00DC2)(15°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x00DC3));
        // Up Move Range (0xF3E6C)(-210°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xF3E6D));
        // Up Move Range (0xF3E6C)(-210°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xF3E6C));

        // Current Position(0xFFFFF) Down
        st_.setPanTiltPosition(0x00000, 0xFFFFF);
        // Up Move Range (0x00DD2)(15°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x00DD3));
        // Up Move Range (0x00DD2>)(15°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x00DD4));
        // Up Move Range (0xF3E7D)(-210°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xF3E7E));
        // Up Move Range (0xF3E7D)(-210°>)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xF3E7D));

        // Tilt Down to Up Case
        // Current Position(0xF3E7D) Down
        st_.setPanTiltPosition(0x00000, 0xF3E7D);
        // Up Move Range (0x0CF55)(15°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x0CF55));
        // Up Move Range (0x0CF55<)(15°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x0CF56));

        // Tilt Up to Down Case
        // Current Position(0x00DD2) Up
        st_.setPanTiltPosition(0x00000, 0x00DD2);
        // Down Move Range (0xF30AA)(-210°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xF30AB));
        // Down Move Range (0xF30AA)(-210°)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xF30AA));
    }
    else {
        // Boundary Value Case //
        // Current Position(0x0010) Up
        st_.setPanTiltPosition(0x0000, 0x0010);
        // Up Move Range (0x03F0)(20°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x03F0));
        // Up Move Range (0x03F0)(20°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x03F1));
        // Up Move Range (0xEDF0)(-90°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xEDF0));
        // Up Move Range (0xEDF0)(-90°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xEDEF));

        // Current Position(0xFFFF) Down
        st_.setPanTiltPosition(0x0000, 0xFFFF);
        // Up Move Range (0x0401)(20°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x0401));
        // Up Move Range (0x0401>)(20°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x0402));
        // Up Move Range (0xEE01)(-90°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xEE01));
        // Up Move Range (0xEE01)(-90°>)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xEE00));

        // Tilt Down to Up Case
        // Current Position(0xEE00) Down
        st_.setPanTiltPosition(0x0000, 0xEE00);
        // Up Move Range (0x0800)(20°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0x1600));
        // Up Move Range (0x0800<)(20°<)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0x1601));

        // Tilt Up to Down Case
        // Current Position(0x0400) Up
        st_.setPanTiltPosition(0x0000, 0x0400);
        // Down Move Range (0xEA00)(-90°)
        EXPECT_EQ(true, st_if_.isValidTiltRelativeMoveRange(0xEA00));
        // Down Move Range (0xEA00<)(-90<°)
        EXPECT_EQ(false, st_if_.isValidTiltRelativeMoveRange(0xE9FF));
    }
}

TEST_F(PtzfStatusTest, PanTiltLimitDownLeft)
{
    u32_t pan = U32_T(0x210);
    u32_t tilt = U32_T(0x123);
    st_.setPanTiltLimitDownLeft(pan, tilt);
    EXPECT_EQ(pan, st_if_.getPanLimitLeft());
    EXPECT_EQ(tilt, st_if_.getTiltLimitDown());
}

TEST_F(PtzfStatusTest, PanTiltLimitUpRight)
{
    u32_t pan = U32_T(0x321);
    u32_t tilt = U32_T(0x234);
    st_.setPanTiltLimitUpRight(pan, tilt);
    EXPECT_EQ(pan, st_if_.getPanLimitRight());
    EXPECT_EQ(tilt, st_if_.getTiltLimitUp());
}

TEST_F(PtzfStatusTest, isPanTilitPositionInPanTiltLimitAreaFlipOff)
{
    // IMG-FLIP(Off) //
    st_.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);

    st_.setPanTiltPosition(0x05000, 0x01000);
    u32_t pan = U32_T(0x6000);
    u32_t tilt = U16_T(0xf000);
    st_.setPanTiltLimitDownLeft(pan, tilt);

    pan = U32_T(0xff000);
    tilt = U32_T(0x2000);
    st_.setPanTiltLimitUpRight(pan, tilt);

    EXPECT_TRUE(st_if_.isPanTilitPositionInPanTiltLimitArea());
}

TEST_F(PtzfStatusTest, isPanTilitPositionInPanTiltLimitAreaFlipOn)
{
    // IMG-FLIP(Off) //
    st_.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_ON);

    st_.setPanTiltPosition(0x05000, 0x01000);
    u32_t pan = U32_T(0x6000);
    u32_t tilt = U16_T(0xf000);
    st_.setPanTiltLimitDownLeft(pan, tilt);

    pan = U32_T(0xff000);
    tilt = U32_T(0x2000);
    st_.setPanTiltLimitUpRight(pan, tilt);

    EXPECT_TRUE(st_if_.isPanTilitPositionInPanTiltLimitArea());
}

TEST_F(PtzfStatusTest, isPanTilitPositionInPanTiltLimitAreaPanError)
{
// TODO: MARCO向け実装を行う
#if 0
    // IMG-FLIP(Off) //
    st_.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);

    st_.setPanTiltPosition(0x6001, 0x01000);
    u32_t pan = U32_T(0x6000);
    u16_t tilt = U16_T(0xf000);
    st_.setPanTiltLimitDownLeft(pan, tilt);

    pan = U32_T(0xff000);
    tilt = U16_T(0x2000);
    st_.setPanTiltLimitUpRight(pan, tilt);

    EXPECT_FALSE(st_if_.isPanTilitPositionInPanTiltLimitArea());
#endif
}

TEST_F(PtzfStatusTest, isPanTilitPositionInPanTiltLimitAreaTiltError)
{
// TODO: MARCO向け実装を行う
#if 0
    // IMG-FLIP(Off) //
    st_.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);

    st_.setPanTiltPosition(0x5ffff, 0x02001);
    u32_t pan = U32_T(0x6000);
    u16_t tilt = U16_T(0xf000);
    st_.setPanTiltLimitDownLeft(pan, tilt);

    pan = U32_T(0xff000);
    tilt = U16_T(0x2000);
    st_.setPanTiltLimitUpRight(pan, tilt);

    EXPECT_FALSE(st_if_.isPanTilitPositionInPanTiltLimitArea());
#endif
}

TEST_F(PtzfStatusTest, IRCorrection)
{
    visca::IRCorrection ir_correction[2] = { visca::IR_CORRECTION_STANDARD, visca::IR_CORRECTION_IRLIGHT };

    for (s32_t i = S32_T(0); i < S32_T(2); i++) {
        st_.setIRCorrection(ir_correction[i]);
        EXPECT_EQ(ir_correction[i], st_if_.getIRCorrection());
    }
}

TEST_F(PtzfStatusTest, isValidPanLimitLeft)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_FALSE(st_if_.isValidPanLimitLeft(U32_T(0xf6359)));
        EXPECT_TRUE(st_if_.isValidPanLimitLeft(U32_T(0xf635a)));
        EXPECT_TRUE(st_if_.isValidPanLimitLeft(U32_T(0x09ca7)));
        EXPECT_FALSE(st_if_.isValidPanLimitLeft(U32_T(0x09ca8)));
    }
    else {
        EXPECT_FALSE(st_if_.isValidPanLimitLeft(U32_T(0x2200)));
        EXPECT_TRUE(st_if_.isValidPanLimitLeft(U32_T(0x21FF)));
        EXPECT_TRUE(st_if_.isValidPanLimitLeft(U32_T(0xDE00)));
        EXPECT_FALSE(st_if_.isValidPanLimitLeft(U32_T(0xDDFF)));
    }
}

TEST_F(PtzfStatusTest, isValidPanLimitRight)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_FALSE(st_if_.isValidPanLimitRight(U32_T(0xf6358)));
        EXPECT_TRUE(st_if_.isValidPanLimitRight(U32_T(0xf6359)));
        EXPECT_TRUE(st_if_.isValidPanLimitRight(U32_T(0x09ca6)));
        EXPECT_FALSE(st_if_.isValidPanLimitRight(U32_T(0x09ca7)));
    }
    else {
        EXPECT_FALSE(st_if_.isValidPanLimitRight(U32_T(0x2201)));
        EXPECT_TRUE(st_if_.isValidPanLimitRight(U32_T(0x2200)));
        EXPECT_TRUE(st_if_.isValidPanLimitRight(U32_T(0xDE01)));
        EXPECT_FALSE(st_if_.isValidPanLimitRight(U32_T(0xDE00)));
    }
}

TEST_F(PtzfStatusTest, isValidTiltLimitUp)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);

    st_.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);

    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_FALSE(st_if_.isValidTiltLimitUp(U32_T(0xfe45b)));
        EXPECT_TRUE(st_if_.isValidTiltLimitUp(U32_T(0xfe45c)));
        EXPECT_TRUE(st_if_.isValidTiltLimitUp(U32_T(0x0b3b0)));
        EXPECT_FALSE(st_if_.isValidTiltLimitUp(U32_T(0x0b3b1)));
    }
    else {
        EXPECT_FALSE(st_if_.isValidTiltLimitUp(U32_T(0xFC00)));
        EXPECT_TRUE(st_if_.isValidTiltLimitUp(U32_T(0xFC01)));
        EXPECT_TRUE(st_if_.isValidTiltLimitUp(U32_T(0x1200)));
        EXPECT_FALSE(st_if_.isValidTiltLimitUp(U32_T(0x1201)));
    }

    st_.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_ON);

    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_FALSE(st_if_.isValidTiltLimitUp(U32_T(0xf3e7d)));
        EXPECT_TRUE(st_if_.isValidTiltLimitUp(U32_T(0xf3e7e)));
        EXPECT_TRUE(st_if_.isValidTiltLimitUp(U32_T(0x00dd2)));
        EXPECT_FALSE(st_if_.isValidTiltLimitUp(U32_T(0x00dd3)));
    }
    else {
        EXPECT_FALSE(st_if_.isValidTiltLimitUp(U32_T(0xEE00)));
        EXPECT_TRUE(st_if_.isValidTiltLimitUp(U32_T(0xEE01)));
        EXPECT_TRUE(st_if_.isValidTiltLimitUp(U32_T(0x0400)));
        EXPECT_FALSE(st_if_.isValidTiltLimitUp(U32_T(0x0401)));
    }
}

TEST_F(PtzfStatusTest, isValidTiltLimitDown)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);

    st_.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);

    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_FALSE(st_if_.isValidTiltLimitDown(U32_T(0xfe45a)));
        EXPECT_TRUE(st_if_.isValidTiltLimitDown(U32_T(0xfe45b)));
        EXPECT_TRUE(st_if_.isValidTiltLimitDown(U32_T(0x0b3af)));
        EXPECT_FALSE(st_if_.isValidTiltLimitDown(U32_T(0x0b3b0)));
    }
    else {
        EXPECT_FALSE(st_if_.isValidTiltLimitDown(U32_T(0xFBFF)));
        EXPECT_TRUE(st_if_.isValidTiltLimitDown(U32_T(0xFC00)));
        EXPECT_TRUE(st_if_.isValidTiltLimitDown(U32_T(0x11FF)));
        EXPECT_FALSE(st_if_.isValidTiltLimitDown(U32_T(0x1200)));
    }

    st_.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_ON);

    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_FALSE(st_if_.isValidTiltLimitDown(U32_T(0xf3e7c)));
        EXPECT_TRUE(st_if_.isValidTiltLimitDown(U32_T(0xf3e7d)));
        EXPECT_TRUE(st_if_.isValidTiltLimitDown(U32_T(0x00dd1)));
        EXPECT_FALSE(st_if_.isValidTiltLimitDown(U32_T(0x00dd2)));
    }
    else {
        EXPECT_FALSE(st_if_.isValidTiltLimitDown(U32_T(0xEDFF)));
        EXPECT_TRUE(st_if_.isValidTiltLimitDown(U32_T(0xEE00)));
        EXPECT_TRUE(st_if_.isValidTiltLimitDown(U32_T(0x03FF)));
        EXPECT_FALSE(st_if_.isValidTiltLimitDown(U32_T(0x0400)));
    }
}

TEST_F(PtzfStatusTest, PanTiltLimitConfigurationStatus)
{
    st_.setPanTiltLimitConfigurationStatus(true);
    EXPECT_TRUE(st_if_.isConfiguringPanTiltLimit());

    st_.setPanTiltLimitConfigurationStatus(false);
    EXPECT_FALSE(st_if_.isConfiguringPanTiltLimit());
}

TEST_F(PtzfStatusTest, PTZMode)
{
    PTZMode mode = PTZ_MODE_NORMAL;
    PTZMode get_mode;
    st_.setPTZMode(mode);
    st_if_.getPTZMode(get_mode);
    EXPECT_EQ(PTZ_MODE_NORMAL, get_mode);
}

TEST_F(PtzfStatusTest, PTZPanTiltMove)
{
    u8_t step = U8_T(0);
    u8_t get_step = U8_T(0);
    st_.setPTZPanTiltMove(step);
    st_if_.getPTZPanTiltMove(get_step);
    EXPECT_EQ(U8_T(0), get_step);
}

TEST_F(PtzfStatusTest, PTZZoomMove)
{
    u8_t step = U8_T(0);
    u8_t get_step = U8_T(0);
    st_.setPTZZoomMove(step);
    st_if_.getPTZZoomMove(get_step);
    EXPECT_EQ(U8_T(0), get_step);
}

TEST_F(PtzfStatusTest, isValidSinPanAbsolute)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_TRUE(st_if_.isValidSinPanAbsolute(S32_T(40103)));
        EXPECT_TRUE(st_if_.isValidSinPanAbsolute(S32_T(-40103)));
        EXPECT_FALSE(st_if_.isValidSinPanAbsolute(S32_T(40104)));
        EXPECT_FALSE(st_if_.isValidSinPanAbsolute(S32_T(-40104)));
    }
    else {
        EXPECT_TRUE(st_if_.isValidSinPanAbsolute(S32_T(-8704)));
        EXPECT_TRUE(st_if_.isValidSinPanAbsolute(S32_T(8704)));
        EXPECT_FALSE(st_if_.isValidSinPanAbsolute(S32_T(8705)));
        EXPECT_FALSE(st_if_.isValidSinPanAbsolute(S32_T(-8705)));
    }
}

TEST_F(PtzfStatusTest, isValidSinTiltAbsolute)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);

    visca::PictureFlipMode mode;
    mode = visca::PICTURE_FLIP_MODE_OFF;
    st_.setImageFlipStatusOnBoot(mode);
    EXPECT_EQ(visca::PICTURE_FLIP_MODE_OFF, st_if_.getPanTiltImageFlipMode());

    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_TRUE(st_if_.isValidSinTiltAbsolute(S32_T(46000)));
        EXPECT_TRUE(st_if_.isValidSinTiltAbsolute(S32_T(-7077)));
        EXPECT_FALSE(st_if_.isValidSinTiltAbsolute(S32_T(46001)));
        EXPECT_FALSE(st_if_.isValidSinTiltAbsolute(S32_T(-7078)));
    }
    else {
        EXPECT_TRUE(st_if_.isValidSinTiltAbsolute(S32_T(4608)));
        EXPECT_TRUE(st_if_.isValidSinTiltAbsolute(S32_T(-1024)));
        EXPECT_FALSE(st_if_.isValidSinTiltAbsolute(S32_T(4609)));
        EXPECT_FALSE(st_if_.isValidSinTiltAbsolute(S32_T(-1025)));
    }
}

TEST_F(PtzfStatusTest, isValidSinPanRelative)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_TRUE(st_if_.isValidSinPanRelative(S32_T(80206)));
        EXPECT_TRUE(st_if_.isValidSinPanRelative(S32_T(-80206)));
        EXPECT_FALSE(st_if_.isValidSinPanRelative(S32_T(80207)));
        EXPECT_FALSE(st_if_.isValidSinPanRelative(S32_T(-80207)));
    }
    else {
        EXPECT_TRUE(st_if_.isValidSinPanRelative(S32_T(17408)));
        EXPECT_TRUE(st_if_.isValidSinPanRelative(S32_T(-17408)));
        EXPECT_FALSE(st_if_.isValidSinPanRelative(S32_T(17409)));
        EXPECT_FALSE(st_if_.isValidSinPanRelative(S32_T(-17409)));
    }
}

TEST_F(PtzfStatusTest, isValidSinTiltRelative)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_TRUE(st_if_.isValidSinTiltRelative(S32_T(53077)));
        EXPECT_TRUE(st_if_.isValidSinTiltRelative(S32_T(-53077)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelative(S32_T(53078)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelative(S32_T(-53078)));
    }
    else {
        EXPECT_TRUE(st_if_.isValidSinTiltRelative(S32_T(5632)));
        EXPECT_TRUE(st_if_.isValidSinTiltRelative(S32_T(-5632)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelative(S32_T(5633)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelative(S32_T(-5633)));
    }
}

TEST_F(PtzfStatusTest, getPanTiltMaxSpeed)
{
    u8_t pan_speed = U8_T(0);
    u8_t tilt_speed = U8_T(0);

    if (!capability_infra_if_.isEnableSpeedStep()) {
        st_if_.getPanTiltMaxSpeed(pan_speed, tilt_speed);
        EXPECT_EQ(U8_T(0x18), pan_speed);
        EXPECT_EQ(U8_T(0x17), tilt_speed);
    }
    else {
        st_.setSpeedStep(PAN_TILT_SPEED_STEP_NORMAL);
        st_if_.getPanTiltMaxSpeed(pan_speed, tilt_speed);
        EXPECT_EQ(U8_T(0x18), pan_speed);
        EXPECT_EQ(U8_T(0x18), tilt_speed);
        st_.setSpeedStep(PAN_TILT_SPEED_STEP_EXTENDED);
        st_if_.getPanTiltMaxSpeed(pan_speed, tilt_speed);
        EXPECT_EQ(U8_T(0x32), pan_speed);
        EXPECT_EQ(U8_T(0x32), tilt_speed);
    }
}

TEST_F(PtzfStatusTest, getPanMovementRange)
{
    infra::CoordinateType coordinate_type = infra::COORDINATE_TYPE_TYPE1;
    capability_infra_if_.getCoordinateType(coordinate_type);

    u32_t left = U32_T(0);
    u32_t right = U32_T(0);

    st_if_.getPanMovementRange(left, right);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_EQ(U32_T(0x09CA7), left);
        EXPECT_EQ(U32_T(0xF6359), right);
    }
    else {
        EXPECT_EQ(U32_T(0xde00), left);
        EXPECT_EQ(U32_T(0x2200), right);
    }
}

TEST_F(PtzfStatusTest, getTiltMovementRange)
{
    infra::CoordinateType coordinate_type = infra::COORDINATE_TYPE_TYPE1;
    capability_infra_if_.getCoordinateType(coordinate_type);

    u32_t down = U32_T(0);
    u32_t up = U32_T(0);

    st_.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);
    st_if_.getTiltMovementRange(down, up);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_EQ(U32_T(0xfe45b), down);
        EXPECT_EQ(U32_T(0x0b3b0), up);
    }
    else {
        EXPECT_EQ(U32_T(0xfc00), down);
        EXPECT_EQ(U32_T(0x1200), up);
    }
    st_.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_ON);
    st_if_.getTiltMovementRange(down, up);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        EXPECT_EQ(U32_T(0xf3e7d), down);
        EXPECT_EQ(U32_T(0x00dd2), up);
    }
    else {
        EXPECT_EQ(U32_T(0xee00), down);
        EXPECT_EQ(U32_T(0x0400), up);
    }
}

TEST_F(PtzfStatusTest, getOpticalZoomMaxMagnification)
{
    u8_t Magnification = U8_T(0);

    st_if_.getOpticalZoomMaxMagnification(Magnification);
    EXPECT_EQ(U8_T(0x14), Magnification);
}

TEST_F(PtzfStatusTest, getZoomMovementRange)
{
    u16_t wide = U16_T(1);
    u16_t optical = U16_T(0);
    u16_t clear_image = U16_T(0);
    u16_t digital = U16_T(0);

    st_.setMaxZoomValue(U16_T(0x6000));
    st_if_.getZoomMovementRange(wide, optical, clear_image, digital);
    EXPECT_EQ(U16_T(0x00), wide);
    EXPECT_EQ(U16_T(0x4000), optical);
    EXPECT_EQ(U16_T(0x6000), clear_image);
    EXPECT_EQ(U16_T(0x7ac0), digital);

    st_.setMaxZoomValue(U16_T(0x5556));
    st_if_.getZoomMovementRange(wide, optical, clear_image, digital);
    EXPECT_EQ(U16_T(0x00), wide);
    EXPECT_EQ(U16_T(0x4000), optical);
    EXPECT_EQ(U16_T(0x5556), clear_image);
    EXPECT_EQ(U16_T(0x7ac0), digital);
}

TEST_F(PtzfStatusTest, getZoomMaxVelocity)
{
    u8_t velocity = U8_T(0);

    st_if_.getZoomMaxVelocity(velocity);
    EXPECT_EQ(U8_T(0x08), velocity);
}

TEST_F(PtzfStatusTest, roundPTZPanRelativeMoveRange)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_EQ(S32_T(40103), st_if_.roundPTZPanRelativeMoveRange(S32_T(40103)));
        EXPECT_EQ(S32_T(-40103), st_if_.roundPTZPanRelativeMoveRange(S32_T(-40103)));

        st_.setPanTiltPosition(U32_T(0x9370), U32_T(0));
        EXPECT_EQ(S32_T(2359), st_if_.roundPTZPanRelativeMoveRange(S32_T(40103)));

        st_.setPanTiltPosition(U32_T(0xF6C90), U32_T(0));
        EXPECT_EQ(S32_T(-2359), st_if_.roundPTZPanRelativeMoveRange(S32_T(-40103)));
    }
    else {
        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_EQ(S32_T(8704), st_if_.roundPTZPanRelativeMoveRange(S32_T(8704)));
        EXPECT_EQ(S32_T(-8704), st_if_.roundPTZPanRelativeMoveRange(S32_T(-8704)));

        st_.setPanTiltPosition(U32_T(0x2000), U32_T(0));
        EXPECT_EQ(S32_T(512), st_if_.roundPTZPanRelativeMoveRange(S32_T(8704)));

        st_.setPanTiltPosition(U32_T(0xE000), U32_T(0));
        EXPECT_EQ(S32_T(-512), st_if_.roundPTZPanRelativeMoveRange(S32_T(-8704)));
    }
}

TEST_F(PtzfStatusTest, roundPTZTiltRelativeMoveRange)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);

    visca::PictureFlipMode mode;
    // IMG-FLIP(Off) //
    mode = visca::PICTURE_FLIP_MODE_OFF;
    st_.setImageFlipStatusOnBoot(mode);

    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        // Tilt Flip Off Normal Case //
        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_EQ(S32_T(-7077), st_if_.roundPTZTiltRelativeMoveRange(S32_T(-7077)));
        EXPECT_EQ(S32_T(46000), st_if_.roundPTZTiltRelativeMoveRange(S32_T(46000)));

        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_EQ(S32_T(-7077), st_if_.roundPTZTiltRelativeMoveRange(S32_T(-9436)));

        st_.setPanTiltPosition(U32_T(0), U32_T(0x49B8));
        EXPECT_EQ(S32_T(27128), st_if_.roundPTZTiltRelativeMoveRange(S32_T(46000)));
    }
    else {
        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_EQ(S32_T(-1024), st_if_.roundPTZTiltRelativeMoveRange(S32_T(-1024)));
        EXPECT_EQ(S32_T(4608), st_if_.roundPTZTiltRelativeMoveRange(S32_T(4608)));

        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_EQ(S32_T(-1024), st_if_.roundPTZTiltRelativeMoveRange(S32_T(-9436)));

        st_.setPanTiltPosition(U32_T(0), U32_T(0x1000));
        EXPECT_EQ(S32_T(512), st_if_.roundPTZTiltRelativeMoveRange(S32_T(4608)));
    }
    // IMG-FLIP(On) //
    mode = visca::PICTURE_FLIP_MODE_ON;
    st_.setImageFlipStatusOnBoot(mode);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_EQ(S32_T(3538), st_if_.roundPTZTiltRelativeMoveRange(S32_T(3538)));
        EXPECT_EQ(S32_T(-49539), st_if_.roundPTZTiltRelativeMoveRange(S32_T(-49539)));

        st_.setPanTiltPosition(U32_T(0), U32_T(0x049B));
        EXPECT_EQ(S32_T(2359), st_if_.roundPTZTiltRelativeMoveRange(S32_T(9436)));

        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_EQ(S32_T(-49539), st_if_.roundPTZTiltRelativeMoveRange(S32_T(-51898)));
    }
    else {
        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_EQ(S32_T(1024), st_if_.roundPTZTiltRelativeMoveRange(S32_T(1024)));
        EXPECT_EQ(S32_T(-4608), st_if_.roundPTZTiltRelativeMoveRange(S32_T(-4608)));

        st_.setPanTiltPosition(U32_T(0), U32_T(0x0100));
        EXPECT_EQ(S32_T(768), st_if_.roundPTZTiltRelativeMoveRange(S32_T(2048)));

        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_EQ(S32_T(-4608), st_if_.roundPTZTiltRelativeMoveRange(S32_T(-5632)));
    }
}

TEST_F(PtzfStatusTest, isValidSinPanRelativeMoveRange)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);

    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        // Tilt Flip Off Normal Case //
        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_TRUE(st_if_.isValidSinPanRelativeMoveRange(S32_T(-40103)));
        EXPECT_TRUE(st_if_.isValidSinPanRelativeMoveRange(S32_T(40103)));
        EXPECT_FALSE(st_if_.isValidSinPanRelativeMoveRange(S32_T(-40104)));
        EXPECT_FALSE(st_if_.isValidSinPanRelativeMoveRange(S32_T(40104)));

        st_.setPanTiltPosition(U32_T(1), U32_T(0));
        EXPECT_TRUE(st_if_.isValidSinPanRelativeMoveRange(S32_T(-40104)));
        EXPECT_TRUE(st_if_.isValidSinPanRelativeMoveRange(S32_T(40102)));
        EXPECT_FALSE(st_if_.isValidSinPanRelativeMoveRange(S32_T(-40105)));
        EXPECT_FALSE(st_if_.isValidSinPanRelativeMoveRange(S32_T(40103)));
    }
    else {
        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_TRUE(st_if_.isValidSinPanRelativeMoveRange(S32_T(-8704)));
        EXPECT_TRUE(st_if_.isValidSinPanRelativeMoveRange(S32_T(8704)));
        EXPECT_FALSE(st_if_.isValidSinPanRelativeMoveRange(S32_T(-8705)));
        EXPECT_FALSE(st_if_.isValidSinPanRelativeMoveRange(S32_T(8705)));

        st_.setPanTiltPosition(U32_T(1), U32_T(0));
        EXPECT_TRUE(st_if_.isValidSinPanRelativeMoveRange(S32_T(-8705)));
        EXPECT_TRUE(st_if_.isValidSinPanRelativeMoveRange(S32_T(8703)));
        EXPECT_FALSE(st_if_.isValidSinPanRelativeMoveRange(S32_T(-8706)));
        EXPECT_FALSE(st_if_.isValidSinPanRelativeMoveRange(S32_T(8704)));
    }
}

TEST_F(PtzfStatusTest, isValidSinTiltRelativeMoveRange)
{
    infra::CoordinateType coordinate_type;
    capability_infra_if_.getCoordinateType(coordinate_type);

    visca::PictureFlipMode mode;
    // IMG-FLIP(Off) //
    mode = visca::PICTURE_FLIP_MODE_OFF;
    st_.setImageFlipStatusOnBoot(mode);

    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        // Tilt Flip Off Normal Case //
        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-7077)));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(46000)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-7078)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(46001)));

        st_.setPanTiltPosition(U32_T(0), U32_T(1));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-7078)));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(45999)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-7079)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(46000)));
    }
    else {
        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-1024)));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(4608)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-1025)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(4609)));

        st_.setPanTiltPosition(U32_T(0), U32_T(1));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-1025)));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(4607)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-1026)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(4608)));
    }
    // IMG-FLIP(On) //
    mode = visca::PICTURE_FLIP_MODE_ON;
    st_.setImageFlipStatusOnBoot(mode);
    if (coordinate_type == infra::COORDINATE_TYPE_TYPE1) {
        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-49539)));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(3538)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-49540)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(3539)));

        st_.setPanTiltPosition(U32_T(0), U32_T(1));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-49540)));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(3537)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-49541)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(3538)));
    }
    else {
        st_.setPanTiltPosition(U32_T(0), U32_T(0));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-4608)));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(1024)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-4609)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(1025)));

        st_.setPanTiltPosition(U32_T(0), U32_T(1));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-4609)));
        EXPECT_TRUE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(1023)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(-4610)));
        EXPECT_FALSE(st_if_.isValidSinTiltRelativeMoveRange(S32_T(1024)));
    }
}

TEST_F(PtzfStatusTest, PanLimitMode)
{
    bool pan_limit_mode = true;
    st_.setPanLimitMode(pan_limit_mode);
    EXPECT_EQ(pan_limit_mode, st_if_.getPanLimitMode());

    pan_limit_mode = false;
    st_.setPanLimitMode(pan_limit_mode);
    EXPECT_EQ(pan_limit_mode, st_if_.getPanLimitMode());
}

TEST_F(PtzfStatusTest, TiltLimitMode)
{
    bool tilt_limit_mode = true;
    st_.setTiltLimitMode(tilt_limit_mode);
    EXPECT_EQ(tilt_limit_mode, st_if_.getTiltLimitMode());

    tilt_limit_mode = false;
    st_.setTiltLimitMode(tilt_limit_mode);
    EXPECT_EQ(tilt_limit_mode, st_if_.getTiltLimitMode());
}

TEST_F(PtzfStatusTest, isValidPanSpeed)
{
    bool result = false;
    if (!st_if_.isEnableSpeedStep()) {
        ARRAY_FOREACH (pan_speed_table, i) {
            st_.setSlowMode(pan_speed_table[i].slow_mode);
            result = st_if_.isValidPanSpeed(pan_speed_table[i].pan_speed);
            EXPECT_EQ(pan_speed_table[i].result, result);
        }
    }
    else {
        ARRAY_FOREACH (pan_tilt_speed_with_ss_table, i) {
            st_.setSpeedStep(pan_tilt_speed_with_ss_table[i].speed_step);
            st_.setSlowMode(pan_tilt_speed_with_ss_table[i].slow_mode);
            result = st_if_.isValidPanSpeed(pan_tilt_speed_with_ss_table[i].pt_speed);
            EXPECT_EQ(pan_tilt_speed_with_ss_table[i].result, result);
        }
    }
}

TEST_F(PtzfStatusTest, isValidTiltSpeed)
{
    bool result = false;
    if (!st_if_.isEnableSpeedStep()) {
        ARRAY_FOREACH (tilt_speed_table, i) {
            st_.setSlowMode(tilt_speed_table[i].slow_mode);
            result = st_if_.isValidTiltSpeed(tilt_speed_table[i].tilt_speed);
            EXPECT_EQ(tilt_speed_table[i].result, result);
        }
    }
    else {
        ARRAY_FOREACH (pan_tilt_speed_with_ss_table, i) {
            st_.setSpeedStep(pan_tilt_speed_with_ss_table[i].speed_step);
            st_.setSlowMode(pan_tilt_speed_with_ss_table[i].slow_mode);
            result = st_if_.isValidTiltSpeed(pan_tilt_speed_with_ss_table[i].pt_speed);
            EXPECT_EQ(pan_tilt_speed_with_ss_table[i].result, result);
        }
    }
}

TEST_F(PtzfStatusTest, roundPanMaxSpeed)
{
    u8_t result = U8_T(0x0);
    if (!st_if_.isEnableSpeedStep()) {
        ARRAY_FOREACH (round_pan_speed_table, i) {
            st_.setSlowMode(round_pan_speed_table[i].slow_mode);
            result = st_if_.roundPanMaxSpeed(round_pan_speed_table[i].pan_speed);
            EXPECT_EQ(round_pan_speed_table[i].result, result);
        }
    }
    else {
        ARRAY_FOREACH (round_pan_tilt_speed_with_ss_table, i) {
            st_.setSpeedStep(round_pan_tilt_speed_with_ss_table[i].speed_step);
            st_.setSlowMode(round_pan_tilt_speed_with_ss_table[i].slow_mode);
            result = st_if_.roundPanMaxSpeed(round_pan_tilt_speed_with_ss_table[i].pt_speed);
            EXPECT_EQ(round_pan_tilt_speed_with_ss_table[i].result, result);
        }
    }
}

TEST_F(PtzfStatusTest, roundTiltMaxSpeed)
{
    u8_t result = U8_T(0x0);
    if (!st_if_.isEnableSpeedStep()) {
        ARRAY_FOREACH (round_tilt_speed_table, i) {
            st_.setSlowMode(round_tilt_speed_table[i].slow_mode);
            result = st_if_.roundTiltMaxSpeed(round_tilt_speed_table[i].tilt_speed);
            EXPECT_EQ(round_tilt_speed_table[i].result, result);
        }
    }
    else {
        ARRAY_FOREACH (round_pan_tilt_speed_with_ss_table, i) {
            st_.setSpeedStep(round_pan_tilt_speed_with_ss_table[i].speed_step);
            st_.setSlowMode(round_pan_tilt_speed_with_ss_table[i].slow_mode);
            result = st_if_.roundTiltMaxSpeed(round_pan_tilt_speed_with_ss_table[i].pt_speed);
            EXPECT_EQ(round_pan_tilt_speed_with_ss_table[i].result, result);
        }
    }
}

TEST_F(PtzfStatusTest, getStandbyMode)
{
    const StandbyMode test_list[] = { StandbyMode::NEUTRAL, StandbyMode::SIDE };

    for (auto& item : test_list) {
        EXPECT_CALL(backup_infra_if_mock_, getStandbyMode(_))
            .WillOnce(DoAll(SetArgReferee<0>(item), Return(ERRORCODE_SUCCESS)));
        EXPECT_EQ(item, st_if_.getStandbyMode());
    }

    EXPECT_CALL(backup_infra_if_mock_, getStandbyMode(_)).WillOnce(Return(ERRORCODE_EXEC));
    EXPECT_EQ(StandbyMode::NEUTRAL, st_if_.getStandbyMode());
}

TEST_F(PtzfStatusTest, PanTiltUnlockErrorStatus)
{
    PanTiltLockControlStatus input = PanTiltLockControlStatus::PAN_TILT_LOCK_STATUS_NONE;
    PanTiltLockControlStatus result = PanTiltLockControlStatus::PAN_TILT_LOCK_STATUS_NONE;

    input = PanTiltLockControlStatus::PAN_TILT_LOCK_STATUS_NONE;
    EXPECT_CALL(status_infra_if_mock_, setPanTiltLockControlStatus(Eq(input))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(input), Return(true)));

    st_.setPanTiltLockControlStatus(input);
    st_if_.getPanTiltLockControlStatus(result);
    EXPECT_EQ(PanTiltLockControlStatus::PAN_TILT_LOCK_STATUS_NONE, result);

    input = PanTiltLockControlStatus::PAN_TILT_LOCK_STATUS_UNLOCKED;
    EXPECT_CALL(status_infra_if_mock_, setPanTiltLockControlStatus(Eq(input))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(input), Return(true)));

    st_.setPanTiltLockControlStatus(input);
    st_if_.getPanTiltLockControlStatus(result);
    EXPECT_EQ(PanTiltLockControlStatus::PAN_TILT_LOCK_STATUS_UNLOCKED, result);

    input = PanTiltLockControlStatus::PAN_TILT_LOCK_STATUS_LOCKED;
    EXPECT_CALL(status_infra_if_mock_, setPanTiltLockControlStatus(Eq(input))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(input), Return(true)));

    st_.setPanTiltLockControlStatus(input);
    st_if_.getPanTiltLockControlStatus(result);
    EXPECT_EQ(PanTiltLockControlStatus::PAN_TILT_LOCK_STATUS_LOCKED, result);

    input = PanTiltLockControlStatus::PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING;
    EXPECT_CALL(status_infra_if_mock_, setPanTiltLockControlStatus(Eq(input))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(input), Return(true)));

    st_.setPanTiltLockControlStatus(input);
    st_if_.getPanTiltLockControlStatus(result);
    EXPECT_EQ(PanTiltLockControlStatus::PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING, result);
}

} // namespace ptzf
