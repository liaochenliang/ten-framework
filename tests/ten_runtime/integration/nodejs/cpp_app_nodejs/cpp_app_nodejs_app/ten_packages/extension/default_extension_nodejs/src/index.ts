//
// Copyright © 2025 Agora
// This file is part of TEN Framework, an open source project.
// Licensed under the Apache License, Version 2.0, with certain conditions.
// Refer to the "LICENSE" file in the root directory for more information.
//
import {
  Addon,
  RegisterAddonAsExtension,
  Extension,
  TenEnv,
  LogLevel,
  Cmd,
  Data,
  CmdResult,
  StatusCode,
} from "ten-runtime-nodejs";

function assert(condition: boolean, message: string) {
  if (!condition) {
    throw new Error(message);
  }
}

class DefaultExtension extends Extension {
  constructor(name: string) {
    super(name);
  }

  async onConfigure(tenEnv: TenEnv): Promise<void> {
    tenEnv.log(LogLevel.INFO, "onConfigure");
  }

  async onInit(tenEnv: TenEnv): Promise<void> {
    tenEnv.log(LogLevel.INFO, "onInit");
  }

  async onStart(tenEnv: TenEnv): Promise<void> {
    tenEnv.log(LogLevel.INFO, "onStart");

    const testData = Data.Create("testData");
    testData.allocBuf(10);
    const buf = testData.lockBuf();

    const view = new Uint8Array(buf);
    view[0] = 1;
    view[1] = 2;
    view[2] = 3;

    testData.unlockBuf(buf);

    const copiedBuf = testData.getBuf();
    const copiedView = new Uint8Array(copiedBuf);
    assert(copiedView[0] === 1, "copiedView[0] incorrect");
    assert(copiedView[1] === 2, "copiedView[1] incorrect");
    assert(copiedView[2] === 3, "copiedView[2] incorrect");
  }

  async onStop(tenEnv: TenEnv): Promise<void> {
    tenEnv.log(LogLevel.INFO, "onStop");
  }

  async onDeinit(tenEnv: TenEnv): Promise<void> {
    tenEnv.log(LogLevel.INFO, "onDeinit");
  }

  async onCmd(tenEnv: TenEnv, cmd: Cmd): Promise<void> {
    tenEnv.log(LogLevel.INFO, "onCmd");

    const cmdName = cmd.getName();
    tenEnv.log(LogLevel.INFO, "cmdName:" + cmdName);

    const source = cmd.getSource();
    tenEnv.log(LogLevel.INFO, "getSource: appUri:" + source.appUri);
    tenEnv.log(LogLevel.INFO, "getSource: graphId:" + source.graphId);
    tenEnv.log(
      LogLevel.INFO,
      "getSource: extensionName:" + source.extensionName,
    );

    const testCmd = Cmd.Create("test");
    const [result, _] = await tenEnv.sendCmd(testCmd);
    assert(result === undefined, "result is not undefined");

    const cmdResult = CmdResult.Create(StatusCode.OK, cmd);
    cmdResult.setPropertyFromJson(
      "detail",
      JSON.stringify({ key1: "value1", key2: 2 }),
    );

    const [detailJson, err] = cmdResult.getPropertyToJson("detail");
    tenEnv.log(LogLevel.INFO, "detailJson:" + detailJson);

    tenEnv.returnResult(cmdResult);
  }
}

@RegisterAddonAsExtension("default_extension_nodejs")
class DefaultExtensionAddon extends Addon {
  async onCreateInstance(
    _tenEnv: TenEnv,
    instanceName: string,
  ): Promise<Extension> {
    return new DefaultExtension(instanceName);
  }
}
