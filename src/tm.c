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

int timer(struct timespec *duration) {
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    if (duration) {
        start.tv_sec += duration->tv_sec;
        start.tv_nsec += duration->tv_nsec;
        if (start.tv_nsec > BIL) {
            start.tv_nsec -= BIL;
            start.tv_sec += 1;
        }
    }
    struct pollfd input[] = {{0, POLLIN, 0}};
    int checkpoll = !poll(input, 1, 0);
    for (;;) {
        clock_gettime(CLOCK_MONOTONIC, &now);
        time_t sec_diff = now.tv_sec - start.tv_sec;
        long int nsec_diff = now.tv_nsec - start.tv_nsec;
        if (duration) {
            sec_diff = -sec_diff;
            nsec_diff = -nsec_diff;
        }
        if (nsec_diff < 0) {
            nsec_diff += BIL;
            sec_diff -= 1;
        }
        if (sec_diff < 0 && duration) {
            printf("\x1b[G\x1b[Kcountdown finished\n");
            return 1;
        }
        time_t min_diff = sec_diff / 60; sec_diff %= 60;
        time_t hour_diff = min_diff / 60; min_diff %= 60;
        time_t day_diff = hour_diff / 24; hour_diff %= 24;
        printf("\x1b[G\x1b[K%ldd %02ld:%02ld:%02ld.%09ld",
                day_diff, hour_diff, min_diff, sec_diff, nsec_diff);
        fflush(stdout);
        if (checkpoll && poll(input, 1, POLL_MS)) {
            while (getchar() != '\n');
            break;
        }
    }
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
