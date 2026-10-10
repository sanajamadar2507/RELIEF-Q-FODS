/* =========================================================================
   RELIEF-Q — app.js
   -------------------------------------------------------------------------
   Frontend UI controller for RELIEF-Q.
   Wires the user interface to the compiled C engine via reliefq_bridge.js.

   ALL data structures (Array, Struct, Singly Linked List, FIFO Queue,
   4-Lane Priority Queue, LIFO Stack), priority allocation algorithms,
   Bubble Sort, Linear Search, and Binary Search run in compiled C.
   ========================================================================= */

let currentSystemState = null;

/* ---------------------------------------------------------------------------
   HELPERS & FORMATTING
   --------------------------------------------------------------------------- */
function badge(text, cls) {
  return `<span class="badge badge-${cls}">${text}</span>`;
}

function statusBadge(status) {
  return `<span class="badge badge-status-${status.replace(/\s+/g, "")}">${status}</span>`;
}

function fmt(n) {
  return Number(n || 0).toLocaleString();
}

function timeStr(d) {
  if (!d) {
    const now = new Date();
    return now.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit", second: "2-digit" });
  }
  return d;
}

/* ---------------------------------------------------------------------------
   RENDERING FUNCTIONS (Driven by state produced by C)
   --------------------------------------------------------------------------- */
function renderDashboard(state) {
  const { stats, priorityCounts, inventory, liveFeed } = state;

  document.getElementById("stat-total").textContent          = fmt(stats.total);
  document.getElementById("stat-pending").textContent        = fmt(stats.pending);
  document.getElementById("stat-critical").textContent       = fmt(stats.critical);
  document.getElementById("stat-completed").textContent      = fmt(stats.completed);
  document.getElementById("stat-res-available").textContent  = fmt(stats.resAvailable);
  document.getElementById("stat-res-distributed").textContent= fmt(stats.resDistributed);

  /* Priority distribution bars */
  const levels = ["Critical", "High", "Medium", "Low"];
  const colors = {
    Critical: "var(--critical)",
    High:     "var(--high)",
    Medium:   "var(--medium)",
    Low:      "var(--low)"
  };

  const pBars = levels.map(lv => {
    const count = priorityCounts[lv] || 0;
    const pct = stats.total ? Math.round((count / stats.total) * 100) : 0;
    return `<div class="bar-row">
      <div class="bar-row-top"><span>${lv}</span><span>${count} request(s)</span></div>
      <div class="bar-track"><div class="bar-fill" style="width:${pct}%;background:${colors[lv]}"></div></div>
    </div>`;
  }).join("");
  document.getElementById("priorityBars").innerHTML = pBars;

  /* Inventory bars */
  const invBars = inventory.map(r => {
    const pct = r.total ? Math.round((r.available / r.total) * 100) : 0;
    return `<div class="bar-row">
      <div class="bar-row-top"><span>${r.name}</span><span>${fmt(r.available)} / ${fmt(r.total)} available</span></div>
      <div class="bar-track"><div class="bar-fill" style="width:${pct}%;background:var(--low)"></div></div>
    </div>`;
  }).join("");
  document.getElementById("dashInventory").innerHTML = invBars || `<p class="muted">No resources yet.</p>`;

  /* Live feed */
  const feedItems = liveFeed.slice(-25).map(e =>
    `<li><span class="lf-time">${timeStr(e.time)}</span><span>${e.text}</span></li>`
  ).join("");
  document.getElementById("liveFeed").innerHTML = feedItems || `<li class="muted">No activity yet.</li>`;
}

function renderRequestsPage(state) {
  const { waitingQueue, waitingQueueCount, allRequests } = state;

  document.getElementById("queueCountPill").textContent = waitingQueueCount;

  const qRows = waitingQueue.map((r, i) => `
    <tr>
      <td>${i + 1}</td>
      <td class="mono">${r.requestID}</td>
      <td>${r.area}</td>
      <td>${badge(r.priority, r.priority)}</td>
      <td>${r.resource}</td>
      <td>${fmt(r.quantity)}</td>
    </tr>`).join("");
  document.querySelector("#queueTable tbody").innerHTML =
    qRows || `<tr><td colspan="6" class="muted">Queue is empty — every request has been classified.</td></tr>`;

  const allRows = allRequests.map(r => `
    <tr>
      <td class="mono">${r.requestID}</td>
      <td>${r.area}</td>
      <td>${r.location}</td>
      <td>${r.disasterType}</td>
      <td>${fmt(r.population)}</td>
      <td>${r.resource}</td>
      <td>${fmt(r.quantity)}</td>
      <td>${fmt(r.allocated)}</td>
      <td>${badge(r.priority, r.priority)}</td>
      <td>${statusBadge(r.status)}</td>
    </tr>`).join("");
  document.querySelector("#allRequestsTable tbody").innerHTML =
    allRows || `<tr><td colspan="10" class="muted">No requests yet.</td></tr>`;
}

function renderInventory(state) {
  const { inventory } = state;

  const rows = inventory.map(r => {
    const pct = r.total ? Math.round((r.available / r.total) * 100) : 0;
    return `<tr>
      <td>${r.name}</td>
      <td>${fmt(r.total)}</td>
      <td>${fmt(r.available)}</td>
      <td>${fmt(r.distributed)}</td>
      <td style="min-width:140px">
        <div class="bar-track"><div class="bar-fill" style="width:${pct}%;background:var(--medium)"></div></div>
      </td>
    </tr>`;
  }).join("");
  document.querySelector("#inventoryTable tbody").innerHTML =
    rows || `<tr><td colspan="5" class="muted">No resources yet.</td></tr>`;
}

function renderPQueue(state) {
  const { priorityQueue } = state;
  const levels = ["Critical", "High", "Medium", "Low"];

  const html = levels.map(lv => {
    const items = priorityQueue[lv] || [];
    const cards = items.map(r => `
      <div class="pq-card">
        <b>${r.requestID}</b> — ${r.area}<br>
        ${r.resource} · qty ${fmt(r.quantity)}
      </div>`).join("");
    return `<div class="pq-col">
      <div class="pq-col-head ${lv}">${lv} (${items.length})</div>
      <div class="pq-col-body">${cards || '<div class="pq-empty">— empty —</div>'}</div>
    </div>`;
  }).join("");
  document.getElementById("pqLevels").innerHTML = html;
}

function renderAllocationPage(state) {
  const { waitingForResources } = state;

  const rows = waitingForResources.map(r => `
    <tr>
      <td class="mono">${r.requestID}</td>
      <td>${r.area}</td>
      <td>${r.resource}</td>
      <td>${fmt(r.quantity - r.allocated)}</td>
      <td>${badge(r.priority, r.priority)}</td>
      <td><button class="btn btn-secondary retry-btn" data-id="${r.requestID}">Retry</button></td>
    </tr>`).join("");
  document.querySelector("#waitingTable tbody").innerHTML =
    rows || `<tr><td colspan="6" class="muted">No requests waiting on resources.</td></tr>`;

  document.querySelectorAll(".retry-btn").forEach(btn => {
    btn.addEventListener("click", () => {
      const id = btn.dataset.id;
      const { state: newState } = reliefQBridge.retryAllocation(id);
      currentSystemState = newState;
      showAllocateResult(newState.lastAllocResult);
      renderAll(newState);
    });
  });
}

function showAllocateResult(result) {
  const box = document.getElementById("allocateResult");
  if (!result || result.type === "empty") {
    box.innerHTML = `<div class="result-box">Priority Queue is empty — classify more requests from the Queue first.</div>`;
    return;
  }
  const r = result.req;
  const remaining = r.quantity - r.allocated;

  if (result.type === "wait") {
    box.innerHTML = `<div class="result-box wait">
      <b>${r.requestID}</b> (${r.area}) needs ${fmt(r.quantity)} ${r.resource}, but 0 units are available.<br>
      Status: ${statusBadge(r.status)}
    </div>`;
  } else if (result.type === "partial") {
    box.innerHTML = `<div class="result-box partial">
      <b>${r.requestID}</b> (${r.area}) — Partial Allocation<br>
      Required: ${fmt(r.quantity)} &nbsp;|&nbsp; Allocated: ${fmt(r.allocated)} &nbsp;|&nbsp; Remaining: ${fmt(remaining)}<br>
      Status: ${statusBadge(r.status)}
    </div>`;
  } else {
    box.innerHTML = `<div class="result-box ok">
      <b>${r.requestID}</b> (${r.area}) fully allocated: ${fmt(result.given)} ${r.resource}.<br>
      Status: ${statusBadge(r.status)}
    </div>`;
  }
}

function renderHistory(state) {
  const { historyStack } = state;

  const rows = historyStack.map(rec => `
    <tr>
      <td class="mono">${rec.allocID}</td>
      <td class="mono">${rec.requestID}</td>
      <td>${rec.resource}</td>
      <td>${fmt(rec.quantity)}</td>
      <td>${rec.location}</td>
      <td>${statusBadge(rec.status)}</td>
      <td class="mono">${timeStr(rec.time)}</td>
    </tr>`).join("");
  document.querySelector("#historyTable tbody").innerHTML =
    rows || `<tr><td colspan="7" class="muted">No allocations yet.</td></tr>`;
}

function renderReports(state) {
  const { stats, reports, priorityCounts, completedList } = state;
  const total = stats.total;

  document.getElementById("reportSummary").innerHTML = `
    <li><span>Total resources distributed</span><b>${fmt(stats.resDistributed)}</b></li>
    <li><span>Most requested resource</span><b>${reports.mostRequestedResource}</b></li>
    <li><span>Critical requests</span><b>${fmt(stats.critical)}</b></li>
    <li><span>Completed requests</span><b>${fmt(stats.completed)}</b></li>
    <li><span>Pending requests</span><b>${fmt(stats.pending)}</b></li>
    <li><span>Total requests logged</span><b>${fmt(total)}</b></li>
  `;

  const levels = ["Critical", "High", "Medium", "Low"];
  const colors = {
    Critical: "var(--critical)",
    High:     "var(--high)",
    Medium:   "var(--medium)",
    Low:      "var(--low)"
  };

  document.getElementById("reportPriority").innerHTML = levels.map(lv => {
    const count = priorityCounts[lv] || 0;
    const pct = total ? Math.round((count / total) * 100) : 0;
    return `<div class="bar-row">
      <div class="bar-row-top"><span>${lv}</span><span>${count}</span></div>
      <div class="bar-track"><div class="bar-fill" style="width:${pct}%;background:${colors[lv]}"></div></div>
    </div>`;
  }).join("");

  const disasterTypes = ["Flood", "Earthquake", "Cyclone", "Landslide", "Drought", "Fire"];
  document.getElementById("reportDisaster").innerHTML = disasterTypes.map(dt => {
    const count = reports.disasterCounts[dt] || 0;
    const pct = total ? Math.round((count / total) * 100) : 0;
    return `<div class="bar-row">
      <div class="bar-row-top"><span>${dt}</span><span>${count}</span></div>
      <div class="bar-track"><div class="bar-fill" style="width:${pct}%;background:var(--navy-soft)"></div></div>
    </div>`;
  }).join("");

  const completedRows = completedList.map(r => `
    <tr>
      <td class="mono">${r.requestID}</td>
      <td>${r.area}</td>
      <td>${r.resource}</td>
      <td>${fmt(r.allocated)}</td>
      <td>${statusBadge(r.status)}</td>
    </tr>`).join("");
  document.querySelector("#completedTable tbody").innerHTML =
    completedRows || `<tr><td colspan="5" class="muted">No completed requests yet.</td></tr>`;
}

function renderAll(state) {
  if (!state) return;
  renderDashboard(state);
  renderRequestsPage(state);
  renderInventory(state);
  renderPQueue(state);
  renderAllocationPage(state);
  renderHistory(state);
  renderReports(state);
}

/* ---------------------------------------------------------------------------
   VALIDATION
   --------------------------------------------------------------------------- */
function validateRequestForm() {
  const id         = document.getElementById("f-id").value.trim();
  const area       = document.getElementById("f-area").value.trim();
  const location   = document.getElementById("f-location").value.trim();
  const disaster   = document.getElementById("f-disaster").value;
  const population = document.getElementById("f-population").value;
  const resource   = document.getElementById("f-resource").value;
  const quantity   = document.getElementById("f-quantity").value;
  const priority   = document.getElementById("f-priority").value;

  if (!id) return "Request ID cannot be empty.";
  if (currentSystemState && currentSystemState.allRequests.some(r => r.requestID.toLowerCase() === id.toLowerCase())) {
    return "Request ID already exists.";
  }
  if (!area) return "Disaster Area cannot be empty.";
  if (!location) return "Location cannot be empty.";
  if (!disaster) return "Please select a Disaster Type.";
  if (!population || Number(population) <= 0) return "Population must be greater than 0.";
  if (!resource) return "Please select a Resource.";
  if (!quantity || Number(quantity) <= 0) return "Quantity must be greater than 0.";
  if (!priority) return "Please select a Priority.";

  return null;
}

/* ---------------------------------------------------------------------------
   NAVIGATION
   --------------------------------------------------------------------------- */
const pageTitles = {
  dashboard:  ["Dashboard", "Live overview of the current relief situation"],
  requests:   ["Relief Requests", "Submit new requests and track the waiting Queue"],
  inventory:  ["Resource Inventory", "Array-based store of all available relief resources"],
  pqueue:     ["Priority Queue", "Critical requests are always served first"],
  allocation: ["Allocate Relief Resources", "Match the highest-priority request against available stock"],
  history:    ["Allocation History", "Stack (LIFO) of every allocation performed"],
  search:     ["Search Requests", "Linear Search and Binary Search demonstrations (C11 Core)"],
  sort:       ["Sort Requests", "Bubble Sort demonstration (C11 Core)"],
  reports:    ["Reports", "Aggregate statistics across the whole simulation"],
  about:      ["FDS Map & Docs", "Where every fundamental data structure is used in this C project"]
};

function goToPage(name) {
  document.querySelectorAll(".page").forEach(p => p.classList.remove("active"));
  document.getElementById("page-" + name).classList.add("active");
  document.querySelectorAll(".nav-item").forEach(b => b.classList.toggle("active", b.dataset.page === name));
  document.getElementById("pageTitle").textContent = pageTitles[name][0];
  document.getElementById("pageSubtitle").textContent = pageTitles[name][1];
}

document.querySelectorAll(".nav-item").forEach(btn => {
  btn.addEventListener("click", () => goToPage(btn.dataset.page));
});

/* ---------------------------------------------------------------------------
   EVENT WIRING (Calls C engine via WebAssembly bridge)
   --------------------------------------------------------------------------- */

// Submit relief request -> C Queue
document.getElementById("requestForm").addEventListener("submit", e => {
  e.preventDefault();
  const err = validateRequestForm();
  const errBox = document.getElementById("formError");
  if (err) { errBox.textContent = err; return; }
  errBox.textContent = "";

  const id         = document.getElementById("f-id").value.trim();
  const area       = document.getElementById("f-area").value.trim();
  const location   = document.getElementById("f-location").value.trim();
  const disaster   = document.getElementById("f-disaster").value;
  const population = document.getElementById("f-population").value;
  const resource   = document.getElementById("f-resource").value;
  const quantity   = document.getElementById("f-quantity").value;
  const priority   = document.getElementById("f-priority").value;

  const { state } = reliefQBridge.submitRequest(
    id, area, location, disaster, population, resource, quantity, priority
  );
  currentSystemState = state;
  e.target.reset();
  renderAll(state);
});

// Classify next request: C Queue -> C Priority Queue
document.getElementById("classifyNextBtn").addEventListener("click", () => {
  const { success, state } = reliefQBridge.classifyNext();
  currentSystemState = state;
  if (!success) alert("The waiting Queue is empty.");
  renderAll(state);
});

// Add resource -> C ResourceArray
document.getElementById("resourceForm").addEventListener("submit", e => {
  e.preventDefault();
  const name = document.getElementById("r-name").value.trim();
  const qty = Number(document.getElementById("r-qty").value);
  if (!name || qty <= 0) return;

  const { state } = reliefQBridge.addResource(name, qty);
  currentSystemState = state;
  e.target.reset();
  renderAll(state);
});

// Allocate next request in C Priority Queue
function handleAllocateNext() {
  const { state } = reliefQBridge.allocateNext();
  currentSystemState = state;
  showAllocateResult(state.lastAllocResult);
  renderAll(state);
  goToPage("allocation");
}
document.getElementById("allocateBtn").addEventListener("click", handleAllocateNext);
document.getElementById("pqAllocateBtn").addEventListener("click", handleAllocateNext);

// Undo last allocation -> C Stack pop and state reversal
document.getElementById("undoBtn").addEventListener("click", () => {
  const { ok, state } = reliefQBridge.undoLast();
  currentSystemState = state;
  if (!ok) alert("No allocations to undo.");
  renderAll(state);
});

// Search in C
document.getElementById("searchBtn").addEventListener("click", () => {
  const field   = document.getElementById("searchField").value;
  const keyword = document.getElementById("searchInput").value.trim();
  if (!keyword) {
    document.getElementById("searchMeta").textContent = "Type something to search for.";
    return;
  }
  const result = reliefQBridge.linearSearch(field, keyword);
  document.getElementById("searchMeta").textContent = result.meta;
  renderSearchResults(result.results);
});

document.getElementById("binarySearchBtn").addEventListener("click", () => {
  const keyword = document.getElementById("searchInput").value.trim();
  if (!keyword) {
    document.getElementById("searchMeta").textContent = "Type a Request ID to use Binary Search.";
    return;
  }
  const result = reliefQBridge.binarySearch(keyword);
  document.getElementById("searchMeta").textContent = result.meta;
  renderSearchResults(result.results);
});

function renderSearchResults(results) {
  const rows = results.map(r => `
    <tr>
      <td class="mono">${r.requestID}</td>
      <td>${r.area}</td>
      <td>${r.location}</td>
      <td>${r.disasterType}</td>
      <td>${r.resource}</td>
      <td>${badge(r.priority, r.priority)}</td>
      <td>${statusBadge(r.status)}</td>
    </tr>`).join("");
  document.querySelector("#searchTable tbody").innerHTML =
    rows || `<tr><td colspan="7" class="muted">No matches.</td></tr>`;
}

// Bubble Sort in C
document.getElementById("sortBtn").addEventListener("click", () => {
  const field = document.getElementById("sortField").value;
  const asc   = document.getElementById("sortDir").value === "asc";

  const result = reliefQBridge.bubbleSort(field, asc);

  const beforeRows = result.before.map(r => `
    <tr><td class="mono">${r.requestID}</td><td>${badge(r.priority, r.priority)}</td><td>${fmt(r.population)}</td><td>${fmt(r.quantity)}</td></tr>`).join("");
  document.querySelector("#beforeSortTable tbody").innerHTML = beforeRows;

  const afterRows = result.after.map(r => `
    <tr><td class="mono">${r.requestID}</td><td>${badge(r.priority, r.priority)}</td><td>${fmt(r.population)}</td><td>${fmt(r.quantity)}</td></tr>`).join("");
  document.querySelector("#afterSortTable tbody").innerHTML = afterRows;
});

/* ---------------------------------------------------------------------------
   CLOCK
   --------------------------------------------------------------------------- */
function tickClock() {
  document.getElementById("clock").textContent = new Date().toLocaleString([], {
    weekday: "short", hour: "2-digit", minute: "2-digit", second: "2-digit"
  });
}
setInterval(tickClock, 1000);
tickClock();

/* ---------------------------------------------------------------------------
   APPLICATION BOOTSTRAP (Loads C WebAssembly Engine)
   --------------------------------------------------------------------------- */
(async function init() {
  try {
    const initialState = await reliefQBridge.load();
    currentSystemState = initialState;
    renderAll(initialState);
    document.querySelector("#beforeSortTable tbody").innerHTML =
      `<tr><td colspan="4" class="muted">Click "Sort" to compare before/after.</td></tr>`;
  } catch (err) {
    console.error("Failed to initialize C WASM engine:", err);
    alert("Error initializing RELIEF-Q C engine: " + err.message);
  }
})();
