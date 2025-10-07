# stx

Parse VisualWorks Smalltalk XML-based source code exports (.sou or .st) into structured JavaScript.

- ESM module (importable via `import { parseStx } from 'stx'`)
- Uses fast-xml-parser for robust XML parsing
- Understands `<st-source>` exports, including environments (namespaces) and method sides

## Usage

```js
import { parseStx } from 'stx';
import { readFileSync } from 'node:fs';

const xml = readFileSync('path/to/file.sou', 'utf8'); // or .st when it is XML-based
const result = parseStx(xml);
console.log(result);
```

## API

 - parseStx(xml: string, options?: ParseOptions): StxDocument

- groupByPackage(parsed: StxDocument): { [packageName: string]: StxClass[] }
	- Takes the output of parseStx and returns an object mapping package/category names to arrays of classes.

options.xmlParser is forwarded to fast-xml-parser.

Returned shape (simplified):

```ts
type StxMethod = {
	name: string;
	protocol: string | null;
	kind: 'unary' | 'keyword';
	source: string;
};

type StxClass = {
	name: string;
	environment: string | null;
	category: string | null;
	superclass: string | null;
	comment: string | null;
	instVars: string[];
	classInstVars: string[];
	methods: {
		class: StxMethod[];
		instance: StxMethod[];
	};
};

type StxDocument = { classes: StxClass[] };
```

 Environment matching rules:

- If a `<class-id>` is qualified as `Env.ClassName` (or `Env.ClassName class`), it binds to exactly that environment and class.
- If `<class-id>` is unqualified (just `ClassName`), it resolves to the class in environment `Smalltalk` if present; otherwise to the unique class with that name. Ambiguous cases will pick the first defined class.

## Dev

- Node 18+
- Tests: `npm test`

 Folder layout:

- `.vscode/` - debug launch configuration
- `src/` – library code (ESM)
- `test/` – node:test unit tests
- `playground/` – Vite app: `index.html`, `main.js`, `worker.js`, `vite.config.js`

## Playground

A browser playground is included under `playground/` to test parsing XML-based `.sou` or `.st` files.

Run in dev mode (HMR):

```sh
npm install
npm run dev
```

 This serves the playground at the printed URL (default <http://localhost:5173/>). Use “Load sample FooPkg.sou” or choose your own file.

Build + preview (production):

```sh
npx vite build --config playground/vite.config.js
npm run preview
```

 Debugging in VS Code using Playground:

- Launch configs in `.vscode/launch.json` (Chrome/Edge). Start `npm run dev`, then press F5.

 Large-file behavior:

- XML preview is truncated for very large inputs to keep the UI responsive.
- Parsing uses a Web Worker for large files.
- JSON array nodes render on-demand with a “Show more” button.
