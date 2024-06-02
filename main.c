#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
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
    pid_t pid;
    char *addr;
    struct stat statbuf;
    key_t key;
    int msqid;
    struct msgbuf msg;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        exit(1);
    }

    // 파일 입/출력을 사용하는 코드
    // 파일 생성 및 "abcd" 쓰기
    fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    if (write(fd, "abcd", 4) != 4) {
        perror("write");
        close(fd);
        exit(1);
    }

    // 파일 정보 활용하는 코드
    // 파일 크기 가져오기
    if (stat(argv[1], &statbuf) == -1) {
        perror("stat");
        close(fd);
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

    // 시스템 V의 프로세스 간 통신(메시지 큐) 활용하는 코드
    // 메시지 큐 생성
    key = MSGKEY;
    msqid = msgget(key, IPC_CREAT | 0666);
    if (msqid == -1) {
        perror("msgget");
        exit(1);
    }

   
    // 프로세스 생성 및 메모리 매핑 영역 사용
    switch (pid = fork()) {
        case -1: /* fork 실패 */
            perror("fork");
            exit(1);
            break;
        case 0: /* 자식 프로세스 */
	    // exec 함수군 활용하는 코드
            execlp("./child", "child", argv[1], NULL);
            perror("execlp");
            exit(1);
            break;
        default: /* 부모 프로세스 */
            printf("1. Parent process: addr=%s\n", addr);
            sleep(2);
            printf("2. Parent process: addr=%s\n", addr);
            addr[1] = 'y';
            printf("3. Parent process: addr=%s\n", addr);

            // 메시지 큐에서 자식 프로세스의 종료 메시지 대기
            if (msgrcv(msqid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
                perror("msgrcv");
                exit(1);
            }
            printf("Parent process received message: %s\n", msg.mtext);

            // 메시지 큐 제거
            if (msgctl(msqid, IPC_RMID, NULL) == -1) {
                perror("msgctl");
                exit(1);
            }
            break;
    }

    // 메모리 매핑 해제
    if (munmap(addr, statbuf.st_size) == -1) {
        perror("munmap");
        exit(1);
    }

    return 0;
}

