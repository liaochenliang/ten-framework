//
// Copyright © 2025 Agora
// This file is part of TEN Framework, an open source project.
// Licensed under the Apache License, Version 2.0, with certain conditions.
// Refer to the "LICENSE" file in the root directory for more information.
//
#include <nlohmann/json.hpp>
#include <string>

#include "gtest/gtest.h"
#include "include_internal/ten_runtime/binding/cpp/ten.h"
#include "ten_utils/lang/cpp/lib/value.h"
#include "ten_utils/lib/thread.h"
#include "ten_utils/macro/macros.h"
#include "tests/common/client/cpp/msgpack_tcp.h"
#include "tests/ten_runtime/smoke/util/binding/cpp/check.h"

#define PROP_NAME "test_prop"
#define PROP_VAL 12345

namespace {

class test_extension : public ten::extension_t {
 public:
  explicit test_extension(const char *name) : ten::extension_t(name) {}

  void on_cmd(ten::ten_env_t &ten_env,
              std::unique_ptr<ten::cmd_t> cmd) override {
    if (cmd->get_name() == "hello_world") {
      auto int8_prop_value = ten_env.get_property_int8("app:" PROP_NAME);
      EXPECT_EQ(int8_prop_value, 0);

      auto int16_prop_value = ten_env.get_property_int16("app:" PROP_NAME);
      EXPECT_EQ(int16_prop_value, PROP_VAL);

      auto int32_prop_value = ten_env.get_property_int32("app:" PROP_NAME);
      EXPECT_EQ(int32_prop_value, PROP_VAL);

      auto int64_prop_value = ten_env.get_property_int64("app:" PROP_NAME);
      EXPECT_EQ(int64_prop_value, PROP_VAL);

      auto uint8_prop_value = ten_env.get_property_uint8("app:" PROP_NAME);
      EXPECT_EQ(uint8_prop_value, 0);

      auto uint16_prop_value = ten_env.get_property_uint16("app:" PROP_NAME);
      EXPECT_EQ(uint16_prop_value, PROP_VAL);

      auto uint32_prop_value = ten_env.get_property_uint32("app:" PROP_NAME);
      EXPECT_EQ(uint32_prop_value, static_cast<uint32_t>(PROP_VAL));

      auto uint64_prop_value = ten_env.get_property_uint64("app:" PROP_NAME);
      EXPECT_EQ(uint64_prop_value, static_cast<uint64_t>(PROP_VAL));

      auto float32_prop_value = ten_env.get_property_float32("app:" PROP_NAME);
      EXPECT_EQ(float32_prop_value, 0.0);

      auto float64_prop_value = ten_env.get_property_float64("app:" PROP_NAME);
      EXPECT_EQ(float64_prop_value, 0.0);

      auto string_prop_value = ten_env.get_property_string("app:" PROP_NAME);
      EXPECT_EQ(string_prop_value, "");

      auto *ptr_prop_value = ten_env.get_property_ptr("app:" PROP_NAME);
      EXPECT_EQ(ptr_prop_value, nullptr);

      auto bool_prop_value = ten_env.get_property_bool("app:" PROP_NAME);
      EXPECT_EQ(bool_prop_value, false);

      if (int32_prop_value == PROP_VAL) {
        auto cmd_result = ten::cmd_result_t::create(TEN_STATUS_CODE_OK, *cmd);
        cmd_result->set_property("detail", "hello world, too");
        ten_env.return_result(std::move(cmd_result));
      }
    }
  }
};

class test_app : public ten::app_t {
 public:
  void on_configure(ten::ten_env_t &ten_env) override {
    bool rc = ten::ten_env_internal_accessor_t::init_manifest_from_json(
        ten_env,
        // clang-format off
                 "{\
                    \"type\": \"app\",\
                    \"name\": \"test_app\",\
                    \"version\": \"1.0.0\",\
                    \"api\": {\
                      \"property\": {\
                        \"properties\": {\
                          \"" PROP_NAME "\": {\
                            \"type\": \"int32\"\
                          }\
                        }\
                      }\
                    }\
                  }"
        // clang-format on
    );
    ASSERT_EQ(rc, true);

    rc = ten_env.init_property_from_json(
        "{\
                     \"ten\": {\
                     \"uri\": \"msgpack://127.0.0.1:8001/\"},\
                     \"" PROP_NAME "\":" TEN_XSTR(PROP_VAL) "}");
    ASSERT_EQ(rc, true);

    ten_env.on_configure_done();
  }
};

void *test_app_thread_main(TEN_UNUSED void *args) {
  auto *app = new test_app();
  app->run();
  delete app;

  return nullptr;
}

TEN_CPP_REGISTER_ADDON_AS_EXTENSION(property_get_int32__extension,
                                    test_extension);

}  // namespace

TEST(PropertyTest, GetInt32) {  // NOLINT
  // Start app.
  auto *app_thread =
      ten_thread_create("app thread", test_app_thread_main, nullptr);

  // Create a client and connect to the app.
  auto *client = new ten::msgpack_tcp_client_t("msgpack://127.0.0.1:8001/");

  // Send graph.
  auto start_graph_cmd = ten::cmd_start_graph_t::create();
  start_graph_cmd->set_graph_from_json(R"({
           "nodes": [{
               "type": "extension",
               "name": "test_extension",
               "addon": "property_get_int32__extension",
               "app": "msgpack://127.0.0.1:8001/",
               "extension_group": "property_get_int32__extension_group"
             }]
           })");
  auto cmd_result =
      client->send_cmd_and_recv_result(std::move(start_graph_cmd));
  ten_test::check_status_code(cmd_result, TEN_STATUS_CODE_OK);

  // Send a user-defined 'hello world' command.
  auto hello_world_cmd = ten::cmd_t::create("hello_world");
  hello_world_cmd->set_dests(
      {{"msgpack://127.0.0.1:8001/", "", "test_extension"}});
  cmd_result = client->send_cmd_and_recv_result(std::move(hello_world_cmd));
  ten_test::check_status_code(cmd_result, TEN_STATUS_CODE_OK);
  ten_test::check_detail_with_string(cmd_result, "hello world, too");

  delete client;

  ten_thread_join(app_thread, -1);
}
