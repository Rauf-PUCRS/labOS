#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/file.h>


void read_kernel_version(FILE *file);
void read_uptime(FILE *file);

int main() {
    create_html_page();
}

void create_html_page() {
    FILE *html = fopen("index.html", "w");
    if (!html) {
        perror("Couldn't open index.html, just returning");
        return;
    }

    fprintf(html, "<!DOCTYPE html>\n<html>\n<head>\n<title>Data</title>\n");
    fprintf(html, "<meta http-equiv=\"refresh\" content=\"5\">\n</head>\n<body>\n");
    fprintf(html, "<h1>Data</h1>\n");

    read_kernel_version(html);
    read_uptime(html);

    fprintf(html, "</body>\n</html>\n");
    
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
