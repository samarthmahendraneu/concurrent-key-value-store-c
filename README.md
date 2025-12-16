# Concurrent Key-Value Store (Multithreaded, Networked)

A **concurrent, in-memory key–value store** implemented in C, featuring a **multi-threaded server**, **TCP-based client–server communication**, and **carefully synchronized shared state**.

> Built as part of an advanced OS curriculum, but engineered to production-quality standards.

## System Architecture

```
      +-------------+
      |  dbclient   |
      | (TCP client)|
      +------+------+
             |
             | TCP
             v
+----------------------------------+
|            dbserver              |
|                                  |
| +----------------------------+   |
| |   Console / Main Thread    |   |
| |   - stats                  |   |
| |   - list                   |   |
| |   - quit                   |   |
| +-------------+--------------+   |
|               |                  |
| +-------------v--------------+   |
| | Network Listener Thread    |   |
| | - accepts connections      |   |
| | - enqueues requests        |   |
| +-------------+--------------+   |
|               |                  |
| +-------------v--------------+   |
| |   Worker Thread Pool (N)   |   |
| |   - read / write / delete  |   |
| |   - update stats           |   |
| +----------------------------+   |
+----------------------------------+
```

## Core Features

**🔁 Concurrent Request Processing**
- Listener thread accepts incoming TCP connections
- Requests are placed into a **thread-safe task queue**
- Worker threads dequeue and process operations in parallel

**🧵 Thread-Safe Design (Monitors)**
- Shared components implemented as **monitors**
- Explicit mutex + condition variable discipline
- No data races, no busy waiting

Shared monitored state:
- Task queue
- Key-value storage
- Global operation statistics

## Key-Value Store Semantics

- **Keys**: arbitrary strings
- **Values**: arbitrary strings
- **Operations**:
  - `SET key value`
  - `GET key`
  - `DELETE key`
- Bounded capacity (configurable)
- Deterministic, thread-safe behavior under concurrency

## Example Usage

### Start the server
```bash
make
./dbserver
```

### Run clients
```bash
./dbclient --set=KEY0 ABC
./dbclient --get=KEY0
./dbclient --delete=KEY0
```

### Server console commands
```
stats   # show operation statistics
list    # dump current key-value pairs
quit    # shutdown server
```

## Technology Stack

- C (systems-level programming)
- POSIX threads (pthreads)
- TCP sockets
- Mutexes & condition variables
- Producer–consumer queues
- Thread-safe statistics aggregation
