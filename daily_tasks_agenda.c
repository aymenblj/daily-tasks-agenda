#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <string.h>

#define MAX_TASKS 20
#define MAX_TASKS 20
#define MAX_NAME_LEN 50
#define TIME_STR_LEN 6
#define STATUS_LEN 10
#define NOTIF_LEN 15
#define INPUT_BUF_LEN 10
#define DELAY_SECONDS 3 // Define delay duration

// Task structure to store task details
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

// SharedState structure to manage shared data and synchronization
typedef struct
{
    Task calendar[MAX_TASKS];    // Array of calendar
    int num_tasks;               // Number of calendar
    pthread_mutex_t task_mutex;  // Mutex for task operations
    time_t program_start_time;   // Program start time
    double speedup_factor;       // Speedup factor for virtual time
    pthread_mutex_t print_mutex; // Mutex for print operations
    int print_time;              // Flag to indicate print request
    int input_flag;              // Flag to indicate input availability
    char input_buffer[10];       // Buffer for user input
    int awaiting_response;       // Flag to indicate waiting for user response
    Task *current_task;          // Pointer to current task being processed
    struct tm virtual_tm_info;   // Virtual time information
    pthread_mutex_t time_mutex;  // Mutex for time operations
    int current_day;             // Current day
} SharedState;

// Function prototypes
void add_task(SharedState *state, const char *name, const char *start_time, const char *end_time);
void reset_calendar(SharedState *state);
#ifdef DEBUG
void display_time(struct tm *tm_info);
#endif // DEBUG
void display_task_info(SharedState *state, const char *time_str, int use_virtual_time);
void notify_task_start(Task *task, struct tm *virtual_tm_info);
void notify_task_end(Task *task, struct tm *virtual_tm_info);
void display_task_notification(SharedState *state);
void *start_clock(void *arg);
void *display_and_interaction(void *arg);
void *display_notifications(void *arg);
void *user_input_handle(void *arg);
void *process_input(void *arg);

int main()
{
    SharedState state;

    // Initialize shared state
    state.num_tasks = 0;
    state.print_time = 0;
    state.input_flag = 0;
    state.awaiting_response = 0;
    state.current_task = NULL;
    state.program_start_time = time(NULL);

    // Initialize mutexes
    if (pthread_mutex_init(&state.task_mutex, NULL) != 0)
    {
        fprintf(stderr, "Error: Failed to initialize task_mutex\n");
        return EXIT_FAILURE;
    }
    if (pthread_mutex_init(&state.print_mutex, NULL) != 0)
    {
        fprintf(stderr, "Error: Failed to initialize print_mutex\n");
        return EXIT_FAILURE;
    }
    if (pthread_mutex_init(&state.time_mutex, NULL) != 0)
    {
        fprintf(stderr, "Error: Failed to initialize time_mutex\n");
        return EXIT_FAILURE;
    }

    // Setup an actual clock
    state.speedup_factor = 1;
#ifdef DEBUG
    // Input speedup factor
    printf("Enter the speedup factor: ");
    scanf("%lf", &state.speedup_factor);
    getchar(); // Consume the newline character left by scanf
#endif // DEBUG

    // Add calendar covering 24 hours
    add_task(&state, "Sleep", "00:00", "07:00");
    add_task(&state, "Wake up and wash", "07:00", "07:30");
    add_task(&state, "Make bed", "07:30", "08:00");
    add_task(&state, "Prepare breakfast", "08:00", "08:30");
    add_task(&state, "Have breakfast", "08:30", "09:00");
    add_task(&state, "Morning walk", "09:00", "10:00");
    add_task(&state, "Read newspaper", "10:00", "11:00");
    add_task(&state, "Gardening", "11:00", "12:00");
    add_task(&state, "Lunch preparation", "12:00", "13:00");
    add_task(&state, "Have lunch", "13:00", "13:30");
    add_task(&state, "Nap time", "13:30", "15:00");
    add_task(&state, "Afternoon tea", "15:00", "16:00");
    add_task(&state, "Family time", "16:00", "18:00");
    add_task(&state, "Dinner preparation", "18:00", "19:00");
    add_task(&state, "Have dinner", "19:00", "19:30");
    add_task(&state, "Watch TV", "19:30", "21:00");
    add_task(&state, "Relax and prepare for bed", "21:00", "22:00");
    add_task(&state, "Sleep", "22:00", "23:59");

    // Initialize the current day
    struct tm *initial_tm = localtime(&state.program_start_time);
    state.current_day = initial_tm->tm_mday;

    // Initialize threads
    pthread_t clock_thread, display_thread, input_thread, display_notification_thread, input_processing_thread;
    pthread_create(&clock_thread, NULL, start_clock, &state);
    pthread_create(&display_thread, NULL, display_and_interaction, &state);
    pthread_create(&input_thread, NULL, user_input_handle, &state);
    pthread_create(&display_notification_thread, NULL, display_notifications, &state);
    pthread_create(&input_processing_thread, NULL, process_input, &state);

    // Join threads
    pthread_join(clock_thread, NULL);
    pthread_join(display_thread, NULL);
    pthread_join(input_thread, NULL);
    pthread_join(display_notification_thread, NULL);
    pthread_join(input_processing_thread, NULL);

    // Destroy mutexes
    pthread_mutex_destroy(&state.task_mutex);
    pthread_mutex_destroy(&state.print_mutex);
    pthread_mutex_destroy(&state.time_mutex);

    return EXIT_SUCCESS;
}

// Function to add a task to the shared state
void add_task(SharedState *state, const char *name, const char *start_time, const char *end_time)
{
    pthread_mutex_lock(&state->task_mutex);

    // Add task if there is space
    if (state->num_tasks < MAX_TASKS)
    {
        Task *task = &state->calendar[state->num_tasks];
        strncpy(task->name, name, sizeof(task->name) - 1);
        task->name[sizeof(task->name) - 1] = '\0'; // Ensure null-termination

        strncpy(task->start_time, start_time, sizeof(task->start_time) - 1);
        task->start_time[sizeof(task->start_time) - 1] = '\0'; // Ensure null-termination

        strncpy(task->end_time, end_time, sizeof(task->end_time) - 1);
        task->end_time[sizeof(task->end_time) - 1] = '\0'; // Ensure null-termination

        strncpy(task->status, "undone", sizeof(task->status) - 1);
        task->status[sizeof(task->status) - 1] = '\0'; // Ensure null-termination

        // Calculate reminder time (end_time - 10 minutes)
        int end_hour, end_min;
        sscanf(end_time, "%d:%d", &end_hour, &end_min);
        int total_end_minutes = end_hour * 60 + end_min;
        total_end_minutes -= 10;
        if (total_end_minutes < 0)
        {
            total_end_minutes += 1440; // 1440 minutes in a day (24 hours)
        }
        int reminder_hour = total_end_minutes / 60;
        int reminder_min = total_end_minutes % 60;
        sprintf(task->reminder_time, "%02d:%02d", reminder_hour, reminder_min);

        strncpy(task->start_notification, "not_notified", sizeof(task->start_notification) - 1);
        task->start_notification[sizeof(task->start_notification) - 1] = '\0'; // Ensure null-termination

        strncpy(task->end_notification, "not_notified", sizeof(task->end_notification) - 1);
        task->end_notification[sizeof(task->end_notification) - 1] = '\0'; // Ensure null-termination

        state->num_tasks++;
    }

    pthread_mutex_unlock(&state->task_mutex);
}

// Function to reset the calendar for a new day
void reset_calendar(SharedState *state)
{

    pthread_mutex_lock(&state->task_mutex);

    // Reset task statuses and notifications
    for (int i = 0; i < state->num_tasks; ++i)
    {
        strncpy(state->calendar[i].status, "undone", sizeof(state->calendar[i].status) - 1);
        state->calendar[i].status[sizeof(state->calendar[i].status) - 1] = '\0'; // Ensure null-termination

        strncpy(state->calendar[i].start_notification, "not_notified", sizeof(state->calendar[i].start_notification) - 1);
        state->calendar[i].start_notification[sizeof(state->calendar[i].start_notification) - 1] = '\0'; // Ensure null-termination

        strncpy(state->calendar[i].end_notification, "not_notified", sizeof(state->calendar[i].end_notification) - 1);
        state->calendar[i].end_notification[sizeof(state->calendar[i].end_notification) - 1] = '\0'; // Ensure null-termination
    }

#ifdef DEBUG
            printf("\n\nResetting Calendar for the New Day:\n\n");
            for (int i = 0; i < state->num_tasks; ++i)
            {
                Task *task = &state->calendar[i];
                printf("Task Name: %s\n", task->name);
                printf("Start Time: %s\n", task->start_time);
                printf("End Time: %s\n", task->end_time);
                printf("Reminder Time: %s\n", task->reminder_time);
                printf("Status: %s\n", task->status);
                printf("Start Notification: %s\n", task->start_notification);
                printf("End Notification: %s\n", task->end_notification);
                printf("\n");
            }

            printf("\n\nCalendar has been reset for the new day.\n\n");
#endif // DEBUG

    pthread_mutex_unlock(&state->task_mutex);
}

#ifdef DEBUG
// Function to display time in YYYY-MM-DD HH:MM
void display_time(struct tm *tm_info)
{
    printf("DEBUG INFO: %04d-%02d-%02d %02d:%02d:%02d\n",
           tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
           tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
}
#endif

void display_task_info(SharedState *state, const char *time_str, int use_virtual_time)
{
    int input_hour, input_min;
    if (time_str)
    {
        sscanf(time_str, "%d:%d", &input_hour, &input_min);
    }
    else if (use_virtual_time)
    {
        char current_time_str[6];
        strftime(current_time_str, sizeof(current_time_str), "%H:%M", &state->virtual_tm_info);
        sscanf(current_time_str, "%d:%d", &input_hour, &input_min);
    }

    pthread_mutex_lock(&state->task_mutex);
    int found = 0;

    for (int i = 0; i < state->num_tasks; ++i)
    {
        Task *task = &state->calendar[i];

        int start_hour, start_min, end_hour, end_min;
        sscanf(task->start_time, "%d:%d", &start_hour, &start_min);
        sscanf(task->end_time, "%d:%d", &end_hour, &end_min);

        int input_total_minutes = input_hour * 60 + input_min;
        int start_total_minutes = start_hour * 60 + start_min;
        int end_total_minutes = end_hour * 60 + end_min;

        if (input_total_minutes >= start_total_minutes && input_total_minutes < end_total_minutes)
        {
            printf("Task: %s, Status: %s\n", task->name, task->status);
            // pthread_mutex_unlock(&state->task_mutex);
            sleep(DELAY_SECONDS);
            // pthread_mutex_lock(&state->task_mutex);

            if (strcmp(task->status, "undone") == 0)
            {
                if (!state->awaiting_response)
                {
                    printf("Are you doing this task now? (yes/no): ");
                    fflush(stdout);
                    state->awaiting_response = 1;
                    state->current_task = task;
                }
            }
            else if (strcmp(task->status, "done") == 0)
            {
                printf("Chill, you have already checked '%s'.\n\n", task->name);
            }
            found = 1;
            break;
        }
    }

    if (!found)
    {
        if (time_str)
        {
            printf("No task found for the entered time: %s.\n", time_str);
        }
        else if (use_virtual_time)
        {
            printf("No task found for the current virtual time.\n");
        }
    }

    pthread_mutex_unlock(&state->task_mutex);
}

// Function to notify task start
void notify_task_start(Task *task, struct tm *virtual_tm_info)
{
    printf("*********************************************************************\n");
    printf("TASK START NOTIFICATION:\n");
#ifdef DEBUG
    display_time(virtual_tm_info);
#endif // DEBUG
    printf("Task '%s' has just started at '%s'\n", task->name, task->start_time);
    printf("*********************************************************************\n\n");

    strncpy(task->start_notification, "notified", sizeof(task->start_notification) - 1);
    task->start_notification[sizeof(task->start_notification) - 1] = '\0'; // Ensure null-termination
}

// Function to notify task end
void notify_task_end(Task *task, struct tm *virtual_tm_info)
{
    printf("*********************************************************************\n");
    printf("TASK END NOTIFICATION:\n");
#ifdef DEBUG
    display_time(virtual_tm_info);
#endif // DEBUG
    printf("Task '%s' will end in 10 minutes\n", task->name);
    printf("*********************************************************************\n\n");

    strncpy(task->end_notification, "notified", sizeof(task->end_notification) - 1);
    task->end_notification[sizeof(task->end_notification) - 1] = '\0'; // Ensure null-termination
}

// Function to display task notifications based on virtual time
void display_task_notification(SharedState *state)
{
    char current_time_str[6];
    strftime(current_time_str, sizeof(current_time_str), "%H:%M", &state->virtual_tm_info);

    pthread_mutex_lock(&state->task_mutex);
    for (int i = 0; i < state->num_tasks; ++i)
    {
        Task *task = &state->calendar[i];
        // Check if it's time to notify task start
        if (strcmp(current_time_str, task->start_time) >= 0 &&
            strcmp(current_time_str, task->reminder_time) < 0 &&
            strcmp(task->start_notification, "notified") != 0)
        {
            notify_task_start(task, &state->virtual_tm_info);
            break; // Only notify once per task start
        }
        // Check if it's time to notify task end
        if (strcmp(current_time_str, task->reminder_time) >= 0 &&
            strcmp(current_time_str, task->end_time) < 0 &&
            strcmp(task->end_notification, "notified") != 0 &&
            strcmp(task->status, "undone") == 0)
        {
            notify_task_end(task, &state->virtual_tm_info);
            break; // Only notify once per task start
        }
    }
    pthread_mutex_unlock(&state->task_mutex);
}

// Function to validate time in the formats HH:MM or H:MM
int is_valid_time_format(const char *time_str)
{
    int len = strlen(time_str);
    if (len != 5 && len != 4)
        return 0; // Valid lengths are 4 or 5 characters

    // Check the colon position
    int colon_pos = -1;
    for (int i = 0; i < len; ++i)
    {
        if (time_str[i] == ':')
        {
            colon_pos = i;
            break;
        }
    }
    if (colon_pos != 2 && colon_pos != 1)
        return 0;

    // Check hour part
    for (int i = 0; i < colon_pos; ++i)
    {
        if (!isdigit(time_str[i]))
            return 0;
    }

    // Check minute part
    for (int i = colon_pos + 1; i < len; ++i)
    {
        if (!isdigit(time_str[i]))
            return 0;
    }

    // Convert hour and minute parts to integers
    int hour = 0, minute = 0;
    if (colon_pos == 1)
    {
        hour = time_str[0] - '0';
    }
    else if (colon_pos == 2)
    {
        hour = (time_str[0] - '0') * 10 + (time_str[1] - '0');
    }
    if (len - colon_pos - 1 == 1)
    {
        minute = time_str[colon_pos + 1] - '0';
    }
    else if (len - colon_pos - 1 == 2)
    {
        minute = (time_str[colon_pos + 1] - '0') * 10 + (time_str[colon_pos + 2] - '0');
    }

    // Validate hour and minute ranges
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
        return 0;

    return 1;
}

// Function for the clock thread to update virtual time
void *start_clock(void *arg)
{
    SharedState *state = (SharedState *)arg;
    while (1)
    {
        time_t current_time = time(NULL);
        double elapsed_seconds = difftime(current_time, state->program_start_time) * state->speedup_factor;
        time_t virtual_time = state->program_start_time + (time_t)elapsed_seconds;

        pthread_mutex_lock(&state->time_mutex);
        struct tm *local_tm = localtime(&virtual_time);
        memcpy(&state->virtual_tm_info, local_tm, sizeof(struct tm)); // Update shared virtual time info

        // Check for day change
        if (state->current_day != local_tm->tm_mday)
        {
            state->current_day = local_tm->tm_mday;
            reset_calendar(state);
        }

        pthread_mutex_unlock(&state->time_mutex);

        usleep(1000000 / state->speedup_factor); // Adjust the sleep based on speed-up factor
    }
    return NULL;
}

// Function for the display and interaction thread to handle display calendar and user interaction
void *display_and_interaction(void *arg)
{
    SharedState *state = (SharedState *)arg;
    while (1)
    {
        pthread_mutex_lock(&state->print_mutex);
        if (state->print_time)
        {
            state->print_time = 0;
            pthread_mutex_lock(&state->time_mutex);
#ifdef DEBUG
            display_time(&state->virtual_tm_info);
#endif // DEBUG
            display_task_info(state, NULL, 1);
            pthread_mutex_unlock(&state->time_mutex);
        }
        pthread_mutex_unlock(&state->print_mutex);
        usleep(100000); // Small delay to avoid busy-waiting
    }
    return NULL;
}

// Function to display notifications based on virtual time
void *display_notifications(void *arg)
{
    SharedState *state = (SharedState *)arg;
    while (1)
    {
        pthread_mutex_lock(&state->print_mutex);
        // Check if there's no user input pending and no response awaited
        if (!state->input_flag && !state->awaiting_response)
        {
            pthread_mutex_lock(&state->time_mutex);
            display_task_notification(state);
            pthread_mutex_unlock(&state->time_mutex);
        }
        pthread_mutex_unlock(&state->print_mutex);
        usleep(100000); // Small delay to avoid busy-waiting
    }
    return NULL;
}

// Function to handle user input
void *user_input_handle(void *arg)
{
    SharedState *state = (SharedState *)arg;
    while (1)
    {
        // Read user input from stdin
        fgets(state->input_buffer, sizeof(state->input_buffer), stdin);

        // Remove newline character from input
        state->input_buffer[strcspn(state->input_buffer, "\n")] = '\0';

        // Lock print mutex to set input flag
        pthread_mutex_lock(&state->print_mutex);

        // Set input flag to indicate presence of new user input
        state->input_flag = 1;

        // Unlock print mutex
        pthread_mutex_unlock(&state->print_mutex);
    }
    return NULL;
}

// Function to process user input
void *process_input(void *arg)
{
    SharedState *state = (SharedState *)arg;
    while (1)
    {
        // Lock print mutex to check and process input
        pthread_mutex_lock(&state->print_mutex);

        // Check if there is pending input from the user
        if (state->input_flag)
        {
            // Check if there is a response awaited from the user
            if (state->awaiting_response)
            {
                // Process user response for task completion confirmation
                if (strcmp(state->input_buffer, "yes") == 0)
                {
                    pthread_mutex_lock(&state->task_mutex);
                    // Mark current task as done
                    strncpy(state->current_task->status, "done", sizeof(state->current_task->status) - 1);
                    state->current_task->status[sizeof(state->current_task->status) - 1] = '\0'; // Ensure null-termination
                    // Clear awaiting response flag and current task pointer
                    state->current_task = NULL;
                    pthread_mutex_unlock(&state->task_mutex);
                    state->awaiting_response = 0;
                }
                else if (strcmp(state->input_buffer, "no") == 0)
                {
                    // Clear awaiting response flag and current task pointer
                    pthread_mutex_lock(&state->task_mutex);
                    state->current_task = NULL;
                    pthread_mutex_unlock(&state->task_mutex);
                    state->awaiting_response = 0;
                }
                else
                {
                    // Prompt user for valid response if input is invalid
                    printf("Invalid response. Please enter 'yes' or 'no': ");
                    fflush(stdout);
                }
                printf("\n");
            }
            else
            {
                if (strcmp(state->input_buffer, "now") == 0)
                {
                    state->print_time = 1;
                }
                else if (is_valid_time_format(state->input_buffer))
                {
                    display_task_info(state, state->input_buffer, 0);
                }
                else
                {
                    printf("Invalid input. Please enter 'now' or a time in HH:MM format:\n");
                    fflush(stdout);
                    // Set input_flag to 0 to indicate that new input is required
                    state->input_flag = 0;
                }
            }

            // Clear input flag after processing
            state->input_flag = 0;
        }

        // Unlock print mutex
        pthread_mutex_unlock(&state->print_mutex);

        // Small delay to avoid busy-waiting
        usleep(100000);
    }
    return NULL;
}