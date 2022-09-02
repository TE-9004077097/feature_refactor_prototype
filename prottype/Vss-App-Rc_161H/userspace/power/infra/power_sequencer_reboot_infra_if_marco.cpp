/*
 * power_sequencer_reboot_infra_if.cpp
 *
 * Copyright 2022 Sony Corporation.
 */

#include "bizglobal.h"
#include "inbound/im_messages/reboot_request.h"
#include "inbound/boot_sequence/service_available_status.h"
#include "inbound/im_messages/uud_image_write_comp_status.h"
#include "power_sequencer_finalize_message.h"
#include "power_sequencer_reboot_infra_if.h"
#include "power_control_infra_if.h"
#include "power_sequencer_trace.h"
#include "ptzf/ptzf_status_if.h"
#include "inbound/general/need_pan_tilt_initialize.h"

namespace power {
namespace infra {

class PowerSequencerRebootInfraIfImpl : public PowerSequencerRebootInfraIf
{
public:
    explicit PowerSequencerRebootInfraIfImpl(const char* mq_name)
        : mq_(mq_name),
          biz_global_(bizglobal::BizGlobal::instance()),
          is_waiting_standby_(false)
    {
        registerNotifies();
    }
    virtual ~PowerSequencerRebootInfraIfImpl()
    {
        unregisterNotifies();
    }

    ErrorCode requestReboot(uint_t delay_ms) override
    {
        common::Task::msleep(static_cast<uint32_t>(delay_ms));

        // send message to Instance Manager via BizGlobal
        bizglobal::RebootRequest object;
        biz_global_.update(object);

        return ERRORCODE_SUCCESS;
    }

    ErrorCode reboot() override
    {
        bizglobal::ServiceAvailableStatus object;
        object = biz_global_.getValue<bizglobal::ServiceAvailableStatus>(bizglobal::InboundWaitPolicy::IMMEDIATE);
        if (object.is_available_) {
            is_waiting_standby_ = true;
            // post DoStandbySequence message to PowerSequencer
            common::MessageQueue mq_sender(PowerSequencerFinalizeMessage::getMqName());
            DoStandbySequence request;
            mq_sender.post(request);
        }
        else {
            // post RebootComplete message to PowerSequencer
            RebootComplete request;
            mq_.post(request);
        }
        return ERRORCODE_SUCCESS;
    }

    ErrorCode completeReboot() override
    {
        bizglobal::NeedPanTiltInitialize need_pan_tilt_initialize =
            biz_global_.getValue<bizglobal::NeedPanTiltInitialize>(bizglobal::InboundWaitPolicy::IMMEDIATE);
        if (need_pan_tilt_initialize.is_need_) {
            ptzf::PtzfStatusIf ptzf_if;
            ptzf_if.initializePanTiltPosition();
            need_pan_tilt_initialize.is_need_ = false;
            biz_global_.update(need_pan_tilt_initialize);
        }

        PowerControlInfraIf power_control_infra_if;
        ErrorCode ret = ERRORCODE_SUCCESS;

        bizglobal::UudImageWriteCompStatus object;
        object = biz_global_.getValue<bizglobal::UudImageWriteCompStatus>(bizglobal::InboundWaitPolicy::IMMEDIATE);
        POWER_SEQUENCER_VTRACE(object.is_completed_, object.uud_type_, 0);

        if (object.is_completed_ && (bizglobal::UudType::FIRMWARE == object.uud_type_)) {
            ret = power_control_infra_if.setUudRebootMark();
        }
        else {
            ret = power_control_infra_if.setRebootMark();
        }

        return ret;
    }

private:
    void registerNotifies()
    {
        biz_global_.registerNotify(
            this, &PowerSequencerRebootInfraIfImpl::handleServiceAvailableStatus, bizglobal::InboundWaitPolicy::NEXT);
    }
    void unregisterNotifies()
    {
        biz_global_.unregisterNotify<bizglobal::ServiceAvailableStatus>(this);
    }

    void handleServiceAvailableStatus(const bizglobal::ServiceAvailableStatus& object)
    {
        if (is_waiting_standby_ && !object.is_available_) {
            is_waiting_standby_ = false;

            // post RebootComplete message to PowerSequencer
            RebootComplete request;
            mq_.post(request);
        }
    }

private:
    PowerSequencerRebootInfraIfImpl();
    // Non-copyable
    PowerSequencerRebootInfraIfImpl(const PowerSequencerRebootInfraIfImpl&) = delete;
    PowerSequencerRebootInfraIfImpl& operator=(const PowerSequencerRebootInfraIfImpl&) = delete;

    common::MessageQueue mq_;
    bizglobal::BizGlobal& biz_global_;
    bool is_waiting_standby_;
};

std::unique_ptr<PowerSequencerRebootInfraIf> PowerSequencerRebootInfraIf::makeInfra(const char* mq_name)
{
    return std::unique_ptr<PowerSequencerRebootInfraIf>(new PowerSequencerRebootInfraIfImpl(mq_name));
}

} // namespace infra
} // namespace power
