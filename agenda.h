#ifndef AGENDA_H
#define AGENDA_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

// Define constants
#define MAX_TASKS 20     // Maximum number of tasks
#define MAX_NAME_LEN 50  // Maximum length of task name
#define TIME_STR_LEN 6   // Length of time string (HH:MM)
#define STATUS_LEN 10    // Length of status string
#define NOTIF_LEN 15     // Length of notification status string
#define INPUT_BUF_LEN 10 // Length of input buffer
#define DELAY_SECONDS 3  // Delay duration in seconds

/**
 * @struct Task
 * @brief Structure to store task details.
 */
typedef struct
{
    char name[MAX_NAME_LEN];            // Task name
    char start_time[TIME_STR_LEN];      // Start time (HH:MM)
    char end_time[TIME_STR_LEN];        // End time (HH:MM)
    char reminder_time[TIME_STR_LEN];   // Reminder time (HH:MM)
    char status[STATUS_LEN];            // Task status ("undone" or "done")
    char start_notification[NOTIF_LEN]; // Start notification status ("not_notified" or "notified")
    char end_notification[NOTIF_LEN];   // End notification status ("not_notified" or "notified")
} Task;

/**
 * @struct SharedState
 * @brief Structure to manage shared data and synchronization.
 */
typedef struct
{
    Task calendar[MAX_TASKS];         // Array of tasks
    int num_tasks;                    // Number of tasks
    pthread_mutex_t task_mutex;       // Mutex for task operations
    time_t program_start_time;        // Program start time
    double speedup_factor;            // Speedup factor for virtual time
    pthread_mutex_t print_mutex;      // Mutex for print operations
    int print_time;                   // Flag to indicate print request
    int input_flag;                   // Flag to indicate input availability
    char input_buffer[INPUT_BUF_LEN]; // Buffer for user input
    int awaiting_response;            // Flag to indicate waiting for user response
    Task *current_task;               // Pointer to current task being processed
    struct tm virtual_tm_info;        // Virtual time information
    pthread_mutex_t time_mutex;       // Mutex for time operations
    int current_day;                  // Current day
} SharedState;

/**
 * @brief Adds a task to the calendar.
 * @param state Pointer to the shared state structure.
 * @param name Name of the task.
 * @param start_time Start time of the task (HH:MM).
 * @param end_time End time of the task (HH:MM).
 */
void add_task(SharedState *state, const char *name, const char *start_time, const char *end_time);

/**
 * @brief Resets the calendar by clearing all tasks.
 * @param state Pointer to the shared state structure.
 */
void reset_calendar(SharedState *state);

#ifdef DEBUG
/**
 * @brief Displays the current time (for debugging).
 * @param tm_info Pointer to the tm structure containing the time information.
 */
void display_time(struct tm *tm_info);
#endif // DEBUG

/**
 * @brief Displays information about tasks.
 * @param state Pointer to the shared state structure.
 * @param time_str Optional time string to filter tasks.
 * @param use_virtual_time Flag to indicate whether to use virtual time.
 */
void display_task_info(SharedState *state, const char *time_str, int use_virtual_time);

/**
 * @brief Notifies about the start of a task.
 * @param task Pointer to the task structure.
 * @param virtual_tm_info Pointer to the tm structure containing the virtual time information.
 */
void notify_task_start(Task *task, struct tm *virtual_tm_info);

/**
 * @brief Notifies about the end of a task.
 * @param task Pointer to the task structure.
 * @param virtual_tm_info Pointer to the tm structure containing the virtual time information.
 */
void notify_task_end(Task *task, struct tm *virtual_tm_info);

/**
 * @brief Displays notifications for tasks.
 * @param state Pointer to the shared state structure.
 */
void display_task_notification(SharedState *state);

/**
 * @brief Starts the clock thread.
 * @param arg Pointer to the argument (shared state).
 * @return Pointer to the result (not used).
 */
void *start_clock(void *arg);

/**
 * @brief Handles display and interaction.
 * @param arg Pointer to the argument (shared state).
 * @return Pointer to the result (not used).
 */
void *display_and_interaction(void *arg);

/**
 * @brief Displays notifications.
 * @param arg Pointer to the argument (shared state).
 * @return Pointer to the result (not used).
 */
void *display_notifications(void *arg);

/**
 * @brief Handles user input.
 * @param arg Pointer to the argument (shared state).
 * @return Pointer to the result (not used).
 */
void *user_input_handle(void *arg);

/**
 * @brief Processes user input.
 * @param arg Pointer to the argument (shared state).
 * @return Pointer to the result (not used).
 */
void *process_input(void *arg);

#endif /* AGENDA_H */
