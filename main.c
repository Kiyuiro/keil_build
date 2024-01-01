#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

// 函数声明
char *searchFiles(const char *path, const char *extension);

void printHelp(void);

int main(int argc, char *argv[]) {

    if (argc < 2 || strcmp(argv[1], "help") == 0) {
        printHelp();
        return 0;
    }

    char *UV4Path = NULL;
    char *BoardType = NULL;
    char *HexPath = NULL;

    char isShowGui = 0;
    char isRebuild = 0;

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-show") == 0) {
            isShowGui = 1;
            i -= 1;
            continue;
        }
        if (strcmp(argv[i], "-r") == 0) {
            isRebuild = 1;
            i -= 1;
            continue;
        }
        if (argv[i + 1] == NULL) {
            printHelp();
            exit(EXIT_FAILURE);
        }
        if (strcmp(argv[i], "-u") == 0) {
            UV4Path = argv[i + 1];
        } else if (strcmp(argv[i], "-b") == 0) {
            BoardType = argv[i + 1];
        } else if (strcmp(argv[i], "-h") == 0) {
            HexPath = argv[i + 1];
        }
    }

    char currentPath[1024];
    char command[1024];

    // 获取当前工作目录
    if (getcwd(currentPath, sizeof(currentPath)) != NULL) {
        const char *targetExtension = ".uvprojx"; // 替换为你想查找的扩展名
        char *foundFilePath = searchFiles(currentPath, targetExtension);
        if (foundFilePath == NULL) {
            free(foundFilePath);  // 释放动态分配的内存
            printf("not found .uvprojx file\n");
            exit(EXIT_FAILURE);
        }

        remove("build_log.txt");  // 删除旧的 build_log.txt 文件

        const char *buildType = isRebuild ? "-r" : "-b";
        const char *guiOption = isShowGui ? "" : "-j0";
        sprintf(command, "%s %s %s \"%s\" -o build_log.txt", UV4Path, guiOption, buildType, foundFilePath);

        system(command);
        free(foundFilePath);  // 释放动态分配的内存

        int count = 0;
        // 循环等待 build_log.txt 文件出现
        while (access("build_log.txt", F_OK) == -1) {
            count++;
            usleep(100000); // 100毫秒
            if (count > 300) {
                printf("build timeout\n");
                exit(EXIT_FAILURE);
            }
        }

        // 打开文件并打印内容
        FILE *file = fopen("build_log.txt", "r");
        if (file != NULL) {
            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), file) != NULL) {
                printf("%s", buffer);
            }
            fclose(file);
        } else {
            perror("Error opening build_log.txt");
            exit(EXIT_FAILURE);
        }

        sprintf(command, "pyocd flash --target %s %s", BoardType, HexPath);
        system(command);
        return 0;
    } else {
        perror("error");
        exit(EXIT_FAILURE);
    }

    return 0;
}

// 遍历文件夹并查找特定扩展名的文件
char *searchFiles(const char *path, const char *extension) {
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;

    // 打开目录
    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    // 遍历目录
    while ((entry = readdir(dir)) != NULL) {
        // 获取文件信息
        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s\\%s", path, entry->d_name);

        // 使用 stat 获取文件信息
        if (stat(filePath, &fileStat) == -1) {
            perror("stat");
            closedir(dir);
            exit(EXIT_FAILURE);
        }

        // 检查是否为普通文件
        if (S_ISREG(fileStat.st_mode)) {
            // 检查文件名是否以指定扩展名结尾
            if (strstr(entry->d_name, extension) != NULL) {
                // 找到匹配的文件
                closedir(dir);

                // 动态分配内存保存文件路径
                char *foundFilePath = malloc(strlen(filePath) + 1);
                strcpy(foundFilePath, filePath);
                return foundFilePath;
            }
        }
    }

    closedir(dir);
    return NULL;  // 没有找到匹配的文件
}

void printHelp(void) {
    printf("Options:\n");
    printf("  -u [UV4Path]    : Path to UV4 executable\n");
    printf("  -b [BoardType]  : Board type\n");
    printf("  -h [HexPath]    : Path to HEX file\n");
    printf("  -show           : Show UV4.exe GUI\n");
    printf("  -r              : Enable Rebuild\n");
}
