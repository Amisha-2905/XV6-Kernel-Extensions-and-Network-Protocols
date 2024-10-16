
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

char board[9];
char buffer[512];

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

void clear_board()
{
    for (int i = 0; i < 9; i++)
    {
        board[i] = '.';
    }
}

int if_board_full()
{
    int blank_spaces = 0;
    for (int i = 0; i < 9; i++)
    {
        if (board[i] == '.')
        {
            blank_spaces++;
        }
    }
    return blank_spaces;
}

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

int check_winner()
{
    // Check rows
    for (int i = 0; i < 3; i++)
    {
        if (board[i * 3] != '.' && board[i * 3] == board[i * 3 + 1] && board[i * 3] == board[i * 3 + 2])
        {
            return board[i * 3];
        }
    }
    // Check columns
    for (int i = 0; i < 3; i++)
    {
        if (board[i] != '.' && board[i] == board[i + 3] && board[i] == board[i + 6])
        {
            return board[i];
        }
    }
    // Check diagonals
    if (board[0] != '.' && board[0] == board[4] && board[0] == board[8])
    {
        return board[0];
    }
    if (board[2] != '.' && board[2] == board[4] && board[2] == board[6])
    {
        return board[2];
    }
    return -1;
}

void send_board(int *socketfd, struct sockaddr_in *client_addr, socklen_t client_len)
{
    char board_status[10];
    for (int i = 0; i < 9; i++)
    {
        board_status[i] = board[i];
    }
    board_status[9] = '\0';

    if (write(*socketfd, board_status, sizeof(board_status)) < 0)
    {
        perror("Failed to send the board status");
    }
    printf("Board status sent!.\n");
}

void receive_board(int *socketfd, struct sockaddr_in *client_addr, socklen_t *client_len)
{
    char board_status[10];
    memset(board_status, 0, sizeof(board_status));

    if (read(*socketfd, board_status, sizeof(board_status)) < 0)
    {
        perror("Failed to receive the board status");
    }

    for (int i = 0; i < 9; i++)
    {
        board[i] = board_status[i];
    }

    printf("Board status received from client.\n");
}

int main(int argc, char *argv[])
{
    char player_1[50];
    char player_2[50];
    get_ip_address();
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        perror("Socket creation failed!\n");
        exit(1);
    }
    struct sockaddr_in server_addr, client1_addr, client2_addr;
    socklen_t client1_len = sizeof(client1_addr), client2_len = sizeof(client2_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Binding failed");
        exit(1);
    }

    listen(socketfd, 2);

    int newsockfd1 = accept(socketfd, (struct sockaddr *)&client1_addr, &client1_len);

    int newsockfd2 = accept(socketfd, (struct sockaddr *)&client2_addr, &client2_len);

    if (newsockfd1 < 0 || newsockfd2 < 0)
    {
        perror("Accept");
    }

    memset(buffer, 0, sizeof(buffer));
    if (read(newsockfd1, buffer, sizeof(buffer)) < 0)
    {
        perror("Name not able to receive!");
    }
    strcpy(player_1, buffer);
    printf("Name of Player 1: %s\n", player_1);

    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, "Waiting For Player 2!");
    if (write(newsockfd1, buffer, sizeof(buffer)) < 0)
    {
        perror("Sending Wait status!");
    }
    printf("%s\n", buffer);

    memset(buffer, 0, sizeof(buffer));
    if (read(newsockfd2, buffer, sizeof(buffer)) < 0)
    {
        perror("Name not able to receive!");
    }
    strcpy(player_2, buffer);
    printf("Name of Player 2: %s\n", player_2);

    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, "The wait is over!");
    if (write(newsockfd2, buffer, sizeof(buffer)) < 0)
    {
        perror("Sending status to Player 2!");
    }
    printf("%s\n", buffer);

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "Let's start the match between %s and %s!", player_1, player_2);
        if (write(newsockfd1, buffer, sizeof(buffer)) < 0)
        {
            perror("Begin the Match for Player 1");
        }
        if (write(newsockfd2, buffer, sizeof(buffer)) < 0)
        {
            perror("Begin the Match for Player 2");
        }
        printf("%s\n", buffer);

        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, "X");
        if (write(newsockfd1, buffer, sizeof(buffer)) < 0)
        {
            perror("Begin the Match for Player 1");
        }
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, "O");
        if (write(newsockfd2, buffer, sizeof(buffer)) < 0)
        {
            perror("Begin the Match for Player 2");
        }
        printf("Both received their marks!\n");

        clear_board();
        int turn = 1;
        int winner = -1;
        while (winner == -1 && if_board_full() != 0)
        {
            printf("Sending turn status: %s's turn.\n", (turn % 2 == 0) ? player_2 : player_1);
            strcpy(buffer, (turn % 2 == 0) ? player_2 : player_1);
            if (write(newsockfd1, buffer, sizeof(buffer)) < 0)
            {
                perror("Failed to send turn status to Player 1");
            }
            if (write(newsockfd2, buffer, sizeof(buffer)) < 0)
            {
                perror("Failed to send turn status to Player 2");
            }

            printf("Waiting for Player %d's move...\n", turn % 2 == 0 ? 2 : 1);
            if (turn % 2 == 0)
            {
                send_board(&newsockfd2, &client2_addr, client2_len);
                receive_board(&newsockfd2, &client2_addr, &client2_len);
            }
            else
            {
                send_board(&newsockfd1, &client1_addr, client1_len);
                receive_board(&newsockfd1, &client1_addr, &client1_len);
            }
            winner = check_winner();
            turn++;
        }

        if (winner != -1 || if_board_full() == 0)
        {
            strcpy(buffer, "Game_Over");
            if (write(newsockfd1, buffer, sizeof(buffer)) < 0)
            {
                perror("Sending Game over status");
            }
            if (write(newsockfd2, buffer, sizeof(buffer)) < 0)
            {
                perror("Sending Game over status");
            }
            send_board(&newsockfd2, &client2_addr, client2_len);
            send_board(&newsockfd1, &client1_addr, client1_len);
        }

        if (winner != -1)
        {
            if (winner == 'X')
            {
                printf("%s wins!\n", player_1);
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, player_1);
                if (write(newsockfd1, buffer, sizeof(buffer)) < 0)
                {
                    perror("Failed to send winning status to Player 1");
                }
                if (write(newsockfd2, buffer, sizeof(buffer)) < 0)
                {
                    perror("Failed to send winning status to Player 2");
                }
            }
            else
            {
                printf("%s wins!\n", player_2);
                memset(buffer, 0, sizeof(buffer));
                strcpy(buffer, player_2);
                if (write(newsockfd1, buffer, sizeof(buffer)) < 0)
                {
                    perror("Failed to send winning status to Player 1");
                }
                if (write(newsockfd2, buffer, sizeof(buffer)) < 0)
                {
                    perror("Failed to send winning status to Player 2");
                }
            }
        }
        else
        {
            printf("Game ended in a tie!\n");
            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, "None");
            if (write(newsockfd1, buffer, sizeof(buffer)) < 0)
            {
                perror("Failed to send draw status to Player 1");
            }
            if (write(newsockfd2, buffer, sizeof(buffer)) < 0)
            {
                perror("Failed to send draw status to Player 2");
            }
        }

        // Ask both players if they want to play again
        printf("Asking players if they want to play again...\n");
        memset(buffer, 0, sizeof(buffer));
        strcpy(buffer, "Do you want to play again? (yes/no)");

        if (write(newsockfd1, buffer, sizeof(buffer)) < 0)
        {
            perror("Sending play again request to Player 1");
        }
        if (write(newsockfd2, buffer, sizeof(buffer)) < 0)
        {
            perror("Sending play again request to Player 2");
        }

        memset(buffer, 0, sizeof(buffer));
        if (read(newsockfd1, buffer, sizeof(buffer)) < 0)
        {
            perror("Receiving play again response from Player 1");
        }
        char player1_response[10];
        strcpy(player1_response, buffer);

        memset(buffer, 0, sizeof(buffer));
        if (read(newsockfd2, buffer, sizeof(buffer)) < 0)
        {
            perror("Receiving play again response from Player 2");
        }
        char player2_response[10];
        strcpy(player2_response, buffer);

        if (strcmp(player1_response, "yes") == 0 && strcmp(player2_response, "yes") == 0)
        {
            printf("Both players agreed to play again. Restarting...\n");
        }
        else
        {
            memset(buffer, 0, sizeof(buffer));
            printf("One or both players declined to play again. Ending session.\n");
            strcpy(buffer, "no");
            // printf("yutity:%s\n", buffer);
            if (write(newsockfd1, buffer, sizeof(buffer)) < 0)
            {
                perror("Sending end game status to Player 1");
            }
            // printf("------------------.\n");
            if (write(newsockfd2, buffer, sizeof(buffer)) < 0)
            {
                perror("Sending end game status to Player 2");
            }
            break;
        }
    }
    close(socketfd);
    return 0;
}