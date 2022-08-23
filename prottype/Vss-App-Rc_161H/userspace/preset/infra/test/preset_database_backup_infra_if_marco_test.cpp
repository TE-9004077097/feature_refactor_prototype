/**
  preset_database_backup_infra_if_marco_test.cpp

  Copyright 2021 Sony Corporation
*/

#include <vector>

#include "types.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gtl_memory.h"
#include "gtl_container_foreach.h"
#include "common_message_queue.h"
#include "common_shm.h"
#include "common_gmock_util.h"
#include "common_thread_object.h"

#include "preset_database_backup_infra_if.h"
#include "preset/preset_manager_message.h"
#include "preset_database_manager_config_list_marco.h"

#include "preset_database_backup_infra_message_handler_marco.h"
#include "preset_database_backup_infra_message_handler_marco_mock.h"

#include "config/visca_config_db_mock.h"
#include "config/visca_config_backup_mock.h"
#include "config/visca_config_database_manager_mock.h"
#include "config/visca_config_preset_database_backup.h"
#include "config/model_info_rc.h"
#include "visca/visca_config_if.h"
#include "visca/config_service_common.h"
#include "visca/dboutputs/config_pan_tilt_service.h"
#include "visca/dboutputs/config_remote_camera_service.h"
#include "visca/dboutputs/config_backup_service.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::StrEq;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::Invoke;
using ::testing::InSequence;

namespace config {

struct DatabaseReplaceEntry
{
    u32_t id;
    u32_t size;
    void* data;
};
} // namespace config

namespace preset {
namespace infra {
namespace {

// ○テストリスト
// + 起動時にBackupからPresetの内容をshmにリストアする
//     + Backupに存在するものだけをshmに復元する
//     + Backupに存在しないものはデフォルト値でshmを作成する
// - 起動時にConfigFirstAccessCheckerのロックを解除する → UnitTestでは表現不能
// + saveメソッドでBackupにshm上の情報を保存
// + resetメソッドでBackupに保存されている情報をConstantDBで上書き
// + recallメソッドでBackupからshm上に情報をリストア
// + save, reset, recallメソッドのpreset#の範囲チェック
// + PowerOn時に最終値を復元する必要のあるパラメータの扱い
//     + backupとshmのどちらも空の場合
//     + backupが空でshmに何かデータがある場合
//     + backupに何かデータがありshmが空の場合
//     + backupとshmどちらも何かデータがある場合
// + clear()から異常が返却されるパターン
//     + PRESET_IDが異常値
// + clear()から正常が返却されるパターン
//     + PRESET_IDが正常値 (0～255)
//     + config::ViscaConfigDb::set() に渡す引数が正しいこと
//     + config::ViscaConfigDatabaseManager() に渡す引数が正しいこと
// + view()から異常が返却されるパターン
//     + PRESET_IDが異常値
// + view()から正常が返却されるパターン
//     + PRESET_IDが正常値（0）
// + resetBackupOnUud(正常系)
//     + UUD時にresetするBackupが全て初期値に戻っていること

MATCHER(IsEmpty, "")
{
    return arg.empty();
}

template <typename T1, typename T2>
void setPresetValue(const T2& value, const u32_t preset_num = 0)
{
    T1 config;
    config::ViscaConfig config_if(config::ViscaConfig::read_write);
    config.entry.setParam(value);
    config_if.setPreset(config, preset_num);
}

class InvalidIdExpector
{
public:
    explicit InvalidIdExpector(const char_t* section) : section_(section)
    {}

    template <typename T>
    void operator()()
    {
        typedef typename T::service service;
        common::ShmImplMap& shm_map = common::ShmImplMap::instance();
        const u32_t size = static_cast<u32_t>(sizeof(service));
        common::ShmImpl* shm_impl_ptr = shm_map.retrieve(section_, service::id_, size);
        if (0 == shm_impl_ptr) {
            return;
        }
        u8_t* shm_mem_ptr = reinterpret_cast<u8_t*>(shm_impl_ptr->getAddr());

        EXPECT_FALSE(*reinterpret_cast<bool*>(shm_mem_ptr + size));
    }

private:
    const char_t* section_;
};

class ValidIdExpector
{
public:
    explicit ValidIdExpector(const char_t* section) : section_(section)
    {}

    template <typename T>
    void operator()()
    {
        typedef typename T::service service;
        common::ShmImplMap& shm_map = common::ShmImplMap::instance();
        const u32_t size = static_cast<u32_t>(sizeof(service));
        common::ShmImpl* shm_impl_ptr = shm_map.retrieve(section_, service::id_, size);
        if (0 == shm_impl_ptr) {
            return;
        }
        u8_t* shm_mem_ptr = reinterpret_cast<u8_t*>(shm_impl_ptr->getAddr());

        EXPECT_TRUE(*reinterpret_cast<bool*>(shm_mem_ptr + size));
    }

private:
    const char_t* section_;
};

class ShmInitializer
{
public:
    ShmInitializer()
    {}

    template <typename T>
    void operator()()
    {
        typedef typename T::service service;
        common::ShmImplMap& shm_map = common::ShmImplMap::instance();
        const u32_t size = static_cast<u32_t>(sizeof(service));
        for (u32_t i = 0; i <= MAX_PRESET_ID; ++i) {
            const char_t* section = visca::SectionConfig::getName(i);
            common::ShmImpl* shm_impl_ptr = shm_map.retrieve(section, service::id_, size);
            if (0 == shm_impl_ptr) {
                continue;
            }
            shm_impl_ptr->unlink();
        }
    }
};

class ConfigIdEqualFinder
{
public:
    explicit ConfigIdEqualFinder(const u32_t id) : id_(id)
    {}

    template <class T>
    bool operator()()
    {
        typedef typename T::service service;
        if (service::id_ == id_) {
            return true;
        }
        return false;
    }

private:
    const u32_t id_;
};

void setInvalidReturnDataForRecall(const char_t*, u32_t, u32_t, u32_t, void* data)
{
    u8_t expect_data[config::PresetDatabaseEntry::DATA_SIZE];
    gtl::fillMemory(expect_data, U8_T(0xFF));

    u8_t* tmp = reinterpret_cast<u8_t*>(data);
    memcpy(tmp, expect_data, sizeof(expect_data));
}

#pragma GCC diagnostic ignored "-Weffc++"

class PresetDatabaseBackupInfraTest : public ::testing::Test
{
public:
    PresetDatabaseBackupInfraTest()
        : db_mock_holder_object(),
          db_mock(),
          backup_mock_holder_object_(),
          backup_mock_(),
          handler_mock_holder_(),
          handler_mock_(handler_mock_holder_.getMock()),
          message_handler_(),
          thread_mq_(PresetDatabaseBackupInfraMessageHandler::getName())
    {}

    virtual void SetUp()
    {
        db_mock_holder_object.reset(new MockHolderObject<config::ViscaConfigDBMock>);
        db_mock = &(db_mock_holder_object->getMock());
        backup_mock_holder_object_.reset(new MockHolderObject<config::ViscaConfigDatabaseManagerMock>);
        backup_mock_ = &(backup_mock_holder_object_->getMock());
        visca_config_fake::MOCK_Prepare();
        visca_config_backup_fake::MOCK_Prepare();

        config::ModelInfoRc::instance().setModelName("BRC1000BASE");

        gtl::forEach<PresetRecallOnBootConfigsForAllTL>(ShmInitializer());
        gtl::forEach<PresetSetConfigsForAllTL>(ShmInitializer());
        gtl::forEach<PresetSetConfigsForPreset1TL>(ShmInitializer());
        gtl::forEach<PresetResetConfigsForAllTL>(ShmInitializer());
        gtl::forEach<PresetRecallConfigsForAllTL>(ShmInitializer());
        gtl::forEach<PresetForPowerOnConfigsForPreset1TL>(ShmInitializer());
        gtl::forEach<PresetForPowerOnConfigsForAllTL>(ShmInitializer());

        EXPECT_CALL(*backup_mock_, restoreAll()).WillRepeatedly(Return(ERRORCODE_SUCCESS));
    }

    virtual void TearDown()
    {
        backup_mock_ = 0;
        backup_mock_holder_object_.reset();
        db_mock_holder_object.reset();
        db_mock = 0;
        visca_config_fake::MOCK_Finalize();
        visca_config_backup_fake::MOCK_Finalize();

        common::MessageQueue mq("ConfigReadyMQ");
        mq.unlink();

        gtl::forEach<PresetRecallOnBootConfigsForAllTL>(ShmInitializer());
        gtl::forEach<PresetSetConfigsForAllTL>(ShmInitializer());
        gtl::forEach<PresetSetConfigsForPreset1TL>(ShmInitializer());
        gtl::forEach<PresetResetConfigsForAllTL>(ShmInitializer());
        gtl::forEach<PresetRecallConfigsForAllTL>(ShmInitializer());
        gtl::forEach<PresetForPowerOnConfigsForPreset1TL>(ShmInitializer());
        gtl::forEach<PresetForPowerOnConfigsForAllTL>(ShmInitializer());
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

protected:
    gtl::AutoPtr<MockHolderObject<config::ViscaConfigDBMock>> db_mock_holder_object;
    config::ViscaConfigDBMock* db_mock;
    gtl::AutoPtr<MockHolderObject<config::ViscaConfigDatabaseManagerMock>> backup_mock_holder_object_;
    config::ViscaConfigDatabaseManagerMock* backup_mock_;
    MockHolderObject<PresetDatabaseBackupInfraMessageHandlerMock> handler_mock_holder_;
    PresetDatabaseBackupInfraMessageHandlerMock& handler_mock_;
    common::ThreadObject<PresetDatabaseBackupInfraMessageHandler> message_handler_;
    common::MessageQueue thread_mq_;
};

#pragma GCC diagnostic warning "-Weffc++"

MATCHER_P2(EqSetPresetRequest, preset_id, mq_name, "")
{
    const SetPresetRequest& req = reinterpret_cast<const SetPresetRequest&>(arg);
    if (req.preset_id == preset_id) {
        return true;
    }
    return false;
}

TEST_F(PresetDatabaseBackupInfraTest, createWithoutValidData)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();

    const char_t* section = visca::SectionConfig::getName(DEFAULT_PRESET_ID);
    gtl::forEach<PresetRecallOnBootConfigsForAllTL>(ValidIdExpector(section));
}

TEST_F(PresetDatabaseBackupInfraTest, saveWithEmptyShm)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();

    EXPECT_CALL(*db_mock, set(_, _, _)).Times(2).WillRepeatedly(Return());
    EXPECT_CALL(*backup_mock_, backup(_)).Times(2).WillRepeatedly(Return(ERRORCODE_SUCCESS));
    EXPECT_CALL(handler_mock_, handleSetPresetRequest(EqSetPresetRequest(DEFAULT_PRESET_ID, _)))
        .Times(1)
        .WillOnce(Return());

    EXPECT_TRUE(preset_database_backup_infra_if.save(DEFAULT_PRESET_ID));
    waitQueueEmpty(thread_mq_);

    EXPECT_CALL(handler_mock_, handleSetPresetRequest(EqSetPresetRequest(MAX_PRESET_ID, _)))
        .Times(1)
        .WillOnce(Return());
    EXPECT_TRUE(preset_database_backup_infra_if.save(MAX_PRESET_ID));
    waitQueueEmpty(thread_mq_);
}

TEST_F(PresetDatabaseBackupInfraTest, saveWithSmallBackup)
{
    // remove this test. (visca_config does not generate length error)
}

void expectValidSaveEntriesForAll(const char_t*, u32_t, const std::vector<config::DatabaseReplaceEntry>& entries)
{
    CONTAINER_FOREACH_CONST (const config::DatabaseReplaceEntry& item, entries) {
        const bool expect_id_found = gtl::findIf<PresetSetConfigsForAllTL>(ConfigIdEqualFinder(item.id));
        EXPECT_TRUE(expect_id_found);
    }
    EXPECT_EQ(U32_T(10), entries.size());
}

TEST_F(PresetDatabaseBackupInfraTest, saveWithSomeShm)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();
    preset_database_backup_infra_if.recall(DEFAULT_PRESET_ID);
    preset_database_backup_infra_if.recall(U32_T(0x80));
    preset_database_backup_infra_if.recall(MAX_PRESET_ID);

    // do not count non-preset configuration entry
    visca::PTDirectionMoveDirectionMoveParam direct;
    direct.pan_speed = U8_T(0x18);
    direct.tilt_speed = U8_T(0x18);
    visca::setPresetValue<visca::ConfigPTDirectionMoveDirectionMoveService>(direct, DEFAULT_PRESET_ID);

    visca::RCPresetModePresetModeParam mode;
    mode.preset_mode = visca::PRESET_MODE_MODE1;
    visca::setPresetValue<visca::ConfigRCPresetModePresetModeService>(mode, DEFAULT_PRESET_ID);

    ::testing::InSequence dummy;

    EXPECT_CALL(*db_mock, set(_, _, _)).Times(1).WillOnce(Invoke(expectValidSaveEntriesForAll));
    EXPECT_CALL(*backup_mock_, backup(_)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));

    EXPECT_TRUE(preset_database_backup_infra_if.save(DEFAULT_PRESET_ID));
    waitQueueEmpty(thread_mq_);

    EXPECT_CALL(*db_mock, set(_, _, _)).Times(1).WillOnce(Invoke(expectValidSaveEntriesForAll));
    EXPECT_CALL(*backup_mock_, backup(_)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));

    EXPECT_TRUE(preset_database_backup_infra_if.save(U32_T(0x80)));
    waitQueueEmpty(thread_mq_);

    EXPECT_CALL(*db_mock, set(_, _, _)).Times(1).WillOnce(Invoke(expectValidSaveEntriesForAll));
    EXPECT_CALL(*backup_mock_, backup(_)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));

    EXPECT_TRUE(preset_database_backup_infra_if.save(MAX_PRESET_ID));
    waitQueueEmpty(thread_mq_);
}

void expectValidResetEntriesForAll(const char_t*, u32_t, const std::vector<config::DatabaseReplaceEntry>& entries)
{
    CONTAINER_FOREACH_CONST (const config::DatabaseReplaceEntry& item, entries) {
        const bool expect_id_found = gtl::findIf<PresetResetConfigsForAllTL>(ConfigIdEqualFinder(item.id));
        EXPECT_TRUE(expect_id_found);

        if (visca::ConfigRCPresetMoveSpeedPresetMoveSpeedService::id_ == item.id) {
            visca::RCPresetMoveSpeedPresetMoveSpeedParam move_speed, ms_default;
            memcpy(&move_speed, item.data, item.size);
            EXPECT_NE(U8_T(0x18), move_speed.preset_move_speed);
            EXPECT_EQ(ms_default.preset_move_speed, move_speed.preset_move_speed);
        }
    }
    const u32_t expect_size = 0;
    EXPECT_EQ(expect_size, entries.size());
}

TEST_F(PresetDatabaseBackupInfraTest, resetWithEmptyShm)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();

    EXPECT_CALL(*db_mock, set(_, _, _)).Times(2).WillRepeatedly(Return());
    EXPECT_CALL(*backup_mock_, backup(_)).Times(2).WillRepeatedly(Return(ERRORCODE_SUCCESS));

    EXPECT_TRUE(preset_database_backup_infra_if.reset(DEFAULT_PRESET_ID));
    EXPECT_TRUE(preset_database_backup_infra_if.reset(MAX_PRESET_ID));
}

TEST_F(PresetDatabaseBackupInfraTest, resetWithSomeShm)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();

    // do not count non-preset configuration entry
    visca::PTDirectionMoveDirectionMoveParam direct;
    direct.pan_speed = U8_T(0x18);
    direct.tilt_speed = U8_T(0x18);
    visca::setPresetValue<visca::ConfigPTDirectionMoveDirectionMoveService>(direct, DEFAULT_PRESET_ID);

    visca::RCPresetModePresetModeParam mode;
    mode.preset_mode = visca::PRESET_MODE_MODE2;
    visca::setPresetValue<visca::ConfigRCPresetModePresetModeService>(mode, DEFAULT_PRESET_ID);

    ::testing::InSequence dummy;

    EXPECT_CALL(*db_mock, set(_, _, _)).Times(1).WillOnce(Invoke(expectValidResetEntriesForAll));
    EXPECT_CALL(*backup_mock_, backup(_)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));

    EXPECT_TRUE(preset_database_backup_infra_if.reset(DEFAULT_PRESET_ID));

    EXPECT_CALL(*db_mock, set(_, _, _)).Times(1).WillOnce(Invoke(expectValidResetEntriesForAll));
    EXPECT_CALL(*backup_mock_, backup(_)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));

    EXPECT_TRUE(preset_database_backup_infra_if.reset(MAX_PRESET_ID));
}

void verifyShmDataForRecall(const char_t* section)
{
    common::ShmImplMap& shm_map = common::ShmImplMap::instance();
    common::ShmImpl* shm_impl_ptr = shm_map.retrieve(section,
                                                     visca::ConfigRCPresetMoveSpeedPresetMoveSpeedService::id_,
                                                     sizeof(visca::ConfigRCPresetMoveSpeedPresetMoveSpeedService));
    ASSERT(shm_impl_ptr != 0);
}

TEST_F(PresetDatabaseBackupInfraTest, recallWithEmptyBackup)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();
    ::testing::Mock::VerifyAndClearExpectations(db_mock);

    ::testing::InSequence dummy;
    const char_t* section = visca::SectionConfig::getName(DEFAULT_PRESET_ID);
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(visca::ConfigPTAbsolutePositionPositionInqService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FocusModeStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFTransitionSpeedStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFSubjShiftSensStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FaceEyeDitectionAFStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FocusAreaModeStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFCAreaPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFSAreaPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::ZoomPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FocusPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_TRUE(preset_database_backup_infra_if.recall(DEFAULT_PRESET_ID));
    gtl::forEach<PresetRecallConfigsForAllTL>(ValidIdExpector(section));
    verifyShmDataForRecall(section);

    section = visca::SectionConfig::getName(MAX_PRESET_ID);
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(visca::ConfigPTAbsolutePositionPositionInqService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FocusModeStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFTransitionSpeedStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFSubjShiftSensStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FaceEyeDitectionAFStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FocusAreaModeStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFCAreaPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFSAreaPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::ZoomPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FocusPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_TRUE(preset_database_backup_infra_if.recall(MAX_PRESET_ID));
    gtl::forEach<PresetRecallConfigsForAllTL>(ValidIdExpector(section));
    verifyShmDataForRecall(section);
}

TEST_F(PresetDatabaseBackupInfraTest, recallWithSomeBackup)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();
    ::testing::Mock::VerifyAndClearExpectations(db_mock);

    ::testing::InSequence dummy;

    const char_t* section = visca::SectionConfig::getName(DEFAULT_PRESET_ID);
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(visca::ConfigPTAbsolutePositionPositionInqService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FocusModeStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFTransitionSpeedStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFSubjShiftSensStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FaceEyeDitectionAFStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FocusAreaModeStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFCAreaPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFSAreaPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::ZoomPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FocusPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_TRUE(preset_database_backup_infra_if.recall(DEFAULT_PRESET_ID));
    gtl::forEach<PresetRecallConfigsForAllTL>(ValidIdExpector(section));
    verifyShmDataForRecall(section);

    section = visca::SectionConfig::getName(MAX_PRESET_ID);
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(visca::ConfigPTAbsolutePositionPositionInqService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FocusModeStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFTransitionSpeedStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFSubjShiftSensStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FaceEyeDitectionAFStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FocusAreaModeStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFCAreaPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::AFSAreaPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::ZoomPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(ptzf::FocusPositionStatusService::id_), _, _))
        .Times(1)
        .WillOnce(Invoke(setInvalidReturnDataForRecall));
    EXPECT_TRUE(preset_database_backup_infra_if.recall(MAX_PRESET_ID));
    gtl::forEach<PresetRecallConfigsForAllTL>(ValidIdExpector(section));
    verifyShmDataForRecall(section);
}

TEST_F(PresetDatabaseBackupInfraTest, saveWithInvalidPresetNumber)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();

    EXPECT_FALSE(preset_database_backup_infra_if.save(INVALID_PRESET_ID));
}

TEST_F(PresetDatabaseBackupInfraTest, resetWithInvalidPresetNumber)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();

    EXPECT_FALSE(preset_database_backup_infra_if.reset(INVALID_PRESET_ID));
}

TEST_F(PresetDatabaseBackupInfraTest, recallWithInvalidPresetNumber)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();

    EXPECT_FALSE(preset_database_backup_infra_if.recall(INVALID_PRESET_ID));
}

TEST_F(PresetDatabaseBackupInfraTest, saveForPowerOnParameterWithEmptyShmAndEmptyBackup)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();

    s32_t expect_get_called = gtl::Length<PresetRecallOnBootConfigsForAllTL>::value * U32_T(2);
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(expect_get_called).WillRepeatedly(Return());
    EXPECT_CALL(*db_mock, set(_, _, _)).Times(2).WillRepeatedly(Return());
    EXPECT_CALL(*backup_mock_, backup(_)).Times(2).WillRepeatedly(Return(ERRORCODE_SUCCESS));

    EXPECT_TRUE(preset_database_backup_infra_if.saveForPowerOnParameter(DEFAULT_PRESET_ID));
    EXPECT_TRUE(preset_database_backup_infra_if.saveForPowerOnParameter(MAX_PRESET_ID));
}

void expectValidSaveForPowerOnEntriesForPreset1(const char_t*,
                                                u32_t,
                                                const std::vector<config::DatabaseReplaceEntry>& entries)
{
    CONTAINER_FOREACH_CONST (const config::DatabaseReplaceEntry& item, entries) {
        const bool expect_id_found =
            gtl::findIf<PresetForPowerOnConfigsForPreset1TL>(ConfigIdEqualFinder(item.id))
            || gtl::findIf<PresetForInitializingWithImageFlipConfigsForPreset1TL>(ConfigIdEqualFinder(item.id))
            || gtl::findIf<PresetForPowerOnConfigsForAllTL>(ConfigIdEqualFinder(item.id));
        EXPECT_TRUE(expect_id_found);
    }
    const u32_t expect_size = gtl::Length<PresetForPowerOnConfigsForPreset1TL>::value
                              + gtl::Length<PresetForPowerOnConfigsForAllTL>::value
                              + gtl::Length<PresetForInitializingWithImageFlipConfigsForPreset1TL>::value;
    EXPECT_EQ(expect_size, entries.size());
}

void expectValidSaveForPowerOnEntriesForPresetAll(const char_t*,
                                                  u32_t,
                                                  const std::vector<config::DatabaseReplaceEntry>& entries)
{
    CONTAINER_FOREACH_CONST (const config::DatabaseReplaceEntry& item, entries) {
        const bool expect_id_found = gtl::findIf<PresetForPowerOnConfigsForAllTL>(ConfigIdEqualFinder(item.id));
        EXPECT_TRUE(expect_id_found);
    }
    const u32_t expect_size = gtl::Length<PresetForPowerOnConfigsForAllTL>::value;
    EXPECT_EQ(expect_size, entries.size());
}

TEST_F(PresetDatabaseBackupInfraTest, saveForPowerOnParameterWithSomeShmAndEmptyBackup)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();
    ::testing::Mock::VerifyAndClearExpectations(db_mock);

    ::testing::InSequence dummy;

    visca::PTAbsolutePositionPositionInqParam position;
    position.pan_position = U32_T(0x12345);
    position.tilt_position = U16_T(0x6789);
    visca::setPresetValue<visca::ConfigPTAbsolutePositionPositionInqService>(position, DEFAULT_PRESET_ID);
    visca::setPresetValue<visca::ConfigPTAbsolutePositionPositionInqService>(position, MAX_PRESET_ID);

    visca::PTSlowSlowParam pt_slow;
    pt_slow.slow = true;
    visca::setPresetValue<visca::ConfigPTSlowSlowService>(pt_slow, DEFAULT_PRESET_ID);

    visca::CAMPictureFlipPictureFlipParam pict_flip;
    pict_flip.picture_flip = visca::PICTURE_FLIP_MODE_ON;
    visca::setPresetValue<visca::ConfigCAMPictureFlipPictureFlipService>(pict_flip, DEFAULT_PRESET_ID);

    visca::RCPresetModePresetModeParam mode;
    mode.preset_mode = visca::PRESET_MODE_TRACE;
    visca::setPresetValue<visca::ConfigRCPresetModePresetModeService>(mode, DEFAULT_PRESET_ID);

    visca::CAMIRCorrectionIRCorrectionParam param;
    param.ir_correction = visca::IR_CORRECTION_IRLIGHT;
    visca::setPresetValue<visca::ConfigCAMIRCorrectionIRCorrectionService>(param, DEFAULT_PRESET_ID);

    s32_t expect_get_called = gtl::Length<PresetRecallOnBootConfigsForAllTL>::value;
    const char_t* section = visca::SectionConfig::getName(DEFAULT_PRESET_ID);
    EXPECT_CALL(*db_mock, get(StrEq(section), _, _, _, _)).Times(expect_get_called).WillRepeatedly(Return());
    EXPECT_CALL(*db_mock, set(StrEq(section), _, _))
        .Times(1)
        .WillOnce(Invoke(expectValidSaveForPowerOnEntriesForPreset1));
    EXPECT_CALL(*backup_mock_, backup(StrEq(section))).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_TRUE(preset_database_backup_infra_if.saveForPowerOnParameter(DEFAULT_PRESET_ID));

    expect_get_called = gtl::Length<PresetRecallOnBootConfigsForAllTL>::value;
    section = visca::SectionConfig::getName(MAX_PRESET_ID);
    EXPECT_CALL(*db_mock, get(StrEq(section), _, _, _, _)).Times(expect_get_called).WillRepeatedly(Return());
    EXPECT_CALL(*db_mock, set(StrEq(section), _, _))
        .Times(1)
        .WillOnce(Invoke(expectValidSaveForPowerOnEntriesForPresetAll));
    EXPECT_CALL(*backup_mock_, backup(StrEq(section))).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));

    EXPECT_TRUE(preset_database_backup_infra_if.saveForPowerOnParameter(MAX_PRESET_ID));
}

TEST_F(PresetDatabaseBackupInfraTest, saveForPowerOnParameterWithEmptyShmAndSomeBackup)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();
    ::testing::Mock::VerifyAndClearExpectations(db_mock);

    ::testing::InSequence dummy;

    s32_t expect_get_called = gtl::Length<PresetRecallOnBootConfigsForAllTL>::value;

    const char_t* section = visca::SectionConfig::getName(DEFAULT_PRESET_ID);
    EXPECT_CALL(*db_mock, get(StrEq(section), _, _, _, _))
        .Times(expect_get_called)
        .WillRepeatedly(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(0);
    EXPECT_CALL(*db_mock, set(StrEq(section), _, _))
        .Times(1)
        .WillOnce(Invoke(expectValidSaveForPowerOnEntriesForPreset1));
    EXPECT_CALL(*backup_mock_, backup(StrEq(section))).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_TRUE(preset_database_backup_infra_if.saveForPowerOnParameter(DEFAULT_PRESET_ID));

    expect_get_called = gtl::Length<PresetRecallOnBootConfigsForAllTL>::value;
    section = visca::SectionConfig::getName(MAX_PRESET_ID);
    EXPECT_CALL(*db_mock, get(StrEq(section), _, _, _, _))
        .Times(expect_get_called)
        .WillRepeatedly(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(visca::ConfigRCPresetMoveSpeedPresetMoveSpeedService::id_), _, _))
        .Times(0);
    EXPECT_CALL(*db_mock, set(StrEq(section), _, _))
        .Times(1)
        .WillOnce(Invoke(expectValidSaveForPowerOnEntriesForPresetAll));
    EXPECT_CALL(*backup_mock_, backup(StrEq(section))).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_TRUE(preset_database_backup_infra_if.saveForPowerOnParameter(MAX_PRESET_ID));
}

TEST_F(PresetDatabaseBackupInfraTest, saveForPowerOnParameterWithSomeShmAndSomeBackup)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();
    ::testing::Mock::VerifyAndClearExpectations(db_mock);

    ::testing::InSequence dummy;

    visca::PTAbsolutePositionPositionInqParam position;
    position.pan_position = U32_T(0x12345);
    position.tilt_position = U16_T(0x6789);
    visca::setPresetValue<visca::ConfigPTAbsolutePositionPositionInqService>(position, DEFAULT_PRESET_ID);
    visca::setPresetValue<visca::ConfigPTAbsolutePositionPositionInqService>(position, MAX_PRESET_ID);

    visca::PTSlowSlowParam pt_slow;
    pt_slow.slow = true;
    visca::setPresetValue<visca::ConfigPTSlowSlowService>(pt_slow, DEFAULT_PRESET_ID);

    visca::CAMPictureFlipPictureFlipParam pict_flip;
    pict_flip.picture_flip = visca::PICTURE_FLIP_MODE_ON;
    visca::setPresetValue<visca::ConfigCAMPictureFlipPictureFlipService>(pict_flip, DEFAULT_PRESET_ID);

    visca::RCPresetModePresetModeParam mode;
    mode.preset_mode = visca::PRESET_MODE_MODE1;
    visca::setPresetValue<visca::ConfigRCPresetModePresetModeService>(mode, DEFAULT_PRESET_ID);

    s32_t expect_get_called = gtl::Length<PresetRecallOnBootConfigsForAllTL>::value;
    const char_t* section = visca::SectionConfig::getName(DEFAULT_PRESET_ID);
    EXPECT_CALL(*db_mock, get(StrEq(section), _, _, _, _))
        .Times(expect_get_called)
        .WillRepeatedly(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(StrEq(section), _, Eq(visca::ConfigRCPresetMoveSpeedPresetMoveSpeedService::id_), _, _))
        .Times(0);
    EXPECT_CALL(*db_mock, set(StrEq(section), _, _))
        .Times(1)
        .WillOnce(Invoke(expectValidSaveForPowerOnEntriesForPreset1));
    EXPECT_CALL(*backup_mock_, backup(_)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_TRUE(preset_database_backup_infra_if.saveForPowerOnParameter(DEFAULT_PRESET_ID));

    expect_get_called = gtl::Length<PresetRecallOnBootConfigsForAllTL>::value;
    section = visca::SectionConfig::getName(MAX_PRESET_ID);
    EXPECT_CALL(*db_mock, get(StrEq(section), _, _, _, _))
        .Times(expect_get_called)
        .WillRepeatedly(Invoke(setInvalidReturnDataForRecall));
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(0);
    EXPECT_CALL(*db_mock, set(StrEq(section), _, _))
        .Times(1)
        .WillOnce(Invoke(expectValidSaveForPowerOnEntriesForPresetAll));
    EXPECT_CALL(*backup_mock_, backup(_)).Times(1).WillOnce(Return(ERRORCODE_SUCCESS));
    EXPECT_TRUE(preset_database_backup_infra_if.saveForPowerOnParameter(MAX_PRESET_ID));
}

TEST_F(PresetDatabaseBackupInfraTest, saveForPowerOnParameterWithInvalidPresetNumber)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();

    EXPECT_FALSE(preset_database_backup_infra_if.saveForPowerOnParameter(INVALID_PRESET_ID));
}

TEST_F(PresetDatabaseBackupInfraTest, saveForPowerOnParameterWithSmallBackup)
{
    // remove this test. (visca_config does not generate length error)
}

TEST_F(PresetDatabaseBackupInfraTest, resetAll)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();
    InSequence dummy;

    for (u32_t count = DEFAULT_PRESET_ID; count <= MAX_PRESET_ID; ++count) {
        const char* section = visca::SectionConfig::getName(count);
        if (count == DEFAULT_PRESET_ID) {
            EXPECT_CALL(*db_mock, set(StrEq(section), UINT32_C(1), _)).WillOnce(Return());
        }
        else {
            EXPECT_CALL(*db_mock, set(StrEq(section), UINT32_C(1), IsEmpty())).WillOnce(Return());
        }
        EXPECT_CALL(*backup_mock_, setUpdate(StrEq(section))).WillOnce(Return(ERRORCODE_SUCCESS));
    }
    EXPECT_CALL(*backup_mock_, finalize()).WillOnce(Return(ERRORCODE_SUCCESS));

    EXPECT_TRUE(preset_database_backup_infra_if.reset());
}

TEST_F(PresetDatabaseBackupInfraTest, resetBackupOnUud)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(AtLeast(1)).WillRepeatedly(Return());

    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    preset_database_backup_infra_if.recallOnBoot();
    visca::ConfigBackupViscaServerFcbUpdatedService set_service;
    visca::ConfigBackupViscaServerFcbUpdatedService get_service;
    visca::BackupViscaServerFcbUpdatedParam mock_param;
    mock_param.visca_server_fcb_updated = true;
    set_service.entry.setParam(mock_param);
    visca_config_backup_fake::EXPECT_CALL_SET<visca::SectionBackup>(set_service);

    EXPECT_TRUE(preset_database_backup_infra_if.resetBackupOnUud());

    mock_param.visca_server_fcb_updated = false;
    get_service.entry.setParam(mock_param);
    visca_config_backup_fake::EXPECT_CALL_GET<visca::SectionBackup>(get_service);

    visca::BackupViscaServerFcbUpdatedParam param;
    param.visca_server_fcb_updated = true;
    visca::getBackupValue<visca::ConfigBackupViscaServerFcbUpdatedService>(param);
    EXPECT_FALSE(param.visca_server_fcb_updated);
}

TEST_F(PresetDatabaseBackupInfraTest, clear)
{
    struct
    {
        u32_t preset_id_;
    } const test_table[] = {
        { DEFAULT_PRESET_ID },
        { UINT32_C(1) },
        { UINT32_C(2) },
        { MAX_PRESET_ID },
    };
    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;

    for (auto item : test_table) {
        const char* section = visca::SectionConfig::getName(item.preset_id_);
        EXPECT_CALL(*db_mock, set(StrEq(section), UINT32_C(1), IsEmpty())).WillOnce(Return());
        EXPECT_CALL(*backup_mock_, backup(StrEq(section))).WillOnce(Return(ERRORCODE_SUCCESS));

        EXPECT_TRUE(preset_database_backup_infra_if.clear(item.preset_id_));
    }
    EXPECT_FALSE(preset_database_backup_infra_if.clear(INVALID_PRESET_ID));
}

TEST_F(PresetDatabaseBackupInfraTest, view)
{
    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).Times(0);

    PresetDatabaseBackupInfraIf preset_database_backup_infra_if;
    ::testing::Mock::VerifyAndClearExpectations(db_mock);
    ::testing::InSequence dummy;

    EXPECT_FALSE(preset_database_backup_infra_if.view(visca::NUMBER_OF_PRESET_DB));

    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).WillRepeatedly(Invoke(setInvalidReturnDataForRecall));
    EXPECT_TRUE(preset_database_backup_infra_if.view(DEFAULT_PRESET_ID));

    EXPECT_CALL(*db_mock, get(_, _, _, _, _)).WillRepeatedly(Invoke(setInvalidReturnDataForRecall));
    EXPECT_TRUE(preset_database_backup_infra_if.view(visca::NUMBER_OF_PRESET_DB - 0x1));
}

} // namespace
} // namespace infra
} // namespace preset
