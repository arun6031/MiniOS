
# MyOS: A Minimal Real-Time Operating System for ARM Cortex-M0+

This project is a small, cooperative, priority-based, real-time operating system (RTOS) designed from the ground up for the ARM Cortex-M0+ architecture. It serves as an educational tool to understand the core concepts of embedded operating systems, including context switching, task scheduling, system ticks, and kernel services.

This specific implementation targets the Texas Instruments MSPM0L1306 microcontroller but is written to be easily portable to other Cortex-M0+ devices.

## Features

Round-Robin Scheduling: When a task yields, the scheduler cycles through the list of ready tasks in a simple round-robin fashion to select the next one to run.

System Tick Timer (SysTick): A 1ms system tick is used as the time base for delays. The SysTick_Handler wakes up tasks whose delays have expired.

Task Management: Dynamically create tasks with their own stacks and entry points.

Blocking Delays: Tasks can block for a specific number of ticks using OS_delay(), effectively yielding the CPU to other tasks.

Idle Task: Includes a low-power idle task that runs when no other tasks are ready, using the WFI (Wait For Interrupt) instruction to save power.

Privileged-Only Kernel: A simple and robust design where the OS kernel and all tasks run in privileged mode, simplifying the implementation and debugging process.

## Project Structure

main.c: The main application file. It contains hardware initialization, task definitions, and the main() entry point that starts the OS.

myos.c: The core operating system kernel. It contains the scheduler, context switcher (PendSV_Handler), thread creation logic, and API calls.

myos.h: The public header file for the OS, defining the API and data structures available to the application.

startup_mspm0l1306_ticlang.c (or similar): The device startup file, which contains the vector table. Ensure the PendSV_Handler and SysTick_Handler symbols are correctly placed here.

## How to Build and Run

This project is configured for the TI MSPM0L1306 and is intended to be built with a modern ARM toolchain like TI ARM-LLVM Clang or ARM-GCC.

## Hardware:

A TI MSPM0L1306 LaunchPad or custom board.

Two LEDs connected to pins PA27 and PA26.

## Toolchain:

Code Composer Studio (CCS) or another IDE configured with an ARM-LLVM or ARM-GCC compiler.

MSPM0 SDK for device-specific headers and libraries.


## Future Enhancements

Preemptive Scheduling: Modifying the SysTick_Handler to call the scheduler, allowing a higher-priority task to preempt a lower-priority one.

Unprivileged Mode and SVC: Adding a security layer by running tasks in Unprivileged Mode and using the SVC exception for all kernel API calls.

Synchronization Primitives: Implementing mutexes, semaphores, and message queues to allow tasks to communicate and share resources safely.

Stack Overflow Detection: Implementing "stack painting" and checking for overflows during context switches to improve system robustness.
