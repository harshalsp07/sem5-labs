# Simplified Operating Systems Labs (1 - 6)

A clean, easy-to-understand, beginner-friendly collection of all OS lab experiments and assignments (Labs 1 through 6).

All programs:
- Use **only simple, standard C library & POSIX system functions**.
- Are **fully self-contained** (compile with a single command).
- Include **clear comments and step-by-step explanations**.
- Remove unnecessary bloat and complicated boilerplate.

---

## Directory Overview

```
simplified_labs/
├── lab1/                   # Hardware & Resource Monitoring
│   ├── cpu_demo.c          # CPU-bound computation loop
│   ├── memory_demo.c       # Memory allocation & page touching (RSS)
│   └── disk_demo.c         # File block read/write operations (Disk I/O)
│
├── lab2/                   # Processes & Shell
│   ├── fork_basic.c        # Minimal fork() parent/child PID & wait()
│   ├── run_command.c       # fork() + execvp() + wait() command runner
│   └── simple_shell.c      # Interactive shell with cd, exit, and background &
│
├── lab3/                   # Unix File System Calls
│   ├── mycp.c              # File copy using open, read, write, close
│   ├── head5.c             # First 5 lines of a file
│   ├── mygrep.c            # Case-insensitive pattern search with line numbers
│   └── fileinfo.c          # Word, line, byte count and permissions
│
├── lab4/                   # CPU Scheduling Algorithms
│   ├── fcfs.c              # First-Come First-Served
│   ├── sjf.c               # Shortest Job First (Non-preemptive)
│   ├── srtf.c              # Shortest Remaining Time First (Preemptive)
│   ├── priority.c          # Priority Scheduling (Lowest number = Highest)
│   └── round_robin.c       # Round Robin (Fixed time quantum)
│
├── lab5/                   # Inter-Process Communication (IPC & Solutions)
│   ├── pipe_demo.c         # Parent-child pipe IPC
│   ├── shm_writer.c        # Shared memory writer (Producer)
│   ├── shm_reader.c        # Shared memory reader (Consumer)
│   ├── producer_consumer.c # Circular bounded buffer with fork()
│   ├── event_generator.c   # Problem 1: Event posting to shared memory
│   ├── event_monitor.c     # Problem 1: Event reading from shared memory
│   ├── barber.c            # Problem 2: Sleeping Barber server process
│   ├── customer.c          # Problem 2: Customer arrival process
│   ├── message_service.c   # Problem 3: Two-way bidirectional pipe messaging
│   └── job_system.c        # Problem 4: Master-Worker compute system via pipes
│
└── lab6/                   # Sockets & Signals
    ├── calc_server.c       # TCP calculator server
    ├── calc_client.c       # TCP calculator client
    ├── file_server.c       # TCP file transfer server
    ├── file_client.c       # TCP file transfer client
    ├── remote_cmd_server.c # Remote command execution server
    ├── remote_cmd_client.c # Remote command execution client
    └── signal_demo.c       # Signal handling (SIGINT, SIGUSR1, SIGUSR2, SIGTERM)
```

---

## How to Compile and Run

### Lab 1: Resource Utilization
```bash
cd simplified_labs/lab1

# CPU utilization demo
gcc cpu_demo.c -o cpu_demo
./cpu_demo
# In another terminal: top -p <PID>

# Memory utilization demo
gcc memory_demo.c -o memory_demo
./memory_demo

# Disk I/O demo
gcc disk_demo.c -o disk_demo
./disk_demo
```

---

### Lab 2: Process Management & Shell
```bash
cd simplified_labs/lab2

# Basic fork demo
gcc fork_basic.c -o fork_basic
./fork_basic

# Run external command with execvp
gcc run_command.c -o run_command
./run_command ls -la

# Interactive Shell
gcc simple_shell.c -o simple_shell
./simple_shell
# Inside shell: ls, pwd, cd .., sleep 3 &, exit
```

---

### Lab 3: File System Calls
```bash
cd simplified_labs/lab3

# Copy a file
gcc mycp.c -o mycp
./mycp mycp.c copy_test.txt

# Print first 5 lines
gcc head5.c -o head5
./head5 mycp.c

# Search for pattern in file
gcc mygrep.c -o mygrep
./mygrep "int main" mycp.c

# View file statistics and permissions
gcc fileinfo.c -o fileinfo
./fileinfo mycp.c
```

---

### Lab 4: CPU Scheduling Algorithms
Each scheduling algorithm is standalone and displays a complete formatted table with Completion Time (CT), Turnaround Time (TAT), Waiting Time (WT), and averages:
```bash
cd simplified_labs/lab4

gcc fcfs.c -o fcfs && ./fcfs
gcc sjf.c -o sjf && ./sjf
gcc srtf.c -o srtf && ./srtf
gcc priority.c -o priority && ./priority
gcc round_robin.c -o round_robin && ./round_robin
```

---

### Lab 5: Inter-Process Communication & Lab 5 Solutions

```bash
cd simplified_labs/lab5

# 1. Anonymous Pipe Demo
gcc pipe_demo.c -o pipe_demo && ./pipe_demo

# 2. Shared Memory (Writer & Reader)
gcc shm_writer.c -o shm_writer && gcc shm_reader.c -o shm_reader
./shm_writer
./shm_reader

# 3. Bounded Buffer Producer-Consumer
gcc producer_consumer.c -o producer_consumer && ./producer_consumer

# 4. Lab 5 Solutions - Problem 1: Event Logger (Shared Memory)
# Terminal 1:
gcc event_monitor.c -o event_monitor && ./event_monitor
# Terminal 2:
gcc event_generator.c -o event_generator && ./event_generator

# 5. Lab 5 Solutions - Problem 2: Sleeping Barber (Shared Memory)
# Terminal 1:
gcc barber.c -o barber && ./barber
# Terminal 2:
gcc customer.c -o customer && ./customer

# 6. Lab 5 Solutions - Problem 3: Two-Way Bidirectional Pipe Messaging
gcc message_service.c -o message_service && ./message_service

# 7. Lab 5 Solutions - Problem 4: Master-Worker Job System (Pipes)
gcc job_system.c -o job_system && ./job_system
```

---

### Lab 6: Sockets & Signals

#### 1. TCP Calculator:
```bash
cd simplified_labs/lab6

# Terminal 1:
gcc calc_server.c -o calc_server && ./calc_server

# Terminal 2:
gcc calc_client.c -o calc_client && ./calc_client
# Type: 10 + 20, 50 * 3, 100 / 4, exit
```

#### 2. File Transfer over Network:
```bash
# Terminal 1:
gcc file_server.c -o file_server && ./file_server

# Terminal 2:
gcc file_client.c -o file_client && ./file_client calc_client.c
```

#### 3. Remote Command Execution:
```bash
# Terminal 1:
gcc remote_cmd_server.c -o remote_cmd_server && ./remote_cmd_server

# Terminal 2:
gcc remote_cmd_client.c -o remote_cmd_client && ./remote_cmd_client
# Type: ls -l, whoami, date, uname -a, exit
```

#### 4. Signal Handling:
```bash
gcc signal_demo.c -o signal_demo && ./signal_demo
# Press Ctrl+C (interception demo)
# In another terminal: kill -SIGUSR1 <PID>, kill -SIGUSR2 <PID>, kill -SIGTERM <PID>
```
