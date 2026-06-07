(function () {
  const key = "kai.demo.style";
  const styles = [
    { value: "fantasy", label: "Fantasy" },
    { value: "technical", label: "Technical" },
    { value: "plain", label: "Plain" }
  ];

  function selectedStyle() {
    const saved = localStorage.getItem(key);
    return styles.some((style) => style.value === saved) ? saved : "technical";
  }

  function applyStyle(value) {
    document.body.dataset.demoStyle = value;
    localStorage.setItem(key, value);
    document.querySelectorAll("[data-style-select]").forEach((select) => {
      select.value = value;
    });
  }

  function installSelect(nav) {
    const wrap = document.createElement("label");
    wrap.className = "style-control";
    wrap.title = "Choose the visual style for all demo pages";
    wrap.dataset.tech = "demo style selector";

    const label = document.createElement("span");
    label.className = "style-label";
    label.textContent = "Style";
    label.title = "Visual style setting";

    const select = document.createElement("select");
    select.dataset.styleSelect = "true";
    select.setAttribute("aria-label", "Demo style");
    select.title = "Change the demo style";

    styles.forEach((style) => {
      const option = document.createElement("option");
      option.value = style.value;
      option.textContent = style.label;
      select.appendChild(option);
    });

    select.addEventListener("change", () => applyStyle(select.value));
    wrap.append(label, select);
    nav.appendChild(wrap);
  }

  document.addEventListener("DOMContentLoaded", () => {
    document.querySelectorAll(".page-nav").forEach(installSelect);
    applyStyle(selectedStyle());
  });
}());
