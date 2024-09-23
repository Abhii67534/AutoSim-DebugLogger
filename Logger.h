#ifndef LOGGER_H
#define LOGGER_H

enum LOG_LEVEL  {
    DEBUG, 
    WARNING, 
    ERROR, 
    CRITICAL
};


void InitializeLog(); 
void SetLogLevel(LOG_LEVEL _level); 
void Log(LOG_LEVEL, const std::string &, const std::string &, int , const std::string &); 
void ExitLog(); 

#endif
