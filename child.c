#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common.h"

#define MSGKEY 1234

struct msgbuf {
    long mtype;
    char mtext[100];
};

int main(int argc, char *argv[]) {
    int fd;
    char *addr;
    struct stat statbuf;
    key_t key;
    int msqid;
    struct msgbuf msg;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        exit(1);
    }

    // 파일 정보 활용하는 코드
    // 파일 크기 가져오기
    if (stat(argv[1], &statbuf) == -1) {
        perror("stat");
        exit(1);
    }

    // 파일 입/출력을 사용하는 코드
    // 파일 열기
    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    // IPC(메모리 매핑) 활용하는 코드
    // 파일 메모리 매핑
    addr = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(1);
    }
    close(fd);

    printf("1. Child Process: addr=%s\n", addr);
    sleep(1);
    addr[0] = 'x';
    printf("2. Child Process: addr=%s\n", addr);
    sleep(2);
    printf("3. Child Process: addr=%s\n", addr);

    // 시스템 V의 프로세스 간 통신 활용하는 코드
    // 메시지 큐 생성
    key = MSGKEY;
    msqid = msgget(key, 0666);
    if (msqid == -1) {
        perror("msgget");
        exit(1);
    }

    // 자식 프로세스가 종료될 때 메시지 큐에 메시지 전송
    msg.mtype = 1;
    strcpy(msg.mtext, "Child Process Completed");
    if (msgsnd(msqid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    // IPC 활용하는 코드
    // 메모리 매핑 해제
    if (munmap(addr, statbuf.st_size) == -1) {
        perror("munmap");
        exit(1);
    }

    return 0;
}

