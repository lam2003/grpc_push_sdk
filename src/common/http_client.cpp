#include <common/http_client.h>
#include <common/log.h>

#include <evhttp.h>

namespace edu {

template <typename T>
static void http_request_cb(struct evhttp_request* req, void* arg)
{
    T* t = static_cast<T*>(arg);
    t->HandleRequestCallBack(req);
}

void HttpClient::HandleRequestCallBack(struct evhttp_request* req)
{
    if (!req) {
        log_e("http request failed. {}", strerror(errno));
    }

    log_t("post {}, {} {}", evhttp_request_get_host(req),
          evhttp_request_get_response_code(req),
          evhttp_request_get_response_code_line(req));
}

HttpClient::HttpClient() : base_(nullptr), init_(false) {}

HttpClient::~HttpClient()
{
    Close();
}

void HttpClient::Post(const std::string                         ip,
                      int                                       port,
                      const std::string&                        path,
                      const std::map<std::string, std::string>& headers,
                      const std::string&                        data)
{
    if (!init_)
        return;

    int ret;

    std::unique_lock<std::mutex> lock(mux_);

    evhttp_connection* conn =
        evhttp_connection_base_new(base_, nullptr, ip.c_str(), port);
    if (!conn) {
        log_e("evhttp_connection_base_new failed");
        return;
    }

    evhttp_connection_set_retries(conn, 1);
    evhttp_connection_set_timeout(conn, 1);

    evhttp_request* req = evhttp_request_new(http_request_cb<HttpClient>,
                                             static_cast<void*>(this));
    if (!req) {
        log_e("evhttp_request_new failed");
        return;
    }

    evkeyvalq* output_headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(output_headers, "Content-Type",
                      "text/json;charset=UTF-8");
    evhttp_add_header(output_headers, "Host", ip.c_str());
    evhttp_add_header(output_headers, "Connection", "close");

    for (const std::pair<const std::string, std::string>& it : headers) {
        evhttp_add_header(output_headers, it.first.c_str(), it.second.c_str());
    }

    ret = evbuffer_add(req->output_buffer, data.c_str(), data.length());
    if (ret != 0) {
        log_e("evbuffer_add failed");
        return;
    }

    ret = evhttp_make_request(conn, req, EVHTTP_REQ_POST, path.c_str());
    if (ret != 0) {
        log_e("evhttp_make_request failed");
        return;
    }

    event_base_dispatch(base_);
    evhttp_connection_free(conn);
}

void HttpClient::Initialize()
{
    if (init_)
        return;

    log_d("HTTP_CLIENT start");

    base_ = event_base_new();
    init_ = true;
}

void HttpClient::Close()
{
    if (!init_)
        return;
    log_d("HTTP_CLIENT stop");

    event_base_free(base_);
    init_ = false;
}

}  // namespace edu