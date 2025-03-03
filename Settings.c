#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

#define SHARED_MEM_NAME "/stMgr"
#define SHARED_MEM_SIZE 8192

typedef enum {
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_CHAR,
    TYPE_INT32,
    TYPE_INT64
} dataType;

typedef union {
    bool boolVal;
    char stringVal[32];
    char charVal;
    int32_t int32Val;
    int64_t int64Val;
} value;

typedef struct {
    char key[32];
    dataType type;
    value value;
} setting;

typedef struct {
    int count;
    setting settings[128];
} settingsManager;

void setSetting(settingsManager *manager, const char *key, dataType type, value value) {
    for (int i = 0; i < manager->count; i++) {
        if (strcmp(manager->settings[i].key, key) == 0) {
            manager->settings[i].type = type;
            manager->settings[i].value = value;
            return;
        }
    }
    if (manager->count < 128) {
        strncpy(manager->settings[manager->count].key, key, sizeof(manager->settings[manager->count].key) - 1);
        manager->settings[manager->count].key[sizeof(manager->settings[manager->count].key) - 1] = '\0';
        manager->settings[manager->count].type = type;
        manager->settings[manager->count].value = value;
        manager->count++;
    } else {
        fprintf(stderr, "Settings storage full!\n");
    }
}

void getSetting(settingsManager *manager, const char *key) {
    for (int i = 0; i < manager->count; i++) {
        if (strcmp(manager->settings[i].key, key) == 0) {
            printf("%s: ", key);
            switch (manager->settings[i].type) {
                case TYPE_BOOL:
                    printf("%s\n", manager->settings[i].value.boolVal ? "true" : "false");
                    break;
                case TYPE_STRING:
                    printf("%s\n", manager->settings[i].value.stringVal);
                    break;
                case TYPE_CHAR:
                    printf("%c\n", manager->settings[i].value.charVal);
                    break;
                case TYPE_INT32:
                    printf("%d\n", manager->settings[i].value.int32Val);
                    break;
                case TYPE_INT64:
                    printf("%lld\n", (long long)manager->settings[i].value.int64Val);
                    break;
                default:
                    printf("Unknown type\n");
            }
            return;
        }
    }
    printf("%s not set\n", key);
}

// Main function
int main(int argc, char *argv[]) {
    int fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        return -1; // shm open fail.
    }
    ftruncate(fd, SHARED_MEM_SIZE);

    settingsManager *manager = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (manager == MAP_FAILED) {
        return -2; // mmap failed
    }
    if (manager->count == 0) {
        manager->count = 0;
    }

    if (argc > 3 && strcmp(argv[1], "Set") == 0) {
        value value;
        dataType type;

        if (strcmp(argv[3], "bool") == 0) {
            type = TYPE_BOOL;
            value.boolVal = (strcmp(argv[4], "true") == 0);
        } else if (strcmp(argv[3], "string") == 0) {
            type = TYPE_STRING;
            strncpy(value.stringVal, argv[4], sizeof(value.stringVal) - 1);
            value.stringVal[sizeof(value.stringVal) - 1] = '\0';
        } else if (strcmp(argv[3], "char") == 0) {
            type = TYPE_CHAR;
            value.charVal = argv[4][0];
        } else if (strcmp(argv[3], "int32") == 0) {
            type = TYPE_INT32;
            value.int32Val = atoi(argv[4]);
        } else if (strcmp(argv[3], "int64") == 0) {
            type = TYPE_INT64;
            value.int64Val = atoll(argv[4]);
        } else {
            fprintf(stderr, "Invalid type. Use: bool, string, char, int32, or int64.\n");
            return -3;
        }

        setSetting(manager, argv[2], type, value);

    } else if (argc > 2 && strcmp(argv[1], "Get") == 0) {
        getSetting(manager, argv[2]);
    } else {
        fprintf(stderr, "Usage: %s <Set|Get> <key> [type] [value]\n", argv[0]);
    }

    munmap(manager, SHARED_MEM_SIZE);
    close(fd);
    return 0;
}
