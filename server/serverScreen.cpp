#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_LEN 1024

int main()
{
    int fd;
    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr);

    // Initialize the socket and address
    // (Assuming fd is already created and remaddr is properly initialized)

    char buf[BUF_LEN];
    int len;
    int level;
    bool is_running = true;

    while (is_running)
    {
        std::cout << "User Menu:" << std::endl;
        std::cout << "1. Set the log level" << std::endl;
        std::cout << "2. Dump the log file here" << std::endl;
        std::cout << "0. Shut down" << std::endl;

        int choice;
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            std::cout << "Enter log level: ";
            std::cin >> level;

            memset(buf, 0, BUF_LEN);
            len = sprintf(buf, "Set Log Level=%d", level) + 1;
            sendto(fd, buf, len, 0, (struct sockaddr *)&remaddr, addrlen);
            break;

        case 2:
            // Open and read the log file
            FILE *log_file = fopen("server_log.txt", "r");
            if (log_file)
            {
                char log_buffer[BUF_LEN];
                while (fgets(log_buffer, BUF_LEN, log_file) != NULL)
                {
                    std::cout << log_buffer;
                }
                fclose(log_file);
            }
            else
            {
                std::cerr << "Error opening log file!" << std::endl;
            }

            std::cout << "Press any key to continue:";
            std::cin.ignore();
            std::cin.get(); // Wait for user to press Enter
            break;

        case 0:
            is_running = false;
            break;

        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
            break;
        }
    }

    // Join receive thread here

    // Close socket and perform any cleanup
    close(fd);

    return 0;
}
