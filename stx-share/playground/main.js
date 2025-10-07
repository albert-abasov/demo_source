import { parseStx, groupByPackage } from '../src/index.js';
import hljs from 'highlight.js/lib/core';
import xml from 'highlight.js/lib/languages/xml';
import 'highlight.js/styles/github-dark.css';
hljs.registerLanguage('xml', xml);
const worker = new Worker(new URL('./worker.js', import.meta.url), { type: 'module' });
let jobId = 0;
let fullSourceText = '';

const elFile = document.getElementById('file');
const elBtn = document.getElementById('btnParse');
const elSample = document.getElementById('btnLoadSample');
const elSrc = document.getElementById('source');
const elOut = document.getElementById('output');
const elStatus = document.getElementById('status');
const elExpand = document.getElementById('btnExpand');
const elCollapse = document.getElementById('btnCollapse');
const splitRow = document.getElementById('splitRow');
const leftCol = document.getElementById('leftCol');
const rightCol = document.getElementById('rightCol');
const splitter = document.getElementById('splitter');
const elViewMode = document.getElementById('viewMode');
let lastParsed = null;

function setStatus(msg, isError = false) {
  elStatus.textContent = msg || '';
  elStatus.classList.toggle('error', !!isError);
}

async function readFileAsText(file) {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onerror = () => reject(reader.error);
    reader.onload = () => resolve(String(reader.result || ''));
    reader.readAsText(file);
  });
}

function showOutput(obj) {
  elOut.setAttribute('aria-busy', 'true');
  elOut.innerHTML = '';
  let viewObj = obj;
  if (elViewMode && elViewMode.value === 'package' && obj && obj.classes) {
    viewObj = groupByPackage(obj);
  }
  const root = renderTree(viewObj);
  // Auto-expand top-level so differences between Class vs Package are obvious
  if (root && root.tagName === 'DETAILS') root.open = true;
  elOut.appendChild(root);
  elOut.setAttribute('aria-busy', 'false');
}

function renderTree(value, key) {
  const t = typeof value;
  if (value === null || value === undefined) {
    return line(key, span('jt-null', String(value)));
  }
  if (t === 'string') return line(key, span('jt-string', JSON.stringify(value)));
  if (t === 'number') return line(key, span('jt-number', String(value)));
  if (t === 'boolean') return line(key, span('jt-boolean', String(value)));

  if (Array.isArray(value)) {
    const root = details(summaryRow(key, `${value.length} ${value.length === 1 ? 'item' : 'items'} [ ]`));
    const container = document.createElement('div');
    container.className = 'jt-node';
    root.appendChild(container);
    // On-demand chunked rendering to keep UI responsive
    const chunk = 200;
    let i = 0;
    const more = document.createElement('button');
    more.type = 'button';
    more.textContent = `Show ${Math.min(chunk, value.length)} of ${value.length}`;
    more.addEventListener('click', () => {
      const end = Math.min(i + chunk, value.length);
      for (; i < end; i++) {
        container.appendChild(wrapNode(renderTree(value[i], i)));
      }
      if (i < value.length) {
        more.textContent = `Show next ${Math.min(chunk, value.length - i)} (shown ${i}/${value.length})`;
      } else {
        more.remove();
      }
    });
    root.addEventListener('toggle', () => {
      if (root.open && i === 0 && !more.isConnected) container.prepend(more);
    });
    return root;
  }

  // object
  const keys = Object.keys(value);
  const root = details(summaryRow(key, `${keys.length} ${keys.length === 1 ? 'key' : 'keys'} { }`));
  keys.forEach(k => root.appendChild(wrapNode(renderTree(value[k], k))));
  return root;
}

function line(key, valueSpan) {
  const div = document.createElement('div');
  div.className = 'jt-line';
  if (key !== undefined) {
    const k = document.createElement('span');
    k.className = 'jt-key';
    k.textContent = typeof key === 'number' ? `[${key}]` : `${key}:`;
    div.appendChild(k);
    div.appendChild(document.createTextNode(' '));
  }
  div.appendChild(valueSpan);
  return div;
}

function span(cls, text) {
  const s = document.createElement('span');
  s.className = cls;
  s.textContent = text;
  return s;
}

function details(summaryEl) {
  const d = document.createElement('details');
  d.open = false; // default collapsed to avoid heavy initial DOM
  d.appendChild(summaryEl);
  return d;
}

function summaryRow(key, typeText) {
  const s = document.createElement('summary');
  const left = document.createElement('span');
  left.className = 'jt-key';
  left.textContent = key === undefined ? '(root)' : (typeof key === 'number' ? `[${key}]` : key);
  const right = document.createElement('span');
  right.className = 'jt-type';
  right.textContent = ` ${typeText}`;
  s.appendChild(left);
  s.appendChild(right);
  return s;
}

function wrapNode(node) {
  const div = document.createElement('div');
  div.className = 'jt-node';
  div.appendChild(node);
  return div;
}

async function parseCurrentSource() {
  const source = fullSourceText || elSrc.textContent || '';
  if (!source.trim()) {
    setStatus('No source to parse', true);
    return;
  }
  setStatus('Parsing...');
  const id = ++jobId;
  const useWorker = source.length > 500_000;
  if (useWorker) {
    worker.postMessage({ id, xml: source });
    const onMsg = (e) => {
      const msg = e.data || {};
      if (msg.id !== id) return;
      worker.removeEventListener('message', onMsg);
      if (msg.ok) {
        lastParsed = msg.result;
        showOutput(msg.result);
        const mode = (elViewMode && elViewMode.value === 'package') ? 'Package' : 'Class';
        setStatus(`Parsed OK in ${msg.durationMs} ms (View: ${mode})`);
      } else {
        showOutput({ error: msg.error });
        setStatus('Parse error', true);
      }
    };
    worker.addEventListener('message', onMsg);
  } else {
    try {
  const out = parseStx(source);
      lastParsed = out;
      showOutput(out);
  const mode = (elViewMode && elViewMode.value === 'package') ? 'Package' : 'Class';
  setStatus(`Parsed OK (View: ${mode})`);
    } catch (err) {
      showOutput({ error: String(err && err.message || err) });
      setStatus('Parse error', true);
    }
  }
}

elBtn.addEventListener('click', parseCurrentSource);

// Re-render when changing view mode (Class vs Package)
if (elViewMode) {
  elViewMode.addEventListener('change', () => {
    // Reparse the current source so the structure reflects the new mode
    parseCurrentSource();
  });
}

elFile.addEventListener('change', async () => {
  const f = elFile.files && elFile.files[0];
  if (!f) return;
  setStatus(`Reading ${f.name}...`);
  try {
    const text = await readFileAsText(f);
    setSourceText(text);
    await parseCurrentSource();
  } catch (e) {
    setStatus('Failed to read file', true);
  }
});

elSample.addEventListener('click', async () => {
  setStatus('Loading sampleFiles/FooPkg.sou...');
  try {
    const resp = await fetch('/sampleFiles/FooPkg.sou');
    const text = await resp.text();
    setSourceText(text);
    await parseCurrentSource();
  } catch (e) {
    setStatus('Failed to load sample', true);
  }
});

function setSourceText(text) {
  // Use textContent to avoid HTML injection; highlight.js reads text safely
  fullSourceText = String(text || '');
  const MAX_PREVIEW = 300_000; // ~300KB preview to avoid DOM bloat
  const fullLen = fullSourceText.length;
  const preview = fullLen > MAX_PREVIEW
    ? fullSourceText.slice(0, MAX_PREVIEW) + `\n\n/* …truncated preview… (${MAX_PREVIEW}/${fullLen} chars) */`
    : fullSourceText;
  elSrc.textContent = preview;
  // Skip highlighting for very large files to keep UI responsive
  if (elSrc && fullLen < 250_000) {
    hljs.highlightElement(elSrc);
  }
  // no-op for removed top scrollbar
}

elExpand.addEventListener('click', () => {
  elOut.querySelectorAll('details').forEach(d => d.open = true);
});

elCollapse.addEventListener('click', () => {
  elOut.querySelectorAll('details').forEach(d => d.open = false);
});

// ----- Splitter logic -----
let dragging = false;
let startX = 0;
let startLeftWidth = 0;

function setGrid(leftFrac) {
  // clamp
  const lf = Math.min(0.85, Math.max(0.15, leftFrac));
  splitRow.style.gridTemplateColumns = `${lf}fr 6px ${1 - lf}fr`;
  localStorage.setItem('stx.split.leftFrac', String(lf));
}

function getGrid() {
  const v = parseFloat(localStorage.getItem('stx.split.leftFrac'));
  return Number.isFinite(v) ? v : 0.5;
}

function onPointerDown(e) {
  dragging = true;
  startX = e.clientX;
  startLeftWidth = leftCol.getBoundingClientRect().width;
  document.body.style.userSelect = 'none';
}

function onPointerMove(e) {
  if (!dragging) return;
  const dx = e.clientX - startX;
  const total = leftCol.getBoundingClientRect().width + rightCol.getBoundingClientRect().width;
  const newLeft = startLeftWidth + dx;
  setGrid(newLeft / total);
}

function onPointerUp() {
  dragging = false;
  document.body.style.userSelect = '';
}

splitter.addEventListener('mousedown', onPointerDown);
window.addEventListener('mousemove', onPointerMove);
window.addEventListener('mouseup', onPointerUp);

splitter.addEventListener('keydown', (e) => {
  if (e.key === 'ArrowLeft' || e.key === 'ArrowRight') {
    const lf = getGrid() + (e.key === 'ArrowLeft' ? -0.02 : 0.02);
    setGrid(lf);
  }
});

// initialize grid from persisted setting
setGrid(getGrid());

// removed top scrollbar sync code
