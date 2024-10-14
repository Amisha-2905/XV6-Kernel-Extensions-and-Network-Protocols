#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/time.h>

#define CHUNK_SIZE 512
#define TIMEOUT 100000
#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct data_packet
{
    int seq_num;
    int total_chunks;
    char data[CHUNK_SIZE];
} data_packet;

typedef struct ack_packet
{
    int seq_num;
} ack_packet;

void get_ip_address()
{
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(1);
    }

    printf("Server available on the following IP addresses:\n");
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
        {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            printf("%s: %s\n", ifa->ifa_name, host);
        }
    }
    freeifaddrs(ifaddr);
}

int number_of_chunks(size_t file_size)
{
    return (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;
}

size_t get_file_size(const char *filename)
{
    struct stat st;
    if (stat(filename, &st) != 0)
    {
        perror("Could not get file size");
        exit(1);
    }
    return st.st_size;
}

void send_file_chunks(int socketfd, FILE *file, int total_chunks, struct sockaddr_in *addr, socklen_t addr_len)
{
    data_packet packet;
    ack_packet ack;
    int unacknowledged[total_chunks];
    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = TIMEOUT;
    for (int i = 0; i < total_chunks; i++)
    {
        packet.seq_num = i + 1;
        fread(packet.data, 1, CHUNK_SIZE, file);
        sendto(socketfd, &packet, sizeof(packet), 0, (struct sockaddr *)addr, addr_len);
        printf("Sent chunk %d\n", packet.seq_num);
        unacknowledged[i] = 1;
    }
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(socketfd, &readfds);
        int activity = select(socketfd + 1, &readfds, NULL, NULL, &timeout);
        if (activity > 0 && FD_ISSET(socketfd, &readfds))
        {
            recvfrom(socketfd, &ack, sizeof(ack), 0, (struct sockaddr *)addr, &addr_len);
            printf("Received ACK for chunk %d\n", ack.seq_num);
            unacknowledged[ack.seq_num - 1] = 0;
        }
        else if (activity == 0)
        {
            for (int i = 0; i < total_chunks; i++)
            {
                if (unacknowledged[i])
                {
                    printf("Retransmitting chunk %d\n", i + 1);
                    packet.seq_num = i + 1;
                    fseek(file, i * CHUNK_SIZE, SEEK_SET);
                    fread(packet.data, 1, CHUNK_SIZE, file);
                    sendto(socketfd, &packet, sizeof(packet), 0, (struct sockaddr *)addr, addr_len);
                }
            }
        }

        int all_acks_received = 1;
        for (int i = 0; i < total_chunks; i++)
        {
            if (unacknowledged[i])
            {
                all_acks_received = 0;
                break;
            }
        }

        if (all_acks_received)
        {
            printf("All chunks acknowledged.\n");
            break;
        }
    }
}

void receive_file_chunks(int socketfd, FILE *file, int total_chunks, struct sockaddr_in *addr, socklen_t addr_len)
{
    data_packet *packets = (data_packet *)malloc(sizeof(data_packet) * (total_chunks+1));
    if (packets == NULL)
    {
        perror("Memory allocation failed");
        exit(1);
    }

    ack_packet ack;
    int received_packets = 0;
    int i = 0;

    int *received = (int *)calloc(total_chunks+1, sizeof(int));
    if (received == NULL)
    {
        perror("Memory allocation failed");
        free(packets);
        exit(1);
    }

    while (received_packets < total_chunks)
    {
        data_packet packet;
        recvfrom(socketfd, &packet, sizeof(packet), 0, (struct sockaddr *)addr, &addr_len);
        printf("Received chunk %d\n", packet.seq_num);

        if (received[packet.seq_num] == 0)
        {

            if (i % 3 != 0)
            {
                ack.seq_num = packet.seq_num;
                sendto(socketfd, &ack, sizeof(ack), 0, (struct sockaddr *)addr, addr_len);
                printf("Sent ACK for chunk %d\n", ack.seq_num);
                packets[packet.seq_num] = packet;
                received[packet.seq_num] = 1;
                received_packets++;
            }
        }

        i++;
    }

    for (int i = 0; i < total_chunks; i++)
    {
        fwrite(packets[i].data, 1, CHUNK_SIZE, file);
    }

    printf("All chunks received successfully.\n");

    free(received);
    // printf("----\n");
    free(packets);
    // printf("----\n");
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <server|client> <send|receive> <file> [server_ip]\n", argv[0]);
        exit(1);
    }

    const char *mode = argv[2];
    if (strcmp("server", argv[1]) == 0)
    {
        int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (socketfd < 0)
        {
            perror("Socket creation failed!\n");
            exit(1);
        }
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            perror("Bind failed");
            exit(1);
        }
        get_ip_address();
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        if (strcmp(mode, "receive") == 0)
        {
            int total_chunks;
            char ack_message[BUFFER_SIZE];
            recvfrom(socketfd, &total_chunks, sizeof(total_chunks), 0, (struct sockaddr *)&client_addr, &addr_len);
            printf("Received number of chunks: %d\n", total_chunks);

            snprintf(ack_message, sizeof(ack_message), "About to receive %d chunks", total_chunks);
            sendto(socketfd, ack_message, strlen(ack_message), 0, (struct sockaddr *)&client_addr, addr_len);
            printf("Acknowledgment sent: %s\n", ack_message);

            char filename[100] = "received_data.txt";
            FILE *file = fopen(filename, "wb");
            if (!file)
            {
                perror("Failed to open file for writing");
                exit(1);
            }

            receive_file_chunks(socketfd, file, total_chunks, &server_addr, addr_len);

            fclose(file);
        }
        else if (strcmp(argv[2], "send") == 0)
        {
            const char *filename = argv[3];
            size_t file_size = get_file_size(filename);
            int total_chunks = number_of_chunks(file_size);

            printf("File size: %lu bytes\n", file_size);
            printf("Total chunks to send: %d\n", total_chunks);
            printf("Sending total chunks: %d\n", total_chunks);
            recvfrom(socketfd, NULL, 0, 0, (struct sockaddr *)&client_addr, &addr_len);
            sendto(socketfd, &total_chunks, sizeof(total_chunks), 0, (struct sockaddr *)&client_addr, addr_len);

            char buffer[BUFFER_SIZE] = {0};
            recvfrom(socketfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len);
            printf("Client acknowledgment: %s\n", buffer);

            FILE *file = fopen(filename, "rb");
            if (!file)
            {
                perror("Failed to open file");
                exit(1);
            }

            send_file_chunks(socketfd, file, total_chunks, &client_addr, addr_len);

            fclose(file);
        }
        else
        {
            fprintf(stderr, "Usage: %s <server|client> <send|receive> <file> [server_ip]\n", argv[0]);
            exit(1);
        }
        close(socketfd);
    }

    else if (strcmp(argv[1], "client") == 0)
    {
        int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (socketfd < 0)
        {
            perror("Socket creation failed!\n");
            exit(1);
        }
        const char *server_ip;
        if (argc == 4)
        {
            server_ip = argv[3];
        }
        else if (argc == 5)
        {
            server_ip = argv[4];
        }
        else
        {
            fprintf(stderr, "Usage: %s <server|client> <send|receive> <file> [server_ip]\n", argv[0]);
            exit(1);
        }

        struct sockaddr_in server_addr;
        socklen_t addr_len = sizeof(server_addr);
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
        {
            fprintf(stderr, "Invalid server IP address: %s\n", server_ip);
            exit(1);
        }

        if (strcmp(argv[2], "send") == 0)
        {
            const char *filename = argv[3];
            size_t file_size = get_file_size(filename);
            int total_chunks = number_of_chunks(file_size);

            printf("File size: %lu bytes\n", file_size);
            printf("Total chunks to send: %d\n", total_chunks);
            printf("Sending total chunks: %d\n", total_chunks);

            sendto(socketfd, &total_chunks, sizeof(total_chunks), 0, (const struct sockaddr *)&server_addr, addr_len);

            char buffer[BUFFER_SIZE] = {0};
            recvfrom(socketfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, &addr_len);
            printf("Server acknowledgment: %s\n", buffer);

            FILE *file = fopen(filename, "rb");
            if (!file)
            {
                perror("Failed to open file");
                exit(1);
            }

            send_file_chunks(socketfd, file, total_chunks, &server_addr, addr_len);

            fclose(file);
        }
        else if (strcmp(argv[2], "receive") == 0)
        {
            int total_chunks;
            char ack_message[BUFFER_SIZE];
            sendto(socketfd, NULL, 0, 0, (const struct sockaddr *)&server_addr, addr_len);
            recvfrom(socketfd, &total_chunks, sizeof(total_chunks), 0, (struct sockaddr *)&server_addr, &addr_len);
            printf("Received number of chunks: %d\n", total_chunks);

            snprintf(ack_message, sizeof(ack_message), "About to receive %d chunks", total_chunks);
            sendto(socketfd, ack_message, strlen(ack_message), 0, (struct sockaddr *)&server_addr, addr_len);
            printf("Acknowledgment sent: %s\n", ack_message);
            char filename[100] = "received_data.txt";
            FILE *file = fopen(filename, "wb");
            if (!file)
            {
                perror("Failed to open file for writing");
                exit(1);
            }

            receive_file_chunks(socketfd, file, total_chunks, &server_addr, addr_len);

            fclose(file);
        }
        else
        {
            fprintf(stderr, "Usage: %s <server|client> <send|receive> <file> [server_ip]\n", argv[0]);
            exit(1);
        }
        close(socketfd);
    }

    else
    {
        fprintf(stderr, "Usage: %s <server|client> <send|receive> <file> [server_ip]\n", argv[0]);
        exit(1);
    }
}