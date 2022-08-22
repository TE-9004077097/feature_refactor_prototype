/*
 * power_sequencer_reboot_infra_if_marco_test.cpp
 *
 * Copyright 2019,2022 Sony Corporation.
 */

#include <sys/time.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "types.h"
#include "common_gmock_util.h"
#include "common_time_point.h"
#include "bizglobal.h"
#include "inbound/im_messages/reboot_request.h"
#include "inbound/boot_sequence/service_available_status.h"
#include "inbound/im_messages/uud_image_write_comp_status.h"
#include "inbound/general/need_pan_tilt_initialize.h"
#include "power_sequencer_finalize_message.h"
#include "power_sequencer_reboot_infra_if.h"
#include "power_control_infra_if_mock.h"
#include "ptzf/ptzf_status_if_mock.h"
#include "ptzf/ptzf_config_if_mock.h"

using ::testing::Return;

// ○テストリスト
// + requestReboot()
//   + bizglobal::RebootRequest を update していること(正常系)
// + requestRebootDelay100ms()
//   + bizglobal::RebootRequest を update していること(正常系)
//   + update が 100ms 以上経過して呼び出されていること(正常系)
// + rebootOnStandby()
//   + RebootComplete message を自分宛てに発行していること(正常系)
// + rebootOnPowerOn()
//   + DoStandbySequence message を自分宛てに発行していること(正常系)
// + handleServiceAvailableStatusWithReboot()
//   + DoStandbySequence 発行後に bizglobal::ServiceAvailableStatus (false) を受けたら RebootComplete を発行していること(正常系)
//   + DoStandbySequence 発行後に bizglobal::ServiceAvailableStatus (true) を受けたら RebootComplete を発行しないこと(正常系)
// + handleServiceAvailableStatusWithoutReboot()
//   + DoStandbySequence 発行せずに bizglobal::ServiceAvailableStatus (false) を受けたら RebootComplete を発行しないこと(正常系)
//   + DoStandbySequence 発行せずに bizglobal::ServiceAvailableStatus (true) を受けたら RebootComplete を発行しないこと(正常系)
//
// note
//   commom::Task::msleep() の妥当性確認のために時間計測を行う際に TimePoint::setCurrentTime を使用すると、
//   clock_gettime() の制度の問題で実際よりも短く計測されてしまうため、gettimeofday() を直接使用しています。
// + completeReboot()
//   + FW UUD が成功 した場合だけPowerControlInfraIf::setUudRebootMark() を呼び出すこと
//   + 上記以外はPowerControlInfraIf::setRebootMark() を呼び出すこと

struct TestEvent {};

namespace power {
namespace infra {

class PowerSequencerRebootInfraIfTest : public ::testing::Test
{
protected:
    PowerSequencerRebootInfraIfTest()
        : target_(),
          biz_global_(bizglobal::BizGlobal::instance()),
          mq_(),
          power_control_infra_if_mock_holder_(),
          power_control_infra_if_mock_(power_control_infra_if_mock_holder_.getMock()),
          ptzf_status_if_mock_holder_(),
          ptzf_status_if_mock_(ptzf_status_if_mock_holder_.getMock())
    {
    }

    virtual void SetUp()
    {
        target_ = PowerSequencerRebootInfraIf::makeInfra(mq_.getName().name);
        biz_global_.registerNotify(this,
                                   &PowerSequencerRebootInfraIfTest::handleRebootRequest,
                                   bizglobal::InboundWaitPolicy::NEXT);
    }

    virtual void TearDown()
    {
        target_.reset();
        biz_global_.unregisterNotify<bizglobal::RebootRequest>(this);

        common::MessageQueue mq_receiver(PowerSequencerFinalizeMessage::getMqName());
        mq_receiver.unlink();
    }

    virtual void handleRebootRequest(const bizglobal::RebootRequest&)
    {
        TestEvent msg;
        mq_.post(msg);
    }

    void getCurrentTimeMs(uint_t& time_ms)
    {
        struct timeval tv;
        gettimeofday(&tv, 0);
        time_ms = static_cast<uint_t>(tv.tv_sec * 1000 + tv.tv_usec / 1000);
    }

    std::unique_ptr<PowerSequencerRebootInfraIf> target_;
    bizglobal::BizGlobal& biz_global_;
    common::MessageQueue mq_;
    MockHolderObject<PowerControlInfraIfMock> power_control_infra_if_mock_holder_;
    infra::PowerControlInfraIfMock& power_control_infra_if_mock_;
    MockHolderObject<ptzf::PtzfStatusIfMock> ptzf_status_if_mock_holder_;
    ptzf::PtzfStatusIfMock& ptzf_status_if_mock_;
};

TEST_F(PowerSequencerRebootInfraIfTest, requestReboot)
{
    uint_t start_time, end_time;
    getCurrentTimeMs(start_time);

    target_->requestReboot(UINT_T(0));

    // wait notify
    TestEvent msg;
    mq_.pend(msg);

    getCurrentTimeMs(end_time);
    ASSERT_GE(10, (end_time - start_time)); // 10msec >= ts.tv_nsec
}

TEST_F(PowerSequencerRebootInfraIfTest, requestRebootDelay100ms)
{
    const uint_t delay_ms = UINT_T(100);
    uint_t start_time, end_time;
    getCurrentTimeMs(start_time);

    target_->requestReboot(delay_ms);

    // wait notify
    TestEvent msg;
    mq_.pend(msg);

    getCurrentTimeMs(end_time);
    ASSERT_LE(delay_ms, (end_time - start_time));
}

TEST_F(PowerSequencerRebootInfraIfTest, rebootOnStandby)
{
    bizglobal::ServiceAvailableStatus object;
    object.is_available_ = false;
    biz_global_.update(object);

    target_->reboot();

    RebootComplete reply;
    mq_.pend(reply);
}

TEST_F(PowerSequencerRebootInfraIfTest, rebootOnPowerOn)
{
    bizglobal::ServiceAvailableStatus object;
    object.is_available_ = true;
    biz_global_.update(object);

    target_->reboot();

    common::MessageQueue mq_receiver(PowerSequencerFinalizeMessage::getMqName());
    DoStandbySequence message;
    mq_receiver.pend(message);
}

TEST_F(PowerSequencerRebootInfraIfTest, handleServiceAvailableStatusWithReboot)
{
    bizglobal::ServiceAvailableStatus object;
    object.is_available_ = true;
    biz_global_.update(object);

    target_->reboot();

    common::MessageQueue mq_receiver(PowerSequencerFinalizeMessage::getMqName());
    DoStandbySequence message;
    mq_receiver.pend(message);

    // update (true) : do nothing
    object.is_available_ = true;
    biz_global_.update(object);

    common::MessageQueueAttribute attribute;
    mq_.getAttribute(attribute);
    EXPECT_EQ(0, attribute.message_size_current);

    // update (false) : post reply
    object.is_available_ = false;
    biz_global_.update(object);

    RebootComplete reply;
    mq_.pend(reply);
}

TEST_F(PowerSequencerRebootInfraIfTest, handleServiceAvailableStatusWithoutReboot)
{
    bizglobal::ServiceAvailableStatus object;
    object.is_available_ = true;
    biz_global_.update(object);

    common::MessageQueueAttribute attribute;
    mq_.getAttribute(attribute);
    EXPECT_EQ(0, attribute.message_size_current);

    object.is_available_ = false;
    biz_global_.update(object);

    mq_.getAttribute(attribute);
    EXPECT_EQ(0, attribute.message_size_current);
}

TEST_F(PowerSequencerRebootInfraIfTest, completeReboot)
{
    struct {
        bool uud_result;
        bizglobal::UudType uud_type;
        bool is_expect_uud;
        bool is_need_pan_tilt_initialize;
    } test_pattern[] = {
        {false, bizglobal::UudType::NONE,     false, true},
        {false, bizglobal::UudType::FIRMWARE, false, false},
        {false, bizglobal::UudType::LENS,     false, true},
        {true,  bizglobal::UudType::FIRMWARE, true,  false},
        {true,  bizglobal::UudType::LENS,     false, true},
    };

    ARRAY_FOREACH(test_pattern, i) {
        bizglobal::UudImageWriteCompStatus object;
        object.is_completed_ = test_pattern[i].uud_result;
        object.uud_type_ = test_pattern[i].uud_type;
        biz_global_.update(object);

        if (test_pattern[i].is_expect_uud) {
            EXPECT_CALL(power_control_infra_if_mock_, setUudRebootMark())
                .WillOnce(Return(ERRORCODE_SUCCESS));
        }
        else {
            EXPECT_CALL(power_control_infra_if_mock_, setRebootMark())
                .WillOnce(Return(ERRORCODE_SUCCESS));
        }

        bizglobal::NeedPanTiltInitialize need;
        need.is_need_ = test_pattern[i].is_need_pan_tilt_initialize;
        biz_global_.update(need);

        if (test_pattern[i].is_need_pan_tilt_initialize) {
            EXPECT_CALL(ptzf_status_if_mock_, initializePanTiltPosition())
                .Times(1);
        }
        else {
            EXPECT_CALL(ptzf_status_if_mock_, initializePanTiltPosition())
                .Times(0);
        }

        target_->completeReboot();
    }
}

} // namespace infra
} // namespace power
