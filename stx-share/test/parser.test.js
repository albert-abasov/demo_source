import test from 'node:test';
import assert from 'node:assert/strict';
import { parseStx } from '../src/index.js';
import { readFileSync } from 'node:fs';

test('throws on empty input', () => {
  assert.throws(() => parseStx(''), /non-empty string/);
});

test('parse VisualWorks .sou sample (FooPkg)', () => {
  const xml = readFileSync(new URL('../sampleFiles/FooPkg.sou', import.meta.url));
  const out = parseStx(xml.toString());
  const names = out.classes.map(c => c.name).sort();
  assert.deepEqual(names, ['Bar', 'Foo', 'Zip']);
  const bar = out.classes.find(c => c.name === 'Bar');
  assert.ok(bar);
  assert.equal(bar.super, 'Core.Object');
  assert.equal(bar.category, 'FooPkg');
  assert.ok(Array.isArray(bar.instVars));
  assert.ok(bar.instVars.includes('varOne'));
  assert.ok(bar.instVars.includes('varTwo'));
  // methods grouped by side
  assert.ok(bar.methods.class.find(m => m.name === 'new'));
  assert.ok(bar.methods.instance.find(m => m.name === 'aUnaryMethod'));
  assert.ok(bar.methods.instance.find(m => m.name === 'aKeywordMethod:'));
  // original XML children are inlined (without duplicate raw inst-vars fields)
  assert.equal(bar['indexed-type'], 'none'); // unchanged
  assert.equal(bar.private, 'false');
  assert.ok(Array.isArray(bar.instVars) && bar.instVars.length >= 2);
    const zip = out.classes.find(c => c.name === 'Zip');
    assert.ok(zip && zip.environment === 'Lam');
    // attributes restored; package should be under attributes
    assert.ok(bar.attributes);
    assert.equal(bar.attributes.package, 'FooPkg');
    assert.ok(!('package' in bar));
});

test('each class carries attributes and raw fields from XML', () => {
  const xml = readFileSync(new URL('../sampleFiles/FooPkg.sou', import.meta.url));
  const out = parseStx(xml.toString());
  for (const cls of out.classes) {
    // should have attributes subtree when present
    if (cls.name === 'Bar' || cls.name === 'Foo' || cls.name === 'Zip') {
    assert.equal(cls.attributes?.package, 'FooPkg');
    assert.ok(!('package' in cls));
      assert.ok('super' in cls);
      assert.ok('indexed-type' in cls);
  // raw fields removed; we keep parsed arrays instead
  assert.ok(Array.isArray(cls.instVars));
  assert.ok(Array.isArray(cls.classInstVars));
      assert.ok('imports' in cls);
    }
  }
});
