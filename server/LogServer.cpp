#include <iostream>
#include <unistd.h>
#include <cstring> // for strerror
#include <sys/socket.h>
#include <fcntl.h> // for fcntl
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdlib> // For system function
#include <pthread.h>

#include "../Logger.h"

const int SERVERPORT = 9898;
const char *IP = "127.0.0.1";
const char *fileName = "log-file.txt";

struct sockaddr_in serverAddr = {0}, clientAddr = {0};
pthread_t ptId = 0;
int socketFd = 0;
int fd = 0;

bool isRunning = true;
bool showLog = false;

// Reusable function to check for any error:
void check(int status, const std::string &error)
{
    if (status == -1)
    {
        std::cerr << error << ":" << strerror(errno) << std::endl;

        if (ptId > 0)
        {
            isRunning = false;
            pthread_join(ptId, NULL);
        }

        if (socketFd > 0)
        {
            close(socketFd);
        }

        exit(EXIT_FAILURE);
    }
}

// The function that will inside the thread:
void *recieveFunction(void *d)
{
    // struct sockaddr_in *client_adress = (struct sockaddr_in *)clientAddr;
    socklen_t sizeClient = sizeof(clientAddr);

    check(fd = open(fileName, O_WRONLY | O_CREAT, 0666), "open()");

    char buffer[1024]; // To hold the received message.

    while (isRunning)
    {
        buffer[0] = '\0';

        ssize_t bytes_received = recvfrom(socketFd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &sizeClient);
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            write(fd, buffer, strlen(buffer));
            // std::cout << "Serevr-Received: " << buffer << std::endl;
        }
        else
        {
            sleep(1);
        }
    }

    close(fd);
    pthread_exit(EXIT_SUCCESS);
}

int main(void)
{

    int flags = 0;
    socklen_t sizeClient = sizeof(clientAddr);
    socklen_t sizeServer = sizeof(serverAddr);

    // config for socket:
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    serverAddr.sin_port = htons(SERVERPORT);

    check(socketFd = socket(AF_INET, SOCK_DGRAM, 0), "socket()");

    // Make the socket non-blocking:
    check(flags = fcntl(socketFd, F_GETFL, 0), "fcntl(get flags if any)");
    check(fcntl(socketFd, F_SETFL, flags | O_NONBLOCK), "fcntl(set flags)");

    check(bind(socketFd, (struct sockaddr *)&serverAddr, sizeServer), "bind()");

    std::cout << "\nServer listening on port " << SERVERPORT << "......\n\n";

    // create a pthread to recieve data from the client.
    if (pthread_create(&ptId, NULL, recieveFunction, &clientAddr) != 0)
    {
        std::cerr << "pthread_create()" << strerror(errno) << std::endl;
    }

    // To send data the client.
    ssize_t bytes_sent;
    int buff;
    int len;
    int level;

    system("clear");


    while (isRunning)
    {
       
        std::cout << "--- User Menu ---" << std::endl;
        std::cout << "1. Set the log level" << std::endl;
        std::cout << "2. Dump the log file here" << std::endl;
        std::cout << "3. Shut down" << std::endl;

        int choice;
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            system("clear");

            std::cout << "--- Set Log Level ---" << std::endl;
            std::cout << "1. DEBUG " << std::endl;
            std::cout << "2. WARNING " << std::endl;
            std::cout << "3. ERROR " << std::endl;
            std::cout << "4. CRITICAL " << std::endl;

            int logLevelChoice;
            std::cout << "Enter log level choice (1-4): ";
            std::cin >> logLevelChoice;

            system("clear");

            LOG_LEVEL logLevel;

            switch (logLevelChoice)
            {

            case 1:
                logLevel = LOG_LEVEL::DEBUG;
                break;
            case 2:
                logLevel = LOG_LEVEL::WARNING;
                break;
            case 3:
                logLevel = LOG_LEVEL::ERROR;
                break;
            case 4:
                logLevel = LOG_LEVEL::CRITICAL;
                break;
            default:
                std::cout << "Invalid log level choice. \n" << std::endl;

                break;
            }

            // If the client is not connected, do not send.
            if (clientAddr.sin_family != 0 && clientAddr.sin_addr.s_addr != 0 && clientAddr.sin_port != 0)
                check(bytes_sent = sendto(socketFd, &logLevel, sizeof(logLevel), 0, (const struct sockaddr *)&clientAddr, sizeClient), "sendto()");

            break;

        case 2:
            system("clear");

            // Open and read the log file
            char buffer[1024];
            ssize_t bytesRead;
            int fd;

            check(fd = open(fileName, O_RDONLY), "open()");
            while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
            {
                // Write the content to standard output
                check(write(STDOUT_FILENO, buffer, bytesRead), "write(to terminal)");
            }

            if (bytesRead == -1)
            {
                std::cerr << "\nread() Failed:" << strerror(errno) << std::endl << std::endl;
                break;
            }

            // End of file reached
            //close(fd);
            std::cout << "End of file reached." << std::endl;
            std::cout << "Press any key to continue:";
            std::cin.ignore();
            std::cin.get(); // Wait for user to press Enter
            system("clear");
            break;

        case 3:
            system("clear");
            isRunning = false;
            break;

        default:
            system("clear");
            std::cout << "\nInvalid choice. Please try again.\n\n";
            break;
        }
    }

    close(fd);
    close(socketFd);
    std::cout << "\nClosing server....\n";

    pthread_join(ptId, NULL);

    return EXIT_SUCCESS;
}