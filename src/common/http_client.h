#ifndef EDU_PUSH_SDK_HTTP_CLIENT_H
#define EDU_PUSH_SDK_HTTP_CLIENT_H

#include <map>
#include <mutex>
#include <string>

struct event_base;
struct evhttp_request;

namespace edu {
class HttpClient {
  public:
    HttpClient();

    ~HttpClient();

  public:
    void Initialize();

    void Close();

    void HandleRequestCallBack(struct evhttp_request* req);

    void Post(const std::string                         ip,
              int                                       port,
              const std::string&                        path,
              const std::map<std::string, std::string>& headers,
              const std::string&                        data);

  private:
    std::mutex  mux_;
    event_base* base_;
    bool        init_;
};
}  // namespace edu

#endif