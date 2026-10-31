#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_PROCESSES 100

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    bool started;
} Process;

void print_metrics(Process p[], int n) {
    int total_wt = 0;
    int total_tat = 0;

    printf("\nPID\tArrival\tBurst\tPriority\tFinish\tTurnaround\tWaiting\tResponse\n");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < n; i++) {
        p[i].turnaround_time = p[i].completion_time - p[i].arrival_time;
        p[i].waiting_time = p[i].turnaround_time - p[i].burst_time;

        total_wt += p[i].waiting_time;
        total_tat += p[i].turnaround_time;

        printf("P%d\t%d\t%d\t%d\t\t%d\t%d\t\t%d\t%d\n",
               p[i].pid, p[i].arrival_time, p[i].burst_time, p[i].priority,
               p[i].completion_time, p[i].turnaround_time, p[i].waiting_time, p[i].response_time);
    }

    printf("--------------------------------------------------------------------------------\n");
    printf("Average Waiting Time: %.2f\n", (float)total_wt / n);
    printf("Average Turnaround Time: %.2f\n\n", (float)total_tat / n);
}

void fcfs(Process p[], int n) {
    // Sort processes by arrival time
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (p[j].arrival_time > p[j + 1].arrival_time) {
                Process temp = p[j];
                p[j] = p[j + 1];
                p[j + 1] = temp;
            }
        }
    }

    int current_time = 0;
    for (int i = 0; i < n; i++) {
        if (current_time < p[i].arrival_time) {
            current_time = p[i].arrival_time;
        }

        p[i].response_time = current_time - p[i].arrival_time;
        p[i].completion_time = current_time + p[i].burst_time;
        current_time = p[i].completion_time;
    }

    printf("\n--- First-Come, First-Served (FCFS) ---");
    print_metrics(p, n);
}

void sjf_non_preemptive(Process p[], int n) {
    int completed = 0;
    int current_time = 0;
    bool done[MAX_PROCESSES] = {false};

    while (completed < n) {
        int best_idx = -1;
        int min_burst = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && !done[i]) {
                if (p[i].burst_time < min_burst) {
                    min_burst = p[i].burst_time;
                    best_idx = i;
                } else if (p[i].burst_time == min_burst) {
                    if (p[i].arrival_time < p[best_idx].arrival_time) {
                        best_idx = i;
                    }
                }
            }
        }

        if (best_idx != -1) {
            p[best_idx].response_time = current_time - p[best_idx].arrival_time;
            p[best_idx].completion_time = current_time + p[best_idx].burst_time;
            current_time = p[best_idx].completion_time;
            done[best_idx] = true;
            completed++;
        } else {
            current_time++;
        }
    }

    printf("\n--- Shortest Job First (Non-Preemptive) ---");
    print_metrics(p, n);
}

void round_robin(Process p[], int n, int quantum) {
    int queue[MAX_PROCESSES];
    int front = 0, rear = 0;
    bool in_queue[MAX_PROCESSES] = {false};

    for (int i = 0; i < n; i++) {
        p[i].remaining_time = p[i].burst_time;
        p[i].started = false;
    }

    int min_arrival = INT_MAX;
    int first_idx = 0;
    for (int i = 0; i < n; i++) {
        if (p[i].arrival_time < min_arrival) {
            min_arrival = p[i].arrival_time;
            first_idx = i;
        }
    }

    int current_time = min_arrival;
    int completed = 0;

    queue[rear++] = first_idx;
    in_queue[first_idx] = true;

    while (front < rear) {
        int idx = queue[front++];

        if (!p[idx].started) {
            p[idx].response_time = current_time - p[idx].arrival_time;
            p[idx].started = true;
        }

        int exec_time = (p[idx].remaining_time > quantum) ? quantum : p[idx].remaining_time;
        p[idx].remaining_time -= exec_time;
        current_time += exec_time;

        for (int i = 0; i < n; i++) {
            if (i != idx && p[i].arrival_time <= current_time && p[i].remaining_time > 0 && !in_queue[i]) {
                queue[rear++] = i;
                in_queue[i] = true;
            }
        }

        if (p[idx].remaining_time > 0) {
            queue[rear++] = idx;
        } else {
            p[idx].completion_time = current_time;
            completed++;
        }

        if (front == rear && completed < n) {
            for (int i = 0; i < n; i++) {
                if (p[i].remaining_time > 0) {
                    current_time = p[i].arrival_time;
                    queue[rear++] = i;
                    in_queue[i] = true;
                    break;
                }
            }
        }
    }

    printf("\n--- Round Robin (Quantum = %d) ---", quantum);
    print_metrics(p, n);
}

void priority_preemptive(Process p[], int n) {
    int completed = 0;
    int current_time = 0;

    for (int i = 0; i < n; i++) {
        p[i].remaining_time = p[i].burst_time;
        p[i].started = false;
    }

    while (completed < n) {
        int best_idx = -1;
        int highest_priority = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && p[i].remaining_time > 0) {
                if (p[i].priority < highest_priority) {
                    highest_priority = p[i].priority;
                    best_idx = i;
                } else if (p[i].priority == highest_priority) {
                    if (p[i].arrival_time < p[best_idx].arrival_time) {
                        best_idx = i;
                    }
                }
            }
        }

        if (best_idx != -1) {
            if (!p[best_idx].started) {
                p[best_idx].response_time = current_time - p[best_idx].arrival_time;
                p[best_idx].started = true;
            }

            p[best_idx].remaining_time--;
            current_time++;

            if (p[best_idx].remaining_time == 0) {
                p[best_idx].completion_time = current_time;
                completed++;
            }
        } else {
            current_time++;
        }
    }

    printf("\n--- Preemptive Priority Scheduling ---");
    print_metrics(p, n);
}

int main() {
    int n, quantum;
    Process processes[MAX_PROCESSES];
    Process temp_p[MAX_PROCESSES];

    printf("Enter number of processes: ");
    if (scanf("%d", &n) != 1 || n <= 0 || n > MAX_PROCESSES) {
        printf("Invalid number of processes.\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        printf("\n--- Process P%d ---\n", i + 1);
        printf("Arrival Time: ");
        scanf("%d", &processes[i].arrival_time);
        printf("Burst Time: ");
        scanf("%d", &processes[i].burst_time);
        printf("Priority (Lower = Higher Priority): ");
        scanf("%d", &processes[i].priority);
    }

    printf("\nEnter Time Quantum for Round Robin: ");
    scanf("%d", &quantum);

    // Run FCFS
    for (int i = 0; i < n; i++) temp_p[i] = processes[i];
    fcfs(temp_p, n);

    // Run Non-Preemptive SJF
    for (int i = 0; i < n; i++) temp_p[i] = processes[i];
    sjf_non_preemptive(temp_p, n);

    // Run Round Robin
    for (int i = 0; i < n; i++) temp_p[i] = processes[i];
    round_robin(temp_p, n, quantum);

    // Run Preemptive Priority
    for (int i = 0; i < n; i++) temp_p[i] = processes[i];
    priority_preemptive(temp_p, n);

    return 0;
}