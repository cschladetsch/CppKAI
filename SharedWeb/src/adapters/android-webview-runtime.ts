import type { KaiRuntime } from "../runtime/kai-runtime";
import type { KaiEvalResult, KaiStackValue } from "../runtime/kai-types";

interface KaiAndroidBridge {
  evalPi(source: string): string;
  getStack(): string;
  pushStack(value: string): void;
  popStack(): string;
  clearStack(): void;
}

export class AndroidWebViewKaiRuntime implements KaiRuntime {
  constructor(private readonly bridge: KaiAndroidBridge) {}

  async evalPi(source: string): Promise<KaiEvalResult> {
    return JSON.parse(this.bridge.evalPi(source)) as KaiEvalResult;
  }

  async getStack(): Promise<readonly KaiStackValue[]> {
    return JSON.parse(this.bridge.getStack()) as KaiStackValue[];
  }

  async pushStack(value: string): Promise<void> {
    this.bridge.pushStack(value);
  }

  async popStack(): Promise<KaiStackValue | null> {
    return JSON.parse(this.bridge.popStack()) as KaiStackValue | null;
  }

  async clearStack(): Promise<void> {
    this.bridge.clearStack();
  }
}
