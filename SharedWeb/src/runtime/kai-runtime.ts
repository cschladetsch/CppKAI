import type { KaiEvalResult, KaiStackValue } from "./kai-types";

export interface KaiRuntime {
  evalPi(source: string): Promise<KaiEvalResult>;
  getStack(): Promise<readonly KaiStackValue[]>;
  pushStack(value: string): Promise<void>;
  popStack(): Promise<KaiStackValue | null>;
  clearStack(): Promise<void>;
}
