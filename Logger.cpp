#include <iostream>
#include <unistd.h>
#include <ctime>
#include <cstring> // for strerror
#include <sys/socket.h>
#include <fcntl.h> // for fcntl
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "./Logger.h"

const int SERVERPORT = 9898;
const char *IP = "127.0.0.1";
//TODO: set them 0 inside initializeLog
LOG_LEVEL logLevel = (LOG_LEVEL)10;
int flags = 0;
bool isWorking = true;

int socketFd = 0;
struct sockaddr_in serverAddr = {0};

pthread_t ptId = 0;

void check(int status, const std::string &error)
{
    if (status == -1)
    {
        std::cerr << error << ":" << strerror(errno) << std::endl;

        if (ptId > 0)
        {
            isWorking = false;
            pthread_join(ptId, NULL);
        }

        if (socketFd > 0)
        {
            close(socketFd);
        }

        exit(EXIT_FAILURE);
    }
}

void *recieveFunction(void *)
{
    std::cout << "\nServer Thread: receiveFunction()\n";

    ssize_t bytes_received;
    LOG_LEVEL buffer;
    socklen_t size = sizeof(serverAddr);
    while (isWorking)
    {
        bytes_received = recvfrom(socketFd, &buffer, sizeof(buffer), 0, (struct sockaddr *)&serverAddr, &size);

        // Warning: dont use the check function on recvfrom() as we dont want to exit the program.
        if (bytes_received < 0)
        {
            // std::cout << "sleeping for 1 sec \n";
            sleep(1);
        }
        else
        {
            std::cout << "Received: " << buffer << std::endl;
            std::cout << "Updating the Log Level: " << buffer << std::endl;
            SetLogLevel(buffer);
            // sleep(1);
        }
    }

    pthread_exit(EXIT_SUCCESS);
}

void InitializeLog()
{

    // initialize:
    //memset(&logLevel, 10, sizeof(logLevel));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    serverAddr.sin_port = htons(SERVERPORT);

    check(socketFd = socket(AF_INET, SOCK_DGRAM, 0), "socket()");

    // Make the socket non-blocking:
    check(flags = fcntl(socketFd, F_GETFL, 0), "fcntl(get flags if any)");
    check(fcntl(socketFd, F_SETFL, flags | O_NONBLOCK), "fcntl(set flags)");

    // create a pthread to recieve data from the server.
    if (pthread_create(&ptId, NULL, recieveFunction, NULL) != 0)
    {
        std::cerr << "pthread_create()" << strerror(errno) << std::endl;
    }

    //std::cout << "\nSevver: Waitng for server to connect.... \n";
}

void SetLogLevel(LOG_LEVEL _level)
{

    logLevel = _level;
}

void Log(LOG_LEVEL _level, const std::string &_file, const std::string &_func, int _line, const std::string &_msg)
{
    if (isWorking == true)
    {

        ssize_t bytes_sent;
        std::string message = {0};

        time_t now = time(0);
        char *dt = ctime(&now);

        if (_level >= logLevel)
        {
            //std::cout << "_level " << logLevel << std::endl;
            // std::cout << "_server " << serverAddr.sin_port << std::endl;
            message = std::string(dt) + " Level: " + std::to_string(_level) + ", File: " + _file + ", Function: " + _func + ", Line: " + std::to_string(_line) + ", Message: " + _msg;
            check(bytes_sent = sendto(socketFd, message.c_str(), message.length(), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)), "sendto()");
        }
        // else{
        //     std::cout << "Working \n";
        // }
    }
}

void ExitLog()
{
    std::cout << "exiting\n";

    isWorking = false;
    pthread_join(ptId, NULL);

    close(socketFd);
}
