import test from 'node:test';
import assert from 'node:assert/strict';
import { parseStx } from '../src/index.js';

// When environment is Smalltalk in class, subsequent class-ids may be unqualified
// and should resolve to that class by name.

test('unqualified class-id resolves to Smalltalk environment class', () => {
  const xml = `<?xml version="1.0"?>
  <st-source>
    <class>
      <name>Greeter</name>
      <environment>Smalltalk</environment>
      <super>Core.Object</super>
      <inst-vars></inst-vars>
      <class-inst-vars></class-inst-vars>
      <category>Pkg</category>
    </class>

    <methods>
      <class-id>Greeter</class-id>
      <category>run</category>
      <body selector="hello">hello ^'hi'</body>
    </methods>
  </st-source>`;

  const out = parseStx(xml);
  const g = out.classes.find(c => c.name === 'Greeter');
  assert.ok(g);
  assert.equal(g.environment, 'Smalltalk');
  assert.ok(g.methods.instance.find(m => m.name === 'hello'));
});
