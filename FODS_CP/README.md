# RELIEF-Q (C Language Edition)
### Priority-Based Disaster Relief Resource Allocation and Distribution Simulator Using Fundamental Data Structures

> **Rebuilt in C (C11 Core + WebAssembly + Native CLI)**  
> 100% functional equivalence with the original RELIEF-Q project, preserving all UI design tokens, workflows, prioritization rules, and fundamental data structure behaviors.

---

## 1. Project Overview

**RELIEF-Q** is a disaster relief coordination simulator designed for academic demonstration of **Fundamental Data Structures (FDS)**. During natural disasters (floods, fires, earthquakes, cyclones, droughts, landslides), relief organizations face severe resource scarcity. RELIEF-Q automates the process of receiving requests, ordering them by urgency, allocating resources fairly, and tracking distribution history with full audit and undo capability.

In this edition, the entire application logic, decision engine, and data structure implementations have been **rebuilt completely in C (C11 standard)**:
1. **Core Data Structures & Algorithms in C:** Every data structure (Array, Struct, Singly Linked List, FIFO Queue, 4-Lane Priority Queue, LIFO Stack, Linear Search, Binary Search, Bubble Sort) is implemented manually in C without high-level language shortcuts.
2. **WebAssembly Browser Execution:** Compiled via Clang to WebAssembly (`reliefq.wasm`), executing genuine C machine instructions inside the browser.
3. **Preserved Interface & Visuals:** The original HTML5, CSS3, typography (Sora, Inter, JetBrains Mono), cards, tables, badges, and real-time feeds are preserved exactly.
4. **Native C Executable (`reliefq_cli.exe`):** A standalone terminal executable for live command-line demonstrations and viva examinations.
5. **Automated C Test Suite (`test_reliefq.exe`):** 71 unit and integration tests verifying all operations in native C.
6. **Vercel Cloud Deployment Ready:** Optimized as a static WebAssembly web app deployable to Vercel in seconds with a public URL.

---

## 2. Architecture: C + WebAssembly + Native CLI

```
┌────────────────────────────────────────────────────────────────────────┐
│                        USER INTERFACE LAYER                            │
│  - index.html (Responsive Dashboard, Requests, Inventory, PQ, Reports) │
│  - css/style.css (Custom design tokens, navy theme, priority colors)   │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │ DOM Events & State Rendering
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                      JAVASCRIPT BRIDGE LAYER                           │
│  - js/reliefq_bridge.js (WebAssembly loader, UTF-8 memory marshaler)   │
│  - js/reliefq_wasm_bytes.js (Embedded fallback for offline file://)   │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │ Low-level C function calls (WASM)
                                    ▼
┌────────────────────────────────────────────────────────────────────────┐
│                        COMPILED C CORE (C11)                           │
│  - c_src/reliefq_core.c      : Core simulation state & workflow logic  │
│  - c_src/resource_array.c    : Array-based inventory management        │
│  - c_src/queue.c             : FIFO Queue for intake requests          │
│  - c_src/priority_queue.c    : 4-lane Priority Queue (Critical->Low)   │
│  - c_src/linked_list.c       : Singly Linked List for completed cases  │
│  - c_src/stack.c             : LIFO Stack for allocation history/undo  │
│  - c_src/search_sort.c       : Linear Search, Binary Search, BubbleSort│
│  - c_src/json_export.c       : C-side JSON state serialization         │
│  - c_src/memory_pool.c       : Freestanding memory allocator           │
│  - c_src/wasm_api.c          : WebAssembly API exports                 │
└────────────────────────────────────────────────────────────────────────┘
                                    ▲
                                    │ Direct C11 compilation
┌───────────────────────────────────┴────────────────────────────────────┐
│                    NATIVE COMPILATION TARGETS                          │
│  - reliefq_cli.exe           : Interactive terminal CLI program        │
│  - tests/test_reliefq.exe    : 71 automated C unit & integration tests │
└────────────────────────────────────────────────────────────────────────┘
```

### Why this architecture?
- **Zero Simulation:** The browser does not simulate C. It runs `reliefq.wasm`, which is compiled C machine instructions executing in the browser's WebAssembly sandbox.
- **Universal Deployment (Vercel):** Because the compiled C code runs on the client via WebAssembly, the project has no server runtime dependencies. It deploys cleanly as a static site to Vercel, Netlify, or GitHub Pages.
- **Double-Click Offline Support:** In addition to standard `fetch('public/reliefq.wasm')`, an embedded base64 fallback in `js/reliefq_wasm_bytes.js` ensures that double-clicking `index.html` directly from Windows Explorer (`file:///` path) works immediately without browser CORS restrictions.
- **Academic Verification:** MinGW GCC compiles the same C codebase into `tests/test_reliefq.exe` and `reliefq_cli.exe`, enabling terminal demonstrations and automated test verification.

---

## 3. Data Structures & Algorithm Mapping

Every JavaScript construct from the original project has been mapped to standard C:

| # | Data Structure | Original JS (`dataStructures.js`) | New C Implementation (`c_src/`) | Header File (`include/`) | Purpose |
|---|----------------|-----------------------------------|---------------------------------|--------------------------|---------|
| 1 | **Structure** | `class ReliefRequest` | `ReliefRequest` struct | `reliefq_types.h` | Groups request attributes (ID, area, pop, priority, etc.) |
| 2 | **Array** | `class ResourceArray` | `ResourceArray` struct + functions | `resource_array.h` | Manages resource inventory (Food, Water, Medicines...) |
| 3 | **Linked List** | `class LLNode`, `LinkedList` | `LLNode` + `LinkedList` (singly linked) | `linked_list.h` | Stores completed/distributed requests dynamically |
| 4 | **FIFO Queue** | `class QNode`, `Queue` | `QNode` + `Queue` (head/tail pointers) | `queue.h` | First-in-first-out intake queue for incoming requests |
| 5 | **Priority Queue** | `class PriorityQueue` (4 Queues) | `PriorityQueue` (4 `Queue` lanes) | `priority_queue.h` | Critical → High → Medium → Low ordering; FIFO within lane |
| 6 | **Stack** | `class Stack` | `Stack` struct (LIFO array) | `stack.h` | Allocation history stack and Undo operation |
| 7 | **Linear Search** | `linearSearchRequests()` | `linear_search_requests()` in C | `search_sort.h` | O(n) case-insensitive search by field |
| 8 | **Binary Search** | `binarySearchByID()` | `binary_search_by_id()` in C | `search_sort.h` | O(log n) search on array sorted by `requestID` with steps |
| 9 | **Bubble Sort** | `bubbleSortRequests()` | `bubble_sort_requests()` in C | `search_sort.h` | O(n²) sorting by priority rank, population, qty, or ID |

---

## 4. Feature-by-Feature Preservation Checklist

All features from the original RELIEF-Q project are preserved:

- [x] **Disaster & Emergency Reporting:** Form validation, duplicate ID rejection, automatic queuing.
- [x] **Intake Waiting Queue:** FIFO queue with head/tail pointers and visual badge count.
- [x] **Priority Classification:** `Classify Next` and `Classify All` workflows into 4 urgency lanes.
- [x] **Urgency Ordering:** Critical (1) → High (2) → Medium (3) → Low (4).
- [x] **FIFO Tie-Breaking:** Requests with identical priority retain original arrival order.
- [x] **Resource Inventory Management:** Add/top-up stock, case-insensitive resource lookup.
- [x] **Allocation Engine:**
  - **Full Allocation:** If stock $\ge$ quantity $\to$ status becomes `"Completed"`, added to Linked List.
  - **Partial Allocation:** If $0 < \text{stock} < \text{quantity} \to$ consumes available stock, status becomes `"Partially Allocated"`.
  - **Waiting on Stock:** If stock $= 0 \to$ status becomes `"Waiting for Resources"`, added to retry table.
- [x] **Allocation History (Stack):** Every allocation creates an `A###` record pushed onto the Stack.
- [x] **Undo Operation (LIFO):** Pops top allocation record, restores inventory stock, reverts request status, and re-inserts into Priority Queue.
- [x] **Retry Waiting Requests:** One-click re-allocation when new inventory arrives.
- [x] **Linear Search:** Case-insensitive search across Request ID, Location, Disaster Type, Resource, Area.
- [x] **Binary Search by ID:** Requires sorting by Request ID; tracks and displays comparison step count.
- [x] **Bubble Sort:** Interactive before/after tables sorting by Priority, Population, Quantity, or ID.
- [x] **Live Operations Feed:** Real-time chronological audit trail of all simulation events.
- [x] **Dashboard & Reports:** Real-time statistics, priority distribution bars, inventory capacity bars, disaster breakdown.
- [x] **Preloaded Sample Data:** 7 resource types and 10 sample requests across Pune, Kolhapur, Satara, Sangli, and Nashik.

---

## 5. Project Directory Structure

```
RELIEF_Q_C/
├── index.html                  # Main web dashboard interface
├── css/
│   └── style.css               # Complete styling, themes, design tokens
├── js/
│   ├── reliefq_bridge.js       # WebAssembly bridge connecting HTML to C engine
│   ├── reliefq_wasm_bytes.js   # Base64-encoded WASM for offline file:/// execution
│   └── app.js                  # UI DOM events & rendering logic
├── public/
│   └── reliefq.wasm            # Compiled C WebAssembly binary (Clang target)
├── reliefq.wasm                # Root copy for flexible server configurations
├── reliefq_cli.exe             # Compiled native C CLI executable (MinGW GCC)
├── include/                    # C header files (C11)
│   ├── reliefq_types.h         # Core data structs, enums, constants
│   ├── memory_pool.h           # Memory allocator & string helper declarations
│   ├── string_utils.h          # Safe string handling & priority rank conversions
│   ├── resource_array.h        # Array-based inventory declarations
│   ├── linked_list.h           # Singly linked list declarations
│   ├── queue.h                 # FIFO queue declarations
│   ├── priority_queue.h        # 4-level priority queue declarations
│   ├── stack.h                 # LIFO stack declarations
│   ├── search_sort.h           # Linear/Binary search & Bubble sort declarations
│   ├── reliefq_core.h          # System state, workflows, sample data
│   └── json_export.h           # State serialization declarations
├── c_src/                      # C source files (C11)
│   ├── memory_pool.c           # Memory pool allocator implementation
│   ├── string_utils.c          # String operations implementation
│   ├── resource_array.c        # Array inventory management implementation
│   ├── linked_list.c           # Singly linked list implementation
│   ├── queue.c                 # FIFO queue implementation
│   ├── priority_queue.c        # 4-lane priority queue implementation
│   ├── stack.c                 # LIFO history stack implementation
│   ├── search_sort.c           # Searching & sorting implementations
│   ├── reliefq_core.c          # Core workflow engine implementation
│   ├── json_export.c           # C JSON serializer implementation
│   ├── wasm_api.c              # WebAssembly exported C functions
│   └── main_cli.c              # Interactive terminal CLI implementation
├── tests/
│   ├── test_reliefq.c          # 71 automated C unit & integration tests
│   └── test_reliefq.exe        # Compiled test executable
├── scripts/
│   ├── build_wasm.bat          # Windows script: compile C to WebAssembly
│   ├── build_native.bat        # Windows script: compile C to native executables
│   ├── build_all.bat           # Windows script: compile all targets
│   ├── run_tests.bat           # Windows script: compile and run test suite
│   ├── build_wasm.sh           # Unix/Linux/macOS script: compile to WebAssembly
│   ├── build_native.sh         # Unix/Linux/macOS script: compile native executables
│   ├── build_all.sh            # Unix/Linux/macOS script: compile all targets
│   ├── run_tests.sh            # Unix/Linux/macOS script: run test suite
│   └── gen_wasm_bytes.js       # Node utility: generates base64 fallback file
├── vercel.json                 # Vercel deployment configuration
├── package.json                # Project build metadata & npm scripts
├── server.py                   # Local development server (Python 3)
├── start.bat                   # 1-click Windows launcher
├── start.sh                    # 1-click Linux/macOS launcher
└── README.md                   # This documentation
```

---

## 6. How to Run Locally

### Option 1: 1-Click Launch (Recommended)
- **Windows:** Double-click `start.bat`.
- **macOS / Linux:** Run `./start.sh` in terminal (run `chmod +x start.sh` first if required).
- The local server starts and opens **http://localhost:8000** automatically in your browser.

### Option 2: Python Local Server
Open a terminal in the project directory and run:
```bash
python server.py
# or
python3 server.py
```
Then visit **http://localhost:8000**.

### Option 3: Direct File Opening (No Server Required)
Simply double-click `index.html` to open it in Chrome, Edge, Safari, or Firefox.  
Because the project includes `js/reliefq_wasm_bytes.js`, the compiled C WebAssembly engine initializes even from a `file:///` path without needing any server.

### Option 4: Native C Terminal CLI
To run the interactive command-line interface in your terminal:
```cmd
.\reliefq_cli.exe
```
This lets you interact with the C simulation directly from the terminal prompt:
```text
=====================================================================
  RELIEF-Q  --  Disaster Relief Allocation Simulator (Native C)
  Fundamental Data Structures: Array, Struct, LinkedList, Queue, PQ, Stack
=====================================================================

Select an operation:
  1. View Dashboard & Statistics
  2. View Resource Inventory (Array)
  3. View Waiting Queue (FIFO Queue)
  4. View Priority Queue Lanes (4-Level PQ)
  5. View Completed Requests (Linked List)
  6. View Allocation History (Stack - LIFO)
  7. Submit New Relief Request (Queue)
  8. Classify Next Request (Queue -> Priority Queue)
  9. Allocate Next Priority Request
 10. Undo Last Allocation (Stack pop -> Restore)
 11. Search Requests (Linear / Binary Search)
 12. Sort Requests (Bubble Sort)
 13. Add / Top-up Resource Inventory
 14. Reset Simulation to Sample Data
  0. Exit
Choice >
```

---

## 7. How to Build & Compile

### Prerequisites
- **GCC (MinGW on Windows, gcc on Linux/macOS):** For native C executables.
- **Clang / LLVM:** For WebAssembly compilation (`--target=wasm32`).
- **Node.js (Optional):** For regenerating base64 wasm file.

### Build All Targets (Windows)
Run:
```cmd
scripts\build_all.bat
```

### Build All Targets (Linux / macOS)
Run:
```bash
bash scripts/build_all.sh
```

### Individual Build Commands

#### Compile WebAssembly with Clang:
```bash
clang --target=wasm32 -nostdlib -Iinclude -Wl,--no-entry -Wl,--export-all -O3 -o public/reliefq.wasm \
    c_src/memory_pool.c c_src/string_utils.c c_src/resource_array.c c_src/linked_list.c \
    c_src/queue.c c_src/priority_queue.c c_src/stack.c c_src/search_sort.c \
    c_src/reliefq_core.c c_src/json_export.c c_src/wasm_api.c
```

#### Compile Native CLI with GCC:
```bash
gcc -std=c11 -Wall -Wextra -O2 -Iinclude c_src/main_cli.c c_src/memory_pool.c c_src/string_utils.c \
    c_src/resource_array.c c_src/linked_list.c c_src/queue.c c_src/priority_queue.c \
    c_src/stack.c c_src/search_sort.c c_src/reliefq_core.c c_src/json_export.c -o reliefq_cli.exe
```

#### Compile and Run Test Suite with GCC:
```bash
gcc -std=c11 -Wall -Wextra -Iinclude tests/test_reliefq.c c_src/memory_pool.c c_src/string_utils.c \
    c_src/resource_array.c c_src/linked_list.c c_src/queue.c c_src/priority_queue.c \
    c_src/stack.c c_src/search_sort.c c_src/reliefq_core.c c_src/json_export.c -o tests/test_reliefq.exe
.\tests\test_reliefq.exe
```

---

## 8. Automated Test Suite Results

The automated C test suite (`tests/test_reliefq.c`) executes 71 tests covering every data structure, algorithm, and edge case:

```
========================================================
  RUNNING RELIEF-Q C AUTOMATED TEST SUITE (C11)
========================================================

=== TEST SUITE 1: ResourceArray (Array) ===
  [PASS] Initial resource array count is 0
  [PASS] Added Water (500), count is 1
  [PASS] Water available is 500
  [PASS] Added Water (200), count remains 1
  [PASS] Water available is now 700
  [PASS] Case-insensitive lookup finds 'water' at index 0
  [PASS] Consumed 300 units of Water
  [PASS] Water available is now 400
  [PASS] Water distributed is 300
  [PASS] Over-consume Water returned only available 400
  [PASS] Water available is 0
  [PASS] Water distributed is 700
  [PASS] Restored 300 Water, available is 300
  [PASS] Distributed decreased to 400

=== TEST SUITE 2: LinkedList (Singly Linked List) ===
  [PASS] Initial LinkedList size is 0
  [PASS] Inserted 3 items into LinkedList
  [PASS] Searched and found R2 in LinkedList
  [PASS] Non-existent request returns NULL
  [PASS] Deleted R2 (middle node)
  [PASS] Size is now 2
  [PASS] R2 no longer in list
  [PASS] Deleted R1 (head node)
  [PASS] Size is now 1
  [PASS] Cleared list, size is 0

=== TEST SUITE 3: Queue (FIFO) ===
  [PASS] Initial Queue is empty
  [PASS] Enqueued 2 items, size is 2
  [PASS] Peeked front of queue is Q1
  [PASS] Dequeued first item is Q1 (FIFO)
  [PASS] Queue size is now 1
  [PASS] Dequeued second item is Q2 (FIFO)
  [PASS] Queue is now empty

=== TEST SUITE 4: PriorityQueue (4-Lane Urgency) ===
  [PASS] Initial PriorityQueue is empty
  [PASS] Inserted 5 items into PriorityQueue
  [PASS] 1st dequeued is R_CRIT1 (Critical)
  [PASS] 2nd dequeued is R_CRIT2 (FIFO within Critical)
  [PASS] 3rd dequeued is R_HIGH (High)
  [PASS] 4th dequeued is R_MED (Medium)
  [PASS] 5th dequeued is R_LOW (Low)
  [PASS] PriorityQueue is empty after 5 dequeues

=== TEST SUITE 5: Stack (LIFO History / Undo) ===
  [PASS] Initial Stack is empty
  [PASS] Pushed 2 records onto Stack
  [PASS] Top of stack is A2 (most recent)
  [PASS] Popped A2 first (LIFO)
  [PASS] Popped A1 second (LIFO)
  [PASS] Stack is empty after pops

=== TEST SUITE 6: Search & Bubble Sort ===
  [PASS] Linear search found 'kol' -> R102
  [PASS] Linear search found 2 'flood' requests
  [PASS] 1st sorted is Critical
  [PASS] 2nd sorted is Critical
  [PASS] 3rd sorted is High
  [PASS] 4th sorted is Low
  [PASS] Sorted by ID: 1st is R101
  [PASS] Sorted by ID: 2nd is R102
  [PASS] Sorted by ID: 3rd is R103
  [PASS] Sorted by ID: 4th is R104
  [PASS] Binary search found R103
  [PASS] Binary search recorded step comparisons
  [PASS] Binary search for non-existent R999 returns 0

=== TEST SUITE 7: End-to-End Workflow & Undo ===
  [PASS] 10 sample requests loaded
  [PASS] 2 sample requests completed on bootstrap
  [PASS] 2 allocations in history stack
  [PASS] Undo last allocation succeeded
  [PASS] Stack size decreased to 1
  [PASS] Completed list size decreased to 1
  [PASS] Allocated next priority request
  [PASS] Allocation of R105 (needs 600 Medicines, 500 in stock) succeeded with ALLOC_PARTIAL
  [PASS] Massive request resulted in ALLOC_PARTIAL
  [PASS] Allocated all 300 available Tents
  [PASS] Tents inventory is now 0
  [PASS] 0 available stock resulted in ALLOC_WAIT
  [PASS] Status marked WAITING_FOR_RESOURCES

========================================================
  TEST RESULTS: 71 / 71 PASSED (100%)
========================================================
```

---

## 9. How to Push to GitHub

Follow these steps to push your completed C project to a new GitHub repository:

1. **Initialize Git repository:**
   ```bash
   cd RELIEF_Q_C
   git init
   ```
2. **Stage all project files:**
   ```bash
   git add .
   ```
3. **Create the initial commit:**
   ```bash
   git commit -m "Rebuild RELIEF-Q completely in C (C11 Core + WebAssembly + Native CLI)"
   ```
4. **Create a new repository on GitHub:**
   - Go to [github.com/new](https://github.com/new).
   - Name the repository: `RELIEF-Q-C` (or any name you prefer).
   - Leave it **Public**. Do not initialize with a README, .gitignore, or license (they are already included).
5. **Link and push:**
   ```bash
   git branch -M main
   git remote add origin https://github.com/YOUR_USERNAME/RELIEF-Q-C.git
   git push -u origin main
   ```

---

## 10. How to Deploy to Vercel (Get a Public Website URL)

You can deploy the website to Vercel in less than 2 minutes using either of these two methods:

### Method A: Deploy via GitHub (Easiest)
1. Go to [vercel.com](https://vercel.com) and log in with your GitHub account.
2. Click **"Add New..."** $\to$ **"Project"**.
3. Select your `RELIEF-Q-C` repository from the list.
4. Leave **Framework Preset** as **"Other"** (or Root).
5. Leave all build and output settings as default.
6. Click **"Deploy"**.
7. Vercel will build and deploy your project in seconds. You will receive a public URL such as `https://relief-q-c.vercel.app`!

### Method B: Deploy via Vercel CLI
1. Install the Vercel CLI (if not already installed):
   ```bash
   npm install -g vercel
   ```
2. In the `RELIEF_Q_C` folder, run:
   ```bash
   vercel
   ```
3. Follow the interactive prompts (select default options).
4. For production deployment, run:
   ```bash
   vercel --prod
   ```
5. Your public website URL will be displayed in the terminal.

---

## 11. Troubleshooting

| Issue | Cause | Solution |
|-------|-------|----------|
| `fetch` to `reliefq.wasm` blocked in browser | Opening file directly using `file:///` in browsers that enforce local CORS | No action needed! `js/reliefq_wasm_bytes.js` automatically decodes the embedded binary when `fetch` fails. Alternatively, run `python server.py`. |
| `clang: command not found` | Clang is not on the system PATH | The build script `scripts/build_wasm.bat` automatically looks in the Android NDK LLVM toolchain directory. You can also install LLVM from [llvm.org](https://releases.llvm.org/). |
| `gcc: command not found` | MinGW / GCC not installed or not in PATH | Install MinGW-W64 or run inside MSYS2. GCC is only needed for building native executables (`.exe`), not for running the website. |
| Port 8000 already in use | Another application is using port 8000 | Open `server.py`, change `PORT = 8000` to `PORT = 8080`, and restart. |
| Vercel returns 404 for `.wasm` file | Incorrect MIME type configuration | Ensure `vercel.json` is present in the repository root. It explicitly assigns `Content-Type: application/wasm`. |

---

## 12. Viva Examination Questions & Answers

**Q1: Why did you rebuild the project in C?**  
*Answer:* Implementing Fundamental Data Structures in C makes memory management, node pointers, and algorithmic trade-offs explicit and transparent. It demonstrates true mastery of fundamental computer science concepts rather than relying on high-level language wrappers.

**Q2: How does the C code run in the web browser?**  
*Answer:* The C11 source files are compiled using Clang into WebAssembly (`.wasm`), which is a binary instruction format executed by the browser's high-speed WebAssembly virtual machine. A lightweight JavaScript bridge (`reliefq_bridge.js`) transfers data between the C memory buffer and the HTML DOM.

**Q3: Explain how the Priority Queue is designed in your C implementation.**  
*Answer:* The Priority Queue consists of an array of four separate FIFO `Queue` structures: index 0 for Critical, 1 for High, 2 for Medium, and 3 for Low. When a request is inserted, it is placed at the rear of its corresponding priority lane. When dequeuing, the system checks lane 0 first; if empty, it checks lane 1, then 2, then 3. Within each lane, FIFO order is strictly preserved, ensuring that requests with identical priority are served in order of arrival.

**Q4: Why is a singly linked list used for completed requests instead of an array?**  
*Answer:* A singly linked list allows dynamic node allocation as requests are fulfilled, without requiring a fixed pre-allocated buffer or expensive array reallocations. It demonstrates manual node creation (`LLNode`), pointer traversal (`next`), and node deletion during undo operations.

**Q5: Why is a Stack used for allocation history?**  
*Answer:* The Undo operation follows Last-In-First-Out (LIFO) semantics. A Stack's `push` and `pop` operations naturally reverse actions in reverse chronological order, allowing the system to restore inventory quantities and revert request statuses seamlessly.

**Q6: What is the time complexity of Binary Search in your implementation?**  
*Answer:* Binary Search is $O(\log n)$. It operates on an array sorted by `requestID`. In each iteration, it checks the middle element (`mid = (low + high) / 2`), narrowing the search interval by half. If the array is not pre-sorted, it is sorted first using Bubble Sort in $O(n^2)$.

**Q7: What happens during partial resource allocation?**  
*Answer:* If the available resource quantity is greater than zero but less than the requested amount, the system consumes all available stock (`given = available`), updates `req->allocated += given`, marks the status as `"Partially Allocated"`, pushes the allocation to the Stack, and adds the record to the Linked List.

---

## 13. License & Authorship

Developed for the **First-Year Computer Science & Engineering Fundamental Data Structures (FDS)** curriculum.  
Licensed under the **MIT License**.
