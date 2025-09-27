#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main() {
    char source[256], dest[256];
    int src_fd, dest_fd;
    ssize_t bytes_read, bytes_written;
    char buffer[BUFFER_SIZE];

    // Prompt user
    printf("Enter source file path: ");
    scanf("%255s", source);

    printf("Enter destination file path: ");
    scanf("%255s", dest);

    // Open source file
    src_fd = open(source, O_RDONLY);
    if (src_fd < 0) {
        perror("Error opening source file");
        return 1;
    }

    // Open destination file (create if not exists, overwrite if exists)
    dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0) {
        perror("Error opening destination file");
        close(src_fd);
        return 1;
    }

    // Copy loop
    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("Error writing to file");
            close(src_fd);
            close(dest_fd);
            return 1;
        }
    }

    if (bytes_read < 0) {
        perror("Error reading file");
    }

    printf("File copied successfully!\n");

    close(src_fd);
    close(dest_fd);

    return 0;
}
