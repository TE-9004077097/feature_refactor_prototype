/*
 * biz_ptzf_if_mock.cpp
 *
 * Copyright 2022 Sony Corporation
 */

#include "types.h"

#include "gmock/gmock.h"
#include "common_gmock_util.h"

#include "biz_ptzf_if_mock.h"

namespace biz_ptzf {

struct BizPtzfIf::BizPtzfIfImpl
{
    BizPtzfIfImpl()
        : mock_holder(MockHolder<BizPtzfIfMock>::instance())
    {}

    MockHolder<BizPtzfIfMock>& mock_holder;

    virtual ~BizPtzfIfImpl()
    {}
};

BizPtzfIf::BizPtzfIf()
    : pimpl_(new BizPtzfIfImpl())
{}

BizPtzfIf::~BizPtzfIf()
{}

bool BizPtzfIf::setFocusMode(const FocusMode focus_mode, const u32_t seq_id)
{
    BizPtzfIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusMode(focus_mode, seq_id);
}

bool BizPtzfIf::setAfTransitionSpeedValue(const uint8_t& af_transition_speed, const u32_t seq_id)
{
    BizPtzfIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAfTransitionSpeedValue(af_transition_speed, seq_id);
}

bool BizPtzfIf::setAfSubjShiftSensValue(const uint8_t& af_subj_shift_sens, const u32_t seq_id)
{
    BizPtzfIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAfSubjShiftSensValue(af_subj_shift_sens, seq_id);
}

bool BizPtzfIf::setFocusFaceEyedetectionValue(const FocusFaceEyeDetectionMode focus_face_eye_detection_mode,
                                               const u32_t seq_id)
{
    BizPtzfIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusFaceEyedetectionValue(focus_face_eye_detection_mode, seq_id);
}

bool BizPtzfIf::setFocusArea(const FocusArea focus_area, const u32_t seq_id)
{
    BizPtzfIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusArea(focus_area, seq_id);
}

bool BizPtzfIf::setAFAreaPositionAFC(const u16_t position_x, const u16_t position_y, const u32_t seq_id)
{
    BizPtzfIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAFAreaPositionAFC(position_x, position_y, seq_id);
}

bool BizPtzfIf::setAFAreaPositionAFS(const u16_t position_x, const u16_t position_y, const u32_t seq_id)
{
    BizPtzfIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setAFAreaPositionAFS(position_x, position_y, seq_id);
}

bool BizPtzfIf::setZoomPosition(const u32_t position, const u32_t seq_id)
{
    BizPtzfIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setZoomPosition(position, seq_id);
}

bool BizPtzfIf::setFocusPosition(const u32_t position, const u32_t seq_id)
{
    BizPtzfIfMock& mock = pimpl_->mock_holder.getMock();
    return mock.setFocusPosition(position, seq_id);
}

} // namespace biz_ptzf
