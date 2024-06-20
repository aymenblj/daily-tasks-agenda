#include "agenda.h"

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

    // Initialize the current day
    struct tm *initial_tm = localtime(&state.program_start_time);
    state.current_day = initial_tm->tm_mday;

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
    add_task(&state, "Watch TV", "19:30", "20:00");
    add_task(&state, "Relax and prepare for bed", "21:30", "22:00");
    add_task(&state, "Sleep", "22:00", "23:59");

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
