# PPID & Argument Spoofing (Windows Process Techniques)


## Overview
This project demonstrates two Windows process manipulation techniques for **educational and research purposes**:

- **PPID Spoofing**
- **Argument Spoofing**

These techniques are commonly studied in **Windows internals, malware analysis, red team simulations, and process creation research**.

---

## Introduction
Process spoofing techniques are methods used to **manipulate process metadata and creation attributes** in Windows.

They are commonly studied in **Windows internals, malware analysis, red team operations, and defensive research** to understand how processes can appear differently from their real execution context.

This project focuses on two common techniques:

- **PPID Spoofing:** creating a process with a forged parent process ID so it appears to be launched by another process
- **Argument Spoofing:** modifying the process command-line arguments in memory after creation to display misleading startup parameters

These techniques are provided strictly for **educational and authorized security research purposes**.

## Techniques Included

### 1. PPID Spoofing
Creates a new process while assigning a **custom parent process ID (PPID)**.

This makes the created process appear as if it was launched by another legitimate process.

Example:
- `explorer.exe` appears as the parent
- actual creator is your program

---

### 2. Argument Spoofing
Creates a process with **fake startup arguments**, then modifies the process command line in memory to replace them with the **real arguments**.

This technique is useful for studying:
- process command-line visibility
- memory-based argument changes
- Windows PEB manipulation

---

## Compilation
```
x86_64-w64-mingw32-gcc ppid_spoofing.c -o ppid.exe
x86_64-w64-mingw32-gcc argument_spoofing.c -o args.exe
```
