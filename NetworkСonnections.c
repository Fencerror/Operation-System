#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

// Порт для сервера
#define PORT 8080

// Максимальная длина очереди соединений
#define BACKLOG 5

// Буфер для чтения данных
#define BUF_SIZE 1024

// Глобальная переменная для отслеживания сигнала
volatile sig_atomic_t got_sighup = 0;

// Обработчик сигнала
void sighup_handler(int signum) {
    (void)signum; // Подавляем предупреждение о неиспользуемой переменной
    got_sighup = 1;
}

// Устанавливаем неблокирующий режим сокета
void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int listen_fd, conn_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUF_SIZE];
    fd_set read_fds;
    int max_fd;
    sigset_t mask, orig_mask;
    struct sigaction sa;

    // Настраиваем обработчик сигнала SIGHUP
    sa.sa_handler = sighup_handler;
    sa.sa_flags = SA_RESTART; // Перезапуск системных вызовов
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Маска сигналов для блокировки в pselect()
    sigemptyset(&mask);
    sigaddset(&mask, SIGHUP);
    if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) == -1) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    // Создаем слушающий сокет
    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Привязываем сокет к порту
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    // Переводим сокет в режим прослушивания
    if (listen(listen_fd, BACKLOG) == -1) {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    // Основной цикл обработки
    int active_fd = -1;
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(listen_fd, &read_fds);
        if (active_fd != -1) {
            FD_SET(active_fd, &read_fds);
        }
        max_fd = (active_fd > listen_fd) ? active_fd : listen_fd;

        // Используем pselect() для обработки сигналов и сокетов
        int ready = pselect(max_fd + 1, &read_fds, NULL, NULL, NULL, &orig_mask);
        if (ready == -1) {
            if (errno == EINTR) {
                continue; // Сигнал прервал вызов pselect(), продолжаем
            }
            perror("pselect");
            break;
        }

        // Проверяем на поступление сигнала SIGHUP
        if (got_sighup) {
            printf("Received SIGHUP signal\n");
            got_sighup = 0; // Сбрасываем флаг
        }

        // Проверяем новые соединения
        if (FD_ISSET(listen_fd, &read_fds)) {
            conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
            if (conn_fd == -1) {
                perror("accept");
                continue;
            }
            printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // Закрываем предыдущее активное соединение, если есть
            if (active_fd != -1) {
                close(active_fd);
            }
            active_fd = conn_fd;
        }

        // Обработка данных от активного клиента
        if (active_fd != -1 && FD_ISSET(active_fd, &read_fds)) {
            ssize_t bytes_read = read(active_fd, buffer, BUF_SIZE);
            if (bytes_read > 0) {
                printf("Received %zd bytes\n", bytes_read);
            } else if (bytes_read == 0) {
                printf("Connection closed by client\n");
                close(active_fd);
                active_fd = -1;
            } else {
                perror("read");
            }
        }
    }

    close(listen_fd);
    return 0;
}
