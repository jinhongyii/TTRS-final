/**
 * exception.h
 */

#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace sjtu {
//===--------------------------------------------------------------------===//
// Exception Types
//===--------------------------------------------------------------------===//

enum ExceptionType {
  EXCEPTION_TYPE_INVALID = 0,           // invalid type
  EXCEPTION_TYPE_OUT_OF_RANGE = 1,      // value out of range error
  EXCEPTION_TYPE_CONVERSION = 2,        // conversion/casting error
  EXCEPTION_TYPE_UNKNOWN_TYPE = 3,      // unknown type
  EXCEPTION_TYPE_DECIMAL = 4,           // decimal related
  EXCEPTION_TYPE_MISMATCH_TYPE = 5,     // type mismatch
  EXCEPTION_TYPE_DIVIDE_BY_ZERO = 6,    // divide by 0
  EXCEPTION_TYPE_OBJECT_SIZE = 7,       // object size exceeded
  EXCEPTION_TYPE_INCOMPATIBLE_TYPE = 8, // incompatible for operation
  EXCEPTION_TYPE_SERIALIZATION = 9,     // serialization
  EXCEPTION_TYPE_TRANSACTION = 10,      // transaction management
  EXCEPTION_TYPE_NOT_IMPLEMENTED = 11,  // method not implemented
  EXCEPTION_TYPE_EXPRESSION = 12,       // expression parsing
  EXCEPTION_TYPE_CATALOG = 13,          // catalog related
  EXCEPTION_TYPE_PARSER = 14,           // parser related
  EXCEPTION_TYPE_PLANNER = 15,          // planner related
  EXCEPTION_TYPE_SCHEDULER = 16,        // scheduler related
  EXCEPTION_TYPE_EXECUTOR = 17,         // executor related
  EXCEPTION_TYPE_CONSTRAINT = 18,       // constraint related
  EXCEPTION_TYPE_INDEX = 19,            // index related
  EXCEPTION_TYPE_STAT = 20,             // stat related
  EXCEPTION_TYPE_CONNECTION = 21,       // connection related
  EXCEPTION_TYPE_SYNTAX = 22,           // syntax related
};

class Exception : public std::runtime_error {
public:
  Exception(std::string message="error")
      : std::runtime_error(message), type(EXCEPTION_TYPE_INVALID) {
    std::string exception_message = "Message :: " + message + "\n";
    std::cerr << exception_message;
  }

  Exception(ExceptionType exception_type, std::string message)
      : std::runtime_error(message), type(exception_type) {
    std::string exception_message = "\nException Type :: " +
                                    ExpectionTypeToString(exception_type) +
                                    "\nMessage :: " + message + "\n";
    std::cerr << exception_message;
  }

  std::string ExpectionTypeToString(ExceptionType type) {
    switch (type) {
    case EXCEPTION_TYPE_INVALID:
      return "Invalid";
    case EXCEPTION_TYPE_OUT_OF_RANGE:
      return "Out of Range";
    case EXCEPTION_TYPE_CONVERSION:
      return "Conversion";
    case EXCEPTION_TYPE_UNKNOWN_TYPE:
      return "Unknown Type";
    case EXCEPTION_TYPE_DECIMAL:
      return "Decimal";
    case EXCEPTION_TYPE_MISMATCH_TYPE:
      return "Mismatch Type";
    case EXCEPTION_TYPE_DIVIDE_BY_ZERO:
      return "Divide by Zero";
    case EXCEPTION_TYPE_OBJECT_SIZE:
      return "Object Size";
    case EXCEPTION_TYPE_INCOMPATIBLE_TYPE:
      return "Incompatible type";
    case EXCEPTION_TYPE_SERIALIZATION:
      return "Serialization";
    case EXCEPTION_TYPE_TRANSACTION:
      return "Transaction";
    case EXCEPTION_TYPE_NOT_IMPLEMENTED:
      return "Not implemented";
    case EXCEPTION_TYPE_EXPRESSION:
      return "Expression";
    case EXCEPTION_TYPE_CATALOG:
      return "Catalog";
    case EXCEPTION_TYPE_PARSER:
      return "Parser";
    case EXCEPTION_TYPE_PLANNER:
      return "Planner";
    case EXCEPTION_TYPE_SCHEDULER:
      return "Scheduler";
    case EXCEPTION_TYPE_EXECUTOR:
      return "Executor";
    case EXCEPTION_TYPE_CONSTRAINT:
      return "Constraint";
    case EXCEPTION_TYPE_INDEX:
      return "Index";
    case EXCEPTION_TYPE_STAT:
      return "Stat";
    case EXCEPTION_TYPE_CONNECTION:
      return "Connection";
    case EXCEPTION_TYPE_SYNTAX:
      return "Syntax";
    default:
      return "Unknown";
    }
  }

private:
  // type
  ExceptionType type;
};

}