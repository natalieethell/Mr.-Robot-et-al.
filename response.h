#ifndef RESPONSE_H
#define RESPONSE_H

#include <memory>
#include <string>
#include <boost/tokenizer.hpp>
#include <map>

// Represents an HTTP response.
//
// Usage:
//   Response r;
//   r.SetStatus(RESPONSE_200);
//   r.SetBody(...);
//   return r.ToString();
//
// Constructed by the RequestHandler, 
// after which the server should call ToString
// to serialize.
class Response {
 public:
  using Headers = std::vector<std::pair<std::string, std::string>>;
  enum ResponseCode {
    OK = 200,
    NOT_FOUND = 404,
    FOUND = 302
  };
  
  void SetStatus(const ResponseCode response_code);
  void AddHeader(const std::string& header_name, 
                 const std::string& header_value);
  void SetBody(const std::string& body);
  
  std::string ToString();

 private:
  std::map<int, std::string> reason_phrase_ = {
      {200, "OK"},
      {404, "NOT FOUND"},
      {302, "FOUND"}
  };
  std::string first_line_;
  Headers response_header_;
  std::string response_body_;
};

#endif
