import type { KaiRuntime } from "../runtime/kai-runtime";

export class KaiPiStackElement extends HTMLElement {
  runtime: KaiRuntime | null = null;
}

if (!customElements.get("kai-pi-stack")) {
  customElements.define("kai-pi-stack", KaiPiStackElement);
}
