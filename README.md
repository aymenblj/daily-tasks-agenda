# Interactive agenda
## Overview
Interactive Agenda is a program written in C that allows users to track, and get notifications for their activities on a daily basis. 
The program supports virtual time acceleration to facilitate quick testing and debugging.

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
* The user will be notified when a task has started and when the end of the task is 10 minute due if it's sill undone.
* Notifications do not interrupt the user inquiry

## virtual clock
the clock can be setup with a speed factor to run faster (debug mode) 

## Debugging
If compiled with the DEBUG flag, the program will print additional debugging information, such as task details and current virtual time.
