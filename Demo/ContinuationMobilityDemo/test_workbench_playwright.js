const assert = require("node:assert/strict");
const path = require("node:path");
const { pathToFileURL } = require("node:url");
const { chromium } = require("./ThirdParty/playwright/packages/playwright-core");

async function main() {
  const browser = await chromium.launch({ headless: true });
  const page = await browser.newPage({ viewport: { width: 1440, height: 1000 } });
  const consoleErrors = [];
  const pageErrors = [];

  page.on("console", (msg) => {
    if (msg.type() === "error") consoleErrors.push(msg.text());
  });
  page.on("pageerror", (error) => pageErrors.push(error.message));

  const url = pathToFileURL(path.join(__dirname, "ContinuationMobilityDemo.html")).href;
  await page.goto(url);

  await assertVisible(page, "#rho-editor");
  await assertVisible(page, "#pi-editor");
  await assertVisible(page, "#data-stack");
  await assertVisible(page, "#context-stack");
  await assertVisible(page, "#log-output");

  const pi = page.locator("#pi-editor");
  await page.locator("#rho-editor").fill([
    "System.Print(\"SMOKE\")",
    "answer = fun(x)",
    "    return x + 1",
    "answer(41)"
  ].join("\n"));
  await expectText(pi, "emit print(\"SMOKE\")");
  await expectText(pi, "lambda answer(x)");
  await expectText(pi, "call answer(41)");

  await pi.fill("manual pi edit");
  await expectText(page.locator("#pi-mode"), "Manually edited");
  await page.locator("#rho-editor").fill("System.Print(\"REGEN\")");
  await expectText(pi, "emit print(\"REGEN\")");
  await expectText(page.locator("#pi-mode"), "Editable; one-way dependency from Rho");

  await page.locator("#stack-input").fill("42 \"resume\" Agent_7");
  await page.locator("#push-input").click();
  await expectText(page.locator("#stack-count"), "6 values");
  await expectText(page.locator("#data-stack"), "[0] TOS");
  await expectText(page.locator("#data-stack"), "V0");

  await page.locator("#run-pi").click();
  await expectText(page.locator("#data-stack"), "ContinuationResult");
  await expectText(page.locator("#context-stack"), "main");
  await expectText(page.locator("#log-output"), "Ran Pi continuation");

  const missingHoverText = await page.evaluate(() => {
    return [...document.querySelectorAll("main [id], main button, main .panel-title")]
      .filter((node) => !node.getAttribute("title") && !node.getAttribute("data-tech"))
      .map((node) => node.id || node.textContent.trim() || node.tagName);
  });
  assert.deepEqual(missingHoverText, []);
  assert.deepEqual(pageErrors, []);
  assert.deepEqual(consoleErrors, []);

  await browser.close();
  console.log("ContinuationMobilityDemo workbench Playwright smoke test passed");
}

async function assertVisible(page, selector) {
  assert.equal(await page.locator(selector).isVisible(), true, selector + " should be visible");
}

async function expectText(locator, text) {
  await locator.page().waitForFunction(
    ({ selector, expected }) => document.querySelector(selector)?.innerText.includes(expected),
    { selector: await locator.evaluate((node) => node.id ? "#" + node.id : "") || "", expected: text }
  );
}

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
