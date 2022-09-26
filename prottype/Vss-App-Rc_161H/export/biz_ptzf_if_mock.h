/**
  biz_ptzf_if_mock.h

  Copyright 2022 Sony Corporation
*/

#ifndef EXPORT_BIZ_PTZF_IF_MOCK_H_
#define EXPORT_BIZ_PTZF_IF_MOCK_H_

#include <cstdint>
#include <string>
#include "biz_ptzf_if.h"
#include "gmock/gmock.h"
#include "common_message_queue.h"

namespace biz_ptzf {

#pragma GCC diagnostic ignored "-Weffc++"
class BizPtzfIfMock
{
public:
    BizPtzfIfMock()
    {}

    ~BizPtzfIfMock()
    {}
    MOCK_METHOD2(setFocusMode, bool(const FocusMode focus_mode, const u32_t seq_id));
    MOCK_METHOD2(setAfTransitionSpeedValue, bool(const uint8_t& af_transition_speed, const u32_t seq_id));
    MOCK_METHOD2(setAfSubjShiftSensValue, bool(const uint8_t& af_subj_shift_sens, const u32_t seq_id));
    MOCK_METHOD2(setFocusFaceEyedetectionValue, bool(const FocusFaceEyeDetectionMode focus_face_eye_detection_mode,
                                                     const u32_t seq_id));
    MOCK_METHOD2(setFocusArea, bool(const FocusArea focus_area, const u32_t seq_id));
    MOCK_METHOD3(setAFAreaPositionAFC, bool(const u16_t position_x, const u16_t position_y, const u32_t seq_id));
    MOCK_METHOD3(setAFAreaPositionAFS, bool(const u16_t position_x, const u16_t position_y, const u32_t seq_id));
    MOCK_METHOD2(setZoomPosition, bool(const u32_t position, const u32_t seq_id));
    MOCK_METHOD2(setFocusPosition, bool(const u32_t position, const u32_t seq_id));
};
#pragma GCC diagnostic warning "-Weffc++"

} // namespace biz_ptzf

#endif    // EXPORT_BIZ_PTZF_IF_MOCK_H_
