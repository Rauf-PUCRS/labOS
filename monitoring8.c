#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/file.h>


void read_kernel_version(FILE *file);
void read_uptime(FILE *file);
void read_cpu_info(FILE *file);
void read_memory_info(FILE *file);
void read_load_avg(FILE *file);
void read_cpu_usage(FILE *file);
void read_disk_io(FILE *file);
void read_filesystems(FILE *file);
void read_running_processes(FILE *file);
void update_html();
void read_idle_time(FILE *file);
void read_system_datetime(FILE *file);
void read_devices(FILE *file);
void read_network_devices(FILE *file);

int main() {
    while (1) { 
        update_html();
        sleep(5);
    }
    return 0;
}

void update_html() {
    FILE *html = fopen("index.html", "w");
    if (!html) {
        perror("Couldn't open index.html, just returning");
        return;
    }

    int fd = fileno(html);
    if (flock(fd, LOCK_EX) != 0) {
        perror("Couldn't lock index.html, just returning");
        fclose(html);
        return;
    }

    fprintf(html, "<!DOCTYPE html>\n<html>\n<head>\n<title>Data</title>\n");
    fprintf(html, "<meta http-equiv=\"refresh\" content=\"5\">\n</head>\n<body>\n");
    fprintf(html, "<h1>Data</h1>\n");

    read_kernel_version(html);
    read_uptime(html);
    read_cpu_info(html);
    read_memory_info(html);
    read_load_avg(html);
    read_cpu_usage(html);
    read_disk_io(html);
    read_filesystems(html);
    read_running_processes(html);
    read_idle_time(html);
    read_system_datetime(html);
    read_devices(html);
    read_network_devices(html);

    fprintf(html, "</body>\n</html>\n");

    flock(fd, LOCK_UN);
    fclose(html);

    printf("All good!\n");
}

void read_kernel_version(FILE *file) {
    FILE *proc = fopen("/proc/version", "r"); // CAN'T I JUST USE THIS PATH DIRECTLY?
    if (!proc) return;
    char buffer[256];
    fgets(buffer, sizeof(buffer), proc);
    fprintf(file, "<p><strong>Kernel Version:</strong> %s</p>\n", buffer);
    fclose(proc);
}

void read_uptime(FILE *file) {
    FILE *proc = fopen("/proc/uptime", "r");
    if (!proc) return;
    double uptime;
    fscanf(proc, "%lf", &uptime);
    fclose(proc);

    int days = (int)(uptime / 86400);
    int hours = (int)((uptime / 3600) % 24);
    int minutes = (int)((uptime / 60) % 60);
    int seconds = (int)uptime % 60;
    
    fprintf(file, "<p><strong>Uptime:</strong> %d days, %d hours, %d minutes, %d seconds</p>\n", 
            days, hours, minutes, seconds);
}

void read_cpu_info(FILE *file) {
    FILE *proc = fopen("/proc/cpuinfo", "r");
    if (!proc) return;
    char buffer[256], model[256] = "Unknown";
    int cores = 0;

    while (fgets(buffer, sizeof(buffer), proc)) {
        if (strncmp(buffer, "model name", 10) == 0) {
            strcpy(model, strchr(buffer, ':') + 2);
            model[strcspn(model, "\n")] = 0; // rEMOVES ANY NEW LINES
        }
        if (strncmp(buffer, "processor", 9) == 0) {
            cores++;
        }
    }
    fclose(proc);
    
    fprintf(file, "<p><strong>CPU model:</strong> %s</p>\n", model);
    fprintf(file, "<p><strong>CPU n. cores:</strong> %d</p>\n", cores);
}

void read_memory_info(FILE *file) {
    FILE *proc = fopen("/proc/meminfo", "r");
    if (!proc) return;
    char buffer[256];
    unsigned long total_mem = 0, available_mem = 0;

    while (fgets(buffer, sizeof(buffer), proc)) {
        if (strncmp(buffer, "MemTotal", 8) == 0) {
            sscanf(buffer, "MemTotal: %lu kB", &total_mem);
        }
        if (strncmp(buffer, "MemAvailable", 12) == 0) {
            sscanf(buffer, "MemAvailable: %lu kB", &available_mem);
        }
    }
    fclose(proc);

    fprintf(file, "<p><strong>Memory total:</strong> %lu MB</p>\n", total_mem / 1024);
    fprintf(file, "<p><strong>Memory available:</strong> %lu MB</p>\n", available_mem / 1024);
}

void read_load_avg(FILE *file) {
    FILE *proc = fopen("/proc/loadavg", "r");
    if (!proc) return;
    double load1, load5, load15;
    fscanf(proc, "%lf %lf %lf", &load1, &load5, &load15);
    fclose(proc);
    
    fprintf(file, "<p><strong>Average load:</strong> %.2f (1 min), %.2f (5 min), %.2f (15 min)</p>\n",
            load1, load5, load15);
}

void read_cpu_usage(FILE *file) {
    FILE *proc = fopen("/proc/stat", "r");
    if (!proc) return;
    
    char buffer[256];
    unsigned long user, nice, system, idle;
    
    fgets(buffer, sizeof(buffer), proc);
    sscanf(buffer, "cpu %lu %lu %lu %lu", &user, &nice, &system, &idle);
    fclose(proc);
    
    unsigned long total = user + nice + system + idle;
    double usage = ((double)(user + nice + system) / total) * 100.0;
    
    fprintf(file, "<p><strong>CPU usage:</strong> %.2f%%</p>\n", usage);
}

void read_disk_io(FILE *file) {
    FILE *proc = fopen("/proc/diskstats", "r");
    if (!proc) return;
    
    char buffer[256], device[32];
    unsigned long reads, writes;
    
    fprintf(file, "<h2>Disk I/O Operations</h2>\n<ul>\n");
    while (fgets(buffer, sizeof(buffer), proc)) {
        if (sscanf(buffer, " %*d %*d %s %*d %*d %lu %*d %*d %*d %lu", device, &reads, &writes) == 3) {
            fprintf(file, "<li>%s - Reads: %lu, Writes: %lu</li>\n", device, reads, writes);
        }
    }
    fprintf(file, "</ul>\n");
    fclose(proc);
}

void read_filesystems(FILE *file) {
    FILE *proc = fopen("/proc/filesystems", "r");
    if (!proc) return;
    
    char buffer[256];
    fprintf(file, "<h2>Supported Filesystems</h2>\n<ul>\n");
    while (fgets(buffer, sizeof(buffer), proc)) {
        fprintf(file, "<li>%s</li>\n", buffer);
    }
    fprintf(file, "</ul>\n");
    fclose(proc);
}

void read_running_processes(FILE *file) {
    DIR *dir = opendir("/proc");
    if (!dir) return;
    
    struct dirent *entry;
    fprintf(file, "<h2>Running Processes</h2>\n<ul>\n");
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strspn(entry->d_name, "0123456789") == strlen(entry->d_name)) {
            char path[256];
            snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);
            
            FILE *proc = fopen(path, "r");
            if (proc) {
                char process_name[256];
                if (fgets(process_name, sizeof(process_name), proc)) {
                    process_name[strcspn(process_name, "\n")] = 0;
                    fprintf(file, "<li>PID: %s - %s</li>\n", entry->d_name, process_name);
                }
                fclose(proc);
            }
        }
    }
    fprintf(file, "</ul>\n");
    closedir(dir);
}

void read_idle_time(FILE *file) {
    FILE *proc = fopen("/proc/uptime", "r");
    if (!proc) return;
    double uptime, idle;
    fscanf(proc, "%lf %lf", &uptime, &idle);
    fclose(proc);

    int days = (int)(idle / 86400);
    int hours = (int)((idle / 3600) % 24);
    int minutes = (int)((idle / 60) % 60);
    int seconds = (int)idle % 60;

    fprintf(file, "<p><strong>Idle Time:</strong> %d days, %d hours, %d minutes, %d seconds</p>\n", 
            days, hours, minutes, seconds);
}

void read_system_datetime(FILE *file) {
    FILE *proc = fopen("/proc/driver/rtc", "r");
    if (!proc) return;
    char buffer[256], date[20] = "", time[20] = "";

    while (fgets(buffer, sizeof(buffer), proc)) {
        if (strncmp(buffer, "rtc_date", 8) == 0) {
            sscanf(buffer, "rtc_date : %s", date);
        }
        if (strncmp(buffer, "rtc_time", 8) == 0) {
            sscanf(buffer, "rtc_time : %s", time);
        }
    }
    fclose(proc);

    fprintf(file, "<p><strong>System Date & Time:</strong> %s %s</p>\n", date, time);
}

void read_devices(FILE *file) {
    FILE *proc = fopen("/proc/devices", "r");
    if (!proc) return;
    char buffer[256];

    fprintf(file, "<h2>Devices (Character & Block)</h2>\n<ul>\n");
    while (fgets(buffer, sizeof(buffer), proc)) {
        fprintf(file, "<li>%s</li>\n", buffer);
    }
    fprintf(file, "</ul>\n");
    fclose(proc);
}

void read_network_devices(FILE *file) {
    FILE *proc = fopen("/proc/net/dev", "r");
    if (!proc) return;
    char buffer[256];

    fprintf(file, "<h2>Network Devices</h2>\n<table border='1'>\n<tr><th>Interface</th><th>Received Bytes</th><th>Transmitted Bytes</th></tr>\n");
    fgets(buffer, sizeof(buffer), proc); // Skip header line
    fgets(buffer, sizeof(buffer), proc); // Skip header line

    while (fgets(buffer, sizeof(buffer), proc)) {
        char iface[32];
        unsigned long rx_bytes, tx_bytes;
        sscanf(buffer, " %[^:]: %lu %*d %*d %*d %*d %*d %*d %*d %*d %lu", iface, &rx_bytes, &tx_bytes);
        fprintf(file, "<tr><td>%s</td><td>%lu</td><td>%lu</td></tr>\n", iface, rx_bytes, tx_bytes);
    }
    fprintf(file, "</table>\n");
    fclose(proc);
}
