import type { KaiRuntime } from "../runtime/kai-runtime";
import type { KaiEvalResult, KaiStackValue } from "../runtime/kai-types";

export class BrowserDemoKaiRuntime implements KaiRuntime {
  private readonly stack: KaiStackValue[] = [];

  async evalPi(source: string): Promise<KaiEvalResult> {
    return {
      output: source,
      stack: this.stack,
      diagnostics: []
    };
  }

  async getStack(): Promise<readonly KaiStackValue[]> {
    return this.stack;
  }

  async pushStack(value: string): Promise<void> {
    this.stack.unshift({ index: 0, value });
    this.reindexStack();
  }

  async popStack(): Promise<KaiStackValue | null> {
    const value = this.stack.shift() ?? null;
    this.reindexStack();
    return value;
  }

  async clearStack(): Promise<void> {
    this.stack.length = 0;
  }

  private reindexStack(): void {
    this.stack.forEach((entry, index) => {
      this.stack[index] = { ...entry, index };
    });
  }
}
