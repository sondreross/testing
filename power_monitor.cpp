#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

// Define the specific unusual ASCII value to react to
#define UNUSUAL_ASCII 0x1B  // ESC character (27)

pid_t child_pid = -1;

void handle_signal(int sig) {
    if (child_pid > 0) {
        kill(child_pid, SIGTERM);
        waitpid(child_pid, NULL, 0);
    }
}

int main(int argc, char *argv[]) {
    int fd;
    struct termios options;
    unsigned char buffer[256];
    int buf_index = 0;
    int capturing = 0;  // Flag to indicate we're capturing after 0x1B
    ssize_t n;
    char benchmark_name[128];
    int repetition = -1;
    int run_counter = 0;
    
    // Check command line arguments
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <serial_port> <external_binary> [args...]\n", argv[0]);
        fprintf(stderr, "Example: %s /dev/ttyS0 ./my_benchmark arg1 arg2\n", argv[0]);
        return 1;
    }
    
    // Open serial port
    fd = open(argv[1], O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("Error opening serial port");
        return 1;
    }
    
    // Get current serial port settings
    if (tcgetattr(fd, &options) < 0) {
        perror("Error getting serial port attributes");
        close(fd);
        return 1;
    }
    
    // Set baud rate to 38400
    cfsetispeed(&options, B38400);
    cfsetospeed(&options, B38400);
    
    // Configure: 8N1 (8 data bits, no parity, 1 stop bit)
    options.c_cflag &= ~PARENB;        // No parity
    options.c_cflag &= ~CSTOPB;        // 1 stop bit
    options.c_cflag &= ~CSIZE;         // Clear data size bits
    options.c_cflag |= CS8;            // 8 data bits
    options.c_cflag |= (CLOCAL | CREAD); // Enable receiver, ignore modem control lines
    
    // Disable hardware flow control
    options.c_cflag &= ~CRTSCTS;
    
    // Raw input mode
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    
    // Raw output mode
    options.c_oflag &= ~OPOST;
    
    // Disable software flow control
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    
    // Set read timeout (deciseconds) and minimum characters
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10;  // 1 second timeout
    
    // Apply settings
    if (tcsetattr(fd, TCSANOW, &options) < 0) {
        perror("Error setting serial port attributes");
        close(fd);
        return 1;
    }
    
    // Flush any existing data
    tcflush(fd, TCIOFLUSH);
    
    // Set up signal handler for clean termination
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Open log file for unikernel output
    FILE *unikernel_log = fopen("unikernel_output.log", "w");
    if (!unikernel_log) {
        perror("Error opening unikernel_output.log");
        close(fd);
        return 1;
    }
    
    printf("Listening on %s at 38400 baud...\n", argv[1]);
    printf("Waiting for 0x1b markers...\n");
    printf("External binary: %s\n", argv[2]);
    
    int marker_state = 0;  // 0 = waiting for start marker, 1 = running binary, 2 = got end marker
    
    // Main loop
    while (1) {
        unsigned char c;
        if (read(fd, &c, 1) <= 0) continue;
        
        if (c == UNUSUAL_ASCII) {
            marker_state = (marker_state == 0) ? 1 : 0;
            capturing = 1;
            buf_index = 0;
            continue;
        }
        
        if (!capturing) {
            // Log regular output to file
            fputc(c, unikernel_log);
            fflush(unikernel_log);
            continue;
        }
        
        if (c == '\n') {
            buffer[buf_index] = '\0';
            
            // Parse the marker: "benchmark_name,repetition_number"
            int parsed = sscanf((char *)buffer, "%127[^,],%d", benchmark_name, &repetition);
            
            if (parsed == 2) {
                if (marker_state == 1) {
                    // Start marker received - launch external binary
                    printf("START: benchmark=%s, repetition=%d\n", benchmark_name, repetition);
                    fflush(stdout);
                    
                    // Create output filename for this run
                    char output_file[256];
                    snprintf(output_file, sizeof(output_file), "binary_output_%s_%d.log", benchmark_name, repetition);
                    
                    // Fork and execute external binary
                    child_pid = fork();
                    if (child_pid < 0) {
                        perror("fork failed");
                    } else if (child_pid == 0) {
                        // Child process - redirect output to file
                        FILE *output = fopen(output_file, "w");
                        if (output) {
                            dup2(fileno(output), STDOUT_FILENO);
                            dup2(fileno(output), STDERR_FILENO);
                            fclose(output);
                        }
                        
                        // Execute external binary
                        // argv[2] is the binary, argv[3..argc-1] are arguments
                        execvp(argv[2], &argv[2]);
                        perror("execvp failed");
                        exit(1);
                    }
                } else if (marker_state == 0) {
                    // End marker received - terminate external binary
                    printf("END: benchmark=%s, repetition=%d\n", benchmark_name, repetition);
                    fflush(stdout);
                    
                    if (child_pid > 0) {
                        kill(child_pid, SIGTERM);
                        int status;
                        waitpid(child_pid, &status, 0);
                        child_pid = -1;
                    }
                    
                    run_counter++;
                }
            }
            
            capturing = 0;
            buf_index = 0;
        } else if (buf_index < sizeof(buffer) - 1) {
            buffer[buf_index++] = c;
        }
    }
    
    fclose(unikernel_log);
    close(fd);
    return 0;
}
