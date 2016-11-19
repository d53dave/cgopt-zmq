//
// Created by dotdi on 18.11.16.
//

#pragma once

#include <cstdio>
#include <sys/sysinfo.h>
#include <cstring>
#include <cstdlib>
#include <bits/shared_ptr.h>
#include <spdlog/spdlog.h>

#ifdef __linux__

#include "sys/types.h"
#include "sys/sysinfo.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sys/times.h"

#endif

typedef struct {
    long long totalVirtualMemory;
    long long usedVirtualMemory;
    long long usedVirtualMemoryUsedByMe;
    long long totalPhysicalMemory;
    long long usedPhysicalMemoryUsedByMe;
    double usedCPU;
    double usedCPUbyMe;
    long queueSizeTidings;
    long numWorkers;
} Stats;

class StatsGatherer {
public:
    StatsGatherer() {
        initStats();
        this->logger = spdlog::get("csaopt-zmq-logger");
    }

    Stats computeStats() {
        Stats stats{-1, -1, -1, -1, -1, -1.0, -1.0, -1, -1};
        struct sysinfo memInfo;
        int error = sysinfo(&memInfo);

        if(error != 0) {
            logger->error("Could not get stats: {}", std::strerror(error));
            return stats;
        }

        getTotalVirtualMemory(memInfo, stats);
        getTotalPhysicalMemory(memInfo, stats);
        getUsedPhysicalMemory(memInfo, stats);
        getUsedVirtualMemory(memInfo, stats);
        getOwnUsedVirtualMemory(memInfo, stats);
        getUsedVirtualMemory(memInfo, stats);
        getCPULoad(stats);
        getCPULoadByMe(stats);
    }

private:
    clock_t lastCPU, lastSysCPU, lastUserCPU;
    int numProcessors;
    unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;

    std::shared_ptr<spdlog::logger> logger;

    void initStats() {
        FILE *file;
        struct tms timeSample;
        char line[128];

        lastCPU = times(&timeSample);
        lastSysCPU = timeSample.tms_stime;
        lastUserCPU = timeSample.tms_utime;

        file = fopen("/proc/cpuinfo", "r");
        numProcessors = 0;
        while (fgets(line, 128, file) != NULL) {
            if (strncmp(line, "processor", 9) == 0) numProcessors++;
        }
        fclose(file);
    }

    void getTotalVirtualMemory(struct sysinfo &memInfo, Stats &stats) {
#ifdef __linux__
        sysinfo(&memInfo);
        unsigned long long totalVirtualMem = memInfo.totalram;
        totalVirtualMem += memInfo.totalswap;
        totalVirtualMem *= memInfo.mem_unit;
        stats.totalVirtualMemory = totalVirtualMem;
#endif
    }

    void getTotalPhysicalMemory(struct sysinfo &memInfo, Stats &stats) {
#ifdef __linux__

        unsigned long long totalPhysMem = memInfo.totalram;
        totalPhysMem *= memInfo.mem_unit;
        stats.totalPhysicalMemory = totalPhysMem;
#endif
    }

    void getUsedVirtualMemory(struct sysinfo &memInfo, Stats &stats) {
#ifdef __linux__
        unsigned long long virtualMemUsed = memInfo.totalram - memInfo.freeram;
        //Add other values in next statement to avoid int overflow on right hand side...
        virtualMemUsed += memInfo.totalswap - memInfo.freeswap;
        virtualMemUsed *= memInfo.mem_unit;
        stats.usedVirtualMemory = virtualMemUsed;
#endif
    }

    void getUsedPhysicalMemory(struct sysinfo &memInfo, Stats &stats) {
#ifdef __linux__
        unsigned long long physMemUsed = memInfo.totalram - memInfo.freeram;
        physMemUsed *= memInfo.mem_unit;
#endif
    }

    void getOwnUsedVirtualMemory(struct sysinfo &memInfo, Stats &stats) {
#ifdef __linux__
        FILE *file = fopen("/proc/self/status", "r");
        int result = -1;
        char line[128];

        while (fgets(line, 128, file) != NULL) {
            if (strncmp(line, "VmSize:", 7) == 0) {
                size_t i = strlen(line);
                const char *p = line;
                while (*p < '0' || *p > '9') {
                    p++;
                }
                line[i - 3] = '\0';
                result = atoi(p);
                break;
            }
        }
        fclose(file);
        stats.usedVirtualMemoryUsedByMe = result;
#endif
    }

    void getOwnUsedVirtualMemory2(struct sysinfo &memInfo, Stats &stats) {
#ifdef __linux__
        FILE *file = fopen("/proc/self/status", "r");
        int result = -1;
        char line[128];

        while (fgets(line, 128, file) != NULL) {
            if (strncmp(line, "VmSize:", 7) == 0) {
                size_t i = strlen(line);
                const char *p = line;
                while (*p < '0' || *p > '9') {
                    p++;
                }
                line[i - 3] = '\0';
                result = atoi(p);
                break;
            }
        }
        fclose(file);
        stats.usedVirtualMemoryUsedByMe = result;
#endif
    }


    void getCPULoad(Stats &stats) {
#ifdef __linux__
        struct tms timeSample;
        clock_t now;
        double percent;

        now = times(&timeSample);
        if (now <= lastCPU || timeSample.tms_stime < lastSysCPU ||
            timeSample.tms_utime < lastUserCPU) {
            //Overflow detection. Just skip this value.
            percent = -1.0;
        }
        else {
            percent = (timeSample.tms_stime - lastSysCPU) +
                      (timeSample.tms_utime - lastUserCPU);
            percent /= (now - lastCPU);
            percent /= numProcessors;
            percent *= 100;
        }
        lastCPU = now;
        lastSysCPU = timeSample.tms_stime;
        lastUserCPU = timeSample.tms_utime;

        stats.usedCPU = percent;
#endif
    }


    void getCPULoadByMe(Stats &stats) {
#ifdef __linux__
        double percent;
        FILE *file;
        unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

        file = fopen("/proc/stat", "r");
        fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,
               &totalSys, &totalIdle);
        fclose(file);

        if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
            totalSys < lastTotalSys || totalIdle < lastTotalIdle) {
            //Overflow detection. Just skip this value.
            percent = -1.0;
        }
        else {
            total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
                    (totalSys - lastTotalSys);
            percent = total;
            total += (totalIdle - lastTotalIdle);
            percent /= total;
            percent *= 100;
        }

        lastTotalUser = totalUser;
        lastTotalUserLow = totalUserLow;
        lastTotalSys = totalSys;
        lastTotalIdle = totalIdle;

        stats.usedCPUbyMe = percent;
#endif
    }
};