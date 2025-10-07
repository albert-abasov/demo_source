import test from 'node:test';
import assert from 'node:assert/strict';
import { parseStx } from '../src/index.js';

test('parse .sou with environment-qualified class-id', () => {
  const xml = `<?xml version="1.0"?>
  <st-source>
    <class>
      <name>Bar</name>
      <environment>MyEnv</environment>
      <super>Core.Object</super>
      <inst-vars>a b</inst-vars>
      <class-inst-vars></class-inst-vars>
      <category>Pkg</category>
    </class>

    <comment>
      <class-id>MyEnv.Bar</class-id>
      <body>Bar in MyEnv</body>
    </comment>

    <methods>
      <class-id>MyEnv.Bar class</class-id>
      <category>init</category>
      <body selector="new">new
      ^super new</body>
    </methods>

    <methods>
      <class-id>MyEnv.Bar</class-id>
      <category>access</category>
      <body selector="a">a ^a</body>
      <body selector="b">b ^b</body>
    </methods>
  </st-source>`;

  const out = parseStx(xml);
  assert.equal(out.classes.length, 1);
  const bar = out.classes[0];
  assert.equal(bar.name, 'Bar');
  assert.equal(bar.environment, 'MyEnv');
  assert.equal(bar.comment, 'Bar in MyEnv');
  assert.ok(bar.methods.class.find(m => m.name === 'new'));
  assert.ok(bar.methods.instance.find(m => m.name === 'a'));
  assert.ok(bar.methods.instance.find(m => m.name === 'b'));
});
