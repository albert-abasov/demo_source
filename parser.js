/**
 * Group parsed classes by package/category.
 * @param {object} parsed - Output from parseStx (or parseVisualWorksSou)
 * @returns {object} { [packageName]: Array<StxClass> }
 */
export function groupByPackage(parsed) {
  if (!parsed || !Array.isArray(parsed.classes)) return {};
  const result = {};
  for (const cls of parsed.classes) {
      const pkg = (cls.attributes && cls.attributes.package)
        || cls.package /* backward compat if older parsed shape is used */
        || 'Uncategorized';
    if (!result[pkg]) result[pkg] = [];
    result[pkg].push(cls);
  }
  return result;
}
import { XMLParser } from 'fast-xml-parser';

/**
 * Parse Smalltalk XML (.st or .sou) content into a structured JS object.
 * This is a starter that extracts classes and methods with metadata.
 *
 * Contract
 * - input: xml string
 * - output: { classes: Array<StxClass> }
 * - throws on invalid XML
 */
export function parseStx(xml, options = {}) {
  if (typeof xml !== 'string' || xml.trim() === '') {
    throw new Error('parseStx: xml must be a non-empty string');
  }

  const parser = new XMLParser({
    ignoreAttributes: false,
    attributeNamePrefix: '',
    allowBooleanAttributes: true,
    trimValues: false,
    preserveOrder: false,
    ...options.xmlParser
  });

  const doc = parser.parse(xml);

  // Support multiple root formats
  if (doc['st-source']) {
    return parseVisualWorksSou(doc['st-source']);
  }

  // Generic fallback format
  const root = doc.stx || doc.smalltalk || doc.root || doc;
  const classesRaw = normalizeArray(root.class || root.classes || root.Class || []);
  const classes = classesRaw.map(c => normalizeGenericClass(c));
  return { classes };
}

function normalizeArray(maybeArray) {
  if (maybeArray == null) return [];
  return Array.isArray(maybeArray) ? maybeArray : [maybeArray];
}

function normalizeGenericClass(node) {
  // Attributes may be on the node directly due to attributeNamePrefix: ''
  const {
    name = node.name || 'Unknown',
    category = node.category || null,
  superclass = node.super || node.superclass || null,
    comment = node.comment || null
  } = node;

  const classSide = extractMethods(node.classSide || node.classMethods);
  const instanceSide = extractMethods(node.instanceSide || node.instanceMethods);

  return {
    name,
    category,
  // use 'super' like the XML tag
  super: superclass,
    comment,
    instVars: [],
    classInstVars: [],
    methods: {
      class: classSide,
      instance: instanceSide
    }
  };
}

function extractMethods(container) {
  const methodsRaw = normalizeArray(container?.method || container?.methods || container);
  return methodsRaw.map(m => normalizeMethod(m));
}

function normalizeMethod(node) {
  if (!node) return null;
  const {
    name = node.name || 'unknown:',
    protocol = node.protocol || node.category || null,
    source = node.source || node.code || node.body || '',
    selector = node.selector || name,
    kind = node.kind || (selector.includes(':') ? 'keyword' : 'unary')
  } = node;

  return {
    name: selector,
    protocol,
    kind,
    source: coerceString(source)
  };
}

function coerceString(v) {
  if (typeof v === 'string') return v;
  if (v == null) return '';
  // If XML parser nested content as object, try common nodes
  if (typeof v === 'object') {
    if (typeof v['#text'] === 'string') return v['#text'];
    if (Array.isArray(v['#text'])) return v['#text'].join('');
  }
  return String(v);
}

// ===== VisualWorks .sou (st-source) support =====
function parseVisualWorksSou(root) {
  const DEFAULT_ENV = 'Smalltalk';
  const classNodes = normalizeArray(root.class);
  const commentNodes = normalizeArray(root.comment);
  const methodsNodes = normalizeArray(root.methods);

  // Map comments by class-id -> comment body string
  const commentsByClass = new Map();
  for (const cmt of commentNodes) {
    const classId = coerceString(cmt['class-id']);
    const { environment, className } = parseClassId(classId);
    const key = fqcn(environment, className);
    const body = coerceString(cmt.body);
    if (key) {
      commentsByClass.set(key, body);
      if (className && !commentsByClass.has(className)) {
        commentsByClass.set(className, body);
      }
    }
  }

  // Build class map
  const classesByKey = new Map();
  for (const n of classNodes) {
    let name = coerceString(n.name) || 'Unknown';
    let environment = coerceString(n.environment) || null;
    if (!environment && name.includes('.')) {
      const lastDot = name.lastIndexOf('.');
      environment = name.slice(0, lastDot) || null;
      name = name.slice(lastDot + 1);
    }
    const superclass = coerceString(n.super) || null;
    const category = coerceString(n.category) || null;
    const isPrivate = coerceString(n.private) || null;
    const indexedType = coerceString(n['indexed-type']) || null;
    const imports = coerceString(n.imports) || null;
    // Extract <attributes><package> value and preserve attributes subtree
    let pkg = null;
    if (n.attributes && n.attributes.package) {
      pkg = coerceString(n.attributes.package);
    }
    const attributes = n.attributes ? stripHashText(JSON.parse(JSON.stringify(n.attributes))) : null;
    const instVars = splitVars(coerceString(n['inst-vars']));
    const classInstVars = splitVars(coerceString(n['class-inst-vars']));
    const key = fqcn(environment, name);
    const comment = commentsByClass.get(key) || commentsByClass.get(name) || null;

    classesByKey.set(key, {
      name,
      environment,
      // no top-level package; use attributes.package instead
      category,
      // original XML children as top-level fields
      super: superclass,
      private: isPrivate,
      'indexed-type': indexedType,
      imports,
      attributes,
      comment,
      instVars,
      classInstVars,
      methods: { class: [], instance: [] }
    });
  }

  // Methods blocks. Each block has <class-id> like "Foo" or "Foo class" and <category>
  for (const mblock of methodsNodes) {
    const classId = coerceString(mblock['class-id']);
    const protocol = coerceString(mblock.category) || null;
    const bodies = normalizeArray(mblock.body);
    const { environment, className, side } = parseClassId(classId);
    let key = environment ? fqcn(environment, className) : resolveClassKey(classesByKey, className, DEFAULT_ENV);
    let cls = classesByKey.get(key);
    if (!cls) {
      classesByKey.set(key, {
        name: className,
        environment: environment || DEFAULT_ENV || null,
        category: null,
        // original XML children defaults when class node missing
        super: null,
        private: null,
        'indexed-type': null,
        imports: null,
        attributes: null,
        comment: null,
        instVars: [],
        classInstVars: [],
        methods: { class: [], instance: [] }
      });
    }
    const target = classesByKey.get(key).methods[side];
    for (const b of bodies) {
      if (!b) continue;
      const selector = b.selector || coerceSelectorFromBody(b);
      const source = coerceString(b);
      const kind = selector && selector.includes(':') ? 'keyword' : 'unary';
      // Extract package attribute from <body>
      let methodPkg = null;
      if (b.package) methodPkg = coerceString(b.package);
  target.push({
        name: selector,
        protocol,
        kind,
        source,
        package: methodPkg
      });
    }
  }

  return { classes: Array.from(classesByKey.values()) };
}

function stripHashText(obj) {
  if (!obj || typeof obj !== 'object') return obj;
  if (Object.prototype.hasOwnProperty.call(obj, '#text')) {
    try { delete obj['#text']; } catch {}
  }
  for (const k of Object.keys(obj)) {
    const v = obj[k];
    if (v && typeof v === 'object') obj[k] = stripHashText(v);
  }
  return obj;
}

function splitVars(s) {
  if (!s) return [];
  // tokens separated by whitespace; ignore empties
  return s
    .split(/\s+/)
    .map(v => v.trim())
    .filter(Boolean);
}

function parseClassId(classId) {
  const s = (classId || '').trim();
  if (!s) return { environment: null, className: 'Unknown', side: 'instance' };
  const isClassSide = /\s+class$/.test(s);
  const base = s.replace(/\s+class$/, '');
  const lastDot = base.lastIndexOf('.');
  const environment = lastDot >= 0 ? base.slice(0, lastDot) : null;
  const className = lastDot >= 0 ? base.slice(lastDot + 1) : base;
  return { environment, className, side: isClassSide ? 'class' : 'instance' };
}

function fqcn(environment, className) {
  const name = (className || '').trim();
  const env = environment && String(environment).trim();
  return env ? `${env}.${name}` : name;
}

// Resolve an unqualified class name to a fully qualified key in the map.
// Preference order:
// 1) Environment === DEFAULT_ENV if present
// 2) Only match -> that one
// 3) Multiple matches -> first (deterministic by current Map insertion order)
// 4) No match -> fqcn(null, name) (placeholder will be created)
function resolveClassKey(classesByKey, className, DEFAULT_ENV) {
  const entries = Array.from(classesByKey.entries()).filter(([k, v]) => v.name === className);
  if (entries.length === 0) return fqcn(null, className);
  const preferred = entries.find(([k, v]) => (v.environment || null) === DEFAULT_ENV);
  if (preferred) return preferred[0];
  if (entries.length === 1) return entries[0][0];
  return entries[0][0];
}

function coerceSelectorFromBody(bodyNode) {
  // When selector attribute is missing, try to read the first token/line of body text
  const text = coerceString(bodyNode) || '';
  const firstLine = text.split(/\r?\n/)[0] || '';
  // Selector is usually the first token (could include colons)
  return firstLine.trim().split(/\s+/)[0] || 'unknown:';
}
