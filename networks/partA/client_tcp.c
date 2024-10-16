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

#define PORT 8080

char name[50];
char board[9];
char Mark;
char buffer[512];

void print_board()
{
    printf("Current status!\n");
    printf(" %c | %c | %c ", board[0], board[1], board[2]);
    printf("\n---|---|---\n");
    printf(" %c | %c | %c ", board[3], board[4], board[5]);
    printf("\n---|---|---\n");
    printf(" %c | %c | %c ", board[6], board[7], board[8]);
    printf("\n");
    return;
}

void clear_board()
{
    for (int i = 0; i < 9; i++)
    {
        board[i] = '.';
    }
}

void send_board(int socketfd, struct sockaddr_in *server_addr, socklen_t addr_len)
{
    char board_status[10];
    for (int i = 0; i < 9; i++)
    {
        board_status[i] = board[i];
    }
    board_status[9] = '\0';

    if (write(socketfd, board_status, sizeof(board_status)) < 0)
    {
        perror("Failed to send the board status to the server");
    }
    printf("Board status sent to server.\n");
}

void receive_board(int socketfd, struct sockaddr_in *server_addr, socklen_t *addr_len)
{
    char board_status[10];
    memset(board_status, 0, sizeof(board_status));

    if (read(socketfd, board_status, sizeof(board_status)) < 0)
    {
        perror("Failed to receive the board status from the server");
    }

    for (int i = 0; i < 9; i++)
    {
        board[i] = board_status[i];
    }

    printf("Board status received from server.\n");
}

int main(int argc, char *argv[])
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        perror("Socket creation failed!\n");
        exit(1);
    }

    if (argc < 2)
    {
        printf("Usage: %s <IP address>\n", argv[0]);
        exit(1);
    }

    const char *server_ip = argv[1];
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid server IP address: %s\n", server_ip);
        exit(1);
    }

    connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Enter player name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;
    sprintf(buffer, "%s", name);

    if (write(socketfd, buffer, sizeof(buffer)) < 0)
    {
        perror("Sending Name");
    }
    printf("Name of Player: %s\n", buffer);
    memset(buffer, 0, sizeof(buffer));
    if (read(socketfd, buffer, sizeof(buffer)) < 0)
    {
        perror("Receiving wait status");
    }
    printf("%s\n", buffer);
    int play_again = 1;
    while (play_again)
    {
        memset(buffer, 0, sizeof(buffer));
        if (read(socketfd, buffer, sizeof(buffer)) < 0)
        {
            perror("Receiving begin statement");
        }
        printf("%s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
        if (read(socketfd, buffer, sizeof(buffer)) < 0)
        {
            perror("Receiving Mark");
        }
        Mark = buffer[0];
        printf("Your mark is %c\n", Mark);

        clear_board();

        // Game loop
        while (1)
        {
            memset(buffer, 0, sizeof(buffer));
            if (read(socketfd, buffer, sizeof(buffer)) < 0)
            {
                perror("Receiving turn status");
            }
            if (strcmp(buffer, name) == 0)
            {
                printf("Your turn!\n");
                receive_board(socketfd, &server_addr, &addr_len);
                print_board();
                int updated = 0;
                int row, col;
                while (updated == 0)
                {
                    printf("Enter row (1-3): ");
                    scanf("%d", &row);
                    printf("Enter column (1-3): ");
                    scanf("%d", &col);
                    while ((row > 3 || row < 1) || (col > 3 || col < 1))
                    {
                        printf("Invalid row or column!\n");
                        printf("Enter row (1-3): ");
                        scanf("%d", &row);
                        printf("Enter column (1-3): ");
                        scanf("%d", &col);
                    }
                    int index = (row - 1) * 3 + (col - 1);
                    if (board[index] == '.')
                    {
                        board[index] = Mark;
                        updated = 1;
                    }
                    else
                    {
                        printf("Position already taken!\n");
                    }
                }
                send_board(socketfd, &server_addr, addr_len);
            }
            else if (strcmp(buffer, "Game_Over") == 0)
            {
                printf("Game Over!\n");
                break;
            }
            else
            {
                printf("Wait for your turn!\n");
            }
        }

        receive_board(socketfd, &server_addr, &addr_len);
        print_board();

        memset(buffer, 0, sizeof(buffer));
        if (read(socketfd, buffer, sizeof(buffer)) < 0)
        {
            perror("Receiving winning status");
        }

        if (strcmp(buffer, "None") == 0)
        {
            printf("Game ended in a draw!\n");
        }
        else
        {
            printf("%s wins!\n", buffer);
        }

        printf("Do you want to play again? (yes/no): ");
        char response[10];
        scanf("%s", response);

        if (strcmp(response, "yes") == 0)
        {
            strcpy(buffer, "yes");
        }
        else
        {
            strcpy(buffer, "no");
            play_again = 0;
        }

        if (write(socketfd, buffer, sizeof(buffer)) < 0)
        {
            perror("Sending play again status");
        }
        // printf("yutity:%s\n",buffer);
        memset(buffer, 0, sizeof(buffer));
        if (read(socketfd, buffer, sizeof(buffer)) < 0)
        {
            perror("Receiving play again deci sion");
        }
        // printf("---%s----\n",buffer);
        if (strcmp(buffer, "no") == 0)
        {
            printf("Game session ended!\n");
            play_again = 0;
            // close(socketfd);
            break;
        }
    }
    close(socketfd);
    return 0;
}