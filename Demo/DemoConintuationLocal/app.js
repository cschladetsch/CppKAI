(function () {
  const presets = {
    suspend: {
      label: "Suspend: f(a) returns",
      mode: "suspend",
      rho: `triple = fun(x)
    return x * 3

outer = fun(a)
    before = 1
    value = triple(a)
    return value + before

outer(3)`,
      pi: `// define triple(x)
{ 3 * } 'triple #

// outer(3)
1 'before #
3
triple
&
before
+
Return

// expanded triple body
3
*
Return`
    },
    resume: {
      label: "Resume: f(a)... switches",
      mode: "resume",
      rho: `triple = fun(x)
    return x * 3

outer = fun(a)
    before = 1
    triple(a)...
    return 999

outer(3)`,
      pi: `// define triple(x)
{ 3 * } 'triple #

// outer(3)
1 'before #
3
triple
...
999
Return

// expanded triple body
3
*
Return`
    },
    replace: {
      label: "Replace: f(a)! drops top context",
      mode: "replace",
      rho: `triple = fun(x)
    return x * 3

outer = fun(a)
    before = 1
    triple(a)!
    return 999

outer(3)`,
      pi: `// define triple(x)
{ 3 * } 'triple #

// outer(3)
1 'before #
3
triple
!
999
Return

// expanded triple body
3
*
Return`
    },
    nested: {
      label: "Nested caller: see what survives",
      mode: "replace",
      rho: `triple = fun(x)
    return x * 3

outer = fun(a)
    triple(a)!
    return 999

main = fun()
    return outer(4) + 1

main()`,
      pi: `// define triple(x)
{ 3 * } 'triple #

// define outer(a)
{ triple ! 999 } 'outer #

// main()
4
outer
&
1
+
Return
`
    }
  };

  const docs = {
    suspend: {
      op: "Suspend",
      call: "f(a) / f(a)&",
      final: "10",
      detail: "Pushes a return continuation. The callee returns to the caller's next instruction."
    },
    resume: {
      op: "Resume",
      call: "f(a)...",
      final: "9",
      detail: "Switches to the callee without pushing a new return point. Existing context entries remain."
    },
    replace: {
      op: "Replace",
      call: "f(a)!",
      final: "9",
      detail: "Switches to the callee after dropping the current top context entry."
    }
  };

  const $ = (id) => document.getElementById(id);
  const presetSelect = $("presetSelect");
  const rhoSource = $("rhoSource");
  const piSource = $("piSource");
  const timeline = $("timeline");
  const phaseNote = $("phaseNote");
  const modeMetric = $("modeMetric");
  const stepMetric = $("stepMetric");
  const currentStack = $("currentStack");
  const contextStack = $("contextStack");
  const dataStack = $("dataStack");
  const rhoCursor = $("rhoCursor");
  const piCursor = $("piCursor");
  const rhoCursorLabel = $("rhoCursorLabel");
  const piCursorLabel = $("piCursorLabel");
  const debugOutput = $("debugOutput");
  const errorOutput = $("errorOutput");
  const piProgram = $("piProgram");
  const piStack = $("piStack");
  const stackInput = $("stackInput");
  const stackAppendInput = $("stackAppendInput");

  let mode = "suspend";
  let steps = [];
  let cursor = 0;
  let machine = null;
  let selectedStackIndex = -1;

  function init() {
    Object.entries(presets).forEach(([key, preset]) => {
      const option = document.createElement("option");
      option.value = key;
      option.textContent = preset.label;
      presetSelect.appendChild(option);
    });

    presetSelect.addEventListener("change", () => loadPreset(presetSelect.value));
    rhoSource.addEventListener("input", syncPiFromRho);
    rhoSource.addEventListener("scroll", render);
    piSource.addEventListener("scroll", render);
    document.querySelectorAll("[data-mode]").forEach((button) => {
      button.addEventListener("click", () => {
        setMode(button.dataset.mode);
        writeModeIntoSources();
        reset();
      });
    });
    document.querySelectorAll("[data-tab]").forEach((button) => {
      button.addEventListener("click", () => setTab(button.dataset.tab));
    });
    $("resetBtn").addEventListener("click", reset);
    $("stepBtn").addEventListener("click", step);
    $("runBtn").addEventListener("click", run);
    $("syncBtn").addEventListener("click", () => {
      syncPiFromRho();
      setMode(detectMode());
      reset();
    });
    $("pushStackBtn").addEventListener("click", pushStackValue);
    $("popStackBtn").addEventListener("click", popStackValue);
    $("dupStackBtn").addEventListener("click", dupStackValue);
    $("swapStackBtn").addEventListener("click", swapStackValues);
    $("dropStackBtn").addEventListener("click", dropSelectedStackValue);
    $("clearStackBtn").addEventListener("click", clearStackValues);
    $("stackAppendBtn").addEventListener("click", () => pushStackValue(stackAppendInput));
    stackAppendInput.addEventListener("keydown", (event) => {
      if (event.key === "Enter") pushStackValue(stackAppendInput);
    });

    loadPreset("suspend");
  }

  function loadPreset(name) {
    const preset = presets[name];
    rhoSource.value = preset.rho;
    piSource.value = preset.pi;
    setMode(preset.mode);
    reset();
  }

  function setMode(nextMode) {
    mode = nextMode;
    modeMetric.textContent = docs[mode].op;
    document.querySelectorAll("[data-mode]").forEach((button) => {
      button.classList.toggle("active", button.dataset.mode === mode);
    });
    document.querySelectorAll(".compare-card").forEach((card) => {
      card.classList.toggle("active", card.dataset.kind === mode);
    });
  }

  function setTab(name) {
    document.querySelectorAll("[data-tab]").forEach((button) => {
      button.classList.toggle("active", button.dataset.tab === name);
    });
    document.querySelectorAll(".tab-pane").forEach((pane) => {
      pane.classList.toggle("active", pane.id === `tab-${name}`);
    });
  }

  function detectMode() {
    const source = `${rhoSource.value}\n${piSource.value}`;
    return detectModeFromSource(source);
  }

  function detectModeFromSource(source) {
    if (/\w+\s*\([^)]*\)\s*!|\bReplace\b/i.test(source)) return "replace";
    if (/\w+\s*\([^)]*\)\s*\.\.\.|\bResume\b/i.test(source)) return "resume";
    return "suspend";
  }

  function syncPiFromRho() {
    const nextMode = detectModeFromSource(rhoSource.value);
    setMode(nextMode);
    piSource.value = translateRhoToPi(rhoSource.value, nextMode);
    reset();
  }

  function translateRhoToPi(source, nextMode) {
    const arg = readCallArgument(source);
    const op = { suspend: "&", resume: "...", replace: "!" }[nextMode];
    const usesOuterDefinition = /\bmain\s*=\s*fun|\bouter\s*\(\s*\d+\s*\)\s*\+\s*\d+/i.test(source);

    if (usesOuterDefinition) {
      return `// define triple(x)
{ 3 * } 'triple #

// define outer(a)
{ triple ${op} 999 } 'outer #

// main()
${arg}
outer
&
1
+
Return
`;
    }

    const afterCall = nextMode === "suspend"
      ? `before
+
Return`
      : `999
Return`;

    return `// define triple(x)
{ 3 * } 'triple #

// outer(${arg})
1 'before #
${arg}
triple
${op}
${afterCall}

// expanded triple body
3
*
Return`;
  }

  function readCallArgument(source) {
    const outerCall = source.match(/\bouter\s*\(\s*(-?\d+)\s*\)/);
    if (outerCall) return outerCall[1];
    const tripleCall = source.match(/\btriple\s*\(\s*(-?\d+)\s*\)/);
    if (tripleCall) return tripleCall[1];
    return "3";
  }

  function writeModeIntoSources() {
    const preset = presets[mode];
    rhoSource.value = preset.rho;
    piSource.value = preset.pi;
  }

  function reset() {
    cursor = 0;
    selectedStackIndex = -1;
    steps = buildSteps(mode);
    machine = {
      phase: "ready",
      rhoLine: 1,
      piLine: 1,
      piIp: 0,
      current: ["program", "outer(3)"],
      context: ["program-exit"],
      data: [],
      piStack: [],
      debug: ["loaded local continuation debugger"],
      errors: []
    };
    render();
  }

  function buildSteps(kind) {
    const op = docs[kind].op;
    const operationLine = 8;
    const callLine = kind === "suspend" ? 6 : 6;
    return [
      makeStep("Define triple", "Install a real named Pi continuation: { 3 * } 'triple #.", 1, 2, 1, (m) => {
        m.phase = "define";
        m.current = ["registry", "triple -> { 3 * }"];
        m.piStack = ["triple = { 3 * }"];
        m.debug.push("defined continuation triple");
      }),
      makeStep("Enter outer", "Start the caller continuation for outer(3).", 9, 6, 5, (m) => {
        m.phase = "caller";
        m.current = ["outer(a)", "before = 1", docs[kind].call];
        m.context = ["program-exit", "outer-return-to-main"];
        m.piStack.push("3");
        m.debug.push("call outer with argument 3");
      }),
      makeStep("Store local", "The caller puts a normal local value in its context.", 5, 5, 4, (m) => {
        m.phase = "local";
        m.current = ["outer(a)", "before = 1"];
        m.data.push("before = 1");
        m.piStack = ["before = 1"];
        m.debug.push("store local before = 1");
      }),
      makeStep("Prepare callee", "The argument and named callee continuation are placed on the Pi stack.", callLine, 7, 6, (m) => {
        m.phase = "prepare";
        m.current = ["outer(a)", docs[kind].call];
        m.data.push("a = 3");
        m.piStack.push("a = 3", "triple = { 3 * }");
        m.debug.push("push argument and continuation triple");
      }),
      makeStep(op, docs[kind].detail, callLine, operationLine, 9, (m) => {
        m.phase = op.toLowerCase();
        if (kind === "suspend") {
          m.context.push("outer-after-call");
          m.debug.push("Suspend pushes return point outer-after-call");
        } else if (kind === "resume") {
          m.debug.push("Resume switches without pushing a return point");
        } else {
          const dropped = m.context.pop();
          m.debug.push(`Replace drops top context: ${dropped}`);
        }
        m.current = ["triple(x)", "return x * 3"];
        m.piStack = ["before = 1", "a = 3"];
      }),
      makeStep("Run triple", "The callee consumes x and multiplies it by 3.", 2, 13, 12, (m) => {
        m.phase = "callee";
        m.current = ["triple(x)", "return 9"];
        m.data = m.data.filter((value) => value !== "a = 3");
        m.data.push("9");
        m.piStack = ["9"];
        m.debug.push("triple computes 3 * 3 -> 9");
      }),
      makeStep(kind === "suspend" ? "Return to caller" : "No caller return", kind === "suspend"
        ? "The saved return point is popped and execution continues after the call."
        : "No new return point was installed, so return 999 is not reached automatically.",
        kind === "suspend" ? 7 : 7,
        kind === "suspend" ? 10 : 10,
        kind === "suspend" ? 9 : 9,
        (m) => {
          m.phase = "return";
          if (kind === "suspend") {
            const target = m.context.pop();
            m.current = [target, "Load before", "Add"];
            m.piStack = ["9", "before = 1"];
            m.debug.push(`return resumes ${target}`);
          } else {
            m.current = ["callee complete"];
            m.debug.push(`${op} leaves caller body behind`);
          }
        }),
      makeStep("Final result", `Visible result is ${docs[kind].final}.`, kind === "suspend" ? 7 : 2, kind === "suspend" ? 11 : 15, kind === "suspend" ? 10 : 14, (m) => {
        m.phase = "done";
        m.current = ["program-exit"];
        m.data = [docs[kind].final];
        m.piStack = [docs[kind].final];
        m.debug.push(`done: result ${docs[kind].final}`);
      })
    ];
  }

  function makeStep(title, detail, rhoLine, piLine, piIp, apply) {
    return { title, detail, rhoLine, piLine, piIp, apply };
  }

  function step() {
    if (cursor >= steps.length) return;
    steps[cursor].apply(machine);
    machine.rhoLine = steps[cursor].rhoLine;
    machine.piLine = steps[cursor].piLine;
    machine.piIp = steps[cursor].piIp;
    cursor += 1;
    render();
  }

  function run() {
    while (cursor < steps.length) step();
  }

  function render() {
    stepMetric.textContent = String(cursor);
    phaseNote.textContent = cursor >= steps.length ? "complete" : machine.phase;
    renderTimeline();
    renderStack(currentStack, machine.current, cursor > 0 ? 0 : -1);
    renderStack(contextStack, [...machine.context].reverse(), 0);
    renderStack(dataStack, [...machine.data].reverse(), 0);
    renderPiStack();
    renderOutput(debugOutput, machine.debug);
    renderOutput(errorOutput, machine.errors.length ? machine.errors : ["no errors"]);
    renderPiProgram();
    renderSourceCursor(rhoSource, rhoCursor, rhoCursorLabel, machine.rhoLine);
    renderSourceCursor(piSource, piCursor, piCursorLabel, machine.piLine);
    scrollViewportsToEnd();
  }

  function renderTimeline() {
    timeline.innerHTML = "";
    steps.forEach((item, index) => {
      const li = document.createElement("li");
      li.className = `timeline-item${index === cursor ? " active" : ""}`;
      const body = document.createElement("div");
      const title = document.createElement("div");
      title.className = "timeline-title";
      title.textContent = item.title;
      const detail = document.createElement("div");
      detail.className = "timeline-detail";
      detail.textContent = item.detail;
      body.append(title, detail);
      li.append(body);
      timeline.appendChild(li);
    });
  }

  function renderPiProgram() {
    piProgram.innerHTML = "";
    piSource.value.split("\n").forEach((line, index) => {
      const row = document.createElement("button");
      row.type = "button";
      row.className = `instruction-item${index === machine.piIp ? " active" : ""}`;
      row.addEventListener("click", () => runToInstruction(index));
      const num = document.createElement("span");
      num.className = "instruction-index";
      num.textContent = String(index + 1).padStart(2, "0");
      const text = document.createElement("span");
      text.textContent = line || " ";
      row.append(num, text);
      piProgram.appendChild(row);
    });
    piProgram.scrollTop = piProgram.scrollHeight;
  }

  function runToInstruction(index) {
    while (cursor < steps.length && steps[cursor].piIp <= index) {
      step();
    }
  }

  function renderSourceCursor(sourceEl, cursorEl, labelEl, line) {
    const styles = getComputedStyle(sourceEl);
    const lineHeight = parseFloat(styles.lineHeight);
    const paddingTop = parseFloat(styles.paddingTop);
    const scrollOffset = sourceEl.scrollTop || 0;
    const y = paddingTop + Math.max(0, line - 1) * lineHeight - scrollOffset;
    cursorEl.style.setProperty("--source-line-height", `${lineHeight}px`);
    cursorEl.style.setProperty("--source-pad-x", styles.paddingLeft);
    cursorEl.style.transform = `translateY(${y}px)`;
    labelEl.textContent = `line ${line}`;
    sourceEl.scrollTop = sourceEl.scrollHeight;
  }

  function renderOutput(root, values) {
    root.innerHTML = "";
    values.forEach((value) => {
      const item = document.createElement("div");
      item.className = `output-line${/error|failed|invalid/i.test(value) ? " error" : ""}`;
      item.textContent = value;
      root.appendChild(item);
    });
    root.scrollTop = root.scrollHeight;
  }

  function renderStack(root, values, hotIndex) {
    root.innerHTML = "";
    if (!values.length) {
      const empty = document.createElement("div");
      empty.className = "stack-item muted";
      empty.textContent = "empty";
      root.appendChild(empty);
      return;
    }
    values.forEach((value, index) => {
      const item = document.createElement("div");
      item.className = `stack-item${index === hotIndex ? " hot" : ""}`;
      const label = document.createElement("span");
      label.className = "stack-index";
      label.textContent = "";
      const content = document.createElement("span");
      content.className = "stack-value";
      content.textContent = value;
      item.append(label, content);
      root.appendChild(item);
    });
  }

  function renderPiStack() {
    piStack.innerHTML = "";
    if (!machine.piStack.length) {
      const empty = document.createElement("button");
      empty.type = "button";
      empty.className = "stack-item muted";
      empty.textContent = "empty";
      piStack.appendChild(empty);
      return;
    }

    [...machine.piStack].reverse().forEach((value, displayIndex) => {
      const actualIndex = machine.piStack.length - 1 - displayIndex;
      const item = document.createElement("button");
      item.type = "button";
      item.className = `stack-item${displayIndex === 0 ? " hot" : ""}${actualIndex === selectedStackIndex ? " selected" : ""}`;
      const indexLabel = document.createElement("span");
      indexLabel.className = "stack-index";
      indexLabel.textContent = String(actualIndex);
      const valueLabel = document.createElement("span");
      valueLabel.className = "stack-value";
      valueLabel.textContent = value;
      item.append(indexLabel, valueLabel);
      item.addEventListener("click", () => {
        selectedStackIndex = actualIndex;
        stackInput.value = machine.piStack[actualIndex];
        machine.debug.push(`selected stack[${actualIndex}] = ${machine.piStack[actualIndex]}`);
        render();
      });
      piStack.appendChild(item);
    });
    piStack.scrollTop = 0;
  }

  function scrollViewportsToEnd() {
    timeline.scrollTop = timeline.scrollHeight;
    piProgram.scrollTop = piProgram.scrollHeight;
    debugOutput.scrollTop = debugOutput.scrollHeight;
    errorOutput.scrollTop = errorOutput.scrollHeight;
    rhoSource.scrollTop = rhoSource.scrollHeight;
    piSource.scrollTop = piSource.scrollHeight;
    piStack.scrollTop = 0;
  }

  function pushStackValue(inputEl = stackInput) {
    const value = inputEl.value.trim();
    if (!value) {
      machine.errors.push("cannot push an empty stack value");
      render();
      return;
    }
    machine.piStack.push(value);
    selectedStackIndex = machine.piStack.length - 1;
    machine.debug.push(`manual push ${value}`);
    stackInput.value = value;
    stackAppendInput.value = value;
    render();
  }

  function popStackValue() {
    if (!machine.piStack.length) {
      machine.errors.push("pop failed: Pi stack is empty");
      render();
      return;
    }
    const value = machine.piStack.pop();
    selectedStackIndex = machine.piStack.length - 1;
    machine.debug.push(`manual pop ${value}`);
    render();
  }

  function dupStackValue() {
    if (!machine.piStack.length) {
      machine.errors.push("dup failed: Pi stack is empty");
      render();
      return;
    }
    const value = selectedStackIndex >= 0 ? machine.piStack[selectedStackIndex] : machine.piStack[machine.piStack.length - 1];
    machine.piStack.push(value);
    selectedStackIndex = machine.piStack.length - 1;
    machine.debug.push(`manual dup ${value}`);
    render();
  }

  function swapStackValues() {
    if (machine.piStack.length < 2) {
      machine.errors.push("swap failed: need at least two Pi stack values");
      render();
      return;
    }
    const a = 0;
    const b = 1;
    [machine.piStack[a], machine.piStack[b]] = [machine.piStack[b], machine.piStack[a]];
    selectedStackIndex = Math.min(selectedStackIndex < 0 ? 0 : selectedStackIndex, machine.piStack.length - 1);
    machine.debug.push("manual swap last two visible stack values");
    render();
  }

  function dropSelectedStackValue() {
    if (!machine.piStack.length) {
      machine.errors.push("drop failed: Pi stack is empty");
      render();
      return;
    }
    const index = selectedStackIndex >= 0 ? selectedStackIndex : machine.piStack.length - 1;
    const [value] = machine.piStack.splice(index, 1);
    selectedStackIndex = Math.min(index, machine.piStack.length - 1);
    machine.debug.push(`manual drop ${value}`);
    render();
  }

  function clearStackValues() {
    machine.piStack = [];
    selectedStackIndex = -1;
    machine.debug.push("manual clear Pi stack");
    render();
  }

  init();
}());
