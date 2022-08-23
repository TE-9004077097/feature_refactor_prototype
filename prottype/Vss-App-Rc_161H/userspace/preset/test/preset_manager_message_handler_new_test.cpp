/*
 * preset_manager_message_handler_new_test.cpp
 *
 * Copyright 2018,2019 Sony Imaging Products & Solutions Inc.
 */

#include "types.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gtl_memory.h"
#include "common_gmock_util.h"

#include "common_message_queue.h"
#include "common_thread_object.h"
#include "preset_manager_message_handler.h"
#include "preset_status.h"
#include "preset_manager_mock.h"
#include "visca/visca_server_message_if.h"
#include "visca/visca_server_message_if_mock.h"
#include "visca/visca_server_ptzf_if.h"
#include "visca/visca_server_ptzf_if_mock.h"
#include "event_router/event_router_if_mock.h"
#include "ptzf/ptzf_status_if_mock.h"
#include "ptzf/ptzf_config_if_mock.h"
#include "camera_osd/camera_osd_status_if_mock.h"
#include "op_device/op_device_status_if_mock.h"
#include "visca/visca_status_if_mock.h"
#include "video/video_status_if_mock.h"
#include "power/power_status_if_mock.h"
#include "ptzf/ptz_trace_if_mock.h"
#include "ptzf/ptz_trace_status_if_mock.h"
#include "ui/menu_status_mock.h"
#include "preset_capability_infra_if_mock.h"
#include "preset/preset_status_if_mock.h"
#include "preset/preset_common_message.h"
#include "errorcode.h"
#include "gtl_string.h"
#include "preset_manager_ptz_trace_controller_mock.h"
#include "preset_status_mock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Eq;
using ::testing::SetArgReferee;
using ::testing::Field;
using ::testing::Property;
using ::testing::StrCaseEq;

// ○テストリスト
// - resetメッセージを受けてPresetManagerのresetメソッドを呼び出すこと
//     - preset idは境界値(0, 99, 100)をテストする
//     - sircs/visca/bizそれぞれのパスをテストする
//     - プリセットが存在しない場合の処理中断をテストする
// - recallメッセージを受けてPresetManagerのrecallメソッドを呼び出すこと
//     - preset idは境界値(0, 99, 100)をテストする
//     - sircs/visca/bizそれぞれのパスをテストする
//     - プリセットが存在しない場合の処理中断をテストする
// - SetPresetMoveSpeedTypeRequestメッセージを受けて
//   PresetManagerのSetPresetMoveSpeedTypeRequestメソッドを呼び出すこと
//     - visca/bizそれぞれのパスをテストする
//     - 操作中の処理中断をテストする
// - SetPresetCommonMoveSpeedRequestメッセージを受けて
//   PresetManagerのSetPresetCommonMoveSpeedRequestメソッドを呼び出すこと
//     - visca/bizそれぞれのパスをテストする
//     - 操作中の処理中断をテストする
// - SetRecallActionModeRequestメッセージを受けて
//   PresetManagerのSetRecallActionModeRequestメソッドを呼び出すこと
//     - visca/bizそれぞれのパスをテストする
//     - 操作中の処理中断をテストする
// - RecallOnBootRequestメッセージを受けて
//   PresetManagerのrecallOnBootメソッドを呼び出すこと
//     - 正常/異常それぞれのパスをテストする
// - FactoryDefaultRequestメッセージを受けて
//   PresetManagerのdoFactoryDefaultメソッドを呼び出すこと
//     - 正常/異常それぞれのパスをテストする

namespace config {

struct ConfigReadyNotification
{};

} // namespace config

namespace ui {
struct MenuControlMenuOff
{};
struct MenuControlResponse
{
    bool result;
};

class UiModelDummyThread
{
public:
    UiModelDummyThread() : select_(common::Select::tlsInstance()), mq_("MQCharacterUiModel")
    {
        mq_.setHandler(this, &UiModelDummyThread::handleMenuOffRequest);
        select_.addReadHandler(mq_.getFD(), &mq_, &common::MessageQueue::pend);
    }

    void handleMenuOffRequest(const ui::MenuControlMenuOff&, const common::MessageQueueName& mq_name)
    {
        common::MessageQueue mq_reply(mq_name.name);
        ui::MenuControlResponse response;
        response.result = true;
        mq_reply.post(response);
    }

    ~UiModelDummyThread()
    {
        mq_.unlink();
    }

    static const char_t* getName()
    {
        return "cui_model";
    }

private:
    common::Select& select_;
    common::MessageQueue mq_;
};

} // namespace ui
namespace preset {

class PresetManagerMessageHandlerNewTest : public ::testing::Test
{
protected:
    PresetManagerMessageHandlerNewTest()
        : mock_holder_object_(),
          er_mock_(mock_holder_object_.getMock()),
          visca_mock_holder_object_(),
          visca_if_mock_(visca_mock_holder_object_.getMock()),
          visca_status_mock_holder_object_(),
          visca_status_if_mock_(visca_status_mock_holder_object_.getMock()),
          ptzf_mock_holder_object_(),
          ptzf_status_if_mock_(ptzf_mock_holder_object_.getMock()),
          camera_osd_status_mock_holder_object_(),
          camera_osd_status_if_mock_(camera_osd_status_mock_holder_object_.getMock()),
          op_device_status_mock_holder_object_(),
          op_device_status_if_mock_(op_device_status_mock_holder_object_.getMock()),
          video_mock_holder_object_(),
          video_status_if_mock_(video_mock_holder_object_.getMock()),
          app_mock_holder_object_(),
          app_status_if_mock_(app_mock_holder_object_.getMock()),
          pm_mock_(),
          ptz_trace_if_mock_holder_object_(),
          ptz_trace_if_mock_(ptz_trace_if_mock_holder_object_.getMock()),
          ptz_trace_status_if_mock_holder_object_(),
          ptz_trace_status_if_mock_(ptz_trace_status_if_mock_holder_object_.getMock()),
          args_(pm_mock_),
          handler_(),
          reply_(),
          menu_status_mock_holder_object_(),
          menu_status_mock_(menu_status_mock_holder_object_.getMock()),
          capability_infra_if_mock_holder_object_(),
          capability_infra_if_mock_(capability_infra_if_mock_holder_object_.getMock()),
          status_if_mock_holder_object_(),
          status_if_mock_(status_if_mock_holder_object_.getMock()),
          preset_manager_ptz_trace_controller_mock_holder_object_(),
          preset_manager_ptz_trace_controller_mock_(preset_manager_ptz_trace_controller_mock_holder_object_.getMock()),
          status_mock_holder_object_(),
          status_mock_(status_mock_holder_object_.getMock())
    {}

    virtual void SetUp()
    {
        common::MessageQueue mq("ConfigReadyMQ");
        config::ConfigReadyNotification message;
        mq.post(message);

        EXPECT_CALL(er_mock_, create(_)).Times(1).WillOnce(Return());
        EXPECT_CALL(er_mock_, setDestination(_, _)).Times(1).WillOnce(Return());
        handler_.reset(new PresetManagerMessageHandler(&args_));
        common::ThreadObject<ui::UiModelDummyThread> dummy_ui_thread;
    }

    virtual void TearDown()
    {
        common::MessageQueue pm_mq(PresetManagerMQ::getName());
        pm_mq.unlink();
        common::MessageQueue er_mq("EventRouterMQ");
        er_mq.unlink();
        common::MessageQueue config_mq("ConfigReadyMQ");
        config_mq.unlink();
        reply_.unlink();
        handler_.reset();
        common::ThreadObject<ui::UiModelDummyThread> dummy_ui_thread;
    }

    void setDefaultValidCondition(const s32_t cardinality)
    {
        EXPECT_CALL(preset_manager_ptz_trace_controller_mock_, isPresetModeTrace()).WillRepeatedly(Return(false));
        EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).WillRepeatedly(Return(false));
        EXPECT_CALL(app_status_if_mock_, getPowerStatus())
            .Times(cardinality)
            .WillRepeatedly(Return(power::PowerStatus::POWER_ON));
        EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingIfclearCommand()).Times(cardinality).WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(cardinality).WillRepeatedly(Return(false));
        EXPECT_CALL(video_status_if_mock_, isConfiguringHdmiVideoFormat())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(ptzf_status_if_mock_, isConfiguringPanTiltLimit()).Times(cardinality).WillRepeatedly(Return(false));
        EXPECT_CALL(ptz_trace_status_if_mock_, getTraceCondition())
            .WillRepeatedly(Return(ptzf::PTZ_TRACE_CONDITION_IDLE));
    }

    void setDefaultValidRecallCondition(const s32_t cardinality, const bool is_biz = false)
    {
        if (!is_biz) {
            EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(cardinality).WillRepeatedly(Return(true));
        }
        setDefaultValidCondition(cardinality);
        EXPECT_CALL(visca_status_if_mock_, isHandlingExeCommandWithoutRecall())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(menu_status_mock_, isDisplayOn()).Times(cardinality).WillRepeatedly(Return(false));
        EXPECT_CALL(camera_osd_status_if_mock_, isDisplayOn()).Times(cardinality).WillRepeatedly(Return(false));
        EXPECT_CALL(status_mock_, getPresetRecallSlot()).Times(cardinality).WillRepeatedly(Return(U32_T(1)));
    }

    void setDefaultValidSetAndRecallCondition(const s32_t cardinality)
    {
        EXPECT_CALL(op_device_status_if_mock_, isThumbnailScreenDisplayOn())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(op_device_status_if_mock_, isPlaybackScreenDisplayOn())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(ptzf_status_if_mock_, isClearImageZoomOn()).Times(cardinality).WillRepeatedly(Return(false));
        EXPECT_CALL(ptzf_status_if_mock_, getPanTiltLock(_))
            .Times(cardinality)
            .WillRepeatedly(DoAll(SetArgReferee<0>(false), Return()));
        EXPECT_CALL(ptzf_status_if_mock_, getPanTiltLockControlStatus(_))
            .Times(cardinality)
            .WillRepeatedly(DoAll(SetArgReferee<0>(ptzf::PAN_TILT_LOCK_STATUS_UNLOCKED), Return()));
    }

    void setDefaultValidOtherCondition(const s32_t cardinality)
    {
        EXPECT_CALL(preset_manager_ptz_trace_controller_mock_, isPresetModeTrace()).WillRepeatedly(Return(false));
        EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).WillRepeatedly(Return(false));
        EXPECT_CALL(app_status_if_mock_, getPowerStatus())
            .Times(cardinality)
            .WillRepeatedly(Return(power::PowerStatus::POWER_ON));
        EXPECT_CALL(visca_status_if_mock_, isHandlingIfclearCommand()).Times(cardinality).WillRepeatedly(Return(false));
        EXPECT_CALL(video_status_if_mock_, isConfiguringHdmiVideoFormat())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(ptz_trace_status_if_mock_, getTraceCondition())
            .WillRepeatedly(Return(ptzf::PTZ_TRACE_CONDITION_IDLE));
    }

    void setDefaultValidZoomFocusCondition(const s32_t cardinality)
    {
        EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
    }

    void setDefaultValidPanTiltCondition(const s32_t cardinality)
    {
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand())
            .Times(cardinality)
            .WillRepeatedly(Return(false));
        EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(cardinality).WillRepeatedly(Return(false));
        EXPECT_CALL(ptzf_status_if_mock_, isConfiguringPanTiltLimit()).Times(cardinality).WillRepeatedly(Return(false));
    }

protected:
    MockHolderObject<event_router::EventRouterIfMock> mock_holder_object_;
    event_router::EventRouterIfMock& er_mock_;
    MockHolderObject<visca::ViscaServerMessageIfMock> visca_mock_holder_object_;
    visca::ViscaServerMessageIfMock& visca_if_mock_;
    MockHolderObject<visca::ViscaStatusIfMock> visca_status_mock_holder_object_;
    visca::ViscaStatusIfMock& visca_status_if_mock_;
    MockHolderObject<ptzf::PtzfStatusIfMock> ptzf_mock_holder_object_;
    ptzf::PtzfStatusIfMock& ptzf_status_if_mock_;
    MockHolderObject<camera_osd::CameraOsdStatusIfMock> camera_osd_status_mock_holder_object_;
    camera_osd::CameraOsdStatusIfMock& camera_osd_status_if_mock_;
    MockHolderObject<op_device::OpDeviceStatusIfMock> op_device_status_mock_holder_object_;
    op_device::OpDeviceStatusIfMock& op_device_status_if_mock_;
    MockHolderObject<video::VideoStatusIfMock> video_mock_holder_object_;
    video::VideoStatusIfMock& video_status_if_mock_;
    MockHolderObject<power::PowerStatusIfMock> app_mock_holder_object_;
    power::PowerStatusIfMock& app_status_if_mock_;
    PresetManagerMock pm_mock_;
    MockHolderObject<ptzf::PtzTraceIfMock> ptz_trace_if_mock_holder_object_;
    ptzf::PtzTraceIfMock& ptz_trace_if_mock_;
    MockHolderObject<ptzf::PtzTraceStatusIfMock> ptz_trace_status_if_mock_holder_object_;
    ptzf::PtzTraceStatusIfMock& ptz_trace_status_if_mock_;
    PresetManagerMessageHandlerArgs args_;
    gtl::AutoPtr<PresetManagerMessageHandler> handler_;
    common::MessageQueue reply_;
    MockHolderObject<ui::MenuStatusMock> menu_status_mock_holder_object_;
    ui::MenuStatusMock& menu_status_mock_;
    MockHolderObject<infra::CapabilityInfraIfMock> capability_infra_if_mock_holder_object_;
    infra::CapabilityInfraIfMock& capability_infra_if_mock_;
    MockHolderObject<PresetStatusIfMock> status_if_mock_holder_object_;
    PresetStatusIfMock& status_if_mock_;
    MockHolderObject<PresetManagerPtzTraceControllerMock> preset_manager_ptz_trace_controller_mock_holder_object_;
    PresetManagerPtzTraceControllerMock& preset_manager_ptz_trace_controller_mock_;
    MockHolderObject<PresetStatusMock> status_mock_holder_object_;
    PresetStatusMock& status_mock_;
};

TEST_F(PresetManagerMessageHandlerNewTest, ResetRequestFromSircs)
{
    ResetRequest req(DEFAULT_PRESET_ID);
    setDefaultValidCondition(U32_T(2));

    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(DEFAULT_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_,
                reset(Eq(DEFAULT_PRESET_ID),
                      visca::INVALID_PACKET_ID,
                      INVALID_SEQ_ID,
                      Field(&common::MessageQueueName::name, StrCaseEq(common::MessageQueueName().name))))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequestWithReply<ResetRequest>(req, reply_.getName());

    req.preset_id = MAX_PRESET_ID;
    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(MAX_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_,
                reset(Eq(MAX_PRESET_ID),
                      visca::INVALID_PACKET_ID,
                      INVALID_SEQ_ID,
                      Field(&common::MessageQueueName::name, StrCaseEq(common::MessageQueueName().name))))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequestWithReply<ResetRequest>(req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetInvalidRequestFromSircs)
{
    ResetRequest req(MAX_PRESET_ID + U32_T(1));
    setDefaultValidOtherCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);
    EXPECT_CALL(pm_mock_, reset(_, _, _, _)).Times(0);
    handler_->handleRequestWithReply<ResetRequest>(req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetRequestFromVisca)
{
    ResetRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<ResetRequest> visca_req(U32_T(1234567890), req);
    setDefaultValidCondition(U32_T(2));

    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(DEFAULT_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_,
                reset(Eq(DEFAULT_PRESET_ID),
                      Eq(U32_T(1234567890)),
                      INVALID_SEQ_ID,
                      Field(&common::MessageQueueName::name, StrCaseEq(common::MessageQueueName().name))))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<ResetRequest>>(visca_req, reply_.getName());

    req.preset_id = MAX_PRESET_ID;
    visca_req.payload = req;
    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(MAX_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_,
                reset(Eq(MAX_PRESET_ID),
                      Eq(U32_T(1234567890)),
                      INVALID_SEQ_ID,
                      Field(&common::MessageQueueName::name, StrCaseEq(common::MessageQueueName().name))))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<ResetRequest>>(visca_req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetInvalidRequestFromVisca)
{
    ResetRequest req(MAX_PRESET_ID + U32_T(1));
    visca::ViscaMessageSequence<ResetRequest> visca_req(U32_T(1234567890), req);
    setDefaultValidOtherCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);
    EXPECT_CALL(pm_mock_, reset(_, _, _, _)).Times(0);
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<ResetRequest>>(visca_req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetFromSircsInInvalidCondition)
{
    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_, isPresetModeTrace()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(app_status_if_mock_, getPowerStatus()).Times(1).WillRepeatedly(Return(power::PowerStatus::POWER_OFF));
    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);
    ResetRequest req(DEFAULT_PRESET_ID);
    handler_->handleRequestWithReply<ResetRequest>(req, reply_.getName());
    ResetResult result(ERRORCODE_SUCCESS);
    reply_.pend(result);
    EXPECT_EQ(ERRORCODE_EXEC, result.err);
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetFromSircsNoPresetExistence)
{
    setDefaultValidCondition(U32_T(1));
    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(DEFAULT_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(false), Return(true)));
    EXPECT_CALL(pm_mock_, reset(_, _, _, _)).Times(0);

    ResetRequest req(DEFAULT_PRESET_ID);
    handler_->handleRequestWithReply<ResetRequest>(req, reply_.getName());
    ResetResult result(ERRORCODE_SUCCESS);
    reply_.pend(result);
    EXPECT_EQ(ERRORCODE_EXEC, result.err);
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetFromViscaInInvalidCondition)
{
    ResetRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<ResetRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;

    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_, isPresetModeTrace()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(app_status_if_mock_, getPowerStatus()).Times(1).WillRepeatedly(Return(power::PowerStatus::POWER_OFF));
    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<ResetRequest>>(visca_req, reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetFromViscaNoPresetExistence)
{
    setDefaultValidCondition(U32_T(1));
    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(DEFAULT_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(false), Return(true)));
    EXPECT_CALL(pm_mock_, reset(_, _, _, _)).Times(0);

    ResetRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<ResetRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<ResetRequest>>(visca_req, reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetRequestFromPtzTraceDelete)
{
    ResetRequest req(DEFAULT_PRESET_ID);

    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_, isPresetModeTrace()).WillRepeatedly(Return(true));
    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_,
                remove(Eq(DEFAULT_PRESET_ID),
                       Eq(visca::INVALID_PACKET_ID),
                       Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name)),
                       Eq(INVALID_SEQ_ID)))
        .Times(1);
    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).Times(0);
    handler_->handleRequestWithReply<ResetRequest>(req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetRequestFromBiz)
{
    u32_t seq_id = U32_T(2);
    ResetRequest req(DEFAULT_PRESET_ID);
    BizMessage<ResetRequest> biz_req(seq_id, reply_.getName(), req);
    setDefaultValidCondition(U32_T(2));

    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(DEFAULT_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_,
                reset(Eq(DEFAULT_PRESET_ID),
                      visca::INVALID_PACKET_ID,
                      seq_id,
                      Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest<BizMessage<ResetRequest>>(biz_req);

    req.preset_id = MAX_PRESET_ID;
    biz_req.payload = req;
    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(MAX_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_,
                reset(Eq(MAX_PRESET_ID),
                      visca::INVALID_PACKET_ID,
                      seq_id,
                      Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest<BizMessage<ResetRequest>>(biz_req);
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetRequestFromPtzTraceRemoveBiz)
{
    u32_t seq_id = U32_T(2);
    ResetRequest req(DEFAULT_PRESET_ID);
    BizMessage<ResetRequest> biz_req(seq_id, reply_.getName(), req);

    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_, isPresetModeTrace()).WillRepeatedly(Return(true));
    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_,
                remove(Eq(DEFAULT_PRESET_ID),
                       Eq(visca::INVALID_PACKET_ID),
                       Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name)),
                       Eq(seq_id)))
        .Times(1);
    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).Times(0);
    handler_->handleRequest<BizMessage<ResetRequest>>(biz_req);
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetInvalidRequestFromBiz)
{
    u32_t seq_id = U32_T(2);
    ResetRequest req(MAX_PRESET_ID + U32_T(1));
    BizMessage<ResetRequest> biz_req(seq_id, reply_.getName(), req);
    setDefaultValidOtherCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);
    EXPECT_CALL(pm_mock_,
                reset(Eq(MAX_PRESET_ID + U32_T(1)),
                      visca::INVALID_PACKET_ID,
                      seq_id,
                      Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(0);
    handler_->handleRequest<BizMessage<ResetRequest>>(biz_req);
    preset::message::PresetExecComp result;
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_OUT_OF_RANGE);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetInInvalidConditionFromBiz)
{
    ResetRequest req(DEFAULT_PRESET_ID);
    u32_t seq_id = U32_T(2);
    BizMessage<ResetRequest> biz_req(seq_id, reply_.getName(), req);
    preset::message::PresetExecComp result;
    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_, isPresetModeTrace()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(app_status_if_mock_, getPowerStatus()).Times(1).WillRepeatedly(Return(power::PowerStatus::POWER_OFF));
    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);
    handler_->handleRequest<BizMessage<ResetRequest>>(biz_req);
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_EXEC);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetFromBizNoPresetExistence)
{
    setDefaultValidCondition(U32_T(1));
    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(DEFAULT_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(false), Return(true)));
    EXPECT_CALL(pm_mock_, reset(_, _, _, _)).Times(0);

    ResetRequest req(DEFAULT_PRESET_ID);
    u32_t seq_id = U32_T(2);
    BizMessage<ResetRequest> biz_req(seq_id, reply_.getName(), req);
    preset::message::PresetExecComp result;
    handler_->handleRequest<BizMessage<ResetRequest>>(biz_req);
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_EXEC);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, ResetFuncErrorFromBiz)
{
    u32_t seq_id = U32_T(2);
    ResetRequest req(DEFAULT_PRESET_ID);
    BizMessage<ResetRequest> biz_req(seq_id, reply_.getName(), req);
    setDefaultValidCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(DEFAULT_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_,
                reset(Eq(DEFAULT_PRESET_ID),
                      visca::INVALID_PACKET_ID,
                      seq_id,
                      Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(ERRORCODE_BUG));
    handler_->handleRequest<BizMessage<ResetRequest>>(biz_req);
    preset::message::PresetExecComp result;
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_BUG);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallRequestFromSircs)
{
    RecallRequest req(DEFAULT_PRESET_ID);
    setDefaultValidRecallCondition(U32_T(2));
    setDefaultValidSetAndRecallCondition(U32_T(2));

    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(DEFAULT_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_, recall(Eq(DEFAULT_PRESET_ID))).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequestWithReply<RecallRequest>(req, reply_.getName());

    req.preset_id = MAX_PRESET_ID;
    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(MAX_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_, recall(Eq(MAX_PRESET_ID))).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequestWithReply<RecallRequest>(req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallInvalidRequestFromSircs)
{
    RecallRequest req(MAX_PRESET_ID + U32_T(1));
    setDefaultValidOtherCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);
    EXPECT_CALL(pm_mock_, recall(_)).Times(0);
    handler_->handleRequestWithReply<RecallRequest>(req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallRequestFromVisca)
{
    RecallRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<RecallRequest> visca_req(U32_T(1234567890), req);
    setDefaultValidRecallCondition(U32_T(2));
    setDefaultValidSetAndRecallCondition(U32_T(2));

    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(DEFAULT_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_,
                recall(Eq(DEFAULT_PRESET_ID),
                       Eq(INVALID_PRESET_MOVE_SPEED),
                       Eq(1234567890),
                       Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name)),
                       INVALID_SEQ_ID))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
    req.preset_id = MAX_PRESET_ID;
    visca_req.payload = req;

    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(MAX_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_,
                recall(Eq(MAX_PRESET_ID),
                       Eq(INVALID_PRESET_MOVE_SPEED),
                       Eq(1234567890),
                       Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name)),
                       INVALID_SEQ_ID))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallInvalidRequestFromVisca)
{
    RecallRequest req(MAX_PRESET_ID + U32_T(1));
    visca::ViscaMessageSequence<RecallRequest> visca_req(U32_T(1234567890), req);
    setDefaultValidOtherCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);
    EXPECT_CALL(pm_mock_, recall(_, _, _, _, _)).Times(0);
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallFromSircsInInvalidCondition)
{
    RecallRequest req(DEFAULT_PRESET_ID);
    RecallResult result(ERRORCODE_SUCCESS);

    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_, isPresetModeTrace()).WillRepeatedly(Return(false));
    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).WillRepeatedly(Return(false));
    EXPECT_CALL(app_status_if_mock_, getPowerStatus()).Times(2).WillRepeatedly(Return(power::PowerStatus::POWER_ON));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingIfclearCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(3).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(video_status_if_mock_, isConfiguringHdmiVideoFormat()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(ptzf_status_if_mock_, isConfiguringPanTiltLimit()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(ptz_trace_status_if_mock_, getTraceCondition()).WillRepeatedly(Return(ptzf::PTZ_TRACE_CONDITION_IDLE));

    handler_->handleRequestWithReply<RecallRequest>(req, reply_.getName());
    reply_.pend(result);
    EXPECT_EQ(ERRORCODE_EXEC, result.err);

    EXPECT_CALL(visca_status_if_mock_, isHandlingExeCommandWithoutRecall()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);
    handler_->handleRequestWithReply<RecallRequest>(req, reply_.getName());
    reply_.pend(result);
    EXPECT_EQ(ERRORCODE_EXEC, result.err);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallFromSircsNoPresetExistence)
{
    setDefaultValidRecallCondition(U32_T(1));
    setDefaultValidSetAndRecallCondition(U32_T(1));

    RecallRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<RecallRequest> visca_req(U32_T(1234567890), req);
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallFromSircsInInvalidZoomFocusCondition)
{
    RecallRequest req(DEFAULT_PRESET_ID);
    RecallResult result(ERRORCODE_SUCCESS);

    setDefaultValidOtherCondition(U32_T(2));
    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1).WillOnce(Return(true));

    handler_->handleRequestWithReply<RecallRequest>(req, reply_.getName());
    reply_.pend(result);
    EXPECT_EQ(ERRORCODE_EXEC, result.err);

    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);

    handler_->handleRequestWithReply<RecallRequest>(req, reply_.getName());
    reply_.pend(result);
    EXPECT_EQ(ERRORCODE_EXEC, result.err);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallIfclearlnInvalidCondition)
{
    RecallRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<RecallRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;

    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_, isPresetModeTrace()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(app_status_if_mock_, getPowerStatus()).Times(1).WillRepeatedly(Return(power::PowerStatus::POWER_ON));

    EXPECT_CALL(visca_status_if_mock_, isHandlingIfclearCommand()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(ptz_trace_status_if_mock_, getTraceCondition()).WillRepeatedly(Return(ptzf::PTZ_TRACE_CONDITION_IDLE));

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallPTDirectionlnInvalidCondition)
{
    RecallRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<RecallRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;

    setDefaultValidOtherCondition(U32_T(1));
    setDefaultValidZoomFocusCondition(U32_T(1));
    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1).WillRepeatedly(Return(true));

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallPTAbsPosCommandInvalidCondition)
{
    RecallRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<RecallRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;

    setDefaultValidOtherCondition(U32_T(1));
    setDefaultValidZoomFocusCondition(U32_T(1));
    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(1).WillRepeatedly(Return(true));

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallPTRelPosCommandInvalidCondition)
{
    RecallRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<RecallRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;

    setDefaultValidOtherCondition(U32_T(1));
    setDefaultValidZoomFocusCondition(U32_T(1));
    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1).WillRepeatedly(Return(true));

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallPTHomePosCommandInvalidCondition)
{
    RecallRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<RecallRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;

    setDefaultValidOtherCondition(U32_T(1));
    setDefaultValidZoomFocusCondition(U32_T(1));
    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1).WillRepeatedly(Return(true));

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallPTResetCommandInvalidCondition)
{
    RecallRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<RecallRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;

    setDefaultValidOtherCondition(U32_T(1));
    setDefaultValidZoomFocusCondition(U32_T(1));
    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(1).WillRepeatedly(Return(true));

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallFromViscaInInvalidCondition)
{
    RecallRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<RecallRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;

    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_, isPresetModeTrace()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(app_status_if_mock_, getPowerStatus()).Times(2).WillRepeatedly(Return(power::PowerStatus::POWER_ON));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingIfclearCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTDirectionCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTAbsPosCommand()).Times(3).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTRelPosCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTHomePosCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingPTResetCommand()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(video_status_if_mock_, isConfiguringHdmiVideoFormat()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(ptzf_status_if_mock_, isConfiguringPanTiltLimit()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(ptz_trace_status_if_mock_, getTraceCondition()).WillRepeatedly(Return(ptzf::PTZ_TRACE_CONDITION_IDLE));
    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(1).WillOnce(Return(false));
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);

    EXPECT_CALL(visca_status_if_mock_, isHandlingExeCommandWithoutRecall()).Times(1).WillOnce(Return(true));
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallFromViscaNoPresetExistence)
{
    setDefaultValidRecallCondition(U32_T(1));
    setDefaultValidSetAndRecallCondition(U32_T(1));
    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(DEFAULT_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(false), Return(true)));
    EXPECT_CALL(pm_mock_, recall(_, _, _, _, _)).Times(0);

    RecallRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<RecallRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallFromViscaInInvalidZoomFocusCondition)
{
    RecallRequest req(DEFAULT_PRESET_ID);
    visca::ViscaMessageSequence<RecallRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;

    setDefaultValidOtherCondition(U32_T(2));
    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1).WillOnce(Return(true));

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);

    EXPECT_CALL(visca_status_if_mock_, isHandlingZoomDirectCommand()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(visca_status_if_mock_, isHandlingFocusDirectCommand()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);

    handler_->handleRequestWithReply<visca::ViscaMessageSequence<RecallRequest>>(visca_req, reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallRequestFromPtzTraceDelete)
{
    RecallRequest req(DEFAULT_PRESET_ID);

    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_, isPresetModeTrace()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_,
                playback(Eq(DEFAULT_PRESET_ID),
                         Eq(visca::INVALID_PACKET_ID),
                         Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name)),
                         Eq(INVALID_SEQ_ID)))
        .Times(1);
    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);
    handler_->handleRequestWithReply<RecallRequest>(req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallRequestFromBiz)
{
    u32_t seq_id = U32_T(2);
    u32_t speed = U32_T(11);
    RecallRequest req(DEFAULT_PRESET_ID, speed);
    BizMessage<RecallRequest> biz_req(seq_id, reply_.getName(), req);
    setDefaultValidRecallCondition(U32_T(2), true);
    setDefaultValidSetAndRecallCondition(U32_T(2));

    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(DEFAULT_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_,
                recall(Eq(DEFAULT_PRESET_ID),
                       Eq(speed),
                       visca::INVALID_PACKET_ID,
                       Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name)),
                       seq_id))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest<BizMessage<RecallRequest>>(biz_req);

    req.preset_id = MAX_PRESET_ID;
    biz_req.payload = req;
    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(MAX_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_,
                recall(Eq(MAX_PRESET_ID),
                       Eq(speed),
                       visca::INVALID_PACKET_ID,
                       Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name)),
                       seq_id))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest<BizMessage<RecallRequest>>(biz_req);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallRequestFromPtzTraceDeleteBiz)
{
    u32_t seq_id = U32_T(2);
    RecallRequest req(DEFAULT_PRESET_ID);
    BizMessage<RecallRequest> biz_req(seq_id, reply_.getName(), req);

    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_, isPresetModeTrace()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(preset_manager_ptz_trace_controller_mock_,
                playback(Eq(DEFAULT_PRESET_ID),
                         Eq(visca::INVALID_PACKET_ID),
                         Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name)),
                         Eq(seq_id)))
        .Times(1);
    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);
    handler_->handleRequest<BizMessage<RecallRequest>>(biz_req);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallInvalidRequestFromBiz)
{
    u32_t seq_id = U32_T(2);
    RecallRequest req(MAX_PRESET_ID + U32_T(1));
    BizMessage<RecallRequest> biz_req(seq_id, reply_.getName(), req);
    setDefaultValidOtherCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);
    EXPECT_CALL(pm_mock_, recall(_, _, _, _, _)).Times(0);
    handler_->handleRequest<BizMessage<RecallRequest>>(biz_req);

    preset::message::PresetExecComp result;
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_OUT_OF_RANGE);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallInvalidRecallConditionFromBiz)
{
    u32_t seq_id = U32_T(2);
    RecallRequest req(DEFAULT_PRESET_ID);
    BizMessage<RecallRequest> biz_req(seq_id, reply_.getName(), req);
    setDefaultValidCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(visca_status_if_mock_, isHandlingExeCommandWithoutRecall()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(menu_status_mock_, isDisplayOn()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(camera_osd_status_if_mock_, isDisplayOn()).Times(1).WillRepeatedly(Return(false));
    EXPECT_CALL(status_mock_, getPresetRecallSlot()).Times(1).WillRepeatedly(Return(U32_T(2)));
    EXPECT_CALL(status_if_mock_, getPresetExistence(_, _)).Times(0);
    EXPECT_CALL(pm_mock_, recall(_, _, _, _, _)).Times(0);

    handler_->handleRequest<BizMessage<RecallRequest>>(biz_req);

    preset::message::PresetExecComp result;
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_EXEC);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallFromBizNoPresetExistence)
{
    setDefaultValidRecallCondition(U32_T(1), true);
    setDefaultValidSetAndRecallCondition(U32_T(1));
    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(DEFAULT_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(false), Return(true)));
    EXPECT_CALL(pm_mock_, recall(_, _, _, _, _)).Times(0);

    u32_t seq_id = U32_T(2);
    RecallRequest req(DEFAULT_PRESET_ID);
    BizMessage<RecallRequest> biz_req(seq_id, reply_.getName(), req);
    handler_->handleRequest<BizMessage<RecallRequest>>(biz_req);

    preset::message::PresetExecComp result;
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_EXEC);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallFuncErrorFromBiz)
{
    u32_t seq_id = U32_T(2);
    u32_t speed = U32_T(11);
    RecallRequest req(DEFAULT_PRESET_ID, speed);
    BizMessage<RecallRequest> biz_req(seq_id, reply_.getName(), req);
    setDefaultValidRecallCondition(U32_T(2), true);
    setDefaultValidSetAndRecallCondition(U32_T(2));

    EXPECT_CALL(status_if_mock_, isDoingPresetRecall()).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(DEFAULT_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_,
                recall(Eq(DEFAULT_PRESET_ID),
                       Eq(speed),
                       visca::INVALID_PACKET_ID,
                       Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name)),
                       seq_id))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest<BizMessage<RecallRequest>>(biz_req);

    req.preset_id = MAX_PRESET_ID;
    biz_req.payload = req;
    EXPECT_CALL(status_if_mock_, getPresetExistence(Eq(MAX_PRESET_ID), _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(true)));
    EXPECT_CALL(pm_mock_,
                recall(Eq(MAX_PRESET_ID),
                       Eq(speed),
                       visca::INVALID_PACKET_ID,
                       Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name)),
                       seq_id))
        .Times(1)
        .WillOnce(Return(ERRORCODE_BUG));
    handler_->handleRequest<BizMessage<RecallRequest>>(biz_req);

    preset::message::PresetExecComp result;
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_BUG);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetPresetMoveSpeedTypeRequestFromVisca)
{
    SetPresetMoveSpeedTypeRequest req(visca::PRESET_MOVE_SPEED_TYPE_COMPATIBLE);
    visca::ViscaMessageSequence<SetPresetMoveSpeedTypeRequest> visca_req(U32_T(1234567890), req);
    setDefaultValidOtherCondition(U32_T(1));
    setDefaultValidPanTiltCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isChangingPresetMoveSpeedType()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(
        pm_mock_,
        setPresetMoveSpeedType(Eq(visca::PRESET_MOVE_SPEED_TYPE_COMPATIBLE),
                               Eq(1234567890),
                               INVALID_SEQ_ID,
                               Field(&common::MessageQueueName::name, StrCaseEq(common::MessageQueueName().name))))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetPresetMoveSpeedTypeRequest>>(visca_req,
                                                                                                 reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, SetPresetMoveSpeedTypeIsChangingFromVisca)
{
    SetPresetMoveSpeedTypeRequest req(visca::PRESET_MOVE_SPEED_TYPE_COMPATIBLE);
    visca::ViscaMessageSequence<SetPresetMoveSpeedTypeRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;
    setDefaultValidOtherCondition(U32_T(1));
    setDefaultValidPanTiltCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isChangingPresetMoveSpeedType()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pm_mock_, setPresetMoveSpeedType(_, _, _, _)).Times(0);
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetPresetMoveSpeedTypeRequest>>(visca_req,
                                                                                                 reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetPresetMoveSpeedTypeInvalidConditionFromVisca)
{
    SetPresetMoveSpeedTypeRequest req(visca::PRESET_MOVE_SPEED_TYPE_COMPATIBLE);
    visca::ViscaMessageSequence<SetPresetMoveSpeedTypeRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;

    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(status_if_mock_, isChangingPresetMoveSpeedType()).Times(0);
    EXPECT_CALL(pm_mock_, setPresetMoveSpeedType(_, _, _, _)).Times(0);
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetPresetMoveSpeedTypeRequest>>(visca_req,
                                                                                                 reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetPresetMoveSpeedTypeRequestFromBiz)
{
    SetPresetMoveSpeedTypeRequest req(visca::PRESET_MOVE_SPEED_TYPE_COMPATIBLE);
    u32_t seq_id = U32_T(2);
    BizMessage<SetPresetMoveSpeedTypeRequest> biz_req(seq_id, reply_.getName(), req);
    setDefaultValidOtherCondition(U32_T(1));
    setDefaultValidPanTiltCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isChangingPresetMoveSpeedType()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(pm_mock_,
                setPresetMoveSpeedType(Eq(visca::PRESET_MOVE_SPEED_TYPE_COMPATIBLE),
                                       Eq(visca::INVALID_PACKET_ID),
                                       Eq(seq_id),
                                       Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest<BizMessage<SetPresetMoveSpeedTypeRequest>>(biz_req);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetPresetMoveSpeedTypeIsChangingFromBiz)
{
    SetPresetMoveSpeedTypeRequest req(visca::PRESET_MOVE_SPEED_TYPE_COMPATIBLE);
    u32_t seq_id = U32_T(2);
    BizMessage<SetPresetMoveSpeedTypeRequest> biz_req(seq_id, reply_.getName(), req);
    preset::message::PresetExecComp result;
    setDefaultValidOtherCondition(U32_T(1));
    setDefaultValidPanTiltCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isChangingPresetMoveSpeedType()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pm_mock_, setPresetMoveSpeedType(_, _, _, _)).Times(0);
    handler_->handleRequest<BizMessage<SetPresetMoveSpeedTypeRequest>>(biz_req);
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_EXEC);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetPresetMoveSpeedTypeInvalidConditionFromBiz)
{
    SetPresetMoveSpeedTypeRequest req(visca::PRESET_MOVE_SPEED_TYPE_COMPATIBLE);
    u32_t seq_id = U32_T(2);
    BizMessage<SetPresetMoveSpeedTypeRequest> biz_req(seq_id, reply_.getName(), req);
    preset::message::PresetExecComp result;

    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(status_if_mock_, isChangingPresetMoveSpeedType()).Times(0);
    EXPECT_CALL(pm_mock_, setPresetMoveSpeedType(_, _, _, _)).Times(0);
    handler_->handleRequest<BizMessage<SetPresetMoveSpeedTypeRequest>>(biz_req);
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_EXEC);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetPresetCommonMoveSpeedRequestFromVisca)
{
    SetPresetCommonMoveSpeedRequest req(U32_T(1));
    visca::ViscaMessageSequence<SetPresetCommonMoveSpeedRequest> visca_req(U32_T(1234567890), req);
    setDefaultValidOtherCondition(U32_T(1));
    setDefaultValidPanTiltCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isChangingPresetCommonMoveSpeed()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(
        pm_mock_,
        setPresetCommonMoveSpeed(Eq(U32_T(1)),
                                 Eq(1234567890),
                                 INVALID_SEQ_ID,
                                 Field(&common::MessageQueueName::name, StrCaseEq(common::MessageQueueName().name))))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetPresetCommonMoveSpeedRequest>>(visca_req,
                                                                                                   reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, SetPresetCommonMoveSpeedIsChangingFromVisca)
{
    SetPresetCommonMoveSpeedRequest req(U32_T(1));
    visca::ViscaMessageSequence<SetPresetCommonMoveSpeedRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;
    setDefaultValidOtherCondition(U32_T(1));
    setDefaultValidPanTiltCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isChangingPresetCommonMoveSpeed()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pm_mock_, setPresetCommonMoveSpeed(_, _, _, _)).Times(0);
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetPresetCommonMoveSpeedRequest>>(visca_req,
                                                                                                   reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetPresetCommonMoveSpeedInvalidConditionFromVisca)
{
    SetPresetCommonMoveSpeedRequest req(U32_T(1));
    visca::ViscaMessageSequence<SetPresetCommonMoveSpeedRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;

    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(status_if_mock_, isChangingPresetCommonMoveSpeed()).Times(0);
    EXPECT_CALL(pm_mock_, setPresetCommonMoveSpeed(_, _, _, _)).Times(0);
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetPresetCommonMoveSpeedRequest>>(visca_req,
                                                                                                   reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetPresetCommonMoveSpeedRequestFromBiz)
{
    SetPresetCommonMoveSpeedRequest req(U32_T(1));
    u32_t seq_id = U32_T(2);
    BizMessage<SetPresetCommonMoveSpeedRequest> biz_req(seq_id, reply_.getName(), req);
    setDefaultValidOtherCondition(U32_T(1));
    setDefaultValidPanTiltCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isChangingPresetCommonMoveSpeed()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(pm_mock_,
                setPresetCommonMoveSpeed(Eq(U32_T(1)),
                                         Eq(visca::INVALID_PACKET_ID),
                                         Eq(seq_id),
                                         Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest<BizMessage<SetPresetCommonMoveSpeedRequest>>(biz_req);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetPresetCommonMoveSpeedIsChangingFromBiz)
{
    SetPresetCommonMoveSpeedRequest req(U32_T(1));
    u32_t seq_id = U32_T(2);
    BizMessage<SetPresetCommonMoveSpeedRequest> biz_req(seq_id, reply_.getName(), req);
    preset::message::PresetExecComp result;
    setDefaultValidOtherCondition(U32_T(1));
    setDefaultValidPanTiltCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isChangingPresetCommonMoveSpeed()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pm_mock_, setPresetCommonMoveSpeed(_, _, _, _)).Times(0);
    handler_->handleRequest<BizMessage<SetPresetCommonMoveSpeedRequest>>(biz_req);
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_EXEC);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetPresetCommonMoveSpeedInvalidConditionFromBiz)
{
    SetPresetCommonMoveSpeedRequest req(U32_T(1));
    u32_t seq_id = U32_T(2);
    BizMessage<SetPresetCommonMoveSpeedRequest> biz_req(seq_id, reply_.getName(), req);
    preset::message::PresetExecComp result;

    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(status_if_mock_, isChangingPresetCommonMoveSpeed()).Times(0);
    EXPECT_CALL(pm_mock_, setPresetCommonMoveSpeed(_, _, _, _)).Times(0);
    handler_->handleRequest<BizMessage<SetPresetCommonMoveSpeedRequest>>(biz_req);
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_EXEC);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetRecallActionModeRequestFromVisca)
{
    SetRecallActionModeRequest req(visca::RECALL_ACTION_MODE_NORMAL);
    visca::ViscaMessageSequence<SetRecallActionModeRequest> visca_req(U32_T(1234567890), req);
    setDefaultValidOtherCondition(U32_T(1));
    EXPECT_CALL(status_if_mock_, isChangingRecallActionMode()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(pm_mock_,
                setRecallActionMode(Eq(visca::RECALL_ACTION_MODE_NORMAL),
                                    Eq(1234567890),
                                    INVALID_SEQ_ID,
                                    Field(&common::MessageQueueName::name, StrCaseEq(common::MessageQueueName().name))))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetRecallActionModeRequest>>(visca_req,
                                                                                              reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, SetRecallActionModeIsChangingFromVisca)
{
    SetRecallActionModeRequest req(visca::RECALL_ACTION_MODE_NORMAL);
    visca::ViscaMessageSequence<SetRecallActionModeRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;

    setDefaultValidOtherCondition(U32_T(1));
    EXPECT_CALL(status_if_mock_, isChangingRecallActionMode()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pm_mock_, setRecallActionMode(_, _, _, _)).Times(0);
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetRecallActionModeRequest>>(visca_req,
                                                                                              reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetRecallActionModeInvalidConditionFromVisca)
{
    SetRecallActionModeRequest req(visca::RECALL_ACTION_MODE_NORMAL);
    visca::ViscaMessageSequence<SetRecallActionModeRequest> visca_req(U32_T(1234567890), req);
    visca::AckResponse ack;

    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(status_if_mock_, isChangingRecallActionMode()).Times(0);
    EXPECT_CALL(pm_mock_, setRecallActionMode(_, _, _, _)).Times(0);
    handler_->handleRequestWithReply<visca::ViscaMessageSequence<SetRecallActionModeRequest>>(visca_req,
                                                                                              reply_.getName());
    reply_.pend(ack);
    EXPECT_EQ(ERRORCODE_EXEC, ack.status);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetRecallActionModeRequestFromBiz)
{
    SetRecallActionModeRequest req(visca::RECALL_ACTION_MODE_NORMAL);
    u32_t seq_id = U32_T(2);
    BizMessage<SetRecallActionModeRequest> biz_req(seq_id, reply_.getName(), req);
    setDefaultValidOtherCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isChangingRecallActionMode()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(pm_mock_,
                setRecallActionMode(Eq(visca::RECALL_ACTION_MODE_NORMAL),
                                    Eq(visca::INVALID_PACKET_ID),
                                    Eq(seq_id),
                                    Field(&common::MessageQueueName::name, StrCaseEq(reply_.getName().name))))
        .Times(1)
        .WillOnce(Return(ERRORCODE_SUCCESS));
    handler_->handleRequest<BizMessage<SetRecallActionModeRequest>>(biz_req);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetRecallActionModeIsChangingFromBiz)
{
    SetRecallActionModeRequest req(visca::RECALL_ACTION_MODE_NORMAL);
    u32_t seq_id = U32_T(2);
    BizMessage<SetRecallActionModeRequest> biz_req(seq_id, reply_.getName(), req);
    preset::message::PresetExecComp result;
    setDefaultValidOtherCondition(U32_T(1));

    EXPECT_CALL(status_if_mock_, isChangingRecallActionMode()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(pm_mock_, setRecallActionMode(_, _, _, _)).Times(0);
    handler_->handleRequest<BizMessage<SetRecallActionModeRequest>>(biz_req);
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_EXEC);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, SetRecallActionModeInvalidConditionFromBiz)
{
    SetRecallActionModeRequest req(visca::RECALL_ACTION_MODE_NORMAL);
    u32_t seq_id = U32_T(2);
    BizMessage<SetRecallActionModeRequest> biz_req(seq_id, reply_.getName(), req);
    preset::message::PresetExecComp result;

    EXPECT_CALL(status_if_mock_, isDoingAnyPresetOperation()).Times(1).WillRepeatedly(Return(true));
    EXPECT_CALL(status_if_mock_, isChangingRecallActionMode()).Times(0);
    EXPECT_CALL(pm_mock_, setRecallActionMode(_, _, _, _)).Times(0);
    handler_->handleRequest<BizMessage<SetRecallActionModeRequest>>(biz_req);
    reply_.pend(result);
    EXPECT_EQ(result.error, ERRORCODE_EXEC);
    EXPECT_EQ(result.seq_id, seq_id);
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallOnBoot)
{
    RecallOnBootRequest req;

    EXPECT_CALL(pm_mock_, recallOnBoot(_)).WillOnce(Return(ERRORCODE_SUCCESS));

    handler_->handleRequestWithReply<RecallOnBootRequest>(req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, RecallOnBootError)
{
    RecallOnBootRequest req;

    EXPECT_CALL(pm_mock_, recallOnBoot(_)).WillOnce(Return(ERRORCODE_EXEC));

    handler_->handleRequestWithReply<RecallOnBootRequest>(req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, FactoryDefault)
{
    FactoryDefaultRequest req;

    EXPECT_CALL(pm_mock_, doFactoryDefault(_)).WillOnce(Return(ERRORCODE_SUCCESS));

    handler_->handleRequestWithReply<FactoryDefaultRequest>(req, reply_.getName());
}

TEST_F(PresetManagerMessageHandlerNewTest, FactoryDefaultError)
{
    FactoryDefaultRequest req;

    EXPECT_CALL(pm_mock_, doFactoryDefault(_)).WillOnce(Return(ERRORCODE_EXEC));

    handler_->handleRequestWithReply<FactoryDefaultRequest>(req, reply_.getName());
}

} // namespace preset
