/* =========================================================================
   RELIEF-Q — reliefq_bridge.js
   -------------------------------------------------------------------------
   WebAssembly Bridge connecting the HTML/CSS frontend to the compiled C
   engine (c_src/*.c -> reliefq.wasm).

   Every single algorithm, priority rule, queue/stack/list operation,
   sorting, and searching is executed INSIDE the compiled C binary.
   ========================================================================= */

class ReliefQBridge {
  constructor() {
    this.instance = null;
    this.exports  = null;
    this.memory   = null;
    this.ready    = false;
  }

  async load() {
    let wasmBytes = null;

    /* 1. Try fetching public/reliefq.wasm or reliefq.wasm over HTTP */
    try {
      const urls = ['public/reliefq.wasm', 'reliefq.wasm', '/public/reliefq.wasm', '/reliefq.wasm'];
      for (const url of urls) {
        try {
          const res = await fetch(url);
          if (res.ok) {
            wasmBytes = await res.arrayBuffer();
            break;
          }
        } catch (_) {}
      }
    } catch (_) {}

    /* 2. Fall back to embedded base64 (allows double-clicking index.html directly via file:///) */
    if (!wasmBytes && typeof RELIEF_Q_WASM_BASE64 !== 'undefined') {
      const binaryString = atob(RELIEF_Q_WASM_BASE64);
      const len = binaryString.length;
      const bytes = new Uint8Array(len);
      for (let i = 0; i < len; i++) {
        bytes[i] = binaryString.charCodeAt(i);
      }
      wasmBytes = bytes.buffer;
    }

    if (!wasmBytes) {
      throw new Error("Unable to load reliefq.wasm binary. Please run via start.bat, start.sh, or a web server.");
    }

    const { instance } = await WebAssembly.instantiate(wasmBytes, {});
    this.instance = instance;
    this.exports  = instance.exports;
    this.memory   = this.exports.memory;
    this.ready    = true;

    /* Initialize the C simulation */
    this.exports.rq_wasm_init();
    return this.getState();
  }

  _passString(str) {
    if (typeof str !== 'string') str = String(str ?? '');
    const encoder = new TextEncoder();
    const encoded = encoder.encode(str);
    const ptr = this.exports.rq_wasm_alloc(encoded.length + 1);
    const memView = new Uint8Array(this.memory.buffer);
    memView.set(encoded, ptr);
    memView[ptr + encoded.length] = 0;
    return ptr;
  }

  _readString(ptr) {
    if (!ptr) return '';
    const memView = new Uint8Array(this.memory.buffer);
    let end = ptr;
    while (memView[end] !== 0) end++;
    const decoder = new TextDecoder('utf-8');
    return decoder.decode(memView.subarray(ptr, end));
  }

  _freeString(ptr) {
    if (ptr) {
      this.exports.rq_wasm_free(ptr);
    }
  }

  getState() {
    const ptr = this.exports.rq_wasm_get_state();
    const jsonStr = this._readString(ptr);
    return JSON.parse(jsonStr);
  }

  submitRequest(id, area, loc, disaster, pop, res, qty, priority) {
    const pId    = this._passString(id);
    const pArea  = this._passString(area);
    const pLoc   = this._passString(loc);
    const pDis   = this._passString(disaster);
    const pRes   = this._passString(res);
    const pPrio  = this._passString(priority);

    const success = this.exports.rq_wasm_submit_request(
      pId, pArea, pLoc, pDis, Number(pop), pRes, Number(qty), pPrio
    );

    this._freeString(pId);
    this._freeString(pArea);
    this._freeString(pLoc);
    this._freeString(pDis);
    this._freeString(pRes);
    this._freeString(pPrio);

    return { success, state: this.getState() };
  }

  classifyNext() {
    const success = this.exports.rq_wasm_classify_next();
    return { success, state: this.getState() };
  }

  classifyAll() {
    const count = this.exports.rq_wasm_classify_all();
    return { count, state: this.getState() };
  }

  allocateNext() {
    const ok = this.exports.rq_wasm_allocate_next();
    return { ok, state: this.getState() };
  }

  retryAllocation(requestID) {
    const pId = this._passString(requestID);
    const ok = this.exports.rq_wasm_retry_allocation(pId);
    this._freeString(pId);
    return { ok, state: this.getState() };
  }

  undoLast() {
    const ok = this.exports.rq_wasm_undo_last();
    return { ok, state: this.getState() };
  }

  addResource(name, quantity) {
    const pName = this._passString(name);
    const ok = this.exports.rq_wasm_add_resource(pName, Number(quantity));
    this._freeString(pName);
    return { ok, state: this.getState() };
  }

  linearSearch(field, keyword) {
    const pField = this._passString(field);
    const pKey   = this._passString(keyword);
    const outPtr = this.exports.rq_wasm_linear_search(pField, pKey);
    const jsonStr = this._readString(outPtr);
    this._freeString(pField);
    this._freeString(pKey);
    return JSON.parse(jsonStr);
  }

  binarySearch(requestID) {
    const pId = this._passString(requestID);
    const outPtr = this.exports.rq_wasm_binary_search(pId);
    const jsonStr = this._readString(outPtr);
    this._freeString(pId);
    return JSON.parse(jsonStr);
  }

  bubbleSort(field, ascending = true) {
    const pField = this._passString(field);
    const outPtr = this.exports.rq_wasm_bubble_sort(pField, ascending ? 1 : 0);
    const jsonStr = this._readString(outPtr);
    this._freeString(pField);
    return JSON.parse(jsonStr);
  }
}

window.reliefQBridge = new ReliefQBridge();
