/*
 * tm - a text-based stopwatch/countdown timer tool
 * Copyright (C) 2017  Keyboard Fire <andy@keyboardfire.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <poll.h>
#include <time.h>
#include <libnotify/notify.h>

#include "tm.h"

#define POLL_MS 10

struct timespec start, now;
pthread_t input_handler;
int pause_loop = 0;
int get_input = 1;
int run_loop = 1;

// turn this into a state machine
void *manage_input(void* ignored) {
    struct pollfd input[] = {{0, POLLIN, 0}};
    struct timespec offset_start, offset_end;
    while (get_input) {
        if (poll(input, 1, POLL_MS)) {
            int c;
            c = getchar();
            if (c == '\n' || c == ' ') {
                if (pause_loop) {
                    pause_loop = 0;
                    clock_gettime(CLOCK_MONOTONIC, &offset_end);
                    start.tv_sec += offset_end.tv_sec - offset_start.tv_sec;
                    start.tv_nsec += offset_end.tv_nsec - offset_start.tv_nsec;
                } else {
                    pause_loop = 1;
                    clock_gettime(CLOCK_MONOTONIC, &offset_start);
                }
            } else if (c == 'q') {
                run_loop = 0;
            } else if (c == 'r') {
                clock_gettime(CLOCK_MONOTONIC, &start);
                offset_start= start;
            }
        }
    }
}

void print_line(char* s) {
    printf("\r                                 \r%s", s);
}

void end_thread() {
    get_input = 0;
    pthread_join(input_handler, NULL);
}

int timer(struct timespec *duration) {
    pthread_create(&input_handler, NULL, manage_input, NULL);
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (run_loop) {
        if (pause_loop) {
            goto cont;
        }
        clock_gettime(CLOCK_MONOTONIC, &now);
        time_t sec_diff = now.tv_sec - start.tv_sec - duration->tv_sec;
        long int nsec_diff = now.tv_nsec - start.tv_nsec - duration->tv_nsec;
        if (duration) {
            sec_diff = -sec_diff;
            nsec_diff = -nsec_diff;
        }
        while (nsec_diff < 0) {
            nsec_diff += BIL;
            sec_diff -= 1;
        }
        if (sec_diff < 0 && duration) {
            print_line("countdown finished\n");
            end_thread();
            return 1;
        }
        time_t min_diff = sec_diff / 60; sec_diff %= 60;
        time_t hour_diff = min_diff / 60; min_diff %= 60;
        time_t day_diff = hour_diff / 24; hour_diff %= 24;
        char time[30];
        sprintf(time, "%ldd %02ld:%02ld:%02ld.%09ld",
                day_diff, hour_diff, min_diff, sec_diff, nsec_diff);
        print_line(time);
        fflush(stdout);
    cont:
        usleep(POLL_MS*1000);
    }
    end_thread();
    return 0;
}

void stopwatch() {
    timer(NULL);
}

void countdown(struct timespec duration) {
    if (timer(&duration)) {
        notify_init("tm");
        NotifyNotification *notif = notify_notification_new("tm - countdown",
                "countdown has finished", "dialog-information");
        notify_notification_show(notif, NULL);
        g_object_unref(G_OBJECT(notif));
        notify_uninit();
    }
}
