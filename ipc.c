#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SIZE 100
#define KEY 1234
#define PASSWORD "admin"

// ------------------ Encryption ------------------
void encrypt(char *msg) {
    for (int i = 0; msg[i] != '\0'; i++) {
        msg[i] ^= 5;
    }
}

void decrypt(char *msg) {
    encrypt(msg); // XOR reversible
}

// ------------------ AUTH ------------------
int authenticate() {
    char pass[20];
    printf("Enter Password: ");
    scanf("%s", pass);

    if (strcmp(pass, PASSWORD) == 0) {
        return 1;
    } else {
        printf("❌ Access Denied!\n");
        return 0;
    }
}

// ------------------ PIPE ------------------
void pipeIPC() {
    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    if (pid == 0) {
        // Child (Sender)
        char msg[SIZE];
        printf("Enter message (Pipe): ");
        scanf(" %[^\n]", msg);

        encrypt(msg);
        write(fd[1], msg, sizeof(msg));

        exit(0);   // ✅ ADD THIS LINE
    } else {
        // Parent (Receiver)
        char buffer[SIZE];

        wait(NULL);
        read(fd[0], buffer, sizeof(buffer));
        decrypt(buffer);

        printf("Received via Pipe: %s\n", buffer);
    }
}

// ------------------ MESSAGE QUEUE ------------------
struct msg_buffer {
    long msg_type;
    char msg_text[SIZE];
};

void messageQueueIPC() {
    int msgid;
    struct msg_buffer message;

    msgid = msgget(KEY, 0666 | IPC_CREAT);

    printf("Enter message (Queue): ");
    scanf(" %[^\n]", message.msg_text);

    message.msg_type = 1;
    encrypt(message.msg_text);

    msgsnd(msgid, &message, sizeof(message), 0);

    msgrcv(msgid, &message, sizeof(message), 1, 0);
    decrypt(message.msg_text);

    printf("Received via Message Queue: %s\n", message.msg_text);

    msgctl(msgid, IPC_RMID, NULL);
}

// ------------------ SHARED MEMORY ------------------
void sharedMemoryIPC() {
    int shmid;
    char *str;

    shmid = shmget(KEY, SIZE, 0666 | IPC_CREAT);
    str = (char*) shmat(shmid, NULL, 0);

    printf("Enter message (Shared Memory): ");
    scanf(" %[^\n]", str);

    encrypt(str);

    printf("Encrypted stored in memory...\n");

    decrypt(str);
    printf("Received via Shared Memory: %s\n", str);

    shmdt(str);
    shmctl(shmid, IPC_RMID, NULL);
}

// ------------------ MAIN ------------------
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: ./ipc <method> <message>\n");
        return 1;
    }

    char *method = argv[1];
    char message[SIZE];
    strcpy(message, argv[2]);

    // 🔐 Encrypt before processing
    encrypt(message);

    // ---------------- PIPE ----------------
    if (strcmp(method, "pipe") == 0) {
        int fd[2];
        pipe(fd);

        if (fork() == 0) {
            write(fd[1], message, strlen(message) + 1);
            exit(0);
        } else {
            wait(NULL);
            char buffer[SIZE];
            read(fd[0], buffer, SIZE);
            decrypt(buffer);
            printf("Received via PIPE: %s\n", buffer);
        }
    }

    // ---------------- MESSAGE QUEUE ----------------
    else if (strcmp(method, "queue") == 0) {
        struct msg_buffer msg;
        int msgid = msgget(KEY, 0666 | IPC_CREAT);

        msg.msg_type = 1;
        strcpy(msg.msg_text, message);

        msgsnd(msgid, &msg, sizeof(msg), 0);
        msgrcv(msgid, &msg, sizeof(msg), 1, 0);

        decrypt(msg.msg_text);
        printf("Received via QUEUE: %s\n", msg.msg_text);

        msgctl(msgid, IPC_RMID, NULL);
    }

    // ---------------- SHARED MEMORY ----------------
    else if (strcmp(method, "shared") == 0) {
        int shmid = shmget(KEY, SIZE, 0666 | IPC_CREAT);
        char *str = (char*) shmat(shmid, NULL, 0);

        strcpy(str, message);

        decrypt(str);
        printf("Received via SHARED MEMORY: %s\n", str);

        shmdt(str);
        shmctl(shmid, IPC_RMID, NULL);
    }

    else {
        printf("Invalid method\n");
    }

    return 0;
}