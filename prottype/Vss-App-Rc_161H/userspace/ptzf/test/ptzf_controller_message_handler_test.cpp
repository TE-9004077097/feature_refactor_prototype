/*
 * ptzf_controller_message_handler_test.cpp
 *
 * Copyright 2016,2018,2019 Sony Imaging Products & Solutions Inc.
 */

#include "types.h"
#include "gtl_memory.h"
#include "gtl_string.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "common_gmock_util.h"

#include "common_message_queue.h"
#include "common_thread_object.h"

#include "ptzf_controller_message_handler.h"
#include "ptzf_status.h"
#include "ptzf/ptzf_message.h"
#include "ptzf/ptz_trace_if_mock.h"
#include "ptzf_controller_mock.h"
#include "ptzf_controller_finalizer_mock.h"
#include "ptzf_backup_infra_if_mock.h"
#include "ptzf_pan_tilt_infra_if_mock.h"
#include "ptzf_zoom_infra_if_mock.h"
#include "ptzf_focus_infra_if_mock.h"
#include "ptzf_if_clear_infra_if_mock.h"
#include "ptzf_pan_tilt_lock_infra_if_mock.h"
#include "ptzf/ptzf_initialize_infra_if_mock.h"
#include "visca/visca_server_message_if.h"
#include "visca/visca_server_ptzf_if.h"
#include "visca/visca_status_if.h"
#include "visca/visca_server_common.h"

#include "visca/visca_server_message_if_mock.h"
#include "event_router/event_router_if_mock.h"
#include "preset/preset_manager_message_if_mock.h"
#include "visca/visca_server_ptzf_if_mock.h"
#include "visca/visca_status_if_mock.h"
#include "preset/database_initialize_if_mock.h"
#include "ptzf/ptzf_initialize_infra_if_mock.h"
#include "ptzf/ptzf_finalize_infra_if_mock.h"
#include "ptzf_pan_tilt_lock_infra_if_mock.h"
#include "ptzf/ptzf_common_message.h"
#include "pt_micon_power_infra_if_mock.h"
#include "preset/preset_status_if_mock.h"
#include "video/video_status_if_mock.h"
#include "ptzf/ptzf_status_if_mock.h"
#include "ptzf/ptz_trace_status_if_mock.h"
#include "ptzf/ptz_trace_if_mock.h"

#include "bizglobal_modelname_creater.h"
#include "metadata/metadata_control_if_mock.h"
#include "infra/ptzf_infra_message.h"
#include "ptzf_status_infra_if_mock.h"
#include "ptzf/ptzf_message_if_mock.h"
#include "power/power_status_if_mock.h"
#include "infra/sequence_id_controller_mock.h"
#include "ptzf/ptzf_common_message.h"

#include "ptzf_capability_infra_if.h"
#include "ui/menu_status_mock.h"
#include "camera_osd/camera_osd_status_if_mock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Eq;
using ::testing::SetArgReferee;
using ::testing::Property;
using ::testing::Field;
using ::testing::StrCaseEq;
using ::testing::InvokeWithoutArgs;

namespace config {

struct ConfigReadyNotification
{};

} // namespace config

namespace ptzf {

// ○テストリスト
// + PowerOnメッセージを受信したら以下のシーケンスを行うこと
//   + PtzfPanTiltLockInfraIf::getThreadStatus()を呼び出すこと
//   + getThreadStatusでfalseだった場合、PtzfPanTiltLockInfraIf::startPollingLockStatus()を呼び出すこと
//   + PtzfInitializeInfraIf::setPowerOnSequenceStatus()をtrueで呼び出すこと
//   + PtzfPanTiltLockInfraIf::suppressLockStatusEvent()をtrueで呼び出すこと
//   + ViscaServerにPanTiltPowerOnRequestを送ること
// + PowerOffメッセージ受信時
//   - PT Lock制御状態がアンロック状態の場合, ViscaServerにPanTiltPowerOnRequestを送ること
//   - PT Lock制御状態が通電アンロック状態の場合, ViscaServerにPanTiltPowerOnRequestを送ること
//   - PT Lock制御状態がロック状態の場合, ViscaServerにPanTiltPowerOnRequestを送らず, PowerOff完了メッセージを送ること
//   - 共通処理としてPowerOFF処理中フラグをONにし, かつPT Lock/Unlock通知イベントを抑止すること
// + PowerOffメッセージを受信したらPanTiltStateを初期化すること
// + PowerOff完了メッセージ受信時
//   - PT Lock制御状態が通電アンロック状態の場合, Power Standbyかつアンロックの状態に遷移すること
//   - 共通処理としてPowerOFF処理中フラグをOFFにし, かつPT Lock/Unlock通知イベントの抑止を解除すること
// + RampCurveメッセージを受信したらViscaServerにRampCurveRequestを送ること(*)
// + ViscaServer経由のPTマイコンへのリクエストが正しく返ってきたらViscaServerにCompを返すこと
// + ViscaServer経由のPTマイコンへのリクエストがTimeoutしたらViscaServerにNotExeを返すこと
// + SlowModeメッセージを受信したらPtzfControllerThreadを経由してViscaServerにSlowModeRequestを送ること(*)
// + SpeedStepメッセージを受信したらPtzfControllerThreadを経由してViscaServerにSpeedStepRequestを送ること(*)
// + PanReverseメッセージを受信したらViscaServerにPanReverseRequestを送ること
// + TiltReverseメッセージを受信したらViscaServerにTiltReverseRequestを送ること
// + IRCorrectionメッセージを受信したらPtzfControllerThreadを経由してViscaServerにIRCorrectionを送ること(*)
// + TeleShiftModeメッセージを受信したらViscaServerにTeleShiftModeを送ること(*)
// + PanTiltMoveメッセージを受信したらViscaServerにPanTiltMoveを送ること(*)
// + ZoomMoveメッセージを受信したらViscaServerにZoomMoveを送ること(*)
// + FocusModeメッセージを受信したらViscaServerにFocusModeを送ること(*)
// + FocusMoveメッセージを受信したらViscaServerにFocusMoveを送ること(*)
// + PanTiltResetメッセージを受信したら電源起動・終了中以外かつPTLock状態でなければ
//   PtzfController::resetPanTiltPosition()を呼び出す事
// + PanTiltResetメッセージを受信したら電源起動・終了中又はPTLock状態ならエラーcompを返すこと
// + IfClearメッセージを受信したらViscaServerにIfClearを送ること(*)
// + ZoomFineMoveメッセージを受信したらViscaServerにZoomFineMoveを送ること
// + ImageFlipメッセージを受信したら変更ある場合はPtzfControllerThreadを経由してViscaServerにImageFlipを送ること
// + ImageFlipメッセージを受信したら変更ない場合は結果を正常で返すこと
// + ImageFlipメッセージを受信した際、変更できない状態や処理中にエラーが発生した場合は異常を返すこと
// + 複数のImageFlipメッセージを同時に受信した場合、
//   最初のメッセージのみPtzfControllerThreadを経由してViscaServerにImageFlipを送ること
// + SetPanTiltLimitメッセージを受信した際、設定不可能な場合は異常を返すこと
// + DZoomModeメッセージを受信したらViscaServerにDZoomModeを送ること(*)
// + ZoomAbsolutePositionメッセージを受信したらViscaServerにZoomAbsolutePositionを送ること(*)
// + ZoomRelativePositionメッセージを受信したらViscaServerにZoomAbsolutePositionを送ること(*)
// + FocusAbsolutePositionメッセージを受信したらViscaServerにFocusAbsolutePositionを送ること(*)
// + FocusRelativePositionメッセージを受信したらViscaServerにFocusAbsolutePositionを送ること(*)
// + FocusOnePushTriggerメッセージを受信したらViscaServerにFocusOnePushTriggerを送ること(*)
// + FocusAfSensitivityメッセージを受信したらViscaServerにFocusAfSensitivityを送ること(*)
// + FocusNearLimitメッセージを受信したらViscaServerにFocusNearLimitを送ること(*)
// + FocusAFModeメッセージを受信したらViscaServerにFocusAFModeを送ること(*)
// + FocusFaceEyeDetectionModeメッセージを受信したらViscaServerにFocusFaceEyeDetectionModeを送ること(*)
// + AfAssistメッセージを受信したらViscaServerにAfAssistを送ること(*)
// + FocusTrackingPositionメッセージを受信したらViscaServerにFocusTrackingPositionを送ること(*)
// + TouchFunctionInMfメッセージを受信したらViscaServerにTouchFunctionInMfを送ること(*)
// + FocusAFTimerメッセージを受信したらViscaServerにFocusAFTimerを送ること(*)
// + PanTiltRelativeMoveメッセージを受信したらViscaServerにPanTiltRelativeRequestを送ること(*)
// + ZoomRelativeMoveメッセージを受信したらViscaServerにZoomAbsolutePositionを送ること(*)
// + HomePositionRequestメッセージを受信したらViscaServerにHomePositionRequestを送ること
// + Finalizeメッセージを受信したらPtzfControllerFinalizer::finalize()を呼び出すこと
// + SetStandbyModeRequestメッセージ受信処理のテスト
//   + PtzfBackupInfraIf::setStandbyMode()を呼び出すこと
//   + StandbyModeが不正値の場合はエラーを返すこと
//   + PtzfBackupInfraIf::setStandbyMode()が失敗した場合はエラーを返すこと
// + visca::ViscaMessageSequence<SetStandbyModeRequest>メッセージ受信処理のテスト
//   + PtzfBackupInfraIf::setStandbyMode()を呼び出すこと
//   + StandbyModeが不正値の場合は visca::AckResponse でエラーを返すこと
//   + PtzfBackupInfraIf::setStandbyMode()が失敗した場合は ViscaServerMessageIf::sendCompRequest() でエラーを返すこと
// + BizMessage<SetStandbyModeRequest>メッセージ受信処理のテスト
//   + PtzfBackupInfraIf::setStandbyMode()を呼び出すこと
//   + StandbyModeが不正値の場合はエラーを返すこと
//   + PtzfBackupInfraIf::setStandbyMode()が失敗した場合はエラーを返すこと
// + BizMessage<SetPushAFModeRequestForBiz>メッセージ受信処理のテスト
//   + PtzfFocusInfraIf::setPushAFMode()を呼び出すこと
// + BizMessage<SetAfSubjShiftSensRequest>メッセージ受信処理のテスト
//   + PtzfFocusInfraIf::setAfSubjShiftSens()を呼び出すこと
// + BizMessage<SetAfTransitionSpeedRequest>メッセージ受信処理のテスト
//   + PtzfFocusInfraIf::setAfTransitionSpeed()を呼び出すこと
// + BizMessage<ExeCancelZoomPositionRequestForBiz>メッセージ受信処理のテスト
//   + PtzfZoomInfraIf::exeCancelZoomPosition()を呼び出すこと
// + BizMessage<ExeCancelFocusPositionRequestForBiz>メッセージ受信処理のテスト
//   + PtzfFocusInfraIf::exeCancelFocusPosition()を呼び出すこと
//
//   上記のうち、(*)を付けたBizPtzfIf対応箇所について、以下のパターンの動作が行えること
//     + 結果を通知するメッセージキュー名が未設定の場合
//     + 結果を通知するメッセージキュー名が設定されている場合
//         + 結果通知が正常な場合
//         + 結果通知が異常な場合(Viscaからの結果通知が異常)
//         + 結果通知が異常な場合(Viscaからの結果通知がタイムアウト)
//     + 各メッセージを同時に受信した場合
// + receiveUnlockToLockEventWithPowerOn
//   - PT Lock状態変化イベントを受信した際, 電源状態とPT Initialize/Finalize処理実行中かどうかを確認する
//   - 電源状態がPower ONであり, PT Initialize/Finalize処理実行中でなく,
//   かつUnlock-->Lockのイベントを受信したならば下記を実行
//     - PTブロックのFInalize処理(IF Clear)を実行する
//     - PTブロックの電源供給断処理を実行する(Visca経由)
//     - これらを同期的に実行する
//     - Finalize処理実行中は, 該当する内部状態を処理中に変更する
//     - PTブロックの電源OFF実行中は, PT Lock状態変化イベントの通知を停止する
//     - Lock制御状態をロック状態に変更する
// + receiveUnlockToLockEventWithPowerOnInitializing
//   - PT Lock状態変化イベントを受信した際, 電源状態とPT Initialize/Finalize処理実行中かどうかを確認する
//   - PT Initialize/Finalize処理実行中である場合, 処理を実行せずイベントを捨てる
//     - この場合で必要な処理はPT Initialize/Finalize処理の中で行う.
// + receiveUnlockToLockEventWithPowerOff
//   - PT Lock状態変化イベントを受信した際, 電源状態とPT Initialize/Finalize処理実行中かどうかを確認する
//   - 電源状態がPower OFFである場合, 下記を実行
//     - Lock制御状態をロック状態に変更する
// + receiveUnlockToLockEventWithPowerProcessing
//   - PT Lock状態変化イベントを受信した際, 電源状態とPT Initialize/Finalize処理実行中かどうかを確認する
//   - 電源状態がPowerON遷移中またはPowerOFF遷移中である場合, 処理を実行せずイベントを捨てる
//     - この場合で必要な処理はPT Initialize/Finalize処理の中で行う.
// + receiveLockToUnlockEventWithPowerOn
//   - PT Lock状態変化イベントを受信した際, 電源状態とPT Initialize/Finalize処理実行中かどうかを確認する
//   - 電源状態がPower ONであり, PT Initialize/Finalize処理実行中でなく,
//   かつLock-->Unlockのイベントを受信したならば下記が実行されることを確認する
//     - PTブロックの電源供給処理を実行する(Visca経由)
//     - 電源供給処理実行中は, Initialize実行中であることをPtzfStatusInfraに記憶し,
//       他のInitialize/Finalizeが同時動作しないようにする
//     - 電源供給処理実行中は, PT Lock状態変化イベントの通知を停止する
//     - Lock制御状態を通電アンロック状態に変更する
// + receiveLockToUnlockEventWithPowerOnCancel1
//   - Lock-->Unlockのイベントを受信時のシーケンスにおいて, 処理中に再度Lock状態になった場合, それを検出する
//   - この際, 電源供給処理実行前であった場合は, 電源供給処理を実行せずに処理を中止する
//   - 処理終了後にロック制御状態が更新されないことを確認する
// + receiveLockToUnlockEventWithPowerOnCancel2
//   - Lock-->Unlockのイベントを受信時のシーケンスにおいて, 処理中に再度Lock状態になった場合, それを検出する
//   - この際, 電源供給処理実行後であった場合は, 電源供給を止める操作が実行されることを確認する
//   - 処理終了後にロック制御状態が更新されないことを確認する
// + receiveLockToUnlockEventWithPowerOnFinalizing
//   - PT Lock状態変化イベントを受信した際, 電源状態とPT Initialize/Finalize処理実行中かどうかを確認する
//   - PT Initialize/Finalize処理実行中である場合, PT電源供給処理を実行しないことを確認する
// + receiveLockToUnlockEventWithPowerOff
//   - PT Lock状態変化イベントを受信した際, 電源状態とPT Initialize/Finalize処理実行中かどうかを確認する
//   - 電源状態がPower OFFである場合, 下記が実行されることを確認する
//     - Lock制御状態をアンロック状態に変更する
// + receiveLockToUnlockEventWithPowerProcessing
//   - PT Lock状態変化イベントを受信した際, 電源状態とPT Initialize/Finalize処理実行中かどうかを確認する
//   - 電源状態がPowerON遷移中またはPowerOFF遷移中である場合, PT電源供給処理を実行しないことを確認する

#pragma GCC diagnostic ignored "-Wconversion"

namespace {

const s32_t pan_ab_values[] = { S32_T(2359), S32_T(40103), S32_T(-2359), S32_T(-40103) };
const s32_t tilt_ab_values[] = { S32_T(2359), S32_T(21231), S32_T(-2359), S32_T(-7077) };
const s32_t pan_rel_values[] = { S32_T(2359), S32_T(40103), S32_T(-2359), S32_T(-40103) };
const s32_t tilt_rel_values[] = { S32_T(2359), S32_T(46000), S32_T(-2359), S32_T(-7077) };
const uint8_t af_subj_shift_sens_test_values[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
const uint8_t af_transition_speed_test_values[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };

const StandbyMode STANDBY_MODE_LIST[] = {
    StandbyMode::NEUTRAL,
    StandbyMode::SIDE,
};

const char TEST_SYNC_MQ_NAME[] = { "TestSyncMq" };
struct TestSyncMessage
{};
void postSyncMessage()
{
    common::MessageQueue mq(TEST_SYNC_MQ_NAME);
    TestSyncMessage msg;
    mq.post(msg);
}
void pendSyncMessage()
{
    common::MessageQueue mq(TEST_SYNC_MQ_NAME);
    TestSyncMessage msg;
    mq.pend(msg);
}

} // namespace

class PtzfControllerMessageHandlerTest : public ::testing::Test
{
protected:
    PtzfControllerMessageHandlerTest()
        : biz_model_(),
          event_router_mock_holder_(),
          visca_if_mock_holder_(),
          visca_status_if_mock_holder_(),
          preset_if_mock_holder_(),
          ptz_trace_if_mock_holder_(),
          controller_mock_holder_(),
          database_initialize_if_mock_holder_(),
          er_mock_(event_router_mock_holder_.getMock()),
          visca_if_mock_(visca_if_mock_holder_.getMock()),
          visca_status_if_mock_(visca_status_if_mock_holder_.getMock()),
          preset_if_mock_(preset_if_mock_holder_.getMock()),
          ptz_trace_if_mock_(ptz_trace_if_mock_holder_.getMock()),
          database_initialize_if_mock_(database_initialize_if_mock_holder_.getMock()),
          handler_(),
          reply_(),
          thread_mq_(PtzfControllerThreadMQ::getName()),
          controller_mock_(controller_mock_holder_.getMock()),
          finalizre_mock_holder_object_(),
          finalizre_mock_(finalizre_mock_holder_object_.getMock()),
          backup_infra_if_mock_holder_object_(),
          backup_infra_if_mock_(backup_infra_if_mock_holder_object_.getMock()),
          pan_tilt_infra_if_mock_holder_object_(),
          pan_tilt_infra_if_mock_(pan_tilt_infra_if_mock_holder_object_.getMock()),
          zoom_infra_if_mock_holder_object_(),
          zoom_infra_if_mock_(zoom_infra_if_mock_holder_object_.getMock()),
          focus_infra_if_mock_holder_object_(),
          focus_infra_if_mock_(focus_infra_if_mock_holder_object_.getMock()),
          if_clear_infra_if_mock_holder_object_(),
          if_clear_infra_if_mock_(if_clear_infra_if_mock_holder_object_.getMock()),
          pan_tilt_lock_infra_if_mock_holder_object_(),
          pan_tilt_lock_infra_if_mock_(pan_tilt_lock_infra_if_mock_holder_object_.getMock()),
          initialize_infra_if_mock_holder_object_(),
          initialize_infra_if_mock_(initialize_infra_if_mock_holder_object_.getMock()),
          pt_micon_power_infra_if_mock_holder_object_(),
          pt_micon_power_infra_if_mock_(pt_micon_power_infra_if_mock_holder_object_.getMock()),
          finalize_infra_if_mock_holder_object_(),
          finalize_infra_if_mock_(finalize_infra_if_mock_holder_object_.getMock()),
          ptzf_status_infra_if_mock_holder_object_(),
          ptzf_status_infra_if_mock_(ptzf_status_infra_if_mock_holder_object_.getMock()),
          ptzf_message_if_mock_handler_object_(),
          ptzf_message_if_mock_(ptzf_message_if_mock_handler_object_.getMock()),
          power_status_if_mock_holder_object_(),
          power_status_if_mock_(power_status_if_mock_holder_object_.getMock()),
          sequence_id_controller_mock_holder_object_(),
          sequence_id_controller_mock_(sequence_id_controller_mock_holder_object_.getMock()),
          mock_holder_preset_(),
          preset_status_if_mock_(mock_holder_preset_.getMock()),
          mock_holder_video_(),
          video_status_if_mock_(mock_holder_video_.getMock()),
          mock_holder_ptz_trace_status_if_(),
          ptz_trace_status_if_mock_(mock_holder_ptz_trace_status_if_.getMock()),
          menu_status_holder_object_(),
          menu_status_mock_(menu_status_holder_object_.getMock()),
          mock_holder_object_menu_camera_osd_(),
          menu_camera_osd_status_mock_(mock_holder_object_menu_camera_osd_.getMock())
    {}

    virtual void SetUp()
    {
        common::MessageQueue mq("ConfigReadyMQ");
        config::ConfigReadyNotification message;
        mq.post(message);

        // EventRouterIf, PtzfStatus*2
        EXPECT_CALL(er_mock_, create(_)).WillRepeatedly(Return());
        EXPECT_CALL(er_mock_, setDestination(_, _)).Times(1).WillOnce(Return());

        PtzfStatus st;
        handler_.reset(new PtzfControllerMessageHandler);
        st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);
        st.setSlowMode(false);
        st.setSpeedStep(PAN_TILT_SPEED_STEP_NORMAL);
        st.setImageFlipConfigurationStatus(false);
        st.setPanTiltSlowModeConfigurationStatus(false);
        st.setPanTiltSpeedStepConfigurationStatus(false);
        st.setRampCurve(RAMP_CURVE_MODE2);
        st.setPanTiltMotorPower(PAN_TILT_MOTOR_POWER_NORMAL);
        st.setPanReverse(false);
        st.setTiltReverse(false);
        st.setTeleShiftMode(false);
    }

    virtual void TearDown()
    {
        common::MessageQueue pm_mq(PtzfControllerMQ::getName());
        pm_mq.unlink();
        common::MessageQueue er_mq("EventRouterMQ");
        er_mq.unlink();
        common::MessageQueue config_mq("ConfigReadyMQ");
        config_mq.unlink();
        reply_.unlink();
        handler_.reset();
        thread_mq_.unlink();
    }

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

    void setDefaultValidCondition(const u16_t cardinality)
    {
        EXPECT_CALL(visca_status_if_mock_, isHandlingIfclearCommand()).Times(cardinality).WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(cardinality).WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(power_status_if_mock_, getPowerStatus())
            .Times(cardinality)
            .WillRepeatedly(Return(power::PowerStatus::POWER_ON));
        EXPECT_CALL(ptz_trace_status_if_mock_, getTraceCondition())
            .WillRepeatedly(Return(ptzf::PTZ_TRACE_CONDITION_IDLE));
        EXPECT_CALL(menu_status_mock_, isDisplayOn()).Times(cardinality).WillRepeatedly(Return(false));
        EXPECT_CALL(menu_camera_osd_status_mock_, isDisplayOn()).Times(cardinality).WillRepeatedly(Return(false));
        EXPECT_CALL(preset_status_if_mock_, isDoingPresetRecall()).Times(cardinality).WillRepeatedly(Return(false));
        EXPECT_CALL(video_status_if_mock_, isConfiguringHdmiVideoFormat())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
    }

protected:
    bizglobal::BizGlobalModelNameCreater biz_model_;
    MockHolderObject<event_router::EventRouterIfMock> event_router_mock_holder_;
    MockHolderObject<visca::ViscaServerMessageIfMock> visca_if_mock_holder_;
    MockHolderObject<visca::ViscaStatusIfMock> visca_status_if_mock_holder_;
    MockHolderObject<preset::PresetManagerMessageIfMock> preset_if_mock_holder_;
    MockHolderObject<PtzTraceIfMock> ptz_trace_if_mock_holder_;
    MockHolderObject<PtzfControllerMock> controller_mock_holder_;
    MockHolderObject<preset::DatabaseInitializeIfMock> database_initialize_if_mock_holder_;
    event_router::EventRouterIfMock& er_mock_;
    visca::ViscaServerMessageIfMock& visca_if_mock_;
    visca::ViscaStatusIfMock& visca_status_if_mock_;
    preset::PresetManagerMessageIfMock& preset_if_mock_;
    PtzTraceIfMock& ptz_trace_if_mock_;
    preset::DatabaseInitializeIfMock& database_initialize_if_mock_;
    gtl::AutoPtr<PtzfControllerMessageHandler> handler_;
    common::MessageQueue reply_;
    common::MessageQueue thread_mq_;
    PtzfControllerMock& controller_mock_;
    MockHolderObject<PtzfControllerFinalizerMock> finalizre_mock_holder_object_;
    PtzfControllerFinalizerMock& finalizre_mock_;
    MockHolderObject<infra::PtzfBackupInfraIfMock> backup_infra_if_mock_holder_object_;
    infra::PtzfBackupInfraIfMock& backup_infra_if_mock_;
    MockHolderObject<infra::PtzfPanTiltInfraIfMock> pan_tilt_infra_if_mock_holder_object_;
    infra::PtzfPanTiltInfraIfMock& pan_tilt_infra_if_mock_;
    MockHolderObject<infra::PtzfZoomInfraIfMock> zoom_infra_if_mock_holder_object_;
    infra::PtzfZoomInfraIfMock& zoom_infra_if_mock_;
    MockHolderObject<infra::PtzfFocusInfraIfMock> focus_infra_if_mock_holder_object_;
    infra::PtzfFocusInfraIfMock& focus_infra_if_mock_;
    MockHolderObject<infra::PtzfIfClearInfraIfMock> if_clear_infra_if_mock_holder_object_;
    infra::PtzfIfClearInfraIfMock& if_clear_infra_if_mock_;
    MockHolderObject<infra::PtzfPanTiltLockInfraIfMock> pan_tilt_lock_infra_if_mock_holder_object_;
    infra::PtzfPanTiltLockInfraIfMock& pan_tilt_lock_infra_if_mock_;
    MockHolderObject<infra::PtzfInitializeInfraIfMock> initialize_infra_if_mock_holder_object_;
    infra::PtzfInitializeInfraIfMock& initialize_infra_if_mock_;
    MockHolderObject<infra::PtMiconPowerInfraIfMock> pt_micon_power_infra_if_mock_holder_object_;
    infra::PtMiconPowerInfraIfMock& pt_micon_power_infra_if_mock_;
    MockHolderObject<infra::PtzfFinalizeInfraIfMock> finalize_infra_if_mock_holder_object_;
    infra::PtzfFinalizeInfraIfMock& finalize_infra_if_mock_;
    MockHolderObject<infra::PtzfStatusInfraIfMock> ptzf_status_infra_if_mock_holder_object_;
    infra::PtzfStatusInfraIfMock& ptzf_status_infra_if_mock_;
    MockHolderObject<PtzfMessageIfMock> ptzf_message_if_mock_handler_object_;
    PtzfMessageIfMock& ptzf_message_if_mock_;
    MockHolderObject<power::PowerStatusIfMock> power_status_if_mock_holder_object_;
    power::PowerStatusIfMock& power_status_if_mock_;
    MockHolderObject<infra::SequenceIdControllerMock> sequence_id_controller_mock_holder_object_;
    infra::SequenceIdControllerMock& sequence_id_controller_mock_;
    MockHolderObject<preset::PresetStatusIfMock> mock_holder_preset_;
    preset::PresetStatusIfMock& preset_status_if_mock_;
    MockHolderObject<video::VideoStatusIfMock> mock_holder_video_;
    video::VideoStatusIfMock& video_status_if_mock_;
    MockHolderObject<ptzf::PtzTraceStatusIfMock> mock_holder_ptz_trace_status_if_;
    ptzf::PtzTraceStatusIfMock& ptz_trace_status_if_mock_;
    MockHolderObject<ui::MenuStatusMock> menu_status_holder_object_;
    ui::MenuStatusMock& menu_status_mock_;
    MockHolderObject<camera_osd::CameraOsdStatusIfMock> mock_holder_object_menu_camera_osd_;
    camera_osd::CameraOsdStatusIfMock& menu_camera_osd_status_mock_;
};

MATCHER_P(EqNotifyMqName, mq_name, "")
{
    std::string mq_name_str = mq_name.name;
    if (gtl::isStringEqual(arg.name, mq_name_str)) {
        return true;
    }
    return false;
}
TEST_F(PtzfControllerMessageHandlerTest, PowerOn)
{
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getThreadStatus(_))
        .Times(1)
        .WillRepeatedly(testing::Invoke([](const common::MessageQueueName& reply_name) {
            common::MessageQueue local_mq(reply_name.name);
            infra::PanTiltLockPollingThreadStatusResult lock_thread_status;
            lock_thread_status.suppress_mode = false;
            local_mq.post(lock_thread_status);
        }));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, startPollingLockStatus(_))
        .Times(1)
        .WillRepeatedly(testing::Invoke([](const common::MessageQueueName& reply_name) {
            common::MessageQueue local_mq(reply_name.name);
            ptzf::message::PtzfExecComp polling_lock_status_result;
            local_mq.post(polling_lock_status_result);
        }));
    EXPECT_CALL(initialize_infra_if_mock_, setPowerOnSequenceStatus(true, _))
        .Times(1)
        .WillRepeatedly(testing::Invoke([](testing::Unused, const common::MessageQueueName& reply_name) {
            common::MessageQueue local_mq(reply_name.name);
            ptzf::message::PtzfExecComp result;
            local_mq.post(result);
            return true;
        }));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(true, _))
        .Times(1)
        .WillRepeatedly(testing::Invoke([](testing::Unused, const common::MessageQueueName& reply_name) {
            common::MessageQueue local_mq(reply_name.name);
            ptzf::message::PtzfExecComp result;
            local_mq.post(result);
        }));
    EXPECT_CALL(visca_if_mock_, sendPowerOnPanTiltRequest()).Times(1).WillOnce(Return());

    PowerOn msg;
    handler_->handleRequest(msg);
}

TEST_F(PtzfControllerMessageHandlerTest, PowerOnResult)
{
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(false, _))
        .Times(1)
        .WillRepeatedly(testing::Invoke([](testing::Unused, const common::MessageQueueName& reply_name) {
            common::MessageQueue local_mq(reply_name.name);
            ptzf::message::PtzfExecComp result;
            local_mq.post(result);
        }));
    EXPECT_CALL(initialize_infra_if_mock_, setPowerOnSequenceStatus(false, _))
        .Times(1)
        .WillRepeatedly(testing::Invoke([](testing::Unused, const common::MessageQueueName& reply_name) {
            common::MessageQueue local_mq(reply_name.name);
            ptzf::message::PtzfExecComp result;
            local_mq.post(result);
            return true;
        }));
    //    EXPECT_CALL(pt_micon_power_infra_if_mock_, completePtMiconBoot(false)).Times(1).WillOnce(Return());

    PowerOnResult msg;
    msg.result_power_on = true;
    handler_->handleRequest(msg);
}

TEST_F(PtzfControllerMessageHandlerTest, PowerOff)
{
    common::MessageQueue mq_;

    PtzfStatusIf status;

    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(true), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(true), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_UNLOCKED), Return(true)));
    EXPECT_CALL(visca_if_mock_, sendPowerOffPanTiltRequest()).Times(1).WillOnce(Return());
    EXPECT_CALL(ptzf_message_if_mock_, noticePowerOffResult(_)).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingIfclearCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(2).WillRepeatedly(Return(power::PowerStatus::POWER_OFF));
    PowerOff msg;
    handler_->handleRequest(msg);

    EXPECT_EQ(U32_T(0), status.getPanTiltStatus());

    SetPanTiltAbsolutePositionRequest biz_mes_pantilt;
    biz_mes_pantilt.seq_id = U32_T(1);
    biz_mes_pantilt.mq_name = mq_.getName();
    biz_mes_pantilt.pan_speed = U8_T(0x18);
    biz_mes_pantilt.tilt_speed = U8_T(0x17);
    biz_mes_pantilt.pan_position = S32_T(0x1234);
    biz_mes_pantilt.tilt_position = S32_T(0x1234);

    SetZoomAbsolutePositionRequest biz_msg_zoom;
    biz_msg_zoom.seq_id = U32_T(123456);
    biz_msg_zoom.mq_name = mq_.getName();
    biz_msg_zoom.position = U16_T(0x4000);
    ptzf::message::PtzfExecComp result;

    handler_->handleRequest(biz_mes_pantilt);
    mq_.pend(result);
    EXPECT_EQ(biz_mes_pantilt.seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.error);

    handler_->handleRequest(biz_msg_zoom);
    mq_.pend(result);
    EXPECT_EQ(biz_msg_zoom.seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.error);

    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(true), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(true), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_UNLOCKED), Return(true)));
    EXPECT_CALL(visca_if_mock_, sendPowerOffPanTiltRequest()).Times(1).WillOnce(Return());
    EXPECT_CALL(ptzf_message_if_mock_, noticePowerOffResult(_)).Times(0);
    handler_->handleRequest(msg);

    EXPECT_EQ(U32_T(0), status.getPanTiltStatus());
}

TEST_F(PtzfControllerMessageHandlerTest, PowerOffWithPtLocked)
{
    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(true), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(true), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_LOCKED), Return(true)));
    EXPECT_CALL(visca_if_mock_, sendPowerOffPanTiltRequest()).Times(0);
    EXPECT_CALL(ptzf_message_if_mock_, noticePowerOffResult(Eq(true))).Times(1).WillOnce(Return());

    PowerOff msg;
    handler_->handleRequest(msg);
}

TEST_F(PtzfControllerMessageHandlerTest, PowerOffWithPtUnlockedAfterBooting)
{
    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(true), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(true), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING), Return(true)));
    EXPECT_CALL(visca_if_mock_, sendPowerOffPanTiltRequest()).Times(1).WillOnce(Return());
    EXPECT_CALL(ptzf_message_if_mock_, noticePowerOffResult(_)).Times(0);

    PowerOff msg;
    handler_->handleRequest(msg);
}

TEST_F(PtzfControllerMessageHandlerTest, PowerOffResultWithPtUnlocked)
{
    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(1).WillOnce(Return());
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_UNLOCKED), Return(true)));
    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(_)).Times(0);
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(false), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(false), _)).Times(1).WillOnce(Return(true));

    PowerOffResult msg;
    msg.result_power_off = true;
    handler_->handleRequest(msg);
}

TEST_F(PtzfControllerMessageHandlerTest, PowerOffResultWithPtLocked)
{
    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(1).WillOnce(Return());
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_LOCKED), Return(true)));
    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(_)).Times(0);
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(false), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(false), _)).Times(1).WillOnce(Return(true));

    PowerOffResult msg;
    msg.result_power_off = true;
    handler_->handleRequest(msg);
}

TEST_F(PtzfControllerMessageHandlerTest, PowerOffResultWithPtUnlockedAfterBooting)
{
    EXPECT_CALL(er_mock_, post(_, _, _, _)).Times(1).WillOnce(Return());
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING), Return(true)));
    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(PAN_TILT_LOCK_STATUS_UNLOCKED))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(false), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(false), _)).Times(1).WillOnce(Return(true));

    PowerOffResult msg;
    msg.result_power_off = true;
    handler_->handleRequest(msg);
}

TEST_F(PtzfControllerMessageHandlerTest, RampCurveSuccess)
{
    // ### for Visca ### //
    common::MessageQueue mq;
    visca::ViscaMessageSequence<SetRampCurveRequest> msg;
    msg.packet_id = U32_T(123456789);
    msg().mode = RAMP_CURVE_MODE1;

    setDefaultValidCondition(U16_T(4));
    EXPECT_CALL(pan_tilt_infra_if_mock_, setRampCurve(Eq(msg().mode), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetRampCurveRequest>>(msg, mq.getName());

    SetRampCurveReply reply;
    reply.status = ERRORCODE_SUCCESS;
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(msg.packet_id), Eq(reply.status))).Times(1).WillOnce(Return());
    handler_->handleRequest(reply);
    PtzfStatusIf status;
    EXPECT_EQ(msg().mode, status.getPanTiltRampCurve());
    mq.unlink();

    // ### for UI(Sircs) ### //
    common::MessageQueue mq_;
    SetRampCurveRequest ui_msg;
    ui_msg.mode = RAMP_CURVE_MODE2;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setRampCurve(Eq(ui_msg.mode), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<SetRampCurveRequest>(ui_msg, mq_.getName());

    // Reply
    reply.status = ERRORCODE_SUCCESS;
    handler_->handleRequest(reply);

    SetRampCurveResult result;
    mq_.pend(result);
    EXPECT_EQ(ERRORCODE_SUCCESS, result.err);

    // PanTiltRampCurve is set
    EXPECT_EQ(ui_msg.mode, status.getPanTiltRampCurve());

    // ### for Biz(1Way) ### //
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    BizMessage<SetRampCurveRequest> biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way().mode = RAMP_CURVE_MODE3;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setRampCurve(Eq(biz_msg_1way().mode), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest<BizMessage<SetRampCurveRequest>>(biz_msg_1way);

    // Reply
    reply.status = ERRORCODE_SUCCESS;
    handler_->handleRequest(reply);

    // PanTiltRampCurve is set
    EXPECT_EQ(biz_msg_1way().mode, status.getPanTiltRampCurve());

    // ### for Biz(2Way) ### //
    BizMessage<SetRampCurveRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().mode = RAMP_CURVE_MODE1;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setRampCurve(Eq(biz_msg_2way().mode), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest<BizMessage<SetRampCurveRequest>>(biz_msg_2way);

    // Reply
    reply.status = ERRORCODE_SUCCESS;
    handler_->handleRequest(reply);

    ptzf::message::PtzfExecComp biz_result;
    mq_.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(reply.status, biz_result.error);

    // PanTiltRampCurve is set
    EXPECT_EQ(biz_msg_2way().mode, status.getPanTiltRampCurve());
    mq_.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, RampCurveError)
{
    // ### for Visca ### //
    common::MessageQueue mq;
    visca::ViscaMessageSequence<SetRampCurveRequest> msg;
    msg.packet_id = U32_T(123456789);
    msg().mode = RAMP_CURVE_MODE1;

    setDefaultValidCondition(U16_T(4));

    EXPECT_CALL(pan_tilt_infra_if_mock_, setRampCurve(Eq(msg().mode), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetRampCurveRequest>>(msg, mq.getName());

    SetRampCurveReply reply;
    reply.status = ERRORCODE_EXEC;
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(msg.packet_id), Eq(reply.status))).Times(1).WillOnce(Return());
    handler_->handleRequest(reply);
    PtzfStatusIf status;
    EXPECT_NE(RAMP_CURVE_MODE1, status.getPanTiltRampCurve());
    mq.unlink();

    // ### for UI(Sircs) ### //
    common::MessageQueue mq_;
    SetRampCurveRequest ui_msg;
    ui_msg.mode = RAMP_CURVE_MODE1;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setRampCurve(Eq(ui_msg.mode), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<SetRampCurveRequest>(ui_msg, mq_.getName());

    // Visca Error Comp Reply
    reply.status = ERRORCODE_EXEC;
    handler_->handleRequest(reply);

    SetRampCurveResult result;
    mq_.pend(result);
    EXPECT_EQ(reply.status, result.err);

    // PanTiltRampCurve is not set
    EXPECT_NE(ui_msg.mode, status.getPanTiltRampCurve());

    // ### for Biz(1Way) ### //
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    BizMessage<SetRampCurveRequest> biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way().mode = RAMP_CURVE_MODE1;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setRampCurve(Eq(biz_msg_1way().mode), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest<BizMessage<SetRampCurveRequest>>(biz_msg_1way);

    // Visca Error Comp Reply
    reply.status = ERRORCODE_EXEC;
    handler_->handleRequest(reply);

    // PanTiltRampCurve is not set
    EXPECT_NE(biz_msg_1way().mode, status.getPanTiltRampCurve());

    // ### for Biz(2Way) ### //
    BizMessage<SetRampCurveRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().mode = RAMP_CURVE_MODE1;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setRampCurve(Eq(biz_msg_2way().mode), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest<BizMessage<SetRampCurveRequest>>(biz_msg_2way);

    // Visca Error Comp Reply
    reply.status = ERRORCODE_EXEC;
    handler_->handleRequest(reply);

    ptzf::message::PtzfExecComp biz_result;
    mq_.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(reply.status, biz_result.error);

    // PanTiltRampCurve is not set
    EXPECT_NE(biz_msg_2way().mode, status.getPanTiltRampCurve());
    mq.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, RumpCurve2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    BizMessage<SetRampCurveRequest> biz_msg1;
    BizMessage<SetRampCurveRequest> biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1().mode = RAMP_CURVE_MODE1;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2().mode = RAMP_CURVE_MODE2;

    setDefaultValidCondition(U16_T(2));

    EXPECT_CALL(pan_tilt_infra_if_mock_, setRampCurve(Eq(biz_msg1().mode), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest<BizMessage<SetRampCurveRequest>>(biz_msg1);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setRampCurve(Eq(biz_msg2().mode), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest<BizMessage<SetRampCurveRequest>>(biz_msg2);

    // Reply
    SetRampCurveReply reply;
    reply.status = ERRORCODE_SUCCESS;
    handler_->handleRequest(reply);
    handler_->handleRequest(reply);

    ptzf::message::PtzfExecComp biz_result1;
    mq1.pend(biz_result1);
    EXPECT_EQ(biz_msg1.seq_id, biz_result1.seq_id);
    EXPECT_EQ(reply.status, biz_result1.error);

    ptzf::message::PtzfExecComp biz_result2;
    mq2.pend(biz_result2);
    EXPECT_EQ(biz_msg2.seq_id, biz_result2.seq_id);
    EXPECT_EQ(reply.status, biz_result2.error);

    // PanTiltRampCurve is set
    PtzfStatusIf status;
    EXPECT_EQ(biz_msg2().mode, status.getPanTiltRampCurve());

    mq1.unlink();
    mq2.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltMotorPowerSuccess)
{
    // ### for Biz(1Way) ### //
    SetPanTiltMotorPowerReply reply;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    BizMessage<SetPanTiltMotorPowerRequest> biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way().motor_power = PAN_TILT_MOTOR_POWER_LOW;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltMotorPower(Eq(biz_msg_1way().motor_power), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest<BizMessage<SetPanTiltMotorPowerRequest>>(biz_msg_1way);

    // Reply
    reply.status = ERRORCODE_SUCCESS;
    handler_->handleRequest(reply);

    // PanTiltMotorPower is set
    PtzfStatusIf status;
    EXPECT_EQ(biz_msg_1way().motor_power, status.getPanTiltMotorPower());

    // ### for Biz(2Way) ### //
    common::MessageQueue mq;
    BizMessage<SetPanTiltMotorPowerRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq.getName();
    biz_msg_2way().motor_power = PAN_TILT_MOTOR_POWER_NORMAL;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltMotorPower(Eq(biz_msg_2way().motor_power), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest<BizMessage<SetPanTiltMotorPowerRequest>>(biz_msg_2way);

    // Reply
    reply.status = ERRORCODE_SUCCESS;
    handler_->handleRequest(reply);

    ptzf::message::PtzfExecComp biz_result;
    mq.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(reply.status, biz_result.error);

    // PanTiltMotorPower is set
    EXPECT_EQ(biz_msg_2way().motor_power, status.getPanTiltMotorPower());
    mq.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltMotorPowerError)
{
    // ### for Biz(1Way) ### //
    SetPanTiltMotorPowerReply reply;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    BizMessage<SetPanTiltMotorPowerRequest> biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way().motor_power = PAN_TILT_MOTOR_POWER_LOW;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltMotorPower(Eq(biz_msg_1way().motor_power), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest<BizMessage<SetPanTiltMotorPowerRequest>>(biz_msg_1way);

    // Visca Error Comp Reply
    reply.status = ERRORCODE_EXEC;
    handler_->handleRequest(reply);

    // PanTiltMotorPower is not set
    PtzfStatusIf status;
    EXPECT_NE(biz_msg_1way().motor_power, status.getPanTiltMotorPower());

    // ### for Biz(2Way) ### //
    common::MessageQueue mq;
    BizMessage<SetPanTiltMotorPowerRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq.getName();
    biz_msg_2way().motor_power = PAN_TILT_MOTOR_POWER_LOW;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltMotorPower(Eq(biz_msg_2way().motor_power), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest<BizMessage<SetPanTiltMotorPowerRequest>>(biz_msg_2way);

    // Visca Error Comp Reply
    reply.status = ERRORCODE_EXEC;
    handler_->handleRequest(reply);

    ptzf::message::PtzfExecComp biz_result;
    mq.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(reply.status, biz_result.error);

    // PanTiltMotorPower is not set
    EXPECT_NE(biz_msg_2way().motor_power, status.getPanTiltMotorPower());
    mq.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltMotorPower2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    BizMessage<SetPanTiltMotorPowerRequest> biz_msg1;
    BizMessage<SetPanTiltMotorPowerRequest> biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1().motor_power = PAN_TILT_MOTOR_POWER_NORMAL;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2().motor_power = PAN_TILT_MOTOR_POWER_LOW;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltMotorPower(Eq(biz_msg1().motor_power), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest<BizMessage<SetPanTiltMotorPowerRequest>>(biz_msg1);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltMotorPower(Eq(biz_msg2().motor_power), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest<BizMessage<SetPanTiltMotorPowerRequest>>(biz_msg2);

    // Reply
    SetPanTiltMotorPowerReply reply;
    reply.status = ERRORCODE_SUCCESS;
    handler_->handleRequest(reply);
    handler_->handleRequest(reply);

    ptzf::message::PtzfExecComp biz_result1;
    mq1.pend(biz_result1);
    EXPECT_EQ(biz_msg1.seq_id, biz_result1.seq_id);
    EXPECT_EQ(reply.status, biz_result1.error);

    ptzf::message::PtzfExecComp biz_result2;
    mq2.pend(biz_result2);
    EXPECT_EQ(biz_msg2.seq_id, biz_result2.seq_id);
    EXPECT_EQ(reply.status, biz_result2.error);

    // PanTiltMotorPower is set
    PtzfStatusIf status;
    EXPECT_EQ(biz_msg2().motor_power, status.getPanTiltMotorPower());

    mq1.unlink();
    mq2.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, SlowModeSuccess)
{
    common::MessageQueue mq_;
    PtzfStatus st;
    PtzfStatusIf st_if;
    // is No Chang
    st.setPanTiltSlowModeConfigurationStatus(false);

    // ### for UI(Sircs) ### //
    SetPanTiltSlowModeRequest msg;

    // Set Parameter of SlowMode
    msg.enable = true;
    st.setSlowMode(!msg.enable);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSlow(Eq(msg.enable), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequestWithReply<SetPanTiltSlowModeRequest>(msg, mq_.getName());
    common::Task::msleep(U32_T(1));

    // Reply
    SetPanTiltSlowReply comp_req;
    comp_req.status = ERRORCODE_SUCCESS;
    thread_mq_.post(comp_req);
    common::Task::msleep(U32_T(2));

    SetPanTiltSlowModeResult result(comp_req.status);
    mq_.pend(result);
    EXPECT_EQ(ERRORCODE_SUCCESS, result.err);

    // SlowMode is set
    EXPECT_EQ(msg.enable, st_if.getPanTiltSlowMode());

    // ### for Visca ### //
    visca::ViscaMessageSequence<SetPanTiltSlowModeRequest> visca_msg;

    // Set Parameter of SlowMode
    visca_msg.packet_id = U32_T(123456789);
    visca_msg().enable = false;
    st.setSlowMode(!visca_msg().enable);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSlow(Eq(visca_msg().enable), _, _)).Times(1).WillOnce(Return());

    // send Success Comp
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(visca_msg.packet_id), Eq(ERRORCODE_SUCCESS)))
        .Times(1)
        .WillOnce(Return());

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetPanTiltSlowModeRequest>>(visca_msg, mq_.getName());
    common::Task::msleep(U32_T(1));

    // Reply Ack
    visca::AckResponse ack_res;
    mq_.pend(ack_res);
    EXPECT_EQ(ERRORCODE_SUCCESS, ack_res.status);

    // Reply
    SetPanTiltSlowReply comp_result;
    comp_result.status = ERRORCODE_SUCCESS;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(10));

    // SlowMode is set
    EXPECT_EQ(visca_msg().enable, st_if.getPanTiltSlowMode());

    // ### for Biz(1Way) ### //
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    BizMessage<SetPanTiltSlowModeRequest> biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way().enable = true;
    st.setSlowMode(!biz_msg_1way().enable);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSlow(Eq(biz_msg_1way().enable), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest<BizMessage<SetPanTiltSlowModeRequest>>(biz_msg_1way);
    common::Task::msleep(U32_T(1));

    // Reply
    comp_result.status = ERRORCODE_SUCCESS;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(10));

    // SlowMode is set
    EXPECT_EQ(biz_msg_1way().enable, st_if.getPanTiltSlowMode());

    // ### for Biz(2Way) ### //
    BizMessage<SetPanTiltSlowModeRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().enable = false;
    st.setSlowMode(!biz_msg_2way().enable);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSlow(Eq(biz_msg_2way().enable), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest<BizMessage<SetPanTiltSlowModeRequest>>(biz_msg_2way);
    common::Task::msleep(U32_T(1));

    // Reply
    comp_result.status = ERRORCODE_SUCCESS;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(10));

    ptzf::message::PtzfExecComp biz_result;
    mq_.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(comp_result.status, biz_result.error);

    // SlowMode is set
    EXPECT_EQ(biz_msg_2way().enable, st_if.getPanTiltSlowMode());
    mq_.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, SlowModeIsChanging)
{
    common::MessageQueue mq_;
    PtzfStatus st;
    PtzfStatusIf st_if;
    // is changing
    st.setPanTiltSlowModeConfigurationStatus(true);

    // ### for UI(Sircs) ### //
    SetPanTiltSlowModeRequest msg;

    // Set Parameter of SlowMode
    msg.enable = true;
    st.setSlowMode(!msg.enable);

    handler_->handleRequestWithReply<SetPanTiltSlowModeRequest>(msg, mq_.getName());
    common::Task::msleep(U32_T(1));

    // Error Reply
    SetPanTiltSlowModeResult err_res;
    mq_.pend(err_res);
    EXPECT_EQ(ERRORCODE_EXEC, err_res.err);

    // SlowMode is not set
    EXPECT_EQ(!msg.enable, st_if.getPanTiltSlowMode());

    // ### for Visca ### //
    visca::ViscaMessageSequence<SetPanTiltSlowModeRequest> visca_msg;

    // Set Parameter of SlowMode
    visca_msg.packet_id = U32_T(123456789);
    visca_msg().enable = false;
    st.setSlowMode(!visca_msg().enable);

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetPanTiltSlowModeRequest>>(visca_msg, mq_.getName());
    common::Task::msleep(U32_T(1));

    // Error Reply
    visca::AckResponse err_ack;
    mq_.pend(err_ack);
    EXPECT_EQ(ERRORCODE_EXEC, err_ack.status);

    // SlowMode is not set
    EXPECT_EQ(!visca_msg().enable, st_if.getPanTiltSlowMode());

    // ### for Biz(1Way) ### //
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    BizMessage<SetPanTiltSlowModeRequest> biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way().enable = false;
    st.setSlowMode(!biz_msg_1way().enable);

    handler_->handleRequest<BizMessage<SetPanTiltSlowModeRequest>>(biz_msg_1way);

    // SlowMode is not set
    EXPECT_NE(biz_msg_1way().enable, st_if.getPanTiltSlowMode());

    // ### for Biz(2Way) ### //
    BizMessage<SetPanTiltSlowModeRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().enable = true;
    st.setSlowMode(!biz_msg_2way().enable);

    handler_->handleRequest<BizMessage<SetPanTiltSlowModeRequest>>(biz_msg_2way);

    ptzf::message::PtzfExecComp biz_result;
    mq_.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, biz_result.error);

    // SlowMode is not set
    EXPECT_NE(biz_msg_2way().enable, st_if.getPanTiltSlowMode());
    mq_.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, SlowModeCommandError)
{
    common::MessageQueue mq_;
    PtzfStatus st;
    PtzfStatusIf st_if;
    // is No Chang
    st.setPanTiltSlowModeConfigurationStatus(false);

    // ### for UI(Sircs) ### //
    SetPanTiltSlowModeRequest msg;

    // Set Parameter of SlowMode
    msg.enable = true;
    st.setSlowMode(!msg.enable);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSlow(Eq(msg.enable), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequestWithReply<SetPanTiltSlowModeRequest>(msg, mq_.getName());
    common::Task::msleep(U32_T(1));

    // Visca Error Comp Reply
    SetPanTiltSlowReply comp_req;
    comp_req.status = ERRORCODE_EXEC;
    thread_mq_.post(comp_req);
    common::Task::msleep(U32_T(1));

    SetPanTiltSlowModeResult result(comp_req.status);
    mq_.pend(result);
    EXPECT_EQ(ERRORCODE_SUCCESS, result.err);

    // SlowMode is not set
    EXPECT_EQ(!msg.enable, st_if.getPanTiltSlowMode());

    // ### for Visca ### //
    visca::ViscaMessageSequence<SetPanTiltSlowModeRequest> visca_msg;

    // Set Parameter of SlowMode
    visca_msg.packet_id = U32_T(123456789);
    visca_msg().enable = false;
    st.setSlowMode(!visca_msg().enable);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSlow(Eq(visca_msg().enable), _, _)).Times(1).WillOnce(Return());

    // send Exec Error Comp
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(visca_msg.packet_id), Eq(ERRORCODE_EXEC)))
        .Times(1)
        .WillOnce(Return());

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetPanTiltSlowModeRequest>>(visca_msg, mq_.getName());
    common::Task::msleep(U32_T(1));

    // Reply Ack
    visca::AckResponse ack_res;
    mq_.pend(ack_res);
    EXPECT_EQ(ERRORCODE_SUCCESS, ack_res.status);

    // Visca Error Comp Reply
    SetPanTiltSlowReply comp_result;
    comp_result.status = ERRORCODE_EXEC;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(1));

    // SlowMode is not set
    EXPECT_EQ(!visca_msg().enable, st_if.getPanTiltSlowMode());

    // ### for Biz(1Way) ### //
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    BizMessage<SetPanTiltSlowModeRequest> biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way().enable = true;
    st.setSlowMode(!biz_msg_1way().enable);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSlow(Eq(biz_msg_1way().enable), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest<BizMessage<SetPanTiltSlowModeRequest>>(biz_msg_1way);
    common::Task::msleep(U32_T(1));

    // Visca Error Comp Reply
    comp_result.status = ERRORCODE_EXEC;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(1));

    // SlowMode is not set
    EXPECT_NE(biz_msg_1way().enable, st_if.getPanTiltSlowMode());

    // ### for Biz(2Way) ### //
    BizMessage<SetPanTiltSlowModeRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().enable = false;
    st.setSlowMode(!biz_msg_2way().enable);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSlow(Eq(biz_msg_2way().enable), _, _)).Times(1).WillOnce(Return());

    common::Task::msleep(U32_T(1));
    handler_->handleRequest<BizMessage<SetPanTiltSlowModeRequest>>(biz_msg_2way);
    common::Task::msleep(U32_T(1));

    // Visca Error Comp Reply
    comp_result.status = ERRORCODE_EXEC;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(1));

    ptzf::message::PtzfExecComp biz_result;
    mq_.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(comp_result.status, biz_result.error);

    // SlowMode is not set
    EXPECT_NE(biz_msg_2way().enable, st_if.getPanTiltSlowMode());
    mq_.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, SlowModeTimeout)
{
    common::MessageQueue mq_;
    PtzfStatus st;
    PtzfStatusIf st_if;
    // is No Chang
    st.setPanTiltSlowModeConfigurationStatus(false);

    // ### for UI(Sircs) ### //
    SetPanTiltSlowModeRequest msg;

    // Set Parameter of SlowMode
    msg.enable = true;
    st.setSlowMode(!msg.enable);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSlow(Eq(msg.enable), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequestWithReply(msg, mq_.getName());
    // Pseudo Timeouted
    common::Task::msleep(U32_T(100));

    // Timeout Error
    SetPanTiltSlowModeResult result;
    mq_.pend(result);
    EXPECT_EQ(ERRORCODE_SUCCESS, result.err);

    // SlowMode is not set
    EXPECT_EQ(!msg.enable, st_if.getPanTiltSlowMode());

    // ### for Visca ### //
    visca::ViscaMessageSequence<SetPanTiltSlowModeRequest> visca_msg;

    // Set Parameter of SlowMode
    visca_msg.packet_id = U32_T(123456789);
    visca_msg().enable = false;
    st.setSlowMode(!visca_msg().enable);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSlow(Eq(visca_msg().enable), _, _)).Times(1).WillOnce(Return());

    // send Timeout Error Comp
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(visca_msg.packet_id), Eq(ERRORCODE_EXEC)))
        .Times(1)
        .WillOnce(Return());

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetPanTiltSlowModeRequest>>(visca_msg, mq_.getName());
    // Pseudo Timeouted
    common::Task::msleep(U32_T(100));

    // Reply Ack
    visca::AckResponse ack_res;
    mq_.pend(ack_res);
    EXPECT_EQ(ERRORCODE_SUCCESS, ack_res.status);

    // SlowMode is not set
    EXPECT_EQ(!visca_msg().enable, st_if.getPanTiltSlowMode());

    // ### for Biz(1Way) ### //
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    BizMessage<SetPanTiltSlowModeRequest> biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way().enable = true;
    st.setSlowMode(!biz_msg_1way().enable);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSlow(Eq(biz_msg_1way().enable), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest<BizMessage<SetPanTiltSlowModeRequest>>(biz_msg_1way);
    // Pseudo Timeouted
    common::Task::msleep(U32_T(100));

    // SlowMode is not set
    EXPECT_NE(biz_msg_1way().enable, st_if.getPanTiltSlowMode());

    // ### for Biz(2Way) ### //
    BizMessage<SetPanTiltSlowModeRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().enable = false;
    st.setSlowMode(!biz_msg_2way().enable);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSlow(Eq(biz_msg_2way().enable), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest<BizMessage<SetPanTiltSlowModeRequest>>(biz_msg_2way);
    // Pseudo Timeouted
    common::Task::msleep(U32_T(100));

    ptzf::message::PtzfExecComp biz_result;
    mq_.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, biz_result.error);

    // SlowMode is not set
    EXPECT_NE(biz_msg_2way().enable, st_if.getPanTiltSlowMode());
    mq_.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, SlowMode2wayMultiple)
{
    PtzfStatus st;
    PtzfStatusIf st_if;
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    BizMessage<SetPanTiltSlowModeRequest> biz_msg1;
    BizMessage<SetPanTiltSlowModeRequest> biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1().enable = false;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2().enable = true;

    st.setSlowMode(!biz_msg1().enable);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSlow(Eq(biz_msg1().enable), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest<BizMessage<SetPanTiltSlowModeRequest>>(biz_msg1);
    handler_->handleRequest<BizMessage<SetPanTiltSlowModeRequest>>(biz_msg2);
    common::Task::msleep(U32_T(1));

    // Reply
    SetPanTiltSlowReply comp_result;
    comp_result.status = ERRORCODE_SUCCESS;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(10));

    ptzf::message::PtzfExecComp biz_result1;
    mq1.pend(biz_result1);
    EXPECT_EQ(biz_msg1.seq_id, biz_result1.seq_id);
    EXPECT_EQ(comp_result.status, biz_result1.error);

    ptzf::message::PtzfExecComp biz_result2;
    mq2.pend(biz_result2);
    EXPECT_EQ(biz_msg2.seq_id, biz_result2.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, biz_result2.error);

    // SlowMode is set
    EXPECT_EQ(biz_msg1().enable, st_if.getPanTiltSlowMode());

    mq1.unlink();
    mq2.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, PanReverseSuccess)
{
    // ### for Visca ### //
    common::MessageQueue mq;
    visca::ViscaMessageSequence<SetPanReverseRequest> msg;
    msg.packet_id = U32_T(123456789);
    msg().enable = true;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanReverse(Eq(msg().enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetPanReverseRequest>>(msg, mq.getName());

    SetPanReverseReply reply;
    reply.status = ERRORCODE_SUCCESS;
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(msg.packet_id), Eq(reply.status))).Times(1).WillOnce(Return());
    handler_->handleRequest(reply);
    PtzfStatusIf status;
    EXPECT_TRUE(status.getPanReverse());
    mq.unlink();

    // ### for UI(Sircs) ### //
    common::MessageQueue mq_;
    SetPanReverseRequest ui_msg;
    ui_msg.enable = false;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanReverse(Eq(ui_msg.enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<SetPanReverseRequest>(ui_msg, mq_.getName());

    // Reply
    reply.status = ERRORCODE_SUCCESS;
    handler_->handleRequest(reply);

    SetPanReverseResult result;
    mq_.pend(result);
    EXPECT_EQ(reply.status, result.err);

    // PanReverse is set
    EXPECT_EQ(ui_msg.enable, status.getPanReverse());
    mq.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, PanReverseError)
{
    // ### for Visca ### //
    common::MessageQueue mq;
    visca::ViscaMessageSequence<SetPanReverseRequest> msg;
    msg.packet_id = U32_T(123456789);
    msg().enable = true;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanReverse(Eq(msg().enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetPanReverseRequest>>(msg, mq.getName());

    SetPanReverseReply reply;
    reply.status = ERRORCODE_EXEC;
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(msg.packet_id), Eq(reply.status))).Times(1).WillOnce(Return());
    handler_->handleRequest(reply);

    PtzfStatusIf status;
    EXPECT_NE(msg().enable, status.getPanReverse());
    mq.unlink();

    // ### for UI(Sircs) ### //
    common::MessageQueue mq_;
    SetPanReverseRequest ui_msg;
    ui_msg.enable = true;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanReverse(Eq(ui_msg.enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<SetPanReverseRequest>(ui_msg, mq_.getName());

    // Visca Error Comp Reply
    reply.status = ERRORCODE_EXEC;
    handler_->handleRequest(reply);

    SetPanReverseResult result;
    mq_.pend(result);
    EXPECT_EQ(reply.status, result.err);

    // PanReverse is not set
    EXPECT_NE(ui_msg.enable, status.getPanReverse());
    mq_.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, TiltReverseSuccess)
{
    // ### for Visca ### //
    common::MessageQueue mq;
    visca::ViscaMessageSequence<SetTiltReverseRequest> msg;
    msg.packet_id = U32_T(123456789);
    msg().enable = true;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setTiltReverse(Eq(msg().enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetTiltReverseRequest>>(msg, mq.getName());

    SetTiltReverseReply reply;
    reply.status = ERRORCODE_SUCCESS;
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(msg.packet_id), Eq(reply.status))).Times(1).WillOnce(Return());
    handler_->handleRequest(reply);
    PtzfStatusIf status;
    EXPECT_TRUE(status.getTiltReverse());
    mq.unlink();

    // ### for UI(Sircs) ### //
    common::MessageQueue mq_;
    SetTiltReverseRequest ui_msg;
    ui_msg.enable = false;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setTiltReverse(Eq(ui_msg.enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<SetTiltReverseRequest>(ui_msg, mq_.getName());

    // Reply
    reply.status = ERRORCODE_SUCCESS;
    handler_->handleRequest(reply);

    SetTiltReverseResult result;
    mq_.pend(result);
    EXPECT_EQ(reply.status, result.err);

    // TiltReverse is set
    EXPECT_EQ(ui_msg.enable, status.getTiltReverse());
    mq.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, TiltReverseError)
{
    // ### for Visca ### //
    common::MessageQueue mq;
    visca::ViscaMessageSequence<SetTiltReverseRequest> msg;
    msg.packet_id = U32_T(123456789);
    msg().enable = true;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setTiltReverse(Eq(msg().enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetTiltReverseRequest>>(msg, mq.getName());

    SetTiltReverseReply reply;
    reply.status = ERRORCODE_EXEC;
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(msg.packet_id), Eq(reply.status))).Times(1).WillOnce(Return());
    handler_->handleRequest(reply);

    PtzfStatusIf status;
    EXPECT_NE(msg().enable, status.getTiltReverse());
    mq.unlink();

    // ### for UI(Sircs) ### //
    common::MessageQueue mq_;
    SetTiltReverseRequest ui_msg;
    ui_msg.enable = true;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setTiltReverse(Eq(ui_msg.enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<SetTiltReverseRequest>(ui_msg, mq_.getName());

    // Visca Error Comp Reply
    reply.status = ERRORCODE_EXEC;
    handler_->handleRequest(reply);

    SetTiltReverseResult result;
    mq_.pend(result);
    EXPECT_EQ(reply.status, result.err);

    // TiltReverse is not set
    EXPECT_NE(ui_msg.enable, status.getTiltReverse());
    mq_.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, IRCorrectionSuccess)
{
    const IRCorrection ir_corection[2] = { IR_CORRECTION_STANDARD, IR_CORRECTION_IRLIGHT };
    common::MessageQueue mq;
    PtzfStatus st;
    PtzfStatusIf st_if;

    // ### for Visca ### //
    ARRAY_FOREACH (ir_corection, i) {
        // is No Chang
        st.setIRCorrectionConfigurationStatus(false);

        visca::ViscaMessageSequence<SetIRCorrectionRequest> msg;
        msg.packet_id = U32_T(123456789);
        msg().ir_correction = ir_corection[i];

        EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(Eq(msg().ir_correction), _, _)).Times(1).WillOnce(Return());
        handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetIRCorrectionRequest>>(msg, mq.getName());
        common::Task::msleep(U32_T(1));

        SetIRCorrectionReply reply;
        reply.status = ERRORCODE_SUCCESS;
        EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(msg.packet_id), Eq(reply.status))).Times(1).WillOnce(Return());

        // Reply Ack
        visca::AckResponse ack_res;
        mq.pend(ack_res);
        EXPECT_EQ(ERRORCODE_SUCCESS, ack_res.status);

        // Comp Reply
        SetIRCorrectionReply comp_result;
        comp_result.status = ERRORCODE_SUCCESS;
        thread_mq_.post(comp_result);
        common::Task::msleep(U32_T(1));

        // IRCorrection is set
        PtzfStatusIf status;
        EXPECT_EQ(msg().ir_correction, status.getIRCorrection());
    }

    // ### for Others ### //
    ARRAY_FOREACH (ir_corection, i) {
        // is No Chang
        st.setIRCorrectionConfigurationStatus(false);

        SetIRCorrectionRequest request(ir_corection[i]);

        EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(Eq(request.ir_correction), _, _)).Times(1).WillOnce(Return());
        handler_->handleRequestWithReply<SetIRCorrectionRequest>(request, mq.getName());
        common::Task::msleep(U32_T(1));

        SetIRCorrectionReply reply;
        reply.status = ERRORCODE_SUCCESS;

        // Comp Reply
        SetIRCorrectionReply comp_result;
        comp_result.status = ERRORCODE_SUCCESS;
        thread_mq_.post(comp_result);
        common::Task::msleep(U32_T(1));

        SetIRCorrectionResult result;
        mq.pend(result);
        EXPECT_EQ(ERRORCODE_SUCCESS, result.err);

        // IRCorrection is set
        PtzfStatusIf status;
        EXPECT_EQ(request.ir_correction, status.getIRCorrection());
    }

    // ### for Biz(1Way) ### //
    ARRAY_FOREACH (ir_corection, i) {
        // is No Chang
        st.setIRCorrectionConfigurationStatus(false);

        BizMessage<SetIRCorrectionRequest> msg;
        msg.seq_id = U32_T(123456789);
        gtl::copyString(msg.mq_name.name, "");
        msg().ir_correction = ir_corection[i];

        EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(msg().ir_correction, _, _)).Times(1).WillOnce(Return());
        handler_->handleRequest<BizMessage<SetIRCorrectionRequest>>(msg);
        common::Task::msleep(U32_T(1));

        SetIRCorrectionReply reply;
        reply.status = ERRORCODE_SUCCESS;

        // Comp Reply
        SetIRCorrectionReply comp_result;
        comp_result.status = ERRORCODE_SUCCESS;
        thread_mq_.post(comp_result);
        waitQueueEmpty(thread_mq_);
        common::Task::msleep(U32_T(1));

        // IRCorrection is set
        PtzfStatusIf status;
        EXPECT_EQ(msg().ir_correction, status.getIRCorrection());
    }

    // ### for Biz(2Way) ### //
    ARRAY_FOREACH (ir_corection, i) {
        // is No Chang
        st.setIRCorrectionConfigurationStatus(false);

        BizMessage<SetIRCorrectionRequest> msg;
        msg.seq_id = U32_T(123456789);
        msg.mq_name = mq.getName();
        msg().ir_correction = ir_corection[i];

        EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(msg().ir_correction, _, _)).Times(1).WillOnce(Return());
        handler_->handleRequest<BizMessage<SetIRCorrectionRequest>>(msg);
        common::Task::msleep(U32_T(1));

        SetIRCorrectionReply reply;
        reply.status = ERRORCODE_SUCCESS;

        // Comp Reply
        SetIRCorrectionReply comp_result;
        comp_result.status = ERRORCODE_SUCCESS;
        thread_mq_.post(comp_result);
        waitQueueEmpty(thread_mq_);

        ptzf::message::PtzfExecComp result;
        mq.pend(result);
        EXPECT_EQ(msg.seq_id, result.seq_id);
        EXPECT_EQ(ERRORCODE_SUCCESS, result.error);

        // IRCorrection is set
        PtzfStatusIf status;
        EXPECT_EQ(msg().ir_correction, status.getIRCorrection());
    }

    mq.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, IRCorrectionIsChanging)
{
    common::MessageQueue mq_;
    PtzfStatus st;
    PtzfStatusIf st_if;

    // is changing
    st.setIRCorrectionConfigurationStatus(true);

    // ### for Visca ### //
    visca::ViscaMessageSequence<SetIRCorrectionRequest> msg;

    // Set Parameter of IRCorrection
    msg.packet_id = U32_T(123456789);
    msg().ir_correction = IR_CORRECTION_STANDARD;
    st.setIRCorrection(visca::IR_CORRECTION_IRLIGHT);

    EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(_, _, _)).WillRepeatedly(Return());

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetIRCorrectionRequest>>(msg, mq_.getName());
    common::Task::msleep(U32_T(1));

    // Error Reply
    visca::AckResponse err_ack;
    mq_.pend(err_ack);
    EXPECT_EQ(ERRORCODE_EXEC, err_ack.status);

    // IRCorrection is not set
    EXPECT_NE(msg().ir_correction, st_if.getIRCorrection());

    // is changing
    st.setIRCorrectionConfigurationStatus(true);

    // ### for Others ### //
    SetIRCorrectionRequest request;

    // Set Parameter of IRCorrection
    request.ir_correction = IR_CORRECTION_STANDARD;
    st.setIRCorrection(visca::IR_CORRECTION_IRLIGHT);

    handler_->handleRequestWithReply<SetIRCorrectionRequest>(request, mq_.getName());
    common::Task::msleep(U32_T(1));

    SetIRCorrectionReply reply;
    reply.status = ERRORCODE_SUCCESS;

    // Comp Reply
    SetIRCorrectionReply comp_result;
    comp_result.status = ERRORCODE_SUCCESS;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(1));

    SetIRCorrectionResult result;
    mq_.pend(result);
    EXPECT_EQ(ERRORCODE_SUCCESS, result.err);

    // IRCorrection is set
    PtzfStatusIf status;
    EXPECT_EQ(request.ir_correction, status.getIRCorrection());

    // is changing
    st.setIRCorrectionConfigurationStatus(true);

    // ### for Biz(1Way) ### //
    BizMessage<SetIRCorrectionRequest> biz_msg_1way;

    // Set Parameter of IRCorrection
    biz_msg_1way.seq_id = U32_T(123456);
    gtl::copyString(biz_msg_1way.mq_name.name, "");
    biz_msg_1way().ir_correction = IR_CORRECTION_STANDARD;
    st.setIRCorrection(visca::IR_CORRECTION_IRLIGHT);

    handler_->handleRequest<BizMessage<SetIRCorrectionRequest>>(biz_msg_1way);
    common::Task::msleep(U32_T(1));

    // IRCorrection is not set
    EXPECT_NE(biz_msg_1way().ir_correction, st_if.getIRCorrection());

    // is changing
    st.setIRCorrectionConfigurationStatus(true);

    // ### for Biz(2Way) ### //
    BizMessage<SetIRCorrectionRequest> biz_msg_2way;

    // Set Parameter of IRCorrection
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().ir_correction = IR_CORRECTION_STANDARD;
    st.setIRCorrection(visca::IR_CORRECTION_IRLIGHT);

    handler_->handleRequest<BizMessage<SetIRCorrectionRequest>>(biz_msg_2way);

    ptzf::message::PtzfExecComp biz_result;
    mq_.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, biz_result.error);

    // IRCorrection is not set
    EXPECT_NE(biz_msg_2way().ir_correction, st_if.getIRCorrection());

    mq_.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, IRCorrectionError)
{
    // ### Only Visca ### //
    common::MessageQueue mq_;
    PtzfStatus st;
    PtzfStatusIf st_if;

    // is No Chang
    st.setIRCorrectionConfigurationStatus(false);

    // ### for Visca ### //
    visca::ViscaMessageSequence<SetIRCorrectionRequest> msg;

    // Set Parameter of IRCorrection
    msg.packet_id = U32_T(123456789);
    msg().ir_correction = IR_CORRECTION_IRLIGHT;
    st.setIRCorrection(visca::IR_CORRECTION_STANDARD);

    EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(Eq(msg().ir_correction), _, _)).Times(1).WillOnce(Return());

    // send Exec Error Comp
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(msg.packet_id), Eq(ERRORCODE_EXEC))).Times(1).WillOnce(Return());

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetIRCorrectionRequest>>(msg, mq_.getName());
    common::Task::msleep(U32_T(1));

    // Reply Ack
    visca::AckResponse ack_res;
    mq_.pend(ack_res);
    EXPECT_EQ(ERRORCODE_SUCCESS, ack_res.status);

    // Error Comp Reply
    SetIRCorrectionReply comp_result;
    comp_result.status = ERRORCODE_EXEC;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(1));

    // IRCorrection is not set
    EXPECT_NE(msg().ir_correction, st_if.getIRCorrection());

    // ### for Others ### //
    SetIRCorrectionRequest request;

    // Set Parameter of IRCorrection
    request.ir_correction = IR_CORRECTION_IRLIGHT;
    st.setIRCorrection(visca::IR_CORRECTION_STANDARD);

    EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(Eq(request.ir_correction), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequestWithReply<SetIRCorrectionRequest>(request, mq_.getName());
    common::Task::msleep(U32_T(1));

    // Error Comp Reply
    comp_result.status = ERRORCODE_EXEC;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(1));

    SetIRCorrectionResult result;
    mq_.pend(result);
    EXPECT_EQ(ERRORCODE_EXEC, result.err);

    // IRCorrection is not set
    EXPECT_NE(msg().ir_correction, st_if.getIRCorrection());

    // ### for Biz(1Way) ### //
    BizMessage<SetIRCorrectionRequest> biz_msg_1way;

    // Set Parameter of IRCorrection
    biz_msg_1way.seq_id = U32_T(123456);
    gtl::copyString(biz_msg_1way.mq_name.name, "");
    biz_msg_1way().ir_correction = IR_CORRECTION_IRLIGHT;
    st.setIRCorrection(visca::IR_CORRECTION_STANDARD);

    EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(Eq(request.ir_correction), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest<BizMessage<SetIRCorrectionRequest>>(biz_msg_1way);
    common::Task::msleep(U32_T(1));

    // Error Comp Reply
    comp_result.status = ERRORCODE_EXEC;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(1));

    // IRCorrection is not set
    EXPECT_NE(biz_msg_1way().ir_correction, st_if.getIRCorrection());

    // ### for Biz(2Way) ### //
    BizMessage<SetIRCorrectionRequest> biz_msg_2way;

    // Set Parameter of IRCorrection
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().ir_correction = IR_CORRECTION_IRLIGHT;
    st.setIRCorrection(visca::IR_CORRECTION_STANDARD);

    EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(Eq(request.ir_correction), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest<BizMessage<SetIRCorrectionRequest>>(biz_msg_2way);

    // Error Comp Reply
    comp_result.status = ERRORCODE_EXEC;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(1));

    ptzf::message::PtzfExecComp biz_result;
    mq_.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(comp_result.status, biz_result.error);

    // IRCorrection is not set
    EXPECT_NE(biz_msg_2way().ir_correction, st_if.getIRCorrection());

    mq_.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, IRCorrectionTimeout)
{
    // ### Only Visca ### //
    common::MessageQueue mq_;
    PtzfStatus st;
    PtzfStatusIf st_if;

    // is No Chang
    st.setIRCorrectionConfigurationStatus(false);

    // ### for Visca ### //
    visca::ViscaMessageSequence<SetIRCorrectionRequest> msg;

    // Set Parameter of IRCorrection
    msg.packet_id = U32_T(123456789);
    msg().ir_correction = IR_CORRECTION_STANDARD;
    st.setIRCorrection(visca::IR_CORRECTION_IRLIGHT);

    EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(Eq(msg().ir_correction), _, _)).Times(1).WillOnce(Return());

    // send Timeout Error Comp
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(msg.packet_id), Eq(ERRORCODE_EXEC))).Times(1).WillOnce(Return());

    handler_->handleRequestWithReply(msg, mq_.getName());
    // Pseudo Timeouted
    common::Task::msleep(U32_T(100));

    // Reply Ack
    visca::AckResponse ack_res;
    mq_.pend(ack_res);
    EXPECT_EQ(ERRORCODE_SUCCESS, ack_res.status);

    // IRCorrection is not set
    EXPECT_NE(msg().ir_correction, st_if.getIRCorrection());
    // ConfiguringIRCorrection is false (not processing)
    EXPECT_EQ(false, st_if.isConfiguringIRCorrection());

    // ### for Others ### //
    SetIRCorrectionRequest request;

    // Set Parameter of IRCorrection
    request.ir_correction = IR_CORRECTION_STANDARD;
    st.setIRCorrection(visca::IR_CORRECTION_IRLIGHT);

    EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(Eq(request.ir_correction), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequestWithReply<SetIRCorrectionRequest>(request, mq_.getName());
    // Pseudo Timeouted
    common::Task::msleep(U32_T(100));

    SetIRCorrectionResult result;
    mq_.pend(result);
    EXPECT_EQ(ERRORCODE_EXEC, result.err);

    // IRCorrection is not set
    EXPECT_NE(msg().ir_correction, st_if.getIRCorrection());
    // ConfiguringIRCorrection is false (not processing)
    EXPECT_EQ(false, st_if.isConfiguringIRCorrection());

    // ### for Biz(1Way) ### //
    BizMessage<SetIRCorrectionRequest> biz_msg_1way;

    // Set Parameter of IRCorrection
    biz_msg_1way.seq_id = U32_T(123456);
    gtl::copyString(biz_msg_1way.mq_name.name, "");
    biz_msg_1way().ir_correction = IR_CORRECTION_STANDARD;
    st.setIRCorrection(visca::IR_CORRECTION_IRLIGHT);

    EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(Eq(biz_msg_1way().ir_correction), _, _))
        .Times(1)
        .WillOnce(Return());

    handler_->handleRequest<BizMessage<SetIRCorrectionRequest>>(biz_msg_1way);
    // Pseudo Timeouted
    common::Task::msleep(U32_T(100));

    // IRCorrection is not set
    EXPECT_NE(biz_msg_1way().ir_correction, st_if.getIRCorrection());
    // ConfiguringIRCorrection is false (not processing)
    EXPECT_EQ(false, st_if.isConfiguringIRCorrection());

    // ### for Biz(2Way) ### //
    BizMessage<SetIRCorrectionRequest> biz_msg_2way;

    // Set Parameter of IRCorrection
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().ir_correction = IR_CORRECTION_STANDARD;
    st.setIRCorrection(visca::IR_CORRECTION_IRLIGHT);

    EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(Eq(biz_msg_2way().ir_correction), _, _))
        .Times(1)
        .WillOnce(Return());

    handler_->handleRequest<BizMessage<SetIRCorrectionRequest>>(biz_msg_2way);
    // Pseudo Timeouted
    common::Task::msleep(U32_T(100));

    ptzf::message::PtzfExecComp biz_result;
    mq_.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, biz_result.error);

    // IRCorrection is not set
    EXPECT_NE(biz_msg_2way().ir_correction, st_if.getIRCorrection());
    // ConfiguringIRCorrection is false (not processing)
    EXPECT_EQ(false, st_if.isConfiguringIRCorrection());

    mq_.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, IRCorrection2wayMultiple)
{
    const IRCorrection ir_corection[2][2] = { { IR_CORRECTION_STANDARD, IR_CORRECTION_IRLIGHT },
                                              { IR_CORRECTION_IRLIGHT, IR_CORRECTION_STANDARD } };
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    PtzfStatus st;
    PtzfStatusIf st_if;

    ARRAY_FOREACH (ir_corection, i) {
        // is No Chang
        st.setIRCorrectionConfigurationStatus(false);

        BizMessage<SetIRCorrectionRequest> msg1;
        BizMessage<SetIRCorrectionRequest> msg2;

        msg1.seq_id = U32_T(123456);
        msg1.mq_name = mq1.getName();
        msg1().ir_correction = ir_corection[i][0];

        msg2.seq_id = U32_T(456789);
        msg2.mq_name = mq2.getName();
        msg2().ir_correction = ir_corection[i][1];

        EXPECT_CALL(focus_infra_if_mock_, setIRCorrection(msg1().ir_correction, _, _)).Times(1).WillOnce(Return());

        handler_->handleRequest<BizMessage<SetIRCorrectionRequest>>(msg1);
        handler_->handleRequest<BizMessage<SetIRCorrectionRequest>>(msg2);
        common::Task::msleep(U32_T(1));

        // Comp Reply
        SetIRCorrectionReply comp_result;
        comp_result.status = ERRORCODE_SUCCESS;
        thread_mq_.post(comp_result);
        waitQueueEmpty(thread_mq_);

        ptzf::message::PtzfExecComp result1;
        mq1.pend(result1);
        EXPECT_EQ(msg1.seq_id, result1.seq_id);
        EXPECT_EQ(ERRORCODE_SUCCESS, result1.error);

        ptzf::message::PtzfExecComp result2;
        mq2.pend(result2);
        EXPECT_EQ(msg2.seq_id, result2.seq_id);
        EXPECT_EQ(ERRORCODE_EXEC, result2.error);

        // IRCorrection is set
        EXPECT_EQ(msg1().ir_correction, st_if.getIRCorrection());
    }
}

TEST_F(PtzfControllerMessageHandlerTest, TeleShiftModeSuccess)
{
    // ### for Visca ### //
    common::MessageQueue mq;
    visca::ViscaMessageSequence<SetTeleShiftModeRequest> msg;
    msg.packet_id = U32_T(123456789);
    msg().enable = true;

    EXPECT_CALL(zoom_infra_if_mock_, setTeleShiftMode(Eq(msg().enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetTeleShiftModeRequest>>(msg, mq.getName());

    SetTeleShiftModeReply reply;
    reply.status = ERRORCODE_SUCCESS;
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(msg.packet_id), Eq(reply.status))).Times(1).WillOnce(Return());
    handler_->handleRequest(reply);
    PtzfStatusIf status;
    EXPECT_TRUE(status.getTeleShiftMode());
    mq.unlink();

    // ### for UI(Sircs) ### //
    common::MessageQueue mq_;
    SetTeleShiftModeRequest ui_msg;
    ui_msg.enable = true;

    EXPECT_CALL(zoom_infra_if_mock_, setTeleShiftMode(Eq(ui_msg.enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<SetTeleShiftModeRequest>(ui_msg, mq_.getName());

    // Reply
    reply.status = ERRORCODE_SUCCESS;
    handler_->handleRequest(reply);

    SetTeleShiftModeResult result;
    mq_.pend(result);
    EXPECT_EQ(reply.status, result.err);

    // TeleShiftMode is set
    EXPECT_TRUE(status.getTeleShiftMode());

    // ### for Biz(1Way) ### //
    BizMessage<SetTeleShiftModeRequest> biz_msg_1way;
    biz_msg_1way.seq_id = U32_T(123456);
    gtl::copyString(biz_msg_1way.mq_name.name, "");
    biz_msg_1way().enable = true;

    EXPECT_CALL(zoom_infra_if_mock_, setTeleShiftMode(Eq(biz_msg_1way().enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest<BizMessage<SetTeleShiftModeRequest>>(biz_msg_1way);

    // Reply
    reply.status = ERRORCODE_SUCCESS;
    handler_->handleRequest(reply);

    // TeleShiftMode is set
    EXPECT_TRUE(status.getTeleShiftMode());

    // ### for Biz(2Way) ### //
    BizMessage<SetTeleShiftModeRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().enable = true;

    EXPECT_CALL(zoom_infra_if_mock_, setTeleShiftMode(Eq(biz_msg_2way().enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest<BizMessage<SetTeleShiftModeRequest>>(biz_msg_2way);

    // Reply
    reply.status = ERRORCODE_SUCCESS;
    handler_->handleRequest(reply);

    ptzf::message::PtzfExecComp biz_result;
    mq_.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(reply.status, biz_result.error);

    // TeleShiftMode is set
    EXPECT_TRUE(status.getTeleShiftMode());

    mq.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, TeleShiftModeError)
{
    // ### for Visca ### //
    common::MessageQueue mq;
    visca::ViscaMessageSequence<SetTeleShiftModeRequest> msg;
    msg.packet_id = U32_T(123456789);
    msg().enable = true;

    EXPECT_CALL(zoom_infra_if_mock_, setTeleShiftMode(Eq(msg().enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetTeleShiftModeRequest>>(msg, mq.getName());

    SetTeleShiftModeReply reply;
    reply.status = ERRORCODE_EXEC;
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(msg.packet_id), Eq(reply.status))).Times(1).WillOnce(Return());
    handler_->handleRequest(reply);

    PtzfStatusIf status;
    EXPECT_NE(msg().enable, status.getTeleShiftMode());
    mq.unlink();

    // ### for UI(Sircs) ### //
    common::MessageQueue mq_;
    SetTeleShiftModeRequest ui_msg;
    ui_msg.enable = true;

    EXPECT_CALL(zoom_infra_if_mock_, setTeleShiftMode(Eq(ui_msg.enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequestWithReply<SetTeleShiftModeRequest>(ui_msg, mq_.getName());

    reply.status = ERRORCODE_EXEC;
    handler_->handleRequest(reply);

    SetTeleShiftModeResult result;
    mq_.pend(result);
    EXPECT_EQ(reply.status, result.err);

    // TeleShiftMode is not set
    EXPECT_NE(ui_msg.enable, status.getTeleShiftMode());

    // ### for Biz(1Way) ### //
    BizMessage<SetTeleShiftModeRequest> biz_msg_1way;
    biz_msg_1way.seq_id = U32_T(123456);
    gtl::copyString(biz_msg_1way.mq_name.name, "");
    biz_msg_1way().enable = true;

    EXPECT_CALL(zoom_infra_if_mock_, setTeleShiftMode(Eq(biz_msg_1way().enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest<BizMessage<SetTeleShiftModeRequest>>(biz_msg_1way);

    // Reply
    reply.status = ERRORCODE_EXEC;
    handler_->handleRequest(reply);

    // TeleShiftMode is not set
    EXPECT_NE(biz_msg_1way().enable, status.getTeleShiftMode());

    // ### for Biz(1Way) ### //
    BizMessage<SetTeleShiftModeRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().enable = true;

    EXPECT_CALL(zoom_infra_if_mock_, setTeleShiftMode(Eq(biz_msg_2way().enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest<BizMessage<SetTeleShiftModeRequest>>(biz_msg_2way);

    // Reply
    reply.status = ERRORCODE_EXEC;
    handler_->handleRequest(reply);

    ptzf::message::PtzfExecComp biz_result;
    mq_.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(reply.status, biz_result.error);

    // TeleShiftMode is not set
    EXPECT_NE(biz_msg_2way().enable, status.getTeleShiftMode());

    mq.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, TeleShiftMode2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    BizMessage<SetTeleShiftModeRequest> biz_msg1;
    BizMessage<SetTeleShiftModeRequest> biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1().enable = true;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2().enable = false;

    EXPECT_CALL(zoom_infra_if_mock_, setTeleShiftMode(Eq(biz_msg1().enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest<BizMessage<SetTeleShiftModeRequest>>(biz_msg1);

    EXPECT_CALL(zoom_infra_if_mock_, setTeleShiftMode(Eq(biz_msg2().enable), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest<BizMessage<SetTeleShiftModeRequest>>(biz_msg2);

    // Reply
    SetTeleShiftModeReply reply;
    reply.status = ERRORCODE_SUCCESS;
    handler_->handleRequest(reply);
    handler_->handleRequest(reply);

    ptzf::message::PtzfExecComp biz_result1;
    mq1.pend(biz_result1);
    EXPECT_EQ(biz_msg1.seq_id, biz_result1.seq_id);
    EXPECT_EQ(reply.status, biz_result1.error);

    ptzf::message::PtzfExecComp biz_result2;
    mq2.pend(biz_result2);
    EXPECT_EQ(biz_msg2.seq_id, biz_result2.seq_id);
    EXPECT_EQ(reply.status, biz_result2.error);

    // TeleShiftMode is set
    PtzfStatusIf status;
    EXPECT_FALSE(status.getTeleShiftMode());

    mq1.unlink();
    mq2.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltMoveSuccess)
{
    // ### for Sircs/Biz(1Way) ### //
    PanTiltMoveRequest msg;
    msg.direction = PAN_TILT_DIRECTION_UP;

    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(controller_mock_, moveSircsPanTilt(_)).Times(1).WillOnce(Return());

    handler_->handleRequest(msg);

    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    PanTiltMoveRequest biz_msg;
    biz_msg.direction = PAN_TILT_DIRECTION_UP;
    biz_msg.pan_speed = U8_T(24);
    biz_msg.tilt_speed = U8_T(23);
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();

    EXPECT_CALL(controller_mock_, movePanTilt(_, _, _, _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest(biz_msg);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltMoveError)
{
    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    PanTiltMoveRequest biz_msg;
    biz_msg.direction = PAN_TILT_DIRECTION_UP;
    biz_msg.pan_speed = U8_T(24);
    biz_msg.tilt_speed = U8_T(23);
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();

    setDefaultValidCondition(U16_T(1));
    EXPECT_CALL(controller_mock_, movePanTilt(_, _, _, _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest(biz_msg);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltMove2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    PanTiltMoveRequest biz_msg1;
    PanTiltMoveRequest biz_msg2;

    biz_msg1.direction = PAN_TILT_DIRECTION_UP;
    biz_msg1.pan_speed = U8_T(24);
    biz_msg1.tilt_speed = U8_T(23);
    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();

    biz_msg2.direction = PAN_TILT_DIRECTION_DOWN;
    biz_msg2.pan_speed = U8_T(24);
    biz_msg2.tilt_speed = U8_T(23);
    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();

    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(controller_mock_, movePanTilt(_, _, _, _, _)).Times(2).WillOnce(Return()).WillOnce(Return());

    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomMoveSuccess)
{
    // ### for Sircs/Biz(1Way) ### //
    ZoomMoveRequest msg;
    msg.speed = U8_T(1);
    msg.direction = ZOOM_DIRECTION_STOP;

    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(controller_mock_, moveSircsZoom(_, _)).Times(1).WillOnce(Return());

    handler_->handleRequest(msg);

    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    ZoomMoveRequest biz_msg;
    biz_msg.speed = U8_T(1);
    biz_msg.direction = ZOOM_DIRECTION_STOP;
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();

    EXPECT_CALL(controller_mock_, moveZoom(_, _, _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest(biz_msg);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomMoveError)
{
    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    ZoomMoveRequest biz_msg;
    biz_msg.speed = U8_T(1);
    biz_msg.direction = ZOOM_DIRECTION_STOP;
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();

    EXPECT_CALL(visca_status_if_mock_, isHandlingIfclearCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).WillOnce(Return(power::PowerStatus::POWER_OFF));
    handler_->handleRequest(biz_msg);
    ptzf::message::PtzfExecComp result;
    mq.pend(result);
    EXPECT_EQ(biz_msg.seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.error);

    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).WillOnce(Return(power::PowerStatus::POWER_ON));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).WillOnce(Return(false));
    EXPECT_CALL(preset_status_if_mock_, isDoingPresetRecall()).WillOnce(Return(false));
    EXPECT_CALL(video_status_if_mock_, isConfiguringHdmiVideoFormat()).WillOnce(Return(false));
    EXPECT_CALL(ptz_trace_status_if_mock_, getTraceCondition())
        .WillRepeatedly(Return(ptzf::PTZ_TRACE_CONDITION_READY_TO_PLAYBACK));
    handler_->handleRequest(biz_msg);
    mq.pend(result);
    EXPECT_EQ(biz_msg.seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.error);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomMove2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    ZoomMoveRequest biz_msg1;
    ZoomMoveRequest biz_msg2;

    biz_msg1.speed = U8_T(1);
    biz_msg1.direction = ZOOM_DIRECTION_STOP;
    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();

    biz_msg2.speed = U8_T(1);
    biz_msg2.direction = ZOOM_DIRECTION_TELE;
    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();

    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(controller_mock_, moveZoom(_, _, _, _)).Times(2).WillOnce(Return()).WillOnce(Return());

    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusModeSuccess)
{
    // ### for Sircs/Biz(1Way) ### //
    FocusModeRequest msg;
    msg.mode = FOCUS_MODE_AUTO;

    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(controller_mock_, setFocusMode(_, _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest(msg);

    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    FocusModeRequest biz_msg;
    biz_msg.mode = FOCUS_MODE_AUTO;
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();

    EXPECT_CALL(controller_mock_, setFocusMode(_, _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest(biz_msg);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusModeError)
{
    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    FocusModeRequest biz_msg;
    biz_msg.mode = FOCUS_MODE_AUTO;
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();

    EXPECT_CALL(visca_status_if_mock_, isHandlingIfclearCommand()).WillOnce(Return(false));
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).WillOnce(Return(power::PowerStatus::POWER_OFF));
    handler_->handleRequest(biz_msg);
    ptzf::message::PtzfExecComp result;
    mq.pend(result);
    EXPECT_EQ(biz_msg.seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.error);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusMode2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    FocusModeRequest biz_msg1;
    FocusModeRequest biz_msg2;

    biz_msg1.mode = FOCUS_MODE_AUTO;
    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();

    biz_msg2.mode = FOCUS_MODE_MANUAL;
    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();

    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(controller_mock_, setFocusMode(_, _, _)).Times(2).WillOnce(Return()).WillOnce(Return());

    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusMoveSuccess)
{
    // ### for Sircs/Biz(1Way) ### //
    FocusMoveRequest msg;
    msg.direction = FOCUS_DIRECTION_STOP;
    msg.speed = U8_T(1);

    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(controller_mock_, moveFocus(_, _, _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest(msg);

    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    FocusMoveRequest biz_msg;
    biz_msg.direction = FOCUS_DIRECTION_STOP;
    biz_msg.speed = U8_T(1);
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();

    EXPECT_CALL(controller_mock_, moveFocus(_, _, _, _)).Times(1).WillOnce(Return());

    handler_->handleRequest(biz_msg);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusMoveError)
{
    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    FocusMoveRequest biz_msg;
    biz_msg.direction = FOCUS_DIRECTION_STOP;
    biz_msg.speed = U8_T(1);
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();

    EXPECT_CALL(visca_status_if_mock_, isHandlingIfclearCommand()).WillOnce(Return(false));
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).WillOnce(Return(power::PowerStatus::POWER_OFF));
    handler_->handleRequest(biz_msg);
    ptzf::message::PtzfExecComp result;
    mq.pend(result);
    EXPECT_EQ(biz_msg.seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.error);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusMove2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    FocusMoveRequest biz_msg1;
    FocusMoveRequest biz_msg2;

    biz_msg1.direction = FOCUS_DIRECTION_STOP;
    biz_msg1.speed = U8_T(1);
    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();

    biz_msg2.direction = FOCUS_DIRECTION_FAR;
    biz_msg2.speed = U8_T(1);
    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();

    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(controller_mock_, moveFocus(_, _, _, _)).Times(2).WillOnce(Return()).WillOnce(Return());

    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltResetSuccess)
{
    // ### for Sircs/Biz(1Way) ### //
    common::MessageQueue reply_mq;
    u32_t msg_seq_id = U32_T(123);
    PanTiltResetRequest msg;
    msg.seq_id = msg_seq_id;
    msg.mq_name = reply_mq.getName();
    msg.mode_checked = true;
    msg.need_ack = true;

    const struct PanTiltResetParamTable
    {
        power::PowerStatus power_status;
        bool pt_lock;
        bool call_reset;
    } pan_tilt_reset_param_table[] = {
        { power::PowerStatus::POWER_OFF, false, true }, { power::PowerStatus::PROCESSING_ON, false, false },
        { power::PowerStatus::POWER_ON, false, true },  { power::PowerStatus::PROCESSING_OFF, false, false },
        { power::PowerStatus::POWER_OFF, true, false }, { power::PowerStatus::PROCESSING_ON, true, false },
        { power::PowerStatus::POWER_ON, true, false },  { power::PowerStatus::PROCESSING_OFF, true, false },
    };

    ARRAY_FOREACH (pan_tilt_reset_param_table, i) {
        msg.seq_id++;
        EXPECT_CALL(power_status_if_mock_, getPowerStatus())
            .Times(1)
            .WillRepeatedly(Return(pan_tilt_reset_param_table[i].power_status));
        EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
            .Times(1)
            .WillRepeatedly(DoAll(SetArgReferee<0>(pan_tilt_reset_param_table[i].pt_lock), Return(ERRORCODE_SUCCESS)));
        if (pan_tilt_reset_param_table[i].call_reset) {
            EXPECT_CALL(controller_mock_, resetPanTiltPosition(_, true, _)).Times(1).WillOnce(Return());
        }
        else {
            EXPECT_CALL(controller_mock_, resetPanTiltPosition(_, _, _)).Times(0);
        }
        handler_->handleRequest(msg);

        if (pan_tilt_reset_param_table[i].call_reset) {
            // compによるpan_tilt_reset_queue_のクリア
            ResetPanTiltCompReply reply(ERRORCODE_SUCCESS);
            handler_->handleRequest(reply);
        }
    }
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltResetError1)
{
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillRepeatedly(Return(power::PowerStatus::POWER_OFF));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(1)
        .WillRepeatedly(DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS)));

    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    PanTiltResetRequest biz_msg;
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();
    biz_msg.mode_checked = true;
    biz_msg.need_ack = true;

    EXPECT_CALL(controller_mock_, resetPanTiltPosition(_, biz_msg.need_ack, _)).Times(1).WillOnce(Return());

    handler_->handleRequest(biz_msg);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltResetError2)
{
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillRepeatedly(Return(power::PowerStatus::POWER_OFF));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(1)
        .WillRepeatedly(DoAll(SetArgReferee<0>(true), Return(ERRORCODE_SUCCESS)));

    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    PanTiltResetRequest biz_msg;
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();
    biz_msg.mode_checked = false;
    biz_msg.need_ack = true;

    EXPECT_CALL(controller_mock_, resetPanTiltPosition(_, _, _)).Times(0);

    handler_->handleRequest(biz_msg);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltReset2wayExclusive)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    PanTiltResetRequest biz_msg1;
    PanTiltResetRequest biz_msg2;

    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(2).WillRepeatedly(Return(power::PowerStatus::POWER_OFF));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS)));

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.mode_checked = true;
    biz_msg1.need_ack = false;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.mode_checked = true;
    biz_msg2.need_ack = true;

    EXPECT_CALL(controller_mock_, resetPanTiltPosition(_, biz_msg1.need_ack, _)).Times(1).WillOnce(Return());
    EXPECT_CALL(controller_mock_, resetPanTiltPosition(_, biz_msg2.need_ack, _)).Times(0);

    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltResetWithAck)
{
    common::MessageQueue reply_mq;
    PanTiltResetRequest request;

    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillRepeatedly(Return(power::PowerStatus::POWER_ON));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(1)
        .WillRepeatedly(DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS)));

    request.seq_id = U32_T(123456);
    request.mq_name = reply_mq.getName();
    request.mode_checked = true;
    request.need_ack = true;

    EXPECT_CALL(controller_mock_, resetPanTiltPosition(_, request.need_ack, _)).Times(1).WillOnce(Return());

    handler_->handleRequest(request);

    ResetPanTiltAckReply ack_reply;
    handler_->handleRequest(ack_reply);
    ptzf::message::PtzfExeAck common_ack;
    reply_mq.pend(common_ack);
    EXPECT_EQ(common_ack.seq_id, request.seq_id);
}

TEST_F(PtzfControllerMessageHandlerTest, IfClearSuccess)
{
    // ### Biz(1Way) ### //
    IfClearRequest msg;
    msg.seq_id = U32_T(789);
    EXPECT_CALL(if_clear_infra_if_mock_, doIfClear(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(msg);

    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    IfClearRequest biz_msg;
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();
    EXPECT_CALL(if_clear_infra_if_mock_, doIfClear(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg);
}

TEST_F(PtzfControllerMessageHandlerTest, IfClearError)
{
    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    IfClearRequest biz_msg;
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();
    EXPECT_CALL(if_clear_infra_if_mock_, doIfClear(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg);
}

TEST_F(PtzfControllerMessageHandlerTest, IfClear2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    IfClearRequest biz_msg1;
    IfClearRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    EXPECT_CALL(if_clear_infra_if_mock_, doIfClear(_, biz_msg1.seq_id)).Times(1).WillOnce(Return());

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    EXPECT_CALL(if_clear_infra_if_mock_, doIfClear(_, biz_msg2.seq_id)).Times(1).WillOnce(Return());

    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomFineMoveRequestSuccess)
{
    // ### Biz(1Way) ### //
    ZoomFineMoveRequest msg;
    msg.seq_id = U32_T(789);
    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(zoom_infra_if_mock_, sendZoomFineMoveRequest(_, _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(msg);

    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    ZoomFineMoveRequest biz_msg;
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();
    EXPECT_CALL(zoom_infra_if_mock_, sendZoomFineMoveRequest(_, _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomFineMoveRequestError)
{
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    ZoomFineMoveRequest biz_msg;
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();

    EXPECT_CALL(visca_status_if_mock_, isHandlingIfclearCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).WillOnce(Return(power::PowerStatus::POWER_OFF));
    handler_->handleRequest(biz_msg);
    ptzf::message::PtzfExecComp result;
    mq.pend(result);
    EXPECT_EQ(biz_msg.seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.error);

    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).WillOnce(Return(power::PowerStatus::POWER_ON));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).WillOnce(Return(false));
    EXPECT_CALL(preset_status_if_mock_, isDoingPresetRecall()).WillOnce(Return(false));
    EXPECT_CALL(video_status_if_mock_, isConfiguringHdmiVideoFormat()).WillOnce(Return(false));
    EXPECT_CALL(ptz_trace_status_if_mock_, getTraceCondition())
        .WillRepeatedly(Return(ptzf::PTZ_TRACE_CONDITION_READY_TO_PLAYBACK));
    handler_->handleRequest(biz_msg);
    mq.pend(result);
    EXPECT_EQ(biz_msg.seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.error);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomFineMoveRequest2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    ZoomFineMoveRequest biz_msg1;
    ZoomFineMoveRequest biz_msg2;

    biz_msg1.direction = ZOOM_DIRECTION_STOP;
    biz_msg1.fine_move = U16_T(0x00);
    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();

    biz_msg2.direction = ZOOM_DIRECTION_STOP;
    biz_msg2.fine_move = U16_T(0x00);
    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();

    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(zoom_infra_if_mock_, sendZoomFineMoveRequest(Eq(biz_msg1.direction), Eq(biz_msg1.fine_move), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(zoom_infra_if_mock_, sendZoomFineMoveRequest(Eq(biz_msg2.direction), Eq(biz_msg2.fine_move), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltImageFlipModeChangeSuccess)
{
    common::MessageQueue mq;
    PtzfStatusIf stIf;
    PtzfStatus st;
    st.setImageFlipConfigurationStatus(false);
    st.setPanTiltLimitConfigurationStatus(false);

    // ### for UI ### //
    SetImageFlipRequest ui_msg;
    ui_msg.enable = true;
    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(preset_if_mock_, save()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(preset_if_mock_, reset()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(database_initialize_if_mock_, initializeForImgFlip()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptz_trace_if_mock_, deleteData()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1);

    handler_->handleRequestWithReply(ui_msg, mq.getName());

    SetImageFlipResult ui_result;
    mq.pend(ui_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, ui_result.err);
    // Wait thread process end
    waitQueueEmpty(thread_mq_);
    common::Task::msleep(U32_T(1));
    while (stIf.isConfiguringImageFlip()) {
        common::Task::msleep(U32_T(1));
    }

    // ### for Visca ### //
    visca::ViscaMessageSequence<SetImageFlipRequest> visca_msg;
    visca_msg.packet_id = U32_T(123456789);
    visca_msg().enable = false;
    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_ON);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(preset_if_mock_, save()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(preset_if_mock_, reset()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(database_initialize_if_mock_, initializeForImgFlip()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptz_trace_if_mock_, deleteData()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1);
    EXPECT_CALL(visca_if_mock_, sendCompRequest(visca_msg.packet_id, ERRORCODE_SUCCESS)).Times(1).WillOnce(Return());

    handler_->handleRequestWithReply(visca_msg, mq.getName());
    // Reply Ack
    visca::AckResponse ack_res;
    mq.pend(ack_res);
    EXPECT_EQ(ERRORCODE_SUCCESS, ack_res.status);
    // Wait thread process end
    waitQueueEmpty(thread_mq_);
    common::Task::msleep(U32_T(1));
    while (stIf.isConfiguringImageFlip()) {
        common::Task::msleep(U32_T(1));
    }

    // ### for Biz(1Way) ### //
    BizMessage<SetImageFlipRequest> biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way().enable = true;
    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(preset_if_mock_, save()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(preset_if_mock_, reset()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(database_initialize_if_mock_, initializeForImgFlip()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptz_trace_if_mock_, deleteData()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1);

    handler_->handleRequest(biz_msg_1way);
    // Wait thread process end
    waitQueueEmpty(thread_mq_);
    while (stIf.isConfiguringImageFlip()) {
        common::Task::msleep(U32_T(1));
    }

    // ### for Biz(2Way) ### //
    BizMessage<SetImageFlipRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq.getName();
    biz_msg_2way().enable = false;
    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_ON);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(preset_if_mock_, save()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(preset_if_mock_, reset()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(database_initialize_if_mock_, initializeForImgFlip()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptz_trace_if_mock_, deleteData()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1);

    handler_->handleRequest(biz_msg_2way);

    ptzf::message::PtzfExecComp biz_result;
    mq.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(ERRORCODE_SUCCESS, biz_result.error);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltImageFlipModeNoChangeSuccess)
{
    common::MessageQueue mq;
    PtzfStatus st;
    st.setImageFlipConfigurationStatus(false);
    st.setPanTiltLimitConfigurationStatus(false);

    // ### for UI ### //
    SetImageFlipRequest ui_msg;
    ui_msg.enable = false;
    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1);

    handler_->handleRequestWithReply(ui_msg, mq.getName());

    SetImageFlipResult ui_result;
    mq.pend(ui_result);
    EXPECT_EQ(ERRORCODE_SUCCESS, ui_result.err);

    // ### for Visca ### //
    visca::ViscaMessageSequence<SetImageFlipRequest> visca_msg;
    visca_msg.packet_id = U32_T(123456789);
    visca_msg().enable = true;
    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_ON);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1);
    EXPECT_CALL(visca_if_mock_, sendCompRequest(visca_msg.packet_id, ERRORCODE_SUCCESS)).Times(1).WillOnce(Return());

    handler_->handleRequestWithReply(visca_msg, mq.getName());
    // Reply Ack
    visca::AckResponse ack_res;
    mq.pend(ack_res);
    EXPECT_EQ(ERRORCODE_SUCCESS, ack_res.status);

    // ### for Biz(1Way) ### //
    BizMessage<SetImageFlipRequest> biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    gtl::copyString(biz_msg_1way.mq_name.name, "");
    biz_msg_1way().enable = false;
    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1);

    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    BizMessage<SetImageFlipRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq.getName();
    biz_msg_2way().enable = true;
    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_ON);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1);

    handler_->handleRequest(biz_msg_2way);

    ptzf::message::PtzfExecComp result;
    mq.pend(result);
    EXPECT_EQ(biz_msg_2way.seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_SUCCESS, result.error);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltImageFlipModeError)
{
    common::MessageQueue mq;
    PtzfStatus st;

    // ### for Biz(2Way) ### //
    BizMessage<SetImageFlipRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq.getName();
    biz_msg_2way().enable = true;
    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);

    st.setImageFlipConfigurationStatus(true);
    st.setPanTiltLimitConfigurationStatus(false);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));

    handler_->handleRequest(biz_msg_2way);

    ptzf::message::PtzfExecComp result;
    mq.pend(result);
    EXPECT_EQ(biz_msg_2way.seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.error);

    st.setImageFlipConfigurationStatus(false);
    st.setPanTiltLimitConfigurationStatus(true);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));

    handler_->handleRequest(biz_msg_2way);

    mq.pend(result);
    EXPECT_EQ(biz_msg_2way.seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.error);

    st.setImageFlipConfigurationStatus(false);
    st.setPanTiltLimitConfigurationStatus(false);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(preset_if_mock_, save()).Times(1).WillOnce(Return(ERRORCODE_CONFLICT));
    EXPECT_CALL(preset_if_mock_, reset()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(database_initialize_if_mock_, initializeForImgFlip()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptz_trace_if_mock_, deleteData()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1);

    handler_->handleRequest(biz_msg_2way);

    ptzf::message::PtzfExecComp biz_result;
    mq.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(ERRORCODE_SUCCESS, biz_result.error);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(preset_if_mock_, save()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(preset_if_mock_, reset()).Times(1).WillOnce(Return(ERRORCODE_INTERRUPTED));
    EXPECT_CALL(database_initialize_if_mock_, initializeForImgFlip()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptz_trace_if_mock_, deleteData()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1);

    handler_->handleRequest(biz_msg_2way);

    mq.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(ERRORCODE_INTERRUPTED, biz_result.error);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(preset_if_mock_, save()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(preset_if_mock_, reset()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(database_initialize_if_mock_, initializeForImgFlip()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptz_trace_if_mock_, deleteData()).Times(1).WillOnce(Return(ERRORCODE_OPEN));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1);

    handler_->handleRequest(biz_msg_2way);

    mq.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(ERRORCODE_OPEN, biz_result.error);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltImageFlipMode2wayMultiple)
{
    PtzfStatusIf stIf;
    PtzfStatus st;
    st.setImageFlipConfigurationStatus(false);
    st.setPanTiltLimitConfigurationStatus(false);

    common::MessageQueue mq1;
    common::MessageQueue mq2;
    common::MessageQueue mq3;
    BizMessage<SetImageFlipRequest> biz_msg1;
    BizMessage<SetImageFlipRequest> biz_msg2;
    BizMessage<SetImageFlipRequest> biz_msg3;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1().enable = true;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2().enable = false;

    biz_msg3.seq_id = U32_T(111999);
    biz_msg3.mq_name = mq3.getName();
    biz_msg3().enable = true;

    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(3)
        .WillRepeatedly(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_ENABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(preset_if_mock_, save()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(preset_if_mock_, reset()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(database_initialize_if_mock_, initializeForImgFlip()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptz_trace_if_mock_, deleteData()).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1).WillOnce(Return(false));

    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
    handler_->handleRequest(biz_msg3);

    ptzf::message::PtzfExecComp biz_result1;
    mq1.pend(biz_result1);
    EXPECT_EQ(biz_msg1.seq_id, biz_result1.seq_id);
    EXPECT_EQ(ERRORCODE_SUCCESS, biz_result1.error);

    ptzf::message::PtzfExecComp biz_result2;
    mq2.pend(biz_result2);
    EXPECT_EQ(biz_msg2.seq_id, biz_result2.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, biz_result2.error);

    ptzf::message::PtzfExecComp biz_result3;
    mq3.pend(biz_result3);
    EXPECT_EQ(biz_msg3.seq_id, biz_result3.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, biz_result3.error);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltImageFlipModeDisabled)
{
    common::MessageQueue mq;
    PtzfStatus st;
    st.setImageFlipConfigurationStatus(false);
    st.setPanTiltLimitConfigurationStatus(false);

    // ### for UI ### //
    SetImageFlipRequest ui_msg;
    ui_msg.enable = false;
    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_DISABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(0);

    handler_->handleRequestWithReply(ui_msg, mq.getName());

    SetImageFlipResult ui_result;
    mq.pend(ui_result);
    EXPECT_EQ(ERRORCODE_EXEC, ui_result.err);

    // ### for Visca ### //
    visca::ViscaMessageSequence<SetImageFlipRequest> visca_msg;
    visca_msg.packet_id = U32_T(123456789);
    visca_msg().enable = true;
    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_ON);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_DISABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(0);
    EXPECT_CALL(visca_if_mock_, sendCompRequest(_, _)).Times(0);

    handler_->handleRequestWithReply(visca_msg, mq.getName());
    // Reply Ack
    visca::AckResponse ack_res;
    mq.pend(ack_res);
    EXPECT_EQ(ERRORCODE_EXEC, ack_res.status);

    // ### for Biz(1Way) ### //
    BizMessage<SetImageFlipRequest> biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    gtl::copyString(biz_msg_1way.mq_name.name, "");
    biz_msg_1way().enable = false;
    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_OFF);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_DISABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(0);

    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    BizMessage<SetImageFlipRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq.getName();
    biz_msg_2way().enable = true;
    st.setImageFlipStatusOnBoot(visca::PICTURE_FLIP_MODE_ON);

    EXPECT_CALL(pan_tilt_infra_if_mock_, getPanTiltEnabledState(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_ENABLED_STATE_DISABLE), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(0);
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(0);

    handler_->handleRequest(biz_msg_2way);

    ptzf::message::PtzfExecComp result;
    mq.pend(result);
    EXPECT_EQ(biz_msg_2way.seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.error);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltLimitError)
{
    common::MessageQueue mq;

    // ### For BIZ(2Way) ### //
    SetPanTiltLimitRequestForBiz payload(PAN_TILT_LIMIT_TYPE_UP_RIGHT, U32_T(0xF000), U32_T(0x10));
    BizMessage<SetPanTiltLimitRequestForBiz> biz_msg_2way(U32_T(123456), mq.getName(), payload);

    handler_->handleRequest(biz_msg_2way);

    ptzf::message::PtzfExecComp biz_result;
    mq.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(ERRORCODE_OUT_OF_RANGE, biz_result.error);

    biz_msg_2way().type = PAN_TILT_LIMIT_TYPE_UP_RIGHT;
    biz_msg_2way().pan = U32_T(0x10);
    biz_msg_2way().tilt = U32_T(0xc000);
    handler_->handleRequest(biz_msg_2way);

    mq.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(ERRORCODE_OUT_OF_RANGE, biz_result.error);

    biz_msg_2way().type = PAN_TILT_LIMIT_TYPE_DOWN_LEFT;
    biz_msg_2way().pan = U32_T(0xF000);
    biz_msg_2way().tilt = U32_T(0x10);
    handler_->handleRequest(biz_msg_2way);

    mq.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(ERRORCODE_OUT_OF_RANGE, biz_result.error);

    biz_msg_2way().type = PAN_TILT_LIMIT_TYPE_DOWN_LEFT;
    biz_msg_2way().pan = U32_T(0x10);
    biz_msg_2way().tilt = U32_T(0xC000);
    handler_->handleRequest(biz_msg_2way);

    mq.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(ERRORCODE_OUT_OF_RANGE, biz_result.error);
}

TEST_F(PtzfControllerMessageHandlerTest, DZoomModeSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetDZoomModeRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.d_zoom = ptzf::DZOOM_FULL;

    EXPECT_CALL(zoom_infra_if_mock_, setDZoomMode(Eq(biz_msg_1way.d_zoom), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetDZoomModeRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.d_zoom = ptzf::DZOOM_OPTICAL;

    EXPECT_CALL(zoom_infra_if_mock_, setDZoomMode(Eq(biz_msg_2way.d_zoom), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, DZoomModeError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetDZoomModeRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.d_zoom = ptzf::DZOOM_OPTICAL;

    EXPECT_CALL(zoom_infra_if_mock_, setDZoomMode(Eq(biz_msg_2way.d_zoom), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, DZoomMode2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetDZoomModeRequest biz_msg1;
    SetDZoomModeRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.d_zoom = ptzf::DZOOM_OPTICAL;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.d_zoom = ptzf::DZOOM_OPTICAL;

    EXPECT_CALL(zoom_infra_if_mock_, setDZoomMode(Eq(biz_msg1.d_zoom), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(zoom_infra_if_mock_, setDZoomMode(Eq(biz_msg2.d_zoom), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomAbsolutePositionSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    common::MessageQueue biz_mq;
    SetZoomAbsolutePositionRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.biz_mq_name = biz_mq.getName();
    biz_msg_1way.position = U16_T(0x4000);

    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(zoom_infra_if_mock_, moveZoomAbsolute(Eq(biz_msg_1way.position), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetZoomAbsolutePositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq.getName();
    biz_msg_2way.biz_mq_name = biz_mq.getName();
    biz_msg_2way.position = U16_T(0x4000);

    EXPECT_CALL(zoom_infra_if_mock_, moveZoomAbsolute(Eq(biz_msg_2way.position), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomAbsolutePositionError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq;
    common::MessageQueue biz_mq;
    SetZoomAbsolutePositionRequest biz_msg;
    biz_msg.seq_id = U32_T(123456);
    biz_msg.mq_name = mq.getName();
    biz_msg.biz_mq_name = biz_mq.getName();
    biz_msg.position = U16_T(0);

    EXPECT_CALL(visca_status_if_mock_, isHandlingIfclearCommand()).WillOnce(Return(false));
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).WillOnce(Return(power::PowerStatus::POWER_OFF));
    handler_->handleRequest(biz_msg);
    ptzf::message::PtzfExecComp result;
    mq.pend(result);
    EXPECT_EQ(biz_msg.seq_id, result.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, result.error);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomAbsolutePosition2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue biz_mq1;
    common::MessageQueue mq2;
    common::MessageQueue biz_mq2;
    SetZoomAbsolutePositionRequest biz_msg1;
    SetZoomAbsolutePositionRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.biz_mq_name = biz_mq1.getName();
    biz_msg1.position = U16_T(0x4000);

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.biz_mq_name = biz_mq2.getName();
    biz_msg2.position = U16_T(0x4000);

    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(zoom_infra_if_mock_, moveZoomAbsolute(Eq(biz_msg1.position), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(zoom_infra_if_mock_, moveZoomAbsolute(Eq(biz_msg2.position), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomRelativePositionSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetZoomRelativePositionRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.position = S32_T(0x4000);

    EXPECT_CALL(controller_mock_, moveZoomRelative(_, _, _)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetZoomRelativePositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.position = S32_T(0x4000);

    EXPECT_CALL(controller_mock_, moveZoomRelative(_, _, _)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomRelativePositionError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetZoomRelativePositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.position = S32_T(0);

    EXPECT_CALL(controller_mock_, moveZoomRelative(_, _, _)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg_2way);

    // Ptzf Controller error
    EXPECT_CALL(controller_mock_, moveZoomRelative(_, _, _)).Times(1).WillOnce(Return(ERRORCODE_OUT_OF_RANGE));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomRelativePosition2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetZoomRelativePositionRequest biz_msg1;
    SetZoomRelativePositionRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.position = S32_T(0x4000);

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.position = S32_T(0x4000);

    EXPECT_CALL(controller_mock_, moveZoomRelative(_, _, _))
        .Times(2)
        .WillOnce(Return(ERRORCODE_SUCCESS))
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusAbsolutePositionSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueue biz_mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetFocusAbsolutePositionRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.biz_mq_name = biz_mq_.getName();
    biz_msg_1way.position = U16_T(0x1000);

    setDefaultValidCondition(U16_T(1));
    EXPECT_CALL(focus_infra_if_mock_, moveFocusAbsolute(Eq(biz_msg_1way.position), _, _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetFocusAbsolutePositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.biz_mq_name = biz_mq_.getName();
    biz_msg_2way.position = U16_T(0x1000);

    setDefaultValidCondition(U16_T(1));
    EXPECT_CALL(focus_infra_if_mock_, moveFocusAbsolute(Eq(biz_msg_2way.position), _, _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusAbsolutePositionError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    common::MessageQueue biz_mq_;
    SetFocusAbsolutePositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.biz_mq_name = biz_mq_.getName();
    biz_msg_2way.position = U16_T(0);

    setDefaultValidCondition(U16_T(1));
    EXPECT_CALL(focus_infra_if_mock_, moveFocusAbsolute(Eq(biz_msg_2way.position), _, _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusAbsolutePosition2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue biz_mq1;
    common::MessageQueue mq2;
    common::MessageQueue biz_mq2;
    SetFocusAbsolutePositionRequest biz_msg1;
    SetFocusAbsolutePositionRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.biz_mq_name = biz_mq1.getName();
    biz_msg1.position = U16_T(0x1000);

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.biz_mq_name = biz_mq2.getName();
    biz_msg2.position = U16_T(0x1000);

    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(focus_infra_if_mock_, moveFocusAbsolute(Eq(biz_msg1.position), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, moveFocusAbsolute(Eq(biz_msg2.position), _, _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusRelativePositionSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetFocusRelativePositionRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.position = S32_T(0x1000);

    EXPECT_CALL(controller_mock_, moveFocusRelative(_, _, _)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetFocusRelativePositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.position = S32_T(0x1000);

    EXPECT_CALL(controller_mock_, moveFocusRelative(_, _, _)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusRelativePositionError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetFocusRelativePositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.position = S32_T(0);

    EXPECT_CALL(controller_mock_, moveFocusRelative(_, _, _)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg_2way);

    // Ptzf Controller error
    EXPECT_CALL(controller_mock_, moveFocusRelative(_, _, _)).Times(1).WillOnce(Return(ERRORCODE_OUT_OF_RANGE));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusRelativePosition2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetFocusRelativePositionRequest biz_msg1;
    SetFocusRelativePositionRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.position = S32_T(0x1000);

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.position = S32_T(0x1000);

    EXPECT_CALL(controller_mock_, moveFocusRelative(_, _, _))
        .Times(2)
        .WillOnce(Return(ERRORCODE_SUCCESS))
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusOnePushTriggerSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetFocusOnePushTriggerRequest biz_msg_1way;
    biz_msg_1way.seq_id = U32_T(123456);
    biz_msg_1way.mq_name = blankName;

    EXPECT_CALL(focus_infra_if_mock_, moveFocusOnePushTrigger(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    SetFocusOnePushTriggerRequest biz_msg_2way;
    biz_msg_2way.seq_id = seq_id;
    biz_msg_2way.mq_name = mq.getName();

    EXPECT_CALL(focus_infra_if_mock_, moveFocusOnePushTrigger(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusOnePushTriggerError)
{
    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    SetFocusOnePushTriggerRequest biz_msg;
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();

    EXPECT_CALL(focus_infra_if_mock_, moveFocusOnePushTrigger(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusOnePushTrigger2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetFocusOnePushTriggerRequest biz_msg1;
    SetFocusOnePushTriggerRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();

    EXPECT_CALL(focus_infra_if_mock_, moveFocusOnePushTrigger(_, _)).Times(2).WillOnce(Return()).WillOnce(Return());

    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusAfSensitivitySuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetAFSensitivityModeRequest biz_msg_1way;
    biz_msg_1way.seq_id = U32_T(123456);
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.af_mode = AF_SENSITIVITY_MODE_NORMAL;

    EXPECT_CALL(focus_infra_if_mock_, setAFSensitivityMode(Eq(biz_msg_1way.af_mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);
    // ### for Biz(2Way) ### //
    SetAFSensitivityModeRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.af_mode = AF_SENSITIVITY_MODE_LOW;

    EXPECT_CALL(focus_infra_if_mock_, setAFSensitivityMode(Eq(biz_msg_2way.af_mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusAfSensitivityError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetAFSensitivityModeRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.af_mode = AF_SENSITIVITY_MODE_NORMAL;

    EXPECT_CALL(focus_infra_if_mock_, setAFSensitivityMode(Eq(biz_msg_2way.af_mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusAfSensitivity2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetAFSensitivityModeRequest biz_msg1;
    SetAFSensitivityModeRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.af_mode = AF_SENSITIVITY_MODE_NORMAL;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.af_mode = AF_SENSITIVITY_MODE_NORMAL;

    EXPECT_CALL(focus_infra_if_mock_, setAFSensitivityMode(Eq(biz_msg1.af_mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, setAFSensitivityMode(Eq(biz_msg2.af_mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusNearLimitSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetFocusNearLimitRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.position = U16_T(0x1000);

    EXPECT_CALL(focus_infra_if_mock_, setFocusNearLimit(Eq(biz_msg_1way.position), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetFocusNearLimitRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.position = U16_T(0x1000);

    EXPECT_CALL(focus_infra_if_mock_, setFocusNearLimit(Eq(biz_msg_2way.position), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusNearLimitError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetFocusNearLimitRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.position = U16_T(0);

    EXPECT_CALL(focus_infra_if_mock_, setFocusNearLimit(Eq(biz_msg_2way.position), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusNearLimit2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetFocusNearLimitRequest biz_msg1;
    SetFocusNearLimitRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.position = U16_T(0x1000);

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.position = U16_T(0x1000);

    EXPECT_CALL(focus_infra_if_mock_, setFocusNearLimit(Eq(biz_msg1.position), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, setFocusNearLimit(Eq(biz_msg2.position), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusAFModeSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetFocusAFModeRequest biz_msg_1way;
    biz_msg_1way.seq_id = U32_T(123456);
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.mode = AUTO_FOCUS_MODE_NORMAL;

    EXPECT_CALL(focus_infra_if_mock_, setFocusAFMode(Eq(biz_msg_1way.mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetFocusAFModeRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.mode = AUTO_FOCUS_MODE_INTERVAL;

    EXPECT_CALL(focus_infra_if_mock_, setFocusAFMode(Eq(biz_msg_2way.mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusAFModeError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetFocusAFModeRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.mode = AUTO_FOCUS_MODE_NORMAL;

    EXPECT_CALL(focus_infra_if_mock_, setFocusAFMode(Eq(biz_msg_2way.mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusAFMode2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetFocusAFModeRequest biz_msg1;
    SetFocusAFModeRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.mode = AUTO_FOCUS_MODE_NORMAL;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.mode = AUTO_FOCUS_MODE_ZOOMTRIGGER;

    EXPECT_CALL(focus_infra_if_mock_, setFocusAFMode(Eq(biz_msg1.mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, setFocusAFMode(Eq(biz_msg2.mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusFaceEyedetectionSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetFocusFaceEyeDetectionModeRequest biz_msg_1way;
    biz_msg_1way.seq_id = U32_T(123456);
    biz_msg_1way.reply_name = blankName;
    biz_msg_1way.focus_face_eye_detection_mode = FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY;

    EXPECT_CALL(focus_infra_if_mock_, setFocusFaceEyedetection(Eq(biz_msg_1way.focus_face_eye_detection_mode), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetFocusFaceEyeDetectionModeRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.reply_name = mq_.getName();
    biz_msg_2way.focus_face_eye_detection_mode = FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY;

    EXPECT_CALL(focus_infra_if_mock_, setFocusFaceEyedetection(Eq(biz_msg_2way.focus_face_eye_detection_mode), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusFaceEyedetectionError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetFocusFaceEyeDetectionModeRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.reply_name = mq_.getName();
    biz_msg_2way.focus_face_eye_detection_mode = FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY;

    EXPECT_CALL(focus_infra_if_mock_, setFocusFaceEyedetection(Eq(biz_msg_2way.focus_face_eye_detection_mode), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusFaceEyedetection2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetFocusFaceEyeDetectionModeRequest biz_msg1;
    SetFocusFaceEyeDetectionModeRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.reply_name = mq1.getName();
    biz_msg1.focus_face_eye_detection_mode = FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_ONLY;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.reply_name = mq2.getName();
    biz_msg2.focus_face_eye_detection_mode = FOCUS_FACE_EYE_DETECTION_MODE_FACE_EYE_PRIORITY;

    EXPECT_CALL(focus_infra_if_mock_, setFocusFaceEyedetection(Eq(biz_msg1.focus_face_eye_detection_mode), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, setFocusFaceEyedetection(Eq(biz_msg2.focus_face_eye_detection_mode), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, AfAssistSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetAfAssistRequest biz_msg_1way;
    biz_msg_1way.seq_id = U32_T(123456);
    biz_msg_1way.reply_name = blankName;
    biz_msg_1way.on_off = true;

    EXPECT_CALL(focus_infra_if_mock_, setAfAssist(Eq(biz_msg_1way.on_off), _, _)).Times(1).WillOnce(Return(true));
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetAfAssistRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.reply_name = mq_.getName();
    biz_msg_2way.on_off = true;

    EXPECT_CALL(focus_infra_if_mock_, setAfAssist(Eq(biz_msg_2way.on_off), _, _)).Times(1).WillOnce(Return(true));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, AfAssistError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetAfAssistRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.reply_name = mq_.getName();
    biz_msg_2way.on_off = true;

    EXPECT_CALL(focus_infra_if_mock_, setAfAssist(Eq(biz_msg_2way.on_off), _, _)).Times(1).WillOnce(Return(true));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, AfAssist2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetAfAssistRequest biz_msg1;
    SetAfAssistRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.reply_name = mq1.getName();
    biz_msg1.on_off = true;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.reply_name = mq2.getName();
    biz_msg2.on_off = false;

    EXPECT_CALL(focus_infra_if_mock_, setAfAssist(Eq(biz_msg1.on_off), _, _)).Times(1).WillOnce(Return(true));
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, setAfAssist(Eq(biz_msg2.on_off), _, _)).Times(1).WillOnce(Return(true));
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusTrackingPositionSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetFocusTrackingPositionRequest biz_msg_1way;
    biz_msg_1way.seq_id = U32_T(123456);
    biz_msg_1way.reply_name = blankName;
    biz_msg_1way.pos_x = 0x12;
    biz_msg_1way.pos_y = 0x34;

    EXPECT_CALL(focus_infra_if_mock_, setFocusTrackingPosition(Eq(biz_msg_1way.pos_x), Eq(biz_msg_1way.pos_y), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetFocusTrackingPositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.reply_name = mq_.getName();
    biz_msg_2way.pos_x = 0x12;
    biz_msg_2way.pos_y = 0x34;

    EXPECT_CALL(focus_infra_if_mock_, setFocusTrackingPosition(Eq(biz_msg_2way.pos_x), Eq(biz_msg_2way.pos_y), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusTrackingPositionError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetFocusTrackingPositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.reply_name = mq_.getName();
    biz_msg_2way.pos_x = 0x12;
    biz_msg_2way.pos_y = 0x34;

    EXPECT_CALL(focus_infra_if_mock_, setFocusTrackingPosition(Eq(biz_msg_2way.pos_x), Eq(biz_msg_2way.pos_y), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusTrackingPosition2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetFocusTrackingPositionRequest biz_msg1;
    SetFocusTrackingPositionRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.reply_name = mq1.getName();
    biz_msg1.pos_x = 0x12;
    biz_msg1.pos_y = 0x34;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.reply_name = mq2.getName();
    biz_msg2.pos_x = 0x56;
    biz_msg2.pos_y = 0x78;

    EXPECT_CALL(focus_infra_if_mock_, setFocusTrackingPosition(Eq(biz_msg1.pos_x), Eq(biz_msg1.pos_y), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, setFocusTrackingPosition(Eq(biz_msg2.pos_x), Eq(biz_msg2.pos_y), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, TouchFunctionInMfSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetTouchFunctionInMfRequest biz_msg_1way;
    biz_msg_1way.seq_id = U32_T(123456);
    biz_msg_1way.reply_name = blankName;
    biz_msg_1way.touch_function_in_mf = TOUCH_FUNCTION_IN_MF_TRACKING_AF;

    EXPECT_CALL(focus_infra_if_mock_, setTouchFunctionInMf(Eq(biz_msg_1way.touch_function_in_mf), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetTouchFunctionInMfRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.reply_name = mq_.getName();
    biz_msg_2way.touch_function_in_mf = TOUCH_FUNCTION_IN_MF_TRACKING_AF;

    EXPECT_CALL(focus_infra_if_mock_, setTouchFunctionInMf(Eq(biz_msg_2way.touch_function_in_mf), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, TouchFunctionInMfError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetTouchFunctionInMfRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.reply_name = mq_.getName();
    biz_msg_2way.touch_function_in_mf = TOUCH_FUNCTION_IN_MF_TRACKING_AF;

    EXPECT_CALL(focus_infra_if_mock_, setTouchFunctionInMf(Eq(biz_msg_2way.touch_function_in_mf), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, TouchFunctionInMf2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetTouchFunctionInMfRequest biz_msg1;
    SetTouchFunctionInMfRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.reply_name = mq1.getName();
    biz_msg1.touch_function_in_mf = TOUCH_FUNCTION_IN_MF_TRACKING_AF;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.reply_name = mq2.getName();
    biz_msg2.touch_function_in_mf = TOUCH_FUNCTION_IN_MF_SPOT_FOCUS;

    EXPECT_CALL(focus_infra_if_mock_, setTouchFunctionInMf(Eq(biz_msg1.touch_function_in_mf), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, setTouchFunctionInMf(Eq(biz_msg2.touch_function_in_mf), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusAFTimerSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetFocusAFTimerRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.action_time = U8_T(0x05);
    biz_msg_1way.stop_time = U8_T(0x05);

    EXPECT_CALL(focus_infra_if_mock_, setFocusAFTimer(Eq(biz_msg_1way.action_time), Eq(biz_msg_1way.stop_time), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetFocusAFTimerRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.action_time = U8_T(0x05);
    biz_msg_2way.stop_time = U8_T(0x05);

    EXPECT_CALL(focus_infra_if_mock_, setFocusAFTimer(Eq(biz_msg_2way.action_time), Eq(biz_msg_2way.stop_time), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusAFTimerError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetFocusAFTimerRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.action_time = U8_T(0);
    biz_msg_2way.stop_time = U8_T(0);

    EXPECT_CALL(focus_infra_if_mock_, setFocusAFTimer(Eq(biz_msg_2way.action_time), Eq(biz_msg_2way.stop_time), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusAFTimer2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetFocusAFTimerRequest biz_msg1;
    SetFocusAFTimerRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.action_time = U8_T(0x05);
    biz_msg1.stop_time = U8_T(0x05);

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.action_time = U8_T(0x05);
    biz_msg2.stop_time = U8_T(0x05);

    EXPECT_CALL(focus_infra_if_mock_, setFocusAFTimer(Eq(biz_msg1.action_time), Eq(biz_msg1.stop_time), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, setFocusAFTimer(Eq(biz_msg2.action_time), Eq(biz_msg2.stop_time), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltAbsolutePositionSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetPanTiltAbsolutePositionRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.pan_speed = U8_T(0x18);
    biz_msg_1way.tilt_speed = U8_T(0x17);
    biz_msg_1way.pan_position = S32_T(0x1234);
    biz_msg_1way.tilt_position = S32_T(0x1234);

    PtzfStatusIf status;
    setDefaultValidCondition(U16_T(1));
    EXPECT_CALL(pan_tilt_infra_if_mock_,
                movePanTiltAbsolute(Eq(biz_msg_1way.pan_speed),
                                    Eq(biz_msg_1way.tilt_speed),
                                    Eq(status.panSinDataToViscaData(biz_msg_1way.pan_position)),
                                    Eq(static_cast<u32_t>(status.tiltSinDataToViscaData(biz_msg_1way.tilt_position))),
                                    _,
                                    _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetPanTiltAbsolutePositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(0);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.pan_speed = U8_T(0x01);
    biz_msg_2way.tilt_speed = U8_T(0x17);
    biz_msg_2way.pan_position = S32_T(0);
    biz_msg_2way.tilt_position = S32_T(0);

    ARRAY_FOREACH (pan_ab_values, i) {
        biz_msg_2way.seq_id = i + 1;
        biz_msg_2way.pan_position = pan_ab_values[i];
        biz_msg_2way.tilt_position = tilt_ab_values[i];
        setDefaultValidCondition(U16_T(1));
        EXPECT_CALL(
            pan_tilt_infra_if_mock_,
            movePanTiltAbsolute(Eq(biz_msg_2way.pan_speed),
                                Eq(biz_msg_2way.tilt_speed),
                                Eq(status.panSinDataToViscaData(biz_msg_2way.pan_position)),
                                Eq(static_cast<u32_t>(status.tiltSinDataToViscaData(biz_msg_2way.tilt_position))),
                                _,
                                _))
            .Times(1)
            .WillOnce(Return());
        handler_->handleRequest(biz_msg_2way);
    }
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltAbsolutePositionError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetPanTiltAbsolutePositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.pan_speed = U8_T(0x01);
    biz_msg_2way.tilt_speed = U8_T(0x01);
    biz_msg_2way.pan_position = S32_T(0x1234);
    biz_msg_2way.tilt_position = S32_T(0x1234);

    PtzfStatusIf status;
    setDefaultValidCondition(U16_T(3));
    EXPECT_CALL(pan_tilt_infra_if_mock_,
                movePanTiltAbsolute(Eq(biz_msg_2way.pan_speed),
                                    Eq(biz_msg_2way.tilt_speed),
                                    Eq(status.panSinDataToViscaData(biz_msg_2way.pan_position)),
                                    Eq(static_cast<u32_t>(status.tiltSinDataToViscaData(biz_msg_2way.tilt_position))),
                                    _,
                                    _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);

    // Ptzf Error Comp Reply
    biz_msg_2way.pan_position = S32_T(42462);
    biz_msg_2way.tilt_position = S32_T(-7078);

    handler_->handleRequest(biz_msg_2way);

    biz_msg_2way.pan_speed = U8_T(0x19);
    biz_msg_2way.tilt_speed = U8_T(0x18);
    biz_msg_2way.pan_position = S32_T(0x1234);
    biz_msg_2way.tilt_position = S32_T(0x1234);

    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltAbsolutePosition2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetPanTiltAbsolutePositionRequest biz_msg1;
    SetPanTiltAbsolutePositionRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.pan_speed = U8_T(0x01);
    biz_msg1.tilt_speed = U8_T(0x01);
    biz_msg1.pan_position = S32_T(0x1234);
    biz_msg1.tilt_position = S32_T(0x1234);

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.pan_speed = U8_T(0x01);
    biz_msg2.tilt_speed = U8_T(0x01);
    biz_msg2.pan_position = S32_T(0x1234);
    biz_msg2.tilt_position = S32_T(0x1234);

    PtzfStatusIf status;
    setDefaultValidCondition(U16_T(2));
    EXPECT_CALL(pan_tilt_infra_if_mock_,
                movePanTiltAbsolute(Eq(biz_msg1.pan_speed),
                                    Eq(biz_msg1.tilt_speed),
                                    Eq(status.panSinDataToViscaData(biz_msg1.pan_position)),
                                    Eq(static_cast<u32_t>(status.tiltSinDataToViscaData(biz_msg1.tilt_position))),
                                    _,
                                    _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(pan_tilt_infra_if_mock_,
                movePanTiltAbsolute(Eq(biz_msg2.pan_speed),
                                    Eq(biz_msg2.tilt_speed),
                                    Eq(status.panSinDataToViscaData(biz_msg2.pan_position)),
                                    Eq(static_cast<u32_t>(status.tiltSinDataToViscaData(biz_msg2.tilt_position))),
                                    _,
                                    _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltRelativePositionSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetPanTiltRelativePositionRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.pan_speed = U8_T(0x18);
    biz_msg_1way.tilt_speed = U8_T(0x17);
    biz_msg_1way.pan_position = S32_T(0x1234);
    biz_msg_1way.tilt_position = S32_T(0x1234);

    PtzfStatusIf status;
    EXPECT_CALL(pan_tilt_infra_if_mock_,
                movePanTiltRelative(Eq(biz_msg_1way.pan_speed),
                                    Eq(biz_msg_1way.tilt_speed),
                                    Eq(status.panSinDataToViscaData(biz_msg_1way.pan_position)),
                                    Eq(static_cast<u32_t>(status.tiltSinDataToViscaData(biz_msg_1way.tilt_position))),
                                    _,
                                    _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetPanTiltRelativePositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(0);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.pan_speed = U8_T(0x01);
    biz_msg_2way.tilt_speed = U8_T(0x17);
    biz_msg_2way.pan_position = S32_T(0);
    biz_msg_2way.tilt_position = S32_T(0);

    ARRAY_FOREACH (pan_rel_values, i) {
        biz_msg_2way.seq_id = i + 1;
        biz_msg_2way.pan_position = pan_rel_values[i];
        biz_msg_2way.tilt_position = tilt_rel_values[i];
        EXPECT_CALL(pan_tilt_infra_if_mock_,
                    movePanTiltRelative(Eq(biz_msg_2way.pan_speed),
                                        Eq(biz_msg_2way.tilt_speed),
                                        Eq(status.panSinDataToViscaData(biz_msg_2way.pan_position)),
                                        Eq(status.tiltSinDataToViscaData(biz_msg_2way.tilt_position)),
                                        _,
                                        _))
            .Times(1)
            .WillOnce(Return());
        handler_->handleRequest(biz_msg_2way);
    }
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltRelativePositionError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetPanTiltRelativePositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.pan_speed = U8_T(0x01);
    biz_msg_2way.tilt_speed = U8_T(0x01);
    biz_msg_2way.pan_position = S32_T(0x1234);
    biz_msg_2way.tilt_position = S32_T(0x1234);
    s32_t pan_round_position = S32_T(0);
    s32_t tilt_round_position = S32_T(0);

    PtzfStatusIf status;
    pan_round_position = status.roundPTZPanRelativeMoveRange(biz_msg_2way.pan_position);
    tilt_round_position = status.roundPTZTiltRelativeMoveRange(biz_msg_2way.tilt_position);
    EXPECT_CALL(pan_tilt_infra_if_mock_,
                movePanTiltRelative(Eq(biz_msg_2way.pan_speed),
                                    Eq(biz_msg_2way.tilt_speed),
                                    Eq(status.panSinDataToViscaData(pan_round_position)),
                                    Eq(static_cast<u32_t>(status.tiltSinDataToViscaData(tilt_round_position))),
                                    _,
                                    _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);

    // Ptzf Error Comp Reply
    biz_msg_2way.pan_position = S32_T(80207);
    biz_msg_2way.tilt_position = S32_T(-28309);

    handler_->handleRequest(biz_msg_2way);

    biz_msg_2way.pan_speed = U8_T(0x19);
    biz_msg_2way.tilt_speed = U8_T(0x17);
    biz_msg_2way.pan_position = S32_T(0x1234);
    biz_msg_2way.tilt_position = S32_T(0x1234);

    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltRelativePosition2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetPanTiltRelativePositionRequest biz_msg1;
    SetPanTiltRelativePositionRequest biz_msg2;
    s32_t pan_round_position1 = S32_T(0);
    s32_t tilt_round_position1 = S32_T(0);
    s32_t pan_round_position2 = S32_T(0);
    s32_t tilt_round_position2 = S32_T(0);

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.pan_speed = U8_T(0x01);
    biz_msg1.tilt_speed = U8_T(0x01);
    biz_msg1.pan_position = S32_T(0x1234);
    biz_msg1.tilt_position = S32_T(0x1234);

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.pan_speed = U8_T(0x01);
    biz_msg2.tilt_speed = U8_T(0x01);
    biz_msg2.pan_position = S32_T(0x1234);
    biz_msg2.tilt_position = S32_T(0x1234);

    PtzfStatusIf status;
    pan_round_position1 = status.roundPTZPanRelativeMoveRange(biz_msg1.pan_position);
    tilt_round_position1 = status.roundPTZTiltRelativeMoveRange(biz_msg1.tilt_position);
    pan_round_position2 = status.roundPTZPanRelativeMoveRange(biz_msg2.pan_position);
    tilt_round_position2 = status.roundPTZTiltRelativeMoveRange(biz_msg2.tilt_position);
    EXPECT_CALL(pan_tilt_infra_if_mock_,
                movePanTiltRelative(Eq(biz_msg1.pan_speed),
                                    Eq(biz_msg2.tilt_speed),
                                    Eq(status.panSinDataToViscaData(pan_round_position1)),
                                    Eq(static_cast<u32_t>(status.tiltSinDataToViscaData(tilt_round_position1))),
                                    _,
                                    _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(
        pan_tilt_infra_if_mock_,
        movePanTiltRelative(
            Eq(biz_msg2.pan_speed), Eq(biz_msg2.tilt_speed), Eq(pan_round_position2), Eq(tilt_round_position2), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltRelativeMoveSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetPanTiltRelativeMoveRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.direction = PAN_TILT_DIRECTION_UP;
    biz_msg_1way.amount = PTZ_RELATIVE_AMOUNT_1;

    EXPECT_CALL(controller_mock_, movePTZPanTiltRelative(_, _, _, _)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetPanTiltRelativeMoveRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_1way.direction = PAN_TILT_DIRECTION_UP;
    biz_msg_1way.amount = PTZ_RELATIVE_AMOUNT_1;

    EXPECT_CALL(controller_mock_, movePTZPanTiltRelative(_, _, _, _)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltRelativeMoveError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetPanTiltRelativeMoveRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.direction = PAN_TILT_DIRECTION_UP;
    biz_msg_2way.amount = PTZ_RELATIVE_AMOUNT_1;

    EXPECT_CALL(controller_mock_, movePTZPanTiltRelative(_, _, _, _)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg_2way);

    // Ptzf Controller error
    EXPECT_CALL(controller_mock_, movePTZPanTiltRelative(_, _, _, _)).Times(1).WillOnce(Return(ERRORCODE_OUT_OF_RANGE));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltRelativeMove2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetPanTiltRelativeMoveRequest biz_msg1;
    SetPanTiltRelativeMoveRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.direction = PAN_TILT_DIRECTION_UP;
    biz_msg1.amount = PTZ_RELATIVE_AMOUNT_1;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.direction = PAN_TILT_DIRECTION_UP;
    biz_msg2.amount = PTZ_RELATIVE_AMOUNT_1;

    EXPECT_CALL(controller_mock_, movePTZPanTiltRelative(_, _, _, _))
        .Times(2)
        .WillOnce(Return(ERRORCODE_SUCCESS))
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomRelativeMoveSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetZoomRelativeMoveRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.direction = ZOOM_DIRECTION_TELE;
    biz_msg_1way.amount = PTZ_RELATIVE_AMOUNT_1;

    EXPECT_CALL(controller_mock_, movePTZZoomRelative(_, _, _, _)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetZoomRelativeMoveRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_1way.direction = ZOOM_DIRECTION_TELE;
    biz_msg_1way.amount = PTZ_RELATIVE_AMOUNT_1;

    EXPECT_CALL(controller_mock_, movePTZZoomRelative(_, _, _, _)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomRelativeMoveError)
{
    // ### for Biz(2Way) ### //
    common::MessageQueue mq_;
    SetZoomRelativeMoveRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.direction = ZOOM_DIRECTION_TELE;
    biz_msg_2way.amount = PTZ_RELATIVE_AMOUNT_1;

    EXPECT_CALL(controller_mock_, movePTZZoomRelative(_, _, _, _)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg_2way);

    // Ptzf Controller error
    EXPECT_CALL(controller_mock_, movePTZZoomRelative(_, _, _, _)).Times(1).WillOnce(Return(ERRORCODE_OUT_OF_RANGE));
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomRelativeMove2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetZoomRelativeMoveRequest biz_msg1;
    SetZoomRelativeMoveRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.direction = ZOOM_DIRECTION_TELE;
    biz_msg1.amount = PTZ_RELATIVE_AMOUNT_1;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.direction = ZOOM_DIRECTION_TELE;
    biz_msg2.amount = PTZ_RELATIVE_AMOUNT_1;

    EXPECT_CALL(controller_mock_, movePTZZoomRelative(_, _, _, _))
        .Times(2)
        .WillOnce(Return(ERRORCODE_SUCCESS))
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusHoldSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetFocusHoldRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.focus_hold = FOCUS_HOLD_RELEASE;

    EXPECT_CALL(focus_infra_if_mock_, exeFocusHoldButton(Eq(biz_msg_1way.focus_hold), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetFocusHoldRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.focus_hold = FOCUS_HOLD_PRESS;

    EXPECT_CALL(focus_infra_if_mock_, exeFocusHoldButton(Eq(biz_msg_2way.focus_hold), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusHold2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetFocusHoldRequest biz_msg1;
    SetFocusHoldRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.focus_hold = FOCUS_HOLD_RELEASE;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.focus_hold = FOCUS_HOLD_PRESS;

    EXPECT_CALL(focus_infra_if_mock_, exeFocusHoldButton(Eq(biz_msg1.focus_hold), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, exeFocusHoldButton(Eq(biz_msg2.focus_hold), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, PushFocusSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetPushFocusRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.push_focus = PUSH_FOCUS_RELEASE;

    EXPECT_CALL(focus_infra_if_mock_, exePushFocusButton(Eq(biz_msg_1way.push_focus), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetPushFocusRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.push_focus = PUSH_FOCUS_PRESS;

    EXPECT_CALL(focus_infra_if_mock_, exePushFocusButton(Eq(biz_msg_2way.push_focus), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, PushFocus2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetPushFocusRequest biz_msg1;
    SetPushFocusRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.push_focus = PUSH_FOCUS_RELEASE;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.push_focus = PUSH_FOCUS_PRESS;

    EXPECT_CALL(focus_infra_if_mock_, exePushFocusButton(Eq(biz_msg1.push_focus), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, exePushFocusButton(Eq(biz_msg2.push_focus), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusTrackingCancelSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetFocusTrackingCancelRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.focus_tracking_cancel = FOCUS_TRACKING_CANCEL_RELEASE;

    EXPECT_CALL(focus_infra_if_mock_, setFocusTrackingCancel(Eq(biz_msg_1way.focus_tracking_cancel), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetFocusTrackingCancelRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.focus_tracking_cancel = FOCUS_TRACKING_CANCEL_PRESS;

    EXPECT_CALL(focus_infra_if_mock_, setFocusTrackingCancel(Eq(biz_msg_2way.focus_tracking_cancel), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, FocusTrackingCancel2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetFocusTrackingCancelRequest biz_msg1;
    SetFocusTrackingCancelRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.focus_tracking_cancel = FOCUS_TRACKING_CANCEL_RELEASE;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.focus_tracking_cancel = FOCUS_TRACKING_CANCEL_PRESS;

    EXPECT_CALL(focus_infra_if_mock_, setFocusTrackingCancel(Eq(biz_msg1.focus_tracking_cancel), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, setFocusTrackingCancel(Eq(biz_msg2.focus_tracking_cancel), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, HomePositionSuccess)
{
    // ### for Sircs/Biz(1Way) ### //
    HomePositionRequest msg;

    EXPECT_CALL(controller_mock_, moveToHomePosition(_, _)).Times(1).WillOnce(Return());

    handler_->handleRequest(msg);

    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    HomePositionRequest biz_msg;
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();

    EXPECT_CALL(controller_mock_, moveToHomePosition(_, _)).Times(1).WillOnce(Return());

    handler_->handleRequest(biz_msg);
}

TEST_F(PtzfControllerMessageHandlerTest, HomePositionError)
{
    // ### for Biz(2Way) ### //
    u32_t seq_id = U32_T(123456);
    common::MessageQueue mq;
    HomePositionRequest biz_msg;
    biz_msg.seq_id = seq_id;
    biz_msg.mq_name = mq.getName();

    EXPECT_CALL(controller_mock_, moveToHomePosition(_, _)).Times(1).WillOnce(Return());

    handler_->handleRequest(biz_msg);
}

TEST_F(PtzfControllerMessageHandlerTest, HomePosition2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    HomePositionRequest biz_msg1;
    HomePositionRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();

    EXPECT_CALL(controller_mock_, moveToHomePosition(_, _)).Times(2).WillOnce(Return()).WillOnce(Return());

    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, Finalize)
{
    EXPECT_CALL(finalizre_mock_, finalize()).WillOnce(Return());
    Finalize msg;
    handler_->handleRequest(msg);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomSpeedScaleSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetZoomSpeedScaleRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.zoom_speed_scale = U8_T(10);

    EXPECT_CALL(zoom_infra_if_mock_, setZoomSpeedScale(Eq(biz_msg_1way.zoom_speed_scale), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetZoomSpeedScaleRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.zoom_speed_scale = U8_T(100);

    EXPECT_CALL(zoom_infra_if_mock_, setZoomSpeedScale(Eq(biz_msg_2way.zoom_speed_scale), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, ZoomSpeedScale2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetZoomSpeedScaleRequest biz_msg1;
    SetZoomSpeedScaleRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.zoom_speed_scale = U8_T(10);

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.zoom_speed_scale = U8_T(100);

    EXPECT_CALL(zoom_infra_if_mock_, setZoomSpeedScale(Eq(biz_msg1.zoom_speed_scale), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(zoom_infra_if_mock_, setZoomSpeedScale(Eq(biz_msg2.zoom_speed_scale), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, PushAfModeSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetPushAFModeRequestForBiz biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way.mode = PUSH_AF_MODE_AF;

    EXPECT_CALL(focus_infra_if_mock_, setPushAFMode(Eq(biz_msg_1way.mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetPushAFModeRequestForBiz biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way.mode = PUSH_AF_MODE_AF;

    EXPECT_CALL(focus_infra_if_mock_, setPushAFMode(Eq(biz_msg_2way.mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, PushAfMode2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetPushAFModeRequestForBiz biz_msg1;
    SetPushAFModeRequestForBiz biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1.mode = PUSH_AF_MODE_AF;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2.mode = PUSH_AF_MODE_AF;

    EXPECT_CALL(focus_infra_if_mock_, setPushAFMode(Eq(biz_msg1.mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, setPushAFMode(Eq(biz_msg2.mode), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, SetStandbyMode)
{
    EXPECT_CALL(backup_infra_if_mock_, getStandbyMode(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(StandbyMode::NEUTRAL), Return(ERRORCODE_SUCCESS)));

    for (auto& item : STANDBY_MODE_LIST) {
        EXPECT_CALL(backup_infra_if_mock_, setStandbyMode(item)).WillOnce(Return(ERRORCODE_SUCCESS));
        common::MessageQueue mq;
        SetStandbyModeRequest request(item);
        handler_->handleBypassMessageWithReply(request, mq.getName());

        SetStandbyModeResult reply;
        mq.pend(reply);

        ASSERT_EQ(ERRORCODE_SUCCESS, reply.error_);
        ASSERT_TRUE(::testing::Mock::VerifyAndClear(&backup_infra_if_mock_));
    }

    /* error case (argument error) */
    {
        EXPECT_CALL(backup_infra_if_mock_, setStandbyMode(_)).Times(0);
        common::MessageQueue mq;
        SetStandbyModeRequest request(static_cast<StandbyMode>(-1));
        handler_->handleBypassMessageWithReply(request, mq.getName());

        SetStandbyModeResult reply;
        mq.pend(reply);

        ASSERT_EQ(ERRORCODE_ARG, reply.error_);
        ASSERT_TRUE(::testing::Mock::VerifyAndClear(&backup_infra_if_mock_));
    }

    /* error case (execute error) */
    {
        EXPECT_CALL(backup_infra_if_mock_, setStandbyMode(_)).WillOnce(Return(ERRORCODE_EXEC));
        common::MessageQueue mq;
        SetStandbyModeRequest request(STANDBY_MODE_LIST[0]);
        handler_->handleBypassMessageWithReply(request, mq.getName());

        SetStandbyModeResult reply;
        mq.pend(reply);

        ASSERT_EQ(ERRORCODE_EXEC, reply.error_);
        ASSERT_TRUE(::testing::Mock::VerifyAndClear(&backup_infra_if_mock_));
    }
}

TEST_F(PtzfControllerMessageHandlerTest, SetStandbyModeViscaMessage)
{
    typedef visca::ViscaMessageSequence<SetStandbyModeRequest> RequestMessage;
    u32_t packet_id = U32_T(1);

    for (auto& item : STANDBY_MODE_LIST) {
        EXPECT_CALL(backup_infra_if_mock_, setStandbyMode(item)).WillOnce(Return(ERRORCODE_SUCCESS));
        EXPECT_CALL(visca_if_mock_, sendCompRequest(packet_id, ERRORCODE_SUCCESS))
            .WillOnce(InvokeWithoutArgs(postSyncMessage));
        common::MessageQueue mq;
        SetStandbyModeRequest payload(item);
        RequestMessage request(packet_id, payload);
        handler_->handleBypassMessageWithReply(request, mq.getName());

        visca::AckResponse reply;
        mq.pend(reply);

        pendSyncMessage();

        ASSERT_EQ(ERRORCODE_SUCCESS, reply.status);
        ASSERT_TRUE(::testing::Mock::VerifyAndClear(&backup_infra_if_mock_));
        ASSERT_TRUE(::testing::Mock::VerifyAndClear(&visca_if_mock_));
        ++packet_id;
    }

    /* error case (argument error) */
    {
        EXPECT_CALL(backup_infra_if_mock_, setStandbyMode(_)).Times(0);
        EXPECT_CALL(visca_if_mock_, sendCompRequest(_, _)).Times(0);
        common::MessageQueue mq;
        SetStandbyModeRequest payload(static_cast<StandbyMode>(-1));
        RequestMessage request(packet_id, payload);
        handler_->handleBypassMessageWithReply(request, mq.getName());

        visca::AckResponse reply;
        mq.pend(reply);

        ASSERT_EQ(ERRORCODE_ARG, reply.status);
        ASSERT_TRUE(::testing::Mock::VerifyAndClear(&backup_infra_if_mock_));
        ASSERT_TRUE(::testing::Mock::VerifyAndClear(&visca_if_mock_));
        ++packet_id;
    }

    /* error case (execute error) */
    {
        EXPECT_CALL(backup_infra_if_mock_, setStandbyMode(_)).WillOnce(Return(ERRORCODE_EXEC));
        EXPECT_CALL(visca_if_mock_, sendCompRequest(packet_id, ERRORCODE_EXEC))
            .WillOnce(InvokeWithoutArgs(postSyncMessage));
        common::MessageQueue mq;
        SetStandbyModeRequest payload(STANDBY_MODE_LIST[0]);
        RequestMessage request(packet_id, payload);
        handler_->handleBypassMessageWithReply(request, mq.getName());

        visca::AckResponse reply;
        mq.pend(reply);

        pendSyncMessage();

        ASSERT_EQ(ERRORCODE_SUCCESS, reply.status);
        ASSERT_TRUE(::testing::Mock::VerifyAndClear(&backup_infra_if_mock_));
        ASSERT_TRUE(::testing::Mock::VerifyAndClear(&visca_if_mock_));
        ++packet_id;
    }
}

TEST_F(PtzfControllerMessageHandlerTest, SetStandbyModeBizMessage)
{
    typedef BizMessage<SetStandbyModeRequest> RequestMessage;
    u32_t seq_id = U32_T(1);

    for (auto& item : STANDBY_MODE_LIST) {
        EXPECT_CALL(backup_infra_if_mock_, setStandbyMode(item)).WillOnce(Return(ERRORCODE_SUCCESS));
        common::MessageQueue mq;
        SetStandbyModeRequest payload(item);
        RequestMessage request(seq_id, mq.getName(), payload);
        handler_->handleBypassMessage(request);

        ptzf::message::PtzfExecComp reply;
        mq.pend(reply);

        ASSERT_EQ(ERRORCODE_SUCCESS, reply.error);
        ASSERT_EQ(seq_id, reply.seq_id);
        ASSERT_TRUE(::testing::Mock::VerifyAndClear(&backup_infra_if_mock_));
        ++seq_id;
    }

    /* error case (argument error) */
    {
        EXPECT_CALL(backup_infra_if_mock_, setStandbyMode(_)).Times(0);
        common::MessageQueue mq;
        SetStandbyModeRequest payload(static_cast<StandbyMode>(-1));
        RequestMessage request(seq_id, mq.getName(), payload);
        handler_->handleBypassMessage(request);

        ptzf::message::PtzfExecComp reply;
        mq.pend(reply);

        ASSERT_EQ(ERRORCODE_ARG, reply.error);
        ASSERT_EQ(seq_id, reply.seq_id);
        ASSERT_TRUE(::testing::Mock::VerifyAndClear(&backup_infra_if_mock_));
        ++seq_id;
    }

    /* error case (execute error) */
    {
        EXPECT_CALL(backup_infra_if_mock_, setStandbyMode(_)).WillOnce(Return(ERRORCODE_EXEC));
        common::MessageQueue mq;
        SetStandbyModeRequest payload(STANDBY_MODE_LIST[0]);
        RequestMessage request(seq_id, mq.getName(), payload);
        handler_->handleBypassMessage(request);

        ptzf::message::PtzfExecComp reply;
        mq.pend(reply);

        ASSERT_EQ(ERRORCODE_EXEC, reply.error);
        ASSERT_EQ(seq_id, reply.seq_id);
        ASSERT_TRUE(::testing::Mock::VerifyAndClear(&backup_infra_if_mock_));
        ++seq_id;
    }
}

TEST_F(PtzfControllerMessageHandlerTest, AfSubjShiftSensSuccess)
{
    // ### for Biz(1Way) ###
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    BizMessage<SetAfSubjShiftSensRequest> biz_msg_1way;
    biz_msg_1way.seq_id = U32_T(123456);
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way().af_subj_shift_sens = 0x01;

    EXPECT_CALL(focus_infra_if_mock_, setAfSubjShiftSens(Eq(biz_msg_1way().af_subj_shift_sens), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest<BizMessage<SetAfSubjShiftSensRequest>>(biz_msg_1way);

    // ### for Biz(2Way) ### //
    BizMessage<SetAfSubjShiftSensRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().af_subj_shift_sens = 0x01;

    EXPECT_CALL(focus_infra_if_mock_, setAfSubjShiftSens(Eq(biz_msg_2way().af_subj_shift_sens), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest<BizMessage<SetAfSubjShiftSensRequest>>(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, AfSubjShiftSens2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    BizMessage<SetAfSubjShiftSensRequest> biz_msg1;
    BizMessage<SetAfSubjShiftSensRequest> biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1().af_subj_shift_sens = 0x01;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2().af_subj_shift_sens = 0x01;

    EXPECT_CALL(focus_infra_if_mock_, setAfSubjShiftSens(Eq(biz_msg1().af_subj_shift_sens), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest<BizMessage<SetAfSubjShiftSensRequest>>(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, setAfSubjShiftSens(Eq(biz_msg2().af_subj_shift_sens), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest<BizMessage<SetAfSubjShiftSensRequest>>(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, AfTransitionSpeedSuccess)
{
    // ### for Biz(1Way) ###
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    BizMessage<SetAfTransitionSpeedRequest> biz_msg_1way;
    biz_msg_1way.seq_id = U32_T(123456);
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way().af_transition_speed = 0x01;

    EXPECT_CALL(focus_infra_if_mock_, setAfTransitionSpeed(Eq(biz_msg_1way().af_transition_speed), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest<BizMessage<SetAfTransitionSpeedRequest>>(biz_msg_1way);

    // ### for Biz(2Way) ### //
    BizMessage<SetAfTransitionSpeedRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().af_transition_speed = 0x01;

    EXPECT_CALL(focus_infra_if_mock_, setAfTransitionSpeed(Eq(biz_msg_2way().af_transition_speed), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest<BizMessage<SetAfTransitionSpeedRequest>>(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, AfTransitionSpeed2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    BizMessage<SetAfTransitionSpeedRequest> biz_msg1;
    BizMessage<SetAfTransitionSpeedRequest> biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1().af_transition_speed = 0x01;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2().af_transition_speed = 0x01;

    EXPECT_CALL(focus_infra_if_mock_, setAfTransitionSpeed(Eq(biz_msg1().af_transition_speed), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest<BizMessage<SetAfTransitionSpeedRequest>>(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, setAfTransitionSpeed(Eq(biz_msg2().af_transition_speed), _, _))
        .Times(1)
        .WillOnce(Return(true));
    handler_->handleRequest<BizMessage<SetAfTransitionSpeedRequest>>(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltSpeedModeSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetPanTiltSpeedModeRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.reply_name = blankName;
    biz_msg_1way.speed_mode = PAN_TILT_SPEED_MODE_NORMAL;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSpeedMode(Eq(biz_msg_1way.speed_mode), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetPanTiltSpeedModeRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.reply_name = mq_.getName();
    biz_msg_2way.speed_mode = PAN_TILT_SPEED_MODE_NORMAL;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSpeedMode(Eq(biz_msg_2way.speed_mode), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltSpeedMode2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetPanTiltSpeedModeRequest biz_msg1;
    SetPanTiltSpeedModeRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.reply_name = mq1.getName();
    biz_msg1.speed_mode = PAN_TILT_SPEED_MODE_NORMAL;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.reply_name = mq2.getName();
    biz_msg2.speed_mode = PAN_TILT_SPEED_MODE_NORMAL;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSpeedMode(Eq(biz_msg1.speed_mode), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSpeedMode(Eq(biz_msg2.speed_mode), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltSpeedStepSuccess)
{
    common::MessageQueue mq_;
    PtzfStatus st;
    PtzfStatusIf st_if;
    // is No Chang
    st.setPanTiltSpeedStepConfigurationStatus(false);

    // ### for UI(Sircs) ### //
    SetPanTiltSpeedStepRequest msg;

    // Set Parameter of SpeedStep
    msg.speed_step = PAN_TILT_SPEED_STEP_NORMAL;
    st.setSpeedStep(PAN_TILT_SPEED_STEP_EXTENDED);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSpeedStep(Eq(msg.speed_step), _, _)).Times(1).WillOnce(Return());

    handler_->handleRequestWithReply(msg, mq_.getName());
    common::Task::msleep(U32_T(1));

    // Reply
    SetPanTiltSpeedStepReply comp_req;
    comp_req.status = ERRORCODE_SUCCESS;
    thread_mq_.post(comp_req);
    common::Task::msleep(U32_T(2));

    SetPanTiltSpeedStepResult result(comp_req.status);
    mq_.pend(result);
    EXPECT_EQ(ERRORCODE_SUCCESS, result.err);

    // SpeedStep is set
    EXPECT_EQ(msg.speed_step, st_if.getPanTiltSpeedStep());

    // ### for Visca ### //
    visca::ViscaMessageSequence<SetPanTiltSpeedStepRequest> visca_msg;

    // Set Parameter of SpeedStep
    visca_msg.packet_id = U32_T(123456789);
    visca_msg().speed_step = PAN_TILT_SPEED_STEP_EXTENDED;
    st.setSpeedStep(PAN_TILT_SPEED_STEP_EXTENDED);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSpeedStep(Eq(visca_msg().speed_step), _, _))
        .Times(1)
        .WillOnce(Return());

    // send Success Comp
    EXPECT_CALL(visca_if_mock_, sendCompRequest(Eq(visca_msg.packet_id), Eq(ERRORCODE_SUCCESS)))
        .Times(1)
        .WillOnce(Return());

    handler_->handleRequestWithReply(visca_msg, mq_.getName());
    common::Task::msleep(U32_T(1));

    // Reply Ack
    visca::AckResponse ack_res;
    mq_.pend(ack_res);
    EXPECT_EQ(ERRORCODE_SUCCESS, ack_res.status);

    // Reply
    SetPanTiltSpeedStepReply comp_result;
    comp_result.status = ERRORCODE_SUCCESS;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(10));

    // SpeedStep is set
    EXPECT_EQ(visca_msg().speed_step, st_if.getPanTiltSpeedStep());

    // ### for Biz(1Way) ### //
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    BizMessage<SetPanTiltSpeedStepRequest> biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;
    biz_msg_1way().speed_step = PAN_TILT_SPEED_STEP_NORMAL;
    st.setSpeedStep(PAN_TILT_SPEED_STEP_EXTENDED);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSpeedStep(Eq(biz_msg_1way().speed_step), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);
    common::Task::msleep(U32_T(1));

    // Reply
    comp_result.status = ERRORCODE_SUCCESS;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(10));

    // SpeedStep is set
    EXPECT_EQ(biz_msg_1way().speed_step, st_if.getPanTiltSpeedStep());

    // ### for Biz(2Way) ### //
    BizMessage<SetPanTiltSpeedStepRequest> biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();
    biz_msg_2way().speed_step = PAN_TILT_SPEED_STEP_NORMAL;
    st.setSpeedStep(PAN_TILT_SPEED_STEP_EXTENDED);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSpeedStep(Eq(biz_msg_2way().speed_step), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
    common::Task::msleep(U32_T(1));

    // Reply
    comp_result.status = ERRORCODE_SUCCESS;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(10));

    ptzf::message::PtzfExecComp biz_result;
    mq_.pend(biz_result);
    EXPECT_EQ(biz_msg_2way.seq_id, biz_result.seq_id);
    EXPECT_EQ(comp_result.status, biz_result.error);

    // SpeedStep is set
    EXPECT_EQ(biz_msg_2way().speed_step, st_if.getPanTiltSpeedStep());
    mq_.unlink();
}

TEST_F(PtzfControllerMessageHandlerTest, PanTiltSpeedStep2wayMultiple)
{
    PtzfStatus st;
    PtzfStatusIf st_if;
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    BizMessage<SetPanTiltSpeedStepRequest> biz_msg1;
    BizMessage<SetPanTiltSpeedStepRequest> biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();
    biz_msg1().speed_step = PAN_TILT_SPEED_STEP_NORMAL;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();
    biz_msg2().speed_step = PAN_TILT_SPEED_STEP_EXTENDED;

    st.setSpeedStep(biz_msg2().speed_step);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanTiltSpeedStep(Eq(biz_msg1().speed_step), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg1);
    handler_->handleRequest(biz_msg2);
    common::Task::msleep(U32_T(1));

    // Reply
    SetPanTiltSpeedStepReply comp_result;
    comp_result.status = ERRORCODE_SUCCESS;
    thread_mq_.post(comp_result);
    common::Task::msleep(U32_T(10));

    ptzf::message::PtzfExecComp biz_result1;
    mq1.pend(biz_result1);
    EXPECT_EQ(biz_msg1.seq_id, biz_result1.seq_id);
    EXPECT_EQ(comp_result.status, biz_result1.error);

    ptzf::message::PtzfExecComp biz_result2;
    mq2.pend(biz_result2);
    EXPECT_EQ(biz_msg2.seq_id, biz_result2.seq_id);
    EXPECT_EQ(ERRORCODE_EXEC, biz_result2.error);

    // SpeedStep is set
    EXPECT_EQ(biz_msg1().speed_step, st_if.getPanTiltSpeedStep());
}

TEST_F(PtzfControllerMessageHandlerTest, SettingPositionSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetSettingPositionRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.reply_name = blankName;
    biz_msg_1way.setting_position = SETTING_POSITION_DESKTOP;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setSettingPosition(Eq(biz_msg_1way.setting_position), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetSettingPositionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.reply_name = mq_.getName();
    biz_msg_2way.setting_position = SETTING_POSITION_DESKTOP;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setSettingPosition(Eq(biz_msg_2way.setting_position), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, SettingPosition2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetSettingPositionRequest biz_msg1;
    SetSettingPositionRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.reply_name = mq1.getName();
    biz_msg1.setting_position = SETTING_POSITION_DESKTOP;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.reply_name = mq2.getName();
    biz_msg2.setting_position = SETTING_POSITION_DESKTOP;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setSettingPosition(Eq(biz_msg1.setting_position), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setSettingPosition(Eq(biz_msg2.setting_position), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, PanDirectionSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetPanDirectionRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.reply_name = blankName;
    biz_msg_1way.pan_direction = PAN_DIRECTION_NORMAL;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanDirection(Eq(biz_msg_1way.pan_direction), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetPanDirectionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.reply_name = mq_.getName();
    biz_msg_2way.pan_direction = PAN_DIRECTION_NORMAL;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanDirection(Eq(biz_msg_2way.pan_direction), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, PanDirection2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetPanDirectionRequest biz_msg1;
    SetPanDirectionRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.reply_name = mq1.getName();
    biz_msg1.pan_direction = PAN_DIRECTION_NORMAL;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.reply_name = mq2.getName();
    biz_msg2.pan_direction = PAN_DIRECTION_NORMAL;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanDirection(Eq(biz_msg1.pan_direction), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setPanDirection(Eq(biz_msg2.pan_direction), _, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, ExeCancelZoomPositionSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    ExeCancelZoomPositionRequestForBiz biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;

    EXPECT_CALL(zoom_infra_if_mock_, exeCancelZoomPosition(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    ExeCancelZoomPositionRequestForBiz biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();

    EXPECT_CALL(zoom_infra_if_mock_, exeCancelZoomPosition(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, ExeCancelZoomPosition2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    ExeCancelZoomPositionRequestForBiz biz_msg1;
    ExeCancelZoomPositionRequestForBiz biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();

    EXPECT_CALL(zoom_infra_if_mock_, exeCancelZoomPosition(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(zoom_infra_if_mock_, exeCancelZoomPosition(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, ExeCancelFocusPositionSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    ExeCancelFocusPositionRequestForBiz biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.mq_name = blankName;

    EXPECT_CALL(focus_infra_if_mock_, exeCancelFocusPosition(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    ExeCancelFocusPositionRequestForBiz biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.mq_name = mq_.getName();

    EXPECT_CALL(focus_infra_if_mock_, exeCancelFocusPosition(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, ExeCancelFocusPosition2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    ExeCancelFocusPositionRequestForBiz biz_msg1;
    ExeCancelFocusPositionRequestForBiz biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.mq_name = mq1.getName();

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.mq_name = mq2.getName();

    EXPECT_CALL(focus_infra_if_mock_, exeCancelFocusPosition(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(focus_infra_if_mock_, exeCancelFocusPosition(_, _)).Times(1).WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, TiltDirectionSuccess)
{
    // ### for Biz(1Way) ### //
    common::MessageQueue mq_;
    common::MessageQueueName blankName;
    gtl::copyString(blankName.name, "");
    SetTiltDirectionRequest biz_msg_1way;
    biz_msg_1way.seq_id = INVALID_SEQ_ID;
    biz_msg_1way.reply_name = blankName;
    biz_msg_1way.tilt_direction = TILT_DIRECTION_NORMAL;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setTiltDirection(Eq(biz_msg_1way.tilt_direction), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_1way);

    // ### for Biz(2Way) ### //
    SetTiltDirectionRequest biz_msg_2way;
    biz_msg_2way.seq_id = U32_T(123456);
    biz_msg_2way.reply_name = mq_.getName();
    biz_msg_2way.tilt_direction = TILT_DIRECTION_NORMAL;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setTiltDirection(Eq(biz_msg_2way.tilt_direction), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg_2way);
}

TEST_F(PtzfControllerMessageHandlerTest, TiltDirection2wayMultiple)
{
    common::MessageQueue mq1;
    common::MessageQueue mq2;
    SetTiltDirectionRequest biz_msg1;
    SetTiltDirectionRequest biz_msg2;

    biz_msg1.seq_id = U32_T(123456);
    biz_msg1.reply_name = mq1.getName();
    biz_msg1.tilt_direction = TILT_DIRECTION_NORMAL;

    biz_msg2.seq_id = U32_T(456789);
    biz_msg2.reply_name = mq2.getName();
    biz_msg2.tilt_direction = TILT_DIRECTION_NORMAL;

    EXPECT_CALL(pan_tilt_infra_if_mock_, setTiltDirection(Eq(biz_msg1.tilt_direction), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg1);

    EXPECT_CALL(pan_tilt_infra_if_mock_, setTiltDirection(Eq(biz_msg2.tilt_direction), _, _))
        .Times(1)
        .WillOnce(Return());
    handler_->handleRequest(biz_msg2);
}

TEST_F(PtzfControllerMessageHandlerTest, receiveUnlockToLockEventWithPowerOn)
{
    // PowerON中 Unlock --> Lock処理
    // (step1: Finalize処理開始)
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(2).WillRepeatedly(Return(power::PowerStatus::POWER_ON));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_UNLOCKED), Return(true)));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(true), Return(ERRORCODE_SUCCESS)));
    u32_t seq_id = U32_T(123);
    u32_t seq_id_2 = U32_T(234);
    EXPECT_CALL(sequence_id_controller_mock_, createSeqId())
        .Times(2)
        .WillOnce(Return(seq_id))
        .WillOnce(Return(seq_id_2));
    const char_t* expected_name = PtzfControllerMQ::getUipcName();
    common::MessageQueue expected_mq(expected_name);
    EXPECT_CALL(finalize_infra_if_mock_, finalizePanTilt(EqNotifyMqName(expected_mq.getName()), Eq(seq_id)))
        .Times(1)
        .WillOnce(Return(true));

    infra::PtzfStatusInfraIf status_infra_if;
    status_infra_if.setPowerOnSequenceStatus(false);
    status_infra_if.setPowerOffSequenceStatus(false);

    infra::PanTiltLockStatusChangedEvent event_message(false, true);
    handler_->handleRequest(event_message);

    // (step2: Finalize処理完了 & PT電源断処理開始)
    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(true), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(true), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(visca_if_mock_, sendPowerOffPanTiltRequest(EqNotifyMqName(expected_mq.getName()), Eq(seq_id_2)))
        .Times(1)
        .WillOnce(Return());

    FinalizePanTiltResult comp_message(seq_id, ERRORCODE_SUCCESS);
    handler_->handleRequest(comp_message);

    // (step3: PT電源断処理完了 & 制御状態更新)
    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(Eq(PAN_TILT_LOCK_STATUS_LOCKED)))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(initialize_infra_if_mock_, setPanTiltFunctionLimitForCamera(Eq(true))).Times(1).WillOnce(Return());
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(false), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(false), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_LOCKED), Return(true)));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(true), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillOnce(Return(power::PowerStatus::POWER_ON));

    visca::CompReply visca_reply(ERRORCODE_SUCCESS, seq_id_2);
    handler_->handleRequest(visca_reply);
}

TEST_F(PtzfControllerMessageHandlerTest, receiveUnlockToLockEventWithPowerOnInitializing)
{
    // PowerON中 Unlock --> Lock処理
    // * Initialize中のため, Finalizeせずに処理終了する
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillOnce(Return(power::PowerStatus::POWER_ON));
    EXPECT_CALL(sequence_id_controller_mock_, createSeqId()).Times(0);
    EXPECT_CALL(finalize_infra_if_mock_, finalizePanTilt(_, _)).Times(0);

    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(_)).Times(0);

    infra::PtzfStatusInfraIf status_infra_if;
    status_infra_if.setPowerOnSequenceStatus(true);
    status_infra_if.setPowerOffSequenceStatus(false);

    infra::PanTiltLockStatusChangedEvent event_message(false, true);
    handler_->handleRequest(event_message);
}

TEST_F(PtzfControllerMessageHandlerTest, receiveUnlockToLockEventWithPowerOff)
{
    // PowerOFF中 Unlock --> Lock処理 (制御状態更新のみ)
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(3).WillRepeatedly(Return(power::PowerStatus::POWER_OFF));
    EXPECT_CALL(sequence_id_controller_mock_, createSeqId()).Times(0);
    EXPECT_CALL(finalize_infra_if_mock_, finalizePanTilt(_, _)).Times(0);

    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(Eq(PAN_TILT_LOCK_STATUS_LOCKED)))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(2)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_UNLOCKED), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_LOCKED), Return(true)));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(true), Return(ERRORCODE_SUCCESS)));

    infra::PtzfStatusInfraIf status_infra_if;
    status_infra_if.setPowerOnSequenceStatus(false);
    status_infra_if.setPowerOffSequenceStatus(false);

    infra::PanTiltLockStatusChangedEvent event_message(false, true);
    handler_->handleRequest(event_message);
}

TEST_F(PtzfControllerMessageHandlerTest, receiveUnlockToLockEventWithPowerProcessing)
{
    // PowerON中 Unlock --> Lock処理
    // * 電源遷移中のため, Finalizeせずに処理終了する
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillOnce(Return(power::PowerStatus::PROCESSING_ON));
    EXPECT_CALL(sequence_id_controller_mock_, createSeqId()).Times(0);
    EXPECT_CALL(finalize_infra_if_mock_, finalizePanTilt(_, _)).Times(0);

    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(_)).Times(0);

    infra::PtzfStatusInfraIf status_infra_if;
    status_infra_if.setPowerOnSequenceStatus(false);
    status_infra_if.setPowerOffSequenceStatus(false);

    infra::PanTiltLockStatusChangedEvent event_message(false, true);
    handler_->handleRequest(event_message);
}

TEST_F(PtzfControllerMessageHandlerTest, receiveLockToUnlockEventWithPowerOn)
{
    // PowerON中 Lock --> Unlock処理
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(2).WillRepeatedly(Return(power::PowerStatus::POWER_ON));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_LOCKED), Return(true)));
    u32_t seq_id = U32_T(123);
    EXPECT_CALL(sequence_id_controller_mock_, createSeqId()).Times(1).WillOnce(Return(seq_id));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS)));

    infra::PtzfStatusInfraIf status_infra_if;
    status_infra_if.setPowerOnSequenceStatus(false);
    status_infra_if.setPowerOffSequenceStatus(false);

    // (step1: PT電源供給処理開始)
    EXPECT_CALL(initialize_infra_if_mock_, setPowerOnSequenceStatus(Eq(true), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(true), _)).Times(1).WillOnce(Return());
    const char_t* expected_name = PtzfControllerMQ::getUipcName();
    common::MessageQueue expected_mq(expected_name);
    EXPECT_CALL(visca_if_mock_, sendPowerOnPanTiltRequest(EqNotifyMqName(expected_mq.getName()), Eq(seq_id)))
        .Times(1)
        .WillOnce(Return());

    infra::PanTiltLockStatusChangedEvent event_message(true, false);
    handler_->handleRequest(event_message);

    // (step2: PT電源供給処理完了 & 制御状態更新)
    // * 通電アンロックに遷移することを確認
    EXPECT_CALL(ptzf_status_infra_if_mock_,
                setPanTiltLockControlStatus(Eq(PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING)))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(false), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(initialize_infra_if_mock_, setPowerOnSequenceStatus(Eq(false), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING), Return(true)));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillOnce(Return(power::PowerStatus::POWER_ON));

    visca::CompReply visca_reply(ERRORCODE_SUCCESS, seq_id);
    handler_->handleRequest(visca_reply);
}

TEST_F(PtzfControllerMessageHandlerTest, receiveLockToUnlockEventWithPowerOnCancel1)
{
    // PowerON中 Lock --> Unlock処理
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(3).WillRepeatedly(Return(power::PowerStatus::POWER_ON));
    EXPECT_CALL(sequence_id_controller_mock_, createSeqId()).Times(0);
    EXPECT_CALL(visca_if_mock_, sendPowerOnPanTiltRequest(_, _)).Times(0);

    // * 処理中にLock状態に変更し、処理終了させる(visca処理前のタイミング)
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(3)
        .WillOnce(DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS)))
        .WillRepeatedly(DoAll(SetArgReferee<0>(true), Return(ERRORCODE_SUCCESS)));

    // * 中断したのでLOCKEDに戻ることを確認
    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(Eq(PAN_TILT_LOCK_STATUS_LOCKED)))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(2)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_LOCKED), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_LOCKED), Return(true)));

    infra::PtzfStatusInfraIf status_infra_if;
    status_infra_if.setPowerOnSequenceStatus(false);
    status_infra_if.setPowerOffSequenceStatus(false);

    infra::PanTiltLockStatusChangedEvent event_message(true, false);
    handler_->handleRequest(event_message);
}

TEST_F(PtzfControllerMessageHandlerTest, receiveLockToUnlockEventWithPowerOnCancel2)
{
    // PowerON中 Lock --> Unlock処理
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(2).WillRepeatedly(Return(power::PowerStatus::POWER_ON));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_LOCKED), Return(true)));
    u32_t seq_id = U32_T(123);
    u32_t seq_id_2 = U32_T(234);
    EXPECT_CALL(sequence_id_controller_mock_, createSeqId())
        .Times(2)
        .WillOnce(Return(seq_id))
        .WillOnce(Return(seq_id_2));

    // * 処理中にLock状態に変更し、処理終了させる(visca処理完了のタイミング)
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(3)
        .WillOnce(DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS)))
        .WillOnce(DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS)))
        .WillOnce(DoAll(SetArgReferee<0>(true), Return(ERRORCODE_SUCCESS)));

    infra::PtzfStatusInfraIf status_infra_if;
    status_infra_if.setPowerOnSequenceStatus(false);
    status_infra_if.setPowerOffSequenceStatus(false);

    // (step1: PT電源供給処理開始)
    EXPECT_CALL(initialize_infra_if_mock_, setPowerOnSequenceStatus(Eq(true), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(true), _)).Times(2).WillRepeatedly(Return());
    const char_t* expected_name = PtzfControllerMQ::getUipcName();
    common::MessageQueue expected_mq(expected_name);
    EXPECT_CALL(visca_if_mock_, sendPowerOnPanTiltRequest(EqNotifyMqName(expected_mq.getName()), Eq(seq_id)))
        .Times(1)
        .WillOnce(Return());

    infra::PanTiltLockStatusChangedEvent event_message(true, false);
    handler_->handleRequest(event_message);

    // (step2: PT電源供給処理完了 & 制御状態更新)
    // * このタイミングで中断判定 --> 電源断処理に移行
    EXPECT_CALL(initialize_infra_if_mock_, setPowerOnSequenceStatus(Eq(false), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(true), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(visca_if_mock_, sendPowerOffPanTiltRequest(EqNotifyMqName(expected_mq.getName()), Eq(seq_id_2)))
        .Times(1)
        .WillOnce(Return());

    visca::CompReply visca_reply(ERRORCODE_SUCCESS, seq_id);
    handler_->handleRequest(visca_reply);

    // (step3: PT電源断処理完了 & 制御状態更新)
    // * 中断したのでLOCKEDに戻ることを確認
    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(Eq(PAN_TILT_LOCK_STATUS_LOCKED)))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(initialize_infra_if_mock_, setPanTiltFunctionLimitForCamera(Eq(true))).Times(1).WillOnce(Return());
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(false), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(false), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_LOCKED), Return(true)));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(true), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillOnce(Return(power::PowerStatus::POWER_ON));

    visca::CompReply visca_reply_2(ERRORCODE_SUCCESS, seq_id_2);
    handler_->handleRequest(visca_reply_2);
}

TEST_F(PtzfControllerMessageHandlerTest, receiveLockToUnlockEventWithPowerOnFinalizing)
{
    // PowerON中 Lcok --> Unlock処理
    // * Finalize中のため, Initializeせずに処理終了する
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillOnce(Return(power::PowerStatus::POWER_ON));
    EXPECT_CALL(sequence_id_controller_mock_, createSeqId()).Times(0);
    EXPECT_CALL(visca_if_mock_, sendPowerOnPanTiltRequest(_, _)).Times(0);

    // * 制御状態を更新しないことを確認
    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(_)).Times(0);

    infra::PtzfStatusInfraIf status_infra_if;
    status_infra_if.setPowerOnSequenceStatus(false);
    status_infra_if.setPowerOffSequenceStatus(true);

    infra::PanTiltLockStatusChangedEvent event_message(true, false);
    handler_->handleRequest(event_message);
}

TEST_F(PtzfControllerMessageHandlerTest, receiveLockToUnlockEventWithPowerOff)
{
    // PowerOFF中 Lcok --> Unlock処理
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(3).WillRepeatedly(Return(power::PowerStatus::POWER_OFF));
    EXPECT_CALL(sequence_id_controller_mock_, createSeqId()).Times(0);
    EXPECT_CALL(visca_if_mock_, sendPowerOnPanTiltRequest(_, _)).Times(0);
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(3)
        .WillRepeatedly(DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS)));

    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(Eq(PAN_TILT_LOCK_STATUS_UNLOCKED)))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(2)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_LOCKED), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_UNLOCKED), Return(true)));

    infra::PtzfStatusInfraIf status_infra_if;
    status_infra_if.setPowerOnSequenceStatus(false);
    status_infra_if.setPowerOffSequenceStatus(false);

    infra::PanTiltLockStatusChangedEvent event_message(true, false);
    handler_->handleRequest(event_message);
}

TEST_F(PtzfControllerMessageHandlerTest, receiveLockToUnlockEventWithPowerProcessing)
{
    // PowerON中 Lcok --> Unlock処理
    // * 電源遷移中のため, Finalizeせずに処理終了する
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillOnce(Return(power::PowerStatus::PROCESSING_OFF));
    EXPECT_CALL(sequence_id_controller_mock_, createSeqId()).Times(0);
    EXPECT_CALL(visca_if_mock_, sendPowerOnPanTiltRequest(_, _)).Times(0);

    // * 制御状態を更新しないことを確認
    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(_)).Times(0);

    infra::PtzfStatusInfraIf status_infra_if;
    status_infra_if.setPowerOnSequenceStatus(false);
    status_infra_if.setPowerOffSequenceStatus(false);

    infra::PanTiltLockStatusChangedEvent event_message(true, false);
    handler_->handleRequest(event_message);
}

TEST_F(PtzfControllerMessageHandlerTest, receiveLockToUnlockFollowingUnlockToLock)
{
    // PowerON中 Lock --> Unlock処理
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(2).WillRepeatedly(Return(power::PowerStatus::POWER_ON));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_LOCKED), Return(true)));
    const u32_t seq_id_list[] = { U32_T(123), U32_T(234), U32_T(345) };
    EXPECT_CALL(sequence_id_controller_mock_, createSeqId())
        .Times(ARRAY_LENGTH(seq_id_list))
        .WillOnce(Return(seq_id_list[0]))
        .WillOnce(Return(seq_id_list[1]))
        .WillOnce(Return(seq_id_list[2]));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS)));

    infra::PtzfStatusInfraIf status_infra_if;
    status_infra_if.setPowerOnSequenceStatus(false);
    status_infra_if.setPowerOffSequenceStatus(false);

    // (Initialize-step1: PT電源供給処理開始)
    EXPECT_CALL(initialize_infra_if_mock_, setPowerOnSequenceStatus(Eq(true), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(true), _)).Times(1).WillOnce(Return());
    const char_t* expected_name = PtzfControllerMQ::getUipcName();
    const common::MessageQueue expected_mq(expected_name);
    EXPECT_CALL(visca_if_mock_, sendPowerOnPanTiltRequest(EqNotifyMqName(expected_mq.getName()), Eq(seq_id_list[0])))
        .Times(1)
        .WillOnce(Return());

    const infra::PanTiltLockStatusChangedEvent event_message(true, false);
    handler_->handleRequest(event_message);

    // (Initialize-step2: PT電源供給処理完了 & 制御状態更新)
    // * 通電アンロックに遷移することを確認
    EXPECT_CALL(ptzf_status_infra_if_mock_,
                setPanTiltLockControlStatus(Eq(PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING)))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(false), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(initialize_infra_if_mock_, setPowerOnSequenceStatus(Eq(false), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING), Return(true)));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(2)
        .WillOnce(DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS)))
        .WillOnce(DoAll(SetArgReferee<0>(true), Return(ERRORCODE_SUCCESS))); // この時点でLock状態になっていることを検出
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillOnce(Return(power::PowerStatus::POWER_ON));

    // (Finalize-step1: Finalize処理開始)
    // * 再度ロック状態に遷移するための処理が実行されることを確認
    EXPECT_CALL(finalize_infra_if_mock_, finalizePanTilt(EqNotifyMqName(expected_mq.getName()), Eq(seq_id_list[1])))
        .Times(1)
        .WillOnce(Return(true));

    const visca::CompReply visca_reply_initialize(ERRORCODE_SUCCESS, seq_id_list[0]);
    handler_->handleRequest(visca_reply_initialize);

    // (Finalize-step2: Finalize処理完了 & PT電源断処理開始)
    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(true), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(true), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(visca_if_mock_, sendPowerOffPanTiltRequest(EqNotifyMqName(expected_mq.getName()), Eq(seq_id_list[2])))
        .Times(1)
        .WillOnce(Return());

    const FinalizePanTiltResult comp_message_finalize(seq_id_list[1], ERRORCODE_SUCCESS);
    handler_->handleRequest(comp_message_finalize);

    // (Finalize-step3: PT電源断処理完了 & 制御状態更新)
    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(Eq(PAN_TILT_LOCK_STATUS_LOCKED)))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(initialize_infra_if_mock_, setPanTiltFunctionLimitForCamera(Eq(true))).Times(1).WillOnce(Return());
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(false), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(false), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_LOCKED), Return(true)));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(true), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillOnce(Return(power::PowerStatus::POWER_ON));

    const visca::CompReply visca_reply_finalize(ERRORCODE_SUCCESS, seq_id_list[2]);
    handler_->handleRequest(visca_reply_finalize);
}

TEST_F(PtzfControllerMessageHandlerTest, receiveUnlockToLockFollowingLockToUnlock)
{
    // PowerON中 Unlock --> Lock処理
    // (step1: Finalize処理開始)
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(2).WillRepeatedly(Return(power::PowerStatus::POWER_ON));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_UNLOCKED), Return(true)));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(true), Return(ERRORCODE_SUCCESS)));
    const u32_t seq_id_list[] = { U32_T(456), U32_T(567), U32_T(678) };
    EXPECT_CALL(sequence_id_controller_mock_, createSeqId())
        .Times(ARRAY_LENGTH(seq_id_list))
        .WillOnce(Return(seq_id_list[0]))
        .WillOnce(Return(seq_id_list[1]))
        .WillOnce(Return(seq_id_list[2]));
    const char_t* expected_name = PtzfControllerMQ::getUipcName();
    const common::MessageQueue expected_mq(expected_name);
    EXPECT_CALL(finalize_infra_if_mock_, finalizePanTilt(EqNotifyMqName(expected_mq.getName()), Eq(seq_id_list[0])))
        .Times(1)
        .WillOnce(Return(true));

    infra::PtzfStatusInfraIf status_infra_if;
    status_infra_if.setPowerOnSequenceStatus(false);
    status_infra_if.setPowerOffSequenceStatus(false);

    const infra::PanTiltLockStatusChangedEvent event_message(false, true);
    handler_->handleRequest(event_message);

    // (step2: Finalize処理完了 & PT電源断処理開始)
    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(true), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(true), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(visca_if_mock_, sendPowerOffPanTiltRequest(EqNotifyMqName(expected_mq.getName()), Eq(seq_id_list[1])))
        .Times(1)
        .WillOnce(Return());

    const FinalizePanTiltResult comp_message_finalize(seq_id_list[0], ERRORCODE_SUCCESS);
    handler_->handleRequest(comp_message_finalize);

    // (step3: PT電源断処理完了 & 制御状態更新)
    EXPECT_CALL(ptzf_status_infra_if_mock_, setPanTiltLockControlStatus(Eq(PAN_TILT_LOCK_STATUS_LOCKED)))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(initialize_infra_if_mock_, setPanTiltFunctionLimitForCamera(Eq(true))).Times(1).WillOnce(Return());
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(false), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(finalize_infra_if_mock_, setPowerOffSequenceStatus(Eq(false), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_LOCKED), Return(true)));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(2)
        .WillOnce(
            DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS))) // この時点でUnlock状態になっていることを検出
        .WillOnce(DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillOnce(Return(power::PowerStatus::POWER_ON));

    // (step1: PT電源供給処理開始)
    EXPECT_CALL(initialize_infra_if_mock_, setPowerOnSequenceStatus(Eq(true), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(true), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(visca_if_mock_, sendPowerOnPanTiltRequest(EqNotifyMqName(expected_mq.getName()), Eq(seq_id_list[2])))
        .Times(1)
        .WillOnce(Return());

    const visca::CompReply visca_reply_finalize(ERRORCODE_SUCCESS, seq_id_list[1]);
    handler_->handleRequest(visca_reply_finalize);

    // (step2: PT電源供給処理完了 & 制御状態更新)
    // * 通電アンロックに遷移することを確認
    EXPECT_CALL(ptzf_status_infra_if_mock_,
                setPanTiltLockControlStatus(Eq(PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING)))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, suppressLockStatusEvent(Eq(false), _)).Times(1).WillOnce(Return());
    EXPECT_CALL(initialize_infra_if_mock_, setPowerOnSequenceStatus(Eq(false), _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(ptzf_status_infra_if_mock_, getPanTiltLockControlStatus(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(PAN_TILT_LOCK_STATUS_UNLOCKED_AFTER_BOOTING), Return(true)));
    EXPECT_CALL(pan_tilt_lock_infra_if_mock_, getPanTiltLock(_))
        .Times(2)
        .WillRepeatedly(DoAll(SetArgReferee<0>(false), Return(ERRORCODE_SUCCESS)));
    EXPECT_CALL(power_status_if_mock_, getPowerStatus()).Times(1).WillOnce(Return(power::PowerStatus::POWER_ON));

    const visca::CompReply visca_reply_initialize(ERRORCODE_SUCCESS, seq_id_list[2]);
    handler_->handleRequest(visca_reply_initialize);
}

#pragma GCC diagnostic warning "-Wconversion"

} // namespace ptzf