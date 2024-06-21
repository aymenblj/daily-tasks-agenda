# Interactive agenda
## Overview
Interactive Agenda is a command-line program written in C that allows users to manage their daily tasks interactively. The program supports task tracking, notifications for task start and end times, and virtual time acceleration for testing purposes.

## Features
* Display task information and handle user interactions.
* Notify users of task start and end times.
* Reset the calendar daily.
* Multi-threaded to handle clock updates, user input, and notifications concurrently.

## Dependencies
* C Standard Library
* POSIX Threads (pthread)

## Installation
Clone the repository:

```
git clone https://github.com/aymenblj/daily-tasks-agenda.git
cd <repository_name>
```

## Compile the program:

```
gcc -o agenda main.c agenda.c -lpthread
```
or, in debug mode
```
gcc -o agenda main.c agenda.c -lpthread -DDEBUG
```

## Usage
Run the program:

```
./agenda
```

## Interactive Commands:
* "now": Display tasks that are currently active based on the virtual clock.
* "HH:MM": Display tasks scheduled for a specific time.
* "yes" or "no": Respond to task status inquiries.

## Notifications:
* The user will be notified when a task has started and when the end of the task is 10 minute due if it's still undone.
* Notifications do not interrupt the user inquiry while also could be intrusive if required.

## Virtual Time Acceleration
the clock can be setup with a speed factor to run faster (debug mode).

## Debugging
If compiled with the DEBUG flag, the program will print additional debugging information, such as task details and current virtual time.

## Multi-threading for an interactive agenda (Why?)
Besides notifications feature which was integrated whithout affecting the program core logic incorporating the **separate of concerns and Modularity principles**, an interactive agenda could have several other real-time constrained features that benefit from a multi-threaded approach. These features often involve concurrent processing and timely responses to user interactions. Here are some examples:

* **Time-Sensitive Alerts:** Apart from task notifications, the agenda might generate alerts for urgent events or reminders that require immediate user attention such as medication reminders and appointments. Multi-threading ensures that these alerts are delivered promptly without delaying other program operations.

* **Real-Time Task Updates:** Users may need to update task details while other tasks are being displayed or updated concurrently. A multi-threaded approach allows these updates to occur simultaneously without blocking the user interface.

* **Conflict Resolution:** When adding or modifying tasks, the program might need to check for conflicts such as overlapping schedules or resource availability. Multi-threading enables concurrent checking and resolution of conflicts, ensuring that updates are applied promptly without affecting ongoing user interactions.

* **Dynamic Scheduling Adjustments:** Users may adjust their schedules dynamically based on real-time events or changing priorities. Multi-threading allows the agenda to update and display schedule changes in real-time while continuing to handle user interactions smoothly.

* **External Data Integration:** If the agenda integrates with external systems or APIs to fetch data or perform actions (e.g., weather updates affecting outdoor tasks), multi-threading helps manage these interactions concurrently, maintaining responsiveness and timely updates which make it **extensible**.

* **Collaborative Editing:** In scenarios where multiple users can access and modify the agenda concurrently, multi-threading supports concurrent editing and synchronization of agenda data across users, ensuring consistency and real-time updates.