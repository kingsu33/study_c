#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

static volatile sig_atomic_t g_running = 1;

static void handle_sigint(int sig) {
  (void)sig;
  g_running = 0;
}

static void send_response(int client_fd, int status_code,
                          const char *status_text, const char *content_type,
                          const char *body) {
  char header[1024];
  int body_len = (int)strlen(body);

  int n = snprintf(header, sizeof(header),
                   "HTTP/1.1 %d %s\r\n"
                   "Content-Type: %s\r\n"
                   "Content-Length: %d\r\n"
                   "Connection: close\r\n"
                   "\r\n",
                   status_code, status_text, content_type, body_len);
  // header
  (void)send(client_fd, header, (size_t)n, 0);
  // body
  (void)send(client_fd, body, (size_t)body_len, 0);
}

static void handle_client(int client_fd) {
  char buf[4096];
  memset(buf, 0, sizeof(buf));

  ssize_t r = recv(client_fd, buf, sizeof(buf) - 1, 0);
  if (r <= 0)
    return;

  char *line_end = strstr(buf, "\r\n");
  if (!line_end) {
    send_response(client_fd, 400, "Bad Request", "text/plain; charset=utf-8",
                  "Bad Request\n");
    return;
  }
  *line_end = '\0';

  char method[16], path[1024];
  memset(method, 0, sizeof(method));
  memset(path, 0, sizeof(path));

  if (sscanf(buf, "%15s %1023s", method, path) != 2) {
    send_response(client_fd, 400, "Bad Reequest", "text/plain; charset=utf-8",
                  "Bad Request\n");
    return;
  }

  if (strcmp(method, "GET") == 0 && strcmp(path, "/health") == 0) {
    send_response(client_fd, 200, "OK", "application/json; charset=utf-8",
                  "{\"status\":\"ok\"}\n");
    return;
  }

  send_response(
      client_fd, 404, "Not Found", "application/json; charset=utf-8",
      "{\"error\":{\"code\":\"NOT_FOUND\",\"message\":\'Route not found\"}}\n");
}

int main(void) {
  signal(SIGINT, handle_sigint);

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket");
    return 1;
  }

  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt");
    close(server_fd);
    return 1;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;

  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(8080);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    close(server_fd);
    return 1;
  }

  if (listen(server_fd, 16) < 0) {
    perror("listen");
    close(server_fd);
    return 1;
  }

  printf("C backend listening on 0.0.0.0:8080 (GET /health)\n");

  while (g_running) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

    if (client_fd < 0) {
      if (errno == EINTR)
        continue;
      perror("accept");
      break;
    }
    handle_client(client_fd);
    close(client_fd);
  }

  printf("\nShutting down...\n");
  close(server_fd);
  return 0;
}
