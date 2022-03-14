#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <exception>

class CustomFileException: public std::exception{
  private:
    std::string errMsg;

  public:
    CustomFileException(): errMsg("error: something\'s wrong"){}
    CustomFileException(const char* _errMsg): errMsg(_errMsg){}
    CustomFileException(std::string _errMsg): errMsg(_errMsg){}
    virtual const char* what() const throw() {
      return this->errMsg.c_str();
    }
};

#endif