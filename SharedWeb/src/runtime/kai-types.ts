export interface KaiStackValue {
  readonly index: number;
  readonly value: string;
}

export interface KaiEvalResult {
  readonly output: string;
  readonly stack: readonly KaiStackValue[];
  readonly diagnostics: readonly KaiDiagnostic[];
}

export interface KaiDiagnostic {
  readonly severity: "debug" | "info" | "warning" | "error";
  readonly message: string;
}
