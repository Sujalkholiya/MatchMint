import { spawn } from 'child_process';
import fs from 'fs';
import path from 'path';

const SCREENSHOT_DIR = path.resolve('docs/screenshots');
if (!fs.existsSync(SCREENSHOT_DIR)) {
  fs.mkdirSync(SCREENSHOT_DIR, { recursive: true });
}

const CHROME_PATH = 'C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe';
const PORT = 9222;

async function sleep(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

async function main() {
  console.log('[Capture] Launching Chrome on port ' + PORT);
  const chrome = spawn(CHROME_PATH, [
    '--headless=new',
    '--no-sandbox',
    '--disable-gpu',
    `--remote-debugging-port=${PORT}`,
    '--window-size=1440,900',
    'http://localhost:5173/app',
  ]);

  await sleep(2500);

  let versionRes;
  for (let i = 0; i < 10; i++) {
    try {
      const res = await fetch(`http://localhost:${PORT}/json/list`);
      versionRes = await res.json();
      if (versionRes && versionRes.length > 0) break;
    } catch {
      await sleep(1000);
    }
  }

  const page = versionRes.find((p) => p.type === 'page') || versionRes[0];
  const ws = new WebSocket(page.webSocketDebuggerUrl);

  let msgId = 1;
  const pending = new Map();

  ws.onmessage = (event) => {
    const data = JSON.parse(event.data);
    if (data.id && pending.has(data.id)) {
      const { resolve, reject } = pending.get(data.id);
      pending.delete(data.id);
      if (data.error) reject(data.error);
      else resolve(data.result);
    }
  };

  await new Promise((resolve) => (ws.onopen = resolve));

  function send(method, params = {}) {
    const id = msgId++;
    return new Promise((resolve, reject) => {
      pending.set(id, { resolve, reject });
      ws.send(JSON.stringify({ id, method, params }));
    });
  }

  await send('Page.enable');
  await send('Runtime.enable');

  async function takeScreenshot(filename) {
    const res = await send('Page.captureScreenshot', { format: 'png' });
    const buffer = Buffer.from(res.data, 'base64');
    const dest = path.join(SCREENSHOT_DIR, filename);
    fs.writeFileSync(dest, buffer);
    console.log(`[Capture] Saved: ${filename}`);
  }

  async function evalCode(expression) {
    return await send('Runtime.evaluate', { expression, awaitPromise: true });
  }

  await sleep(1500);

  // 1. Competitions tab
  console.log('[Capture] Navigating to Competitions tab...');
  await evalCode(`
    (() => {
      const btns = Array.from(document.querySelectorAll('aside button'));
      const compBtn = btns.find(b => b.textContent && b.textContent.includes('Competitions'));
      if (compBtn) compBtn.click();
    })()
  `);
  await sleep(1500);
  await takeScreenshot('03-competitions.png');

  // 2. Tournament Details / Bracket
  console.log('[Capture] Clicking into GEHU Winter Cup 2026 card...');
  await evalCode(`
    (() => {
      const headings = Array.from(document.querySelectorAll('h3'));
      const gehu = headings.find(h => h.textContent && h.textContent.includes('GEHU Winter Cup'));
      if (gehu) {
        const card = gehu.closest('div.cursor-pointer') || gehu.parentElement;
        card.click();
      }
    })()
  `);
  await sleep(1500);
  await takeScreenshot('04-bracket-tree.png');

  // 3. Standings Sub-tab
  console.log('[Capture] Clicking Standings sub-tab...');
  await evalCode(`
    (() => {
      const btns = Array.from(document.querySelectorAll('main button'));
      const stBtn = btns.find(b => b.textContent && b.textContent.includes('Standings'));
      if (stBtn) stBtn.click();
    })()
  `);
  await sleep(1200);
  await takeScreenshot('05-standings.png');

  // 4. Activity Sub-tab
  console.log('[Capture] Clicking Activity sub-tab...');
  await evalCode(`
    (() => {
      const btns = Array.from(document.querySelectorAll('main button'));
      const actBtn = btns.find(b => b.textContent && b.textContent.includes('Activity'));
      if (actBtn) actBtn.click();
    })()
  `);
  await sleep(1200);
  await takeScreenshot('06-activity-log.png');

  // 5. Matches page
  console.log('[Capture] Navigating to Matches tab...');
  await evalCode(`
    (() => {
      const btns = Array.from(document.querySelectorAll('aside button'));
      const mBtn = btns.find(b => b.textContent && b.textContent.includes('Matches'));
      if (mBtn) mBtn.click();
    })()
  `);
  await sleep(1500);
  await takeScreenshot('07-matches.png');

  // 6. Enter Result Modal
  console.log('[Capture] Clicking ENTER RESULT button...');
  await evalCode(`
    (() => {
      const btns = Array.from(document.querySelectorAll('main button'));
      const resBtn = btns.find(b => b.textContent && b.textContent.includes('ENTER RESULT'));
      if (resBtn) resBtn.click();
    })()
  `);
  await sleep(1200);
  await takeScreenshot('08-score-modal.png');

  // Close modal by clicking outside or pressing Escape
  await evalCode(`
    (() => {
      const closeBtn = document.querySelector('div.fixed button svg')?.parentElement;
      if (closeBtn) closeBtn.click();
    })()
  `);
  await sleep(600);

  // 7. Teams page
  console.log('[Capture] Navigating to Teams tab...');
  await evalCode(`
    (() => {
      const btns = Array.from(document.querySelectorAll('aside button'));
      const tBtn = btns.find(b => b.textContent && b.textContent.includes('Teams'));
      if (tBtn) tBtn.click();
    })()
  `);
  await sleep(1500);
  await takeScreenshot('09-teams.png');

  // 8. Create Tournament Wizard Modal
  console.log('[Capture] Opening Create Tournament Modal...');
  await evalCode(`
    (() => {
      const btns = Array.from(document.querySelectorAll('aside button'));
      const createBtn = btns.find(b => b.textContent && b.textContent.includes('CREATE TOURNAMENT'));
      if (createBtn) createBtn.click();
    })()
  `);
  await sleep(1500);
  await takeScreenshot('11-create-wizard.png');

  // 9. Settings page
  console.log('[Capture] Navigating to Settings tab...');
  await evalCode(`
    (() => {
      const btns = Array.from(document.querySelectorAll('aside button'));
      const sBtn = btns.find(b => b.textContent && b.textContent.includes('Settings'));
      if (sBtn) sBtn.click();
    })()
  `);
  await sleep(1500);
  await takeScreenshot('10-settings.png');

  ws.close();
  chrome.kill();
  console.log('[Capture] All screenshots captured successfully.');
}

main().catch(console.error);
