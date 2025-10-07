import test from 'node:test';
import assert from 'node:assert/strict';
import { parseStx, groupByPackage } from '../src/index.js';
import { readFileSync } from 'node:fs';

test('groupByPackage groups classes by category', () => {
  const xml = readFileSync(new URL('../sampleFiles/FooPkg.sou', import.meta.url));
  const parsed = parseStx(xml.toString());
  const grouped = groupByPackage(parsed);
  assert.ok(grouped.FooPkg);
  assert.ok(Array.isArray(grouped.FooPkg));
  assert.ok(grouped.FooPkg.find(c => c.name === 'Bar'));
  assert.ok(grouped.FooPkg.find(c => c.name === 'Foo'));
});

test('groupByPackage handles missing category', () => {
  const parsed = { classes: [ { name: 'NoCat', category: null } ] };
  const grouped = groupByPackage(parsed);
  assert.ok(grouped.Uncategorized);
  assert.ok(grouped.Uncategorized.find(c => c.name === 'NoCat'));
});
