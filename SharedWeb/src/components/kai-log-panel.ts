export class KaiLogPanelElement extends HTMLElement {}

if (!customElements.get("kai-log-panel")) {
  customElements.define("kai-log-panel", KaiLogPanelElement);
}
