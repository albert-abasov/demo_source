import { parseStx } from '../src/index.js';

self.onmessage = (e) => {
  const { xml, id } = e.data || {};
  try {
    const t0 = Date.now();
    const result = parseStx(String(xml ?? ''));
    const durationMs = Date.now() - t0;
    self.postMessage({ id, ok: true, result, durationMs });
  } catch (err) {
    self.postMessage({ id, ok: false, error: String(err && err.message || err) });
  }
};
