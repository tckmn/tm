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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "tm.h"

int main(int argc, char* argv[]) {
    printf("\x1b[?25l");

    struct timespec duration = { 0 };
    long int lastval = -1;
    int unit_val = 0;
    long int unitless_vals[10] = { 0 };
    int unitless_idx = 0;

    for (int i = 1; i < argc; ++i) {
        for (char *c = argv[i]; *c;) {
            if (*c >= '0' && *c <= '9') {
                lastval = strtol(c, &c, 10);
                unitless_vals[unitless_idx++] = lastval;
            } else {
                if (unitless_idx > 1) {
                    fputs("cannot mix units and unitless values\n", stderr);
                    printf("\x1b[?25h");
                    return 1;
                }
                unit_val = 1;
                unitless_idx = 0;
                time_t sec = -1;
                long nsec = -1;
                switch (*c) {
                    case 'n': nsec = 1; break;
                    case 's': sec = 1; break;
                    case 'm': if (c[1] == 's') nsec = 1000000;
                              else if (c[1] == 'o') sec = 60*60*24*30;
                              else sec = 60;
                              break;
                    case 'h': sec = 60*60; break;
                    case 'd': sec = 60*60*24; break;
                    case 'y': sec = 60*60*24*365; break;
                }
                if (sec != -1) {
                    duration.tv_sec += sec * lastval;
                } else if (nsec != -1) {
                    duration.tv_nsec += nsec * lastval;
                    duration.tv_sec += duration.tv_nsec / BIL;
                    duration.tv_nsec %= BIL;
                }
                lastval = -1;
                while (*c >= 'a' && *c <= 'z') ++c;
            }
        }
    }

    if (lastval != -1) {
        if (unit_val) {
            fputs("cannot mix units and unitless values\n", stderr);
            printf("\x1b[?25h");
            return 1;
        }
        time_t total = 0;
        time_t mults[] = { 60, 60, 24, 30, 12 };
        for (int i = unitless_idx - 1; i >= 0; --i) {
            total += unitless_vals[i];
            for (int j = i - 1; j >= 0; --j) {
                unitless_vals[j] *= mults[unitless_idx - 1 - i];
            }
        }
        duration.tv_sec = total;
        duration.tv_nsec = 0;
        countdown(duration);
    } else if (unit_val) {
        countdown(duration);
    } else {
        stopwatch();
    }

    printf("\x1b[?25h");
    return 0;
}
