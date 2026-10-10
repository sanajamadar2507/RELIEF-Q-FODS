const fs = require('fs');
const path = require('path');

const wasmPath = path.join(__dirname, '..', 'public', 'reliefq.wasm');
const outPath = path.join(__dirname, '..', 'js', 'reliefq_wasm_bytes.js');

const bytes = fs.readFileSync(wasmPath);
const b64 = bytes.toString('base64');
const content = `const RELIEF_Q_WASM_BASE64 = "${b64}";\n`;

fs.writeFileSync(outPath, content, 'utf8');
console.log(`Generated ${outPath} successfully (${bytes.length} bytes, ${b64.length} base64 chars)`);
