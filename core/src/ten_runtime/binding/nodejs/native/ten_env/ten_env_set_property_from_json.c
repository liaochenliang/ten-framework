//
// Copyright © 2025 Agora
// This file is part of TEN Framework, an open source project.
// Licensed under the Apache License, Version 2.0, with certain conditions.
// Refer to the "LICENSE" file in the root directory for more information.
//
#include "include_internal/ten_runtime/binding/nodejs/error/error.h"
#include "include_internal/ten_runtime/binding/nodejs/ten_env/ten_env.h"
#include "js_native_api.h"
#include "ten_runtime/common/error_code.h"
#include "ten_utils/lib/json.h"
#include "ten_utils/macro/check.h"
#include "ten_utils/macro/mark.h"
#include "ten_utils/value/value_json.h"

static void tsfn_proxy_set_property_from_json_callback(napi_env env,
                                                       napi_value js_cb,
                                                       TEN_UNUSED void *context,
                                                       void *data) {
  ten_nodejs_set_property_call_ctx_t *ctx =
      (ten_nodejs_set_property_call_ctx_t *)data;
  TEN_ASSERT(ctx, "Should not happen.");

  napi_value js_error = NULL;

  if (ctx->success) {
    js_error = js_undefined(env);
  } else {
    if (ctx->error) {
      js_error = ten_nodejs_error_wrap(env, ctx->error);
      ASSERT_IF_NAPI_FAIL(js_error, "Failed to create JS error", NULL);
    } else {
      ten_error_t err;
      TEN_ERROR_INIT(err);
      ten_error_set(&err, TEN_ERROR_CODE_GENERIC,
                    "Failed to set property value");
      js_error = ten_nodejs_error_wrap(env, &err);
      ASSERT_IF_NAPI_FAIL(js_error, "Failed to create JS error", NULL);
      ten_error_deinit(&err);
    }
  }

  napi_value args[] = {js_error};
  napi_value result = NULL;
  napi_status status =
      napi_call_function(env, js_error, js_cb, 1, args, &result);
  ASSERT_IF_NAPI_FAIL(
      status == napi_ok,
      "Failed to call JS callback of TenEnv::setPropertyFromJson: %d", status);

  ten_nodejs_tsfn_release(ctx->cb_tsfn);

  ten_nodejs_set_property_call_ctx_destroy(ctx);
}

napi_value ten_nodejs_ten_env_set_property_from_json(napi_env env,
                                                     napi_callback_info info) {
  const size_t argc = 4;
  napi_value args[argc];  // ten_env, path, json_str, callback
  if (!ten_nodejs_get_js_func_args(env, info, args, argc)) {
    napi_fatal_error(NULL, NAPI_AUTO_LENGTH,
                     "Incorrect number of parameters passed.",
                     NAPI_AUTO_LENGTH);
    TEN_ASSERT(0, "Should not happen.");
  }

  ten_nodejs_ten_env_t *ten_env_bridge = NULL;
  napi_status status = napi_unwrap(env, args[0], (void **)&ten_env_bridge);
  RETURN_UNDEFINED_IF_NAPI_FAIL(status == napi_ok && ten_env_bridge != NULL,
                                "Failed to get ten_env bridge: %d", status);
  TEN_ASSERT(ten_env_bridge, "Should not happen.");
  TEN_ASSERT(ten_nodejs_ten_env_check_integrity(ten_env_bridge, true),
             "Should not happen.");

  if (ten_env_bridge->c_ten_env_proxy == NULL) {
    ten_error_t err;
    TEN_ERROR_INIT(err);

    ten_error_set(
        &err, TEN_ERROR_CODE_TEN_IS_CLOSED,
        "ten_env.setPropertyFromJson() failed because ten is closed.");

    napi_value js_error = ten_nodejs_error_wrap(env, &err);
    RETURN_UNDEFINED_IF_NAPI_FAIL(js_error, "Failed to create JS error");

    ten_error_deinit(&err);

    return js_error;
  }

  ten_error_t err;
  TEN_ERROR_INIT(err);

  ten_string_t path;
  TEN_STRING_INIT(path);

  bool rc = ten_nodejs_get_str_from_js(env, args[1], &path);
  RETURN_UNDEFINED_IF_NAPI_FAIL(rc, "Failed to get property path", NULL);

  ten_string_t property_value_json_str;
  TEN_STRING_INIT(property_value_json_str);

  rc = ten_nodejs_get_str_from_js(env, args[2], &property_value_json_str);
  RETURN_UNDEFINED_IF_NAPI_FAIL(rc, "Failed to get property value", NULL);

  ten_json_t *json = ten_json_from_string(
      ten_string_get_raw_str(&property_value_json_str), &err);
  if (!json) {
    napi_value js_error = ten_nodejs_error_wrap(env, &err);
    RETURN_UNDEFINED_IF_NAPI_FAIL(js_error, "Failed to create JS error");

    ten_string_deinit(&path);
    ten_string_deinit(&property_value_json_str);
    ten_error_deinit(&err);

    return js_error;
  }

  ten_value_t *value = ten_value_from_json(json);
  TEN_ASSERT(value, "Failed to create value from JSON.");

  ten_json_destroy(json);
  ten_string_deinit(&property_value_json_str);

  ten_nodejs_tsfn_t *cb_tsfn = ten_nodejs_tsfn_create(
      env, "[TSFN] TenEnv::setPropertyFromJson callback", args[3],
      tsfn_proxy_set_property_from_json_callback);
  RETURN_UNDEFINED_IF_NAPI_FAIL(cb_tsfn, "Failed to create TSFN", NULL);

  rc = ten_nodejs_ten_env_set_property_value(
      ten_env_bridge, ten_string_get_raw_str(&path), value, cb_tsfn, &err);
  if (!rc) {
    napi_value js_error = ten_nodejs_error_wrap(env, &err);
    RETURN_UNDEFINED_IF_NAPI_FAIL(js_error, "Failed to create JS error");

    // The JS callback will not be called, so we need to clean up the tsfn.
    ten_nodejs_tsfn_release(cb_tsfn);

    ten_string_deinit(&path);
    ten_error_deinit(&err);

    return js_error;
  }

  ten_string_deinit(&path);
  ten_error_deinit(&err);

  return js_undefined(env);
}
