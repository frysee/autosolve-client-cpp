#pragma once

#include <stdexcept>

namespace AutoSolveClient::Exceptions {
  enum AutoSolveExceptionTypes {
    InvalidClientId,
    InvalidApiKeyOrAccessToken,
    InputValueError,
    InitConnectionError,
    TooManyRequests,
    Unknown
  };

  class AutoSolveException : public std::runtime_error {
    AutoSolveExceptionTypes m_type = AutoSolveExceptionTypes::Unknown;

    static inline std::string messageForType(AutoSolveExceptionTypes type) {
      switch (type) {
        case InvalidClientId:
          return "Invalid client id.";
        case InvalidApiKeyOrAccessToken:
          return "Invalid API key or access token";
        case InputValueError:
          return "Input value error";
        case InitConnectionError:
          return "Error initializing connection";
        case TooManyRequests:
          return "Too many requests";
      }
      return "Unknown error.";
    }

  public:
    AutoSolveException(AutoSolveExceptionTypes type)
        : std::runtime_error(messageForType(type)), m_type(type) {}
    AutoSolveException(AutoSolveExceptionTypes type, const std::string &msg)
        : std::runtime_error(msg), m_type(type) {}

    AutoSolveExceptionTypes type() const { return m_type; }
  };
}  // namespace AutoSolveClient::Exceptions