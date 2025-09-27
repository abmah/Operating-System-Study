#include <windows.h>
#include <stdio.h>

#define BUFFER_SIZE 1024

int main() {
    char source[MAX_PATH], dest[MAX_PATH];
    HANDLE hSrc, hDest;
    DWORD bytesRead, bytesWritten;
    char buffer[BUFFER_SIZE];

    // Prompt user
    printf("Enter source file path: ");
    scanf("%s", source);

    printf("Enter destination file path: ");
    scanf("%s", dest);

    // Open source file
    hSrc = CreateFile(source, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSrc == INVALID_HANDLE_VALUE) {
        printf("Error opening source file: %lu\n", GetLastError());
        return 1;
    }

    // Open or create destination file
    hDest = CreateFile(dest, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDest == INVALID_HANDLE_VALUE) {
        printf("Error opening destination file: %lu\n", GetLastError());
        CloseHandle(hSrc);
        return 1;
    }

    // Copy loop
    while (ReadFile(hSrc, buffer, BUFFER_SIZE, &bytesRead, NULL) && bytesRead > 0) {
        if (!WriteFile(hDest, buffer, bytesRead, &bytesWritten, NULL) || bytesRead != bytesWritten) {
            printf("Error writing to destination file: %lu\n", GetLastError());
            CloseHandle(hSrc);
            CloseHandle(hDest);
            return 1;
        }
    }

    printf("File copied successfully!\n");

    CloseHandle(hSrc);
    CloseHandle(hDest);

    return 0;
}
