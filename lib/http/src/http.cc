#include "http.h"

#include <cstring>
#include <sstream>

#include "log.h"

// Based on https://curl.se/libcurl/c/ghiper.html

namespace http {

struct Client::Conn {
  Client *http_ = nullptr;
  CURL *easy_ = nullptr;
  curl_slist *headers_ = nullptr;
  std::string url_;
  char err_[CURL_ERROR_SIZE];
  Request *request_;
  ResponseStreamer handler_;
};

struct Client::Sock {
  Client *http_;
  curl_socket_t sock_fd_;
  CURL *easy_;
  int action_;
  long timeout_;
  GIOChannel *ch_;
  guint ev_;

  Sock(Client *http, curl_socket_t s, CURL *easy, int action)
      : http_(http), ch_(g_io_channel_unix_new(s)), ev_(0) {
    set(s, easy, action);
    curl_multi_assign(http->multi_, s, this);
  }

  ~Sock() {
    if (ev_) g_source_remove(ev_);
    http_ = nullptr;
    easy_ = nullptr;
  }

  void set(curl_socket_t s, CURL *easy, int action) {
    const GIOCondition kind =
        GIOCondition(((action & CURL_POLL_IN) ? G_IO_IN : 0) |
                     ((action & CURL_POLL_OUT) ? G_IO_OUT : 0));

    sock_fd_ = s;
    easy_ = easy;
    action_ = action;
    if (ev_) g_source_remove(ev_);
    ev_ = g_io_add_watch(ch_, kind, Client::event_cb, http_);
  }
};

Client::Request::Request(Conn *conn) : conn_(conn) {}

Client::Request::~Request() { cancel(); }

void Client::Request::cancel() {
  if (conn_) {
    curl_multi_remove_handle(conn_->http_->multi_, conn_->easy_);
    curl_easy_cleanup(conn_->easy_);
    if (conn_->headers_) curl_slist_free_all(conn_->headers_);
    delete conn_;
    conn_ = nullptr;
  }
}

struct ResponseCollector {
  long code_ = 0;
  std::stringstream body_;
  Client::ResponseHandler handler_;
};

Client::Client() : running_count_(0) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  multi_ = curl_multi_init();

  curl_multi_setopt(multi_, CURLMOPT_SOCKETFUNCTION, socket_function_cb);
  curl_multi_setopt(multi_, CURLMOPT_SOCKETDATA, this);
  curl_multi_setopt(multi_, CURLMOPT_TIMERFUNCTION, timer_function_cb);
  curl_multi_setopt(multi_, CURLMOPT_TIMERDATA, this);
}

Client::~Client() {
  curl_multi_setopt(multi_, CURLMOPT_SOCKETDATA, nullptr);
  curl_multi_cleanup(multi_);
  multi_ = nullptr;
  curl_global_cleanup();
}

std::unique_ptr<Client::Request> Client::get(std::string url, Headers headers,
                                             Client::ResponseStreamer handler) {
  auto conn = new Conn;
  conn->http_ = this;
  conn->url_ = url;
  conn->easy_ = curl_easy_init();
  conn->handler_ = handler;
  if (!conn->easy_) {
    delete conn;
    return nullptr;
  }

  for (const auto &[k, v] : headers) {
    const std::string h = k + ": " + v;
    conn->headers_ = curl_slist_append(conn->headers_, h.c_str());
  }

  if (headers.size() > 0)
    curl_easy_setopt(conn->easy_, CURLOPT_HTTPHEADER, conn->headers_);
  curl_easy_setopt(conn->easy_, CURLOPT_URL, conn->url_.c_str());
  curl_easy_setopt(conn->easy_, CURLOPT_WRITEFUNCTION, write_cb);
  curl_easy_setopt(conn->easy_, CURLOPT_WRITEDATA, conn);
  curl_easy_setopt(conn->easy_, CURLOPT_ERRORBUFFER, conn->err_);
  curl_easy_setopt(conn->easy_, CURLOPT_PRIVATE, conn);
  curl_easy_setopt(conn->easy_, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(conn->easy_, CURLOPT_CONNECTTIMEOUT, 10L);

  if (auto ret = curl_multi_add_handle(multi_, conn->easy_); ret != CURLM_OK) {
    delete conn;
    // TODO return curl_multi_strerror(ret);
    return nullptr;
  }

  auto req = std::make_unique<Request>(conn);
  conn->request_ = req.get();
  return req;
}

std::unique_ptr<Client::Request> Client::get(std::string url, Headers headers,
                                             Client::ResponseHandler handler) {
  auto resp = new ResponseCollector;
  resp->handler_ = handler;
  return get(url, headers,
             [resp](long code, std::string_view data, bool final) {
               if (code > 0 && resp->code_ == 0) resp->code_ = code;
               resp->body_ << data;
               if (final) {
                 resp->handler_(resp->code_, resp->body_.str());
                 delete resp;
               }
             });
}

std::unique_ptr<Client::Request> Client::post(std::string url, Headers headers,
                                              std::string body,
                                              ResponseStreamer handler) {
  return custom_req("POST", url, headers, body, handler);
}

std::unique_ptr<Client::Request> Client::post(std::string url, Headers headers,
                                              std::string body,
                                              Client::ResponseHandler handler) {
  auto resp = new ResponseCollector;
  resp->handler_ = handler;
  return post(url, headers, body,
              [resp](long code, std::string_view data, bool final) {
                if (code > 0 && resp->code_ == 0) resp->code_ = code;
                resp->body_ << data;
                if (final) {
                  resp->handler_(resp->code_, resp->body_.str());
                  delete resp;
                }
              });
}

std::unique_ptr<Client::Request> Client::put(std::string url, Headers headers,
                                             std::string body,
                                             ResponseStreamer handler) {
  return custom_req("PUT", url, headers, body, handler);
}

std::unique_ptr<Client::Request> Client::put(std::string url, Headers headers,
                                             std::string body,
                                             Client::ResponseHandler handler) {
  auto resp = new ResponseCollector;
  resp->handler_ = handler;
  return put(url, headers, body,
             [resp](long code, std::string_view data, bool final) {
               if (code > 0 && resp->code_ == 0) resp->code_ = code;
               resp->body_ << data;
               if (final) {
                 resp->handler_(resp->code_, resp->body_.str());
                 delete resp;
               }
             });
}

std::unique_ptr<Client::Request> Client::custom_req(const char *method,
                                                    std::string url,
                                                    Headers headers,
                                                    std::string body,
                                                    ResponseStreamer handler) {
  auto conn = new Conn;
  conn->http_ = this;
  conn->url_ = url;
  conn->easy_ = curl_easy_init();
  conn->handler_ = handler;
  if (!conn->easy_) {
    delete conn;
    return nullptr;
  }

  conn->headers_ =
      curl_slist_append(conn->headers_, "Content-Type: application/json");
  for (const auto &[k, v] : headers) {
    std::string h = k + ": " + v;
    conn->headers_ = curl_slist_append(conn->headers_, h.c_str());
  }

  curl_easy_setopt(conn->easy_, CURLOPT_POSTFIELDSIZE, body.size());
  curl_easy_setopt(conn->easy_, CURLOPT_COPYPOSTFIELDS, body.c_str());
  curl_easy_setopt(conn->easy_, CURLOPT_CUSTOMREQUEST, method);
  curl_easy_setopt(conn->easy_, CURLOPT_HTTPHEADER, conn->headers_);
  curl_easy_setopt(conn->easy_, CURLOPT_URL, conn->url_.c_str());
  curl_easy_setopt(conn->easy_, CURLOPT_WRITEFUNCTION, write_cb);
  curl_easy_setopt(conn->easy_, CURLOPT_WRITEDATA, conn);
  curl_easy_setopt(conn->easy_, CURLOPT_ERRORBUFFER, conn->err_);
  curl_easy_setopt(conn->easy_, CURLOPT_PRIVATE, conn);
  curl_easy_setopt(conn->easy_, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(conn->easy_, CURLOPT_CONNECTTIMEOUT, 10L);

  if (auto ret = curl_multi_add_handle(multi_, conn->easy_); ret != CURLM_OK) {
    curl_slist_free_all(conn->headers_);
    delete conn;
    // TODO return curl_multi_strerror(ret);
    return nullptr;
  }

  auto req = std::make_unique<Request>(conn);
  conn->request_ = req.get();
  return req;
}

int Client::socket_function_cb(CURL *e, curl_socket_t s, int action,
                               void *user_ptr, void *sock_ptr) {
  if (!user_ptr) return -1;
  Client *http = reinterpret_cast<Client *>(user_ptr);
  Sock *sock = reinterpret_cast<Sock *>(sock_ptr);
  if (action == CURL_POLL_REMOVE) {
    if (sock) delete sock;
  } else if (!sock) {
    sock = new Sock(http, s, e, action);
  } else {
    sock->set(s, e, action);
  }
  return 0;
}

gboolean Client::timer_cb(gpointer data) {
  Client *http = reinterpret_cast<Client *>(data);
  if (auto ret = curl_multi_socket_action(http->multi_, CURL_SOCKET_TIMEOUT, 0,
                                          &http->running_count_);
      ret != CURLM_OK) {
    // TODO handle better
    std::exit(1);
  }
  http->check_handles();
  http->timer_event_ = 0;
  return FALSE;
}

int Client::timer_function_cb(CURLM * /*multi*/, long timeout_ms,
                              void *user_ptr) {
  Client *http = reinterpret_cast<Client *>(user_ptr);
  if (timeout_ms >= 0)
    http->timer_event_ = g_timeout_add(timeout_ms, Client::timer_cb, user_ptr);
  return 0;
}

size_t Client::write_cb(void *ptr, size_t size, size_t nmemb, void *user_ptr) {
  Conn *conn = reinterpret_cast<Conn *>(user_ptr);
  const size_t real_size = size * nmemb;

  conn->handler_(
      0, std::string_view(reinterpret_cast<const char *>(ptr), real_size),
      false);

  return real_size;
}

gboolean Client::event_cb(GIOChannel *ch, GIOCondition condition,
                          gpointer data) {
  Client *http = reinterpret_cast<Client *>(data);

  const int fd = g_io_channel_unix_get_fd(ch);
  const int action = ((condition & G_IO_IN) ? CURL_CSELECT_IN : 0) |
                     ((condition & G_IO_OUT) ? CURL_CSELECT_OUT : 0);

  if (auto ret = curl_multi_socket_action(http->multi_, fd, action,
                                          &http->running_count_);
      ret != CURLM_OK) {
    // TODO handle better
    std::exit(1);
    return FALSE;
  }

  http->check_handles();

  if (http->running_count_ > 0) return TRUE;
  if (http->timer_event_) {
    g_source_remove(http->timer_event_);
    http->timer_event_ = 0;
  }
  return FALSE;
}

void Client::check_handles() {
  int msgs_left;
  CURLMsg *msg;
  while ((msg = curl_multi_info_read(multi_, &msgs_left))) {
    if (msg->msg != CURLMSG_DONE) continue;
    CURL *easy = msg->easy_handle;
    CURLcode res = msg->data.result;  // TODO set err code string in request_
    long http_resp_code;
    Conn *conn;
    curl_easy_getinfo(easy, CURLINFO_PRIVATE, &conn);
    curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &http_resp_code);
    conn->handler_(http_resp_code, std::string_view(), true);
    conn->request_->cancel();
  }
}

}  // namespace http