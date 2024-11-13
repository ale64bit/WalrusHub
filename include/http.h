#pragma once

#include <curl/curl.h>
#include <gtk/gtk.h>

#include <functional>
#include <initializer_list>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

class Http {
  struct Conn;
  struct Sock;

 public:
  using Headers = std::initializer_list<std::pair<std::string, std::string>>;
  using ResponseStreamer =
      std::function<void(long code, std::string_view data, bool final)>;
  using ResponseHandler = std::function<void(long code, std::string_view body)>;

  class Request {
   public:
    Request(Conn *conn);
    ~Request();

    void cancel();

   private:
    Conn *conn_ = nullptr;
  };

  Http();
  ~Http();

  std::unique_ptr<Request> get(std::string url, Headers headers,
                               ResponseStreamer h);
  std::unique_ptr<Request> get(std::string url, Headers headers,
                               ResponseHandler h);
  std::unique_ptr<Request> post(std::string url, Headers headers,
                                std::string body, ResponseStreamer h);
  std::unique_ptr<Request> post(std::string url, Headers headers,
                                std::string body, ResponseHandler h);
  std::unique_ptr<Request> put(std::string url, Headers headers,
                               std::string body, ResponseStreamer h);
  std::unique_ptr<Request> put(std::string url, Headers headers,
                               std::string body, ResponseHandler h);

 private:
  CURLM *multi_;
  guint timer_event_;
  int running_count_;

  void check_handles();
  std::unique_ptr<Request> custom_req(const char *method, std::string url,
                                      Headers headers, std::string body,
                                      ResponseStreamer h);

  static int socket_function_cb(CURL *e, curl_socket_t s, int action,
                                void *user_ptr, void *sock_ptr);
  static gboolean timer_cb(gpointer data);
  static int timer_function_cb(CURLM *multi, long timeout_ms, void *user_ptr);
  static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *user_ptr);
  static gboolean event_cb(GIOChannel *ch, GIOCondition condition,
                           gpointer data);
};
