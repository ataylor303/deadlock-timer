Deadlock Timer Simulation


This project demonstrates how a timer can be used to prevent deadlock when multiple processes compete for shared resources.

The program simulates two processes trying to access two shared resources (Resource A and Resource B) in opposite order. This creates a situation where deadlock can occur because each process may hold one resource while waiting for the other.

To avoid this, the program uses a timer when a process attempts to acquire its second resource. If the resource is not available within the set time, the process releases its first resource and exits. The parent process detects this and restarts the process so it can try again.

All system activity is recorded in a log file that shows:

- Which processes are running

- What resources they are trying to access

- When a resource is unavailable

- When a timeout occurs

- When a process is restarted

- When a process completes successfully

How to Compile:

gcc deadlock_timer.c -o deadlock_timer -pthread

How to Run:

./deadlock_timer

Output:

Running the program generates:

activity_log.txt

This file contains the activity log of process behavior and resource usage.

Purpose:

This program shows how deadlock can happen and how a timer-based approach can help prevent processes from becoming permanently stuck
