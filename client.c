
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>


typedef struct message{
	int funcId;
	char senderName[15];
	char senderSurname[25];
	char phNo[11];
	int senderId;
	int receiverId;
	char content[1024];
}message;

struct sockaddr_in* createIPv4Address(char *ip, int port);

int createTCPIpv4Socket();

void startListeningAndPrintMessagesOnNewThread(int fd);

void listenAndPrint(int socketFD);

void sendMessage(int socketFD, char* name, int senderId);

void listContacts(int socketFD, int senderId);

void addUser(int socketFD, int senderId);

void deleteUser(int socketFD, int senderId);

void checkMessages(int socketFD, int senderId);

void listMessages(int socketFD, int senderId);

void displayMessages(int socketFD, int senderId);

void deleteMessage(int socketFD, int senderId);

int main() {
    int socketFD = createTCPIpv4Socket();
    struct sockaddr_in *address = createIPv4Address("127.0.0.1", 2000);


    int result = connect(socketFD,address,sizeof (*address));

    if(result == 0)
        printf("connection was successful\n");

    startListeningAndPrintMessagesOnNewThread(socketFD);
    
    int ch = 0;
    
    
    
    char *name = NULL;
    size_t nameSize= 0;
    printf("please enter your name?\n");
    ssize_t  nameCount = getline(&name,&nameSize,stdin);
    name[nameCount-1]=0;
    
    char *surname = NULL;
    size_t surnameSize= 0;
    printf("please enter your surname?\n");
    ssize_t  surnameCount = getline(&surname,&surnameSize,stdin);
    surname[surnameCount-1]=0;
    
    char *phNo = NULL;
    size_t phNoSize= 0;
    printf("please enter your phone number?\n");
    ssize_t  phNoCount = getline(&phNo,&phNoSize,stdin);
    phNo[phNoCount-1]=0;
    
    int id;
    printf("Please enter your ID: ");
    scanf("%d", &id);
    
    
    
    message* meet = (message*)malloc(sizeof(message));
    meet->funcId=0;
    strcpy(meet->senderName, name);
    strcpy(meet->senderSurname, surname);
    strcpy(meet->phNo, phNo);
    meet->senderId=id;
    
    printf("burdaa\n");
    ssize_t amountWasSent0 =  send(socketFD,
		                   meet,
		                   sizeof(message), 0);

    
    while(ch!=8){
    	printf("1. List Contacts\n2. Add User\n3. Delete User\n4. Send Message\n5. Check Messages\n6. Display Messages\n7. Delete Message\n8. Exit\n");
	scanf("%d", &ch);
	
	switch(ch){
		case 1:
			listContacts(socketFD, id);
			break;
		case 2:
			addUser(socketFD, id);
			break;
		case 3:
			deleteUser(socketFD, id);
			break;
		case 4:
			sendMessage(socketFD, name, id);
			break;
		case 5:
			checkMessages(socketFD, id);
			listMessages(socketFD, id);
			break;
		case 6:
			displayMessages(socketFD, id);
			break;
		case 7:
			deleteMessage(socketFD, id);
			break;
		case 8:
			printf("Exiting the program.");
			break;
		default:
			printf("Invalid choice. Please ented a valid option.");	
		
	}
    }

    close(socketFD);

    return 0;
}

void deleteMessage(int socketFD, int senderId){
    message* mesaj = (message*)malloc(sizeof(message));
    int receiverId;
    printf("Please enter person Id for delete message: ");
    scanf("%d", &receiverId);
    printf("Which message do you want to delete?\n");
    
    char *clean = NULL;
    size_t cleanSize=0;
    ssize_t  cleanCount = getline(&clean,&cleanSize,stdin);
    clean[cleanCount-1]=0;
    
    
    char *line = NULL;
    size_t lineSize= 0;
    ssize_t  charCount = getline(&line,&lineSize,stdin);
    line[charCount-1]=0;
    strcpy(mesaj->content, line);
    
    mesaj->senderId=senderId;
    mesaj->receiverId=receiverId;
    mesaj->funcId=8;
    
    ssize_t amountWasSent8 =  send(socketFD,
		                   mesaj,
		                   sizeof(message), 0);
}

void displayMessages(int socketFD, int senderId){
    int receiverId;
    printf("Please enter person Id: ");
    scanf("%d", &receiverId);
    
    message* mesaj = (message*)malloc(sizeof(message));
    mesaj->senderId=senderId;
    mesaj->receiverId=receiverId;
    mesaj->funcId=7;
    
    ssize_t amountWasSent7 =  send(socketFD,
		                   mesaj,
		                   sizeof(message), 0);
}

void checkMessages(int socketFD, int senderId){
    message* mesaj = (message*)malloc(sizeof(message));
    mesaj->senderId=senderId;
    mesaj->funcId=4;
    
    ssize_t amountWasSent4 =  send(socketFD,
		                   mesaj,
		                   sizeof(message), 0);

}

void listMessages(int socketFD, int senderId){
    int receiverId;
    printf("Whose message do you want to read?");
    scanf("%d", &receiverId);
    
    message* mesaj = (message*)malloc(sizeof(message));
    mesaj->receiverId=receiverId;
    mesaj->senderId=senderId;
    mesaj->funcId=6;
    
    ssize_t amountWasSent6 =  send(socketFD,
		                   mesaj,
		                   sizeof(message), 0);
}

void listContacts(int socketFD, int senderId){
    message* mesaj = (message*)malloc(sizeof(message));
    mesaj->senderId=senderId;
    mesaj->funcId=1;
    
    ssize_t amountWasSent1 =  send(socketFD,
		                   mesaj,
		                   sizeof(message), 0);
}

void deleteUser(int socketFD, int senderId){
    int receiverId;
    printf("Silinecek kisinin Id numarasini giriniz: ");
    scanf("%d", &receiverId);
    
    message* mesaj = (message*)malloc(sizeof(message));
    mesaj->funcId=3;
    mesaj->receiverId=receiverId;
    mesaj->senderId=senderId;
    
    ssize_t amountWasSent3 =  send(socketFD,
		                   mesaj,
		                   sizeof(message), 0);
}

void addUser(int socketFD, int senderId){
    int receiverId;
    printf("Eklenecek kisinin Id numarasini giriniz: ");
    scanf("%d",&receiverId);
    
    message* mesaj = (message*)malloc(sizeof(message));
    mesaj->funcId=2;
    mesaj->receiverId=receiverId;
    mesaj->senderId=senderId;
    
    ssize_t amountWasSent2 =  send(socketFD,
		                   mesaj,
		                   sizeof(message), 0);
    
}

void sendMessage(int socketFD, char* name, int senderId) {
    int receiverId;
    printf("Gonderilecek kisinin Id numarasini giriniz: ");
    scanf("%d",&receiverId);

    /*message* mList = (message*)malloc(sizeof(message));
    mList->senderId=senderId;
    mList->receiverId=receiverId;
    mList->funcId=4;
    
    ssize_t amountWasSent4 =  send(socketFD,
		                  mList,
		                  sizeof(message), 0);*/

    message* mesaj = (message*)malloc(sizeof(message));
    mesaj->funcId=5;
    mesaj->receiverId=receiverId;
    
    char *clean = NULL;
    size_t cleanSize=0;
    ssize_t  cleanCount = getline(&clean,&cleanSize,stdin);
    clean[cleanCount-1]=0;
    	
    printf("type exit to exit.\n");

    char *line = NULL;
    size_t lineSize= 0;
    
    int flag=0;
    while(flag==0){
	    ssize_t  charCount = getline(&line,&lineSize,stdin);
	    line[charCount-1]=0;
	    
	    strcpy(mesaj->content, line);
	    mesaj->senderId=senderId;
	    
	    if(charCount>0){
		if(strcmp(mesaj->content,"exit")==0)
			flag=1;
		else{
		ssize_t amountWasSent =  send(socketFD,
		                              mesaj,
		                              sizeof(message), 0);
	    	}
	    }

    }	
}

void startListeningAndPrintMessagesOnNewThread(int socketFD) {
    pthread_t id ;
    pthread_create(&id,NULL,listenAndPrint,socketFD);
}

void listenAndPrint(int socketFD) {
    char buffer[1024];
    int flag=0;
    while (flag==0)
    {
        ssize_t  amountReceived = recv(socketFD,buffer,1024,0);

        if(amountReceived>0)
        {
            buffer[amountReceived] = '\0';
            printf("%s\n ",buffer);
        }

        if(amountReceived==0)
            flag=1;
    }

    close(socketFD);
}

int createTCPIpv4Socket() { return socket(AF_INET, SOCK_STREAM, 0); }


struct sockaddr_in* createIPv4Address(char *ip, int port) {

    struct sockaddr_in  *address = malloc(sizeof(struct sockaddr_in));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    if(strlen(ip) ==0)
        address->sin_addr.s_addr = INADDR_ANY;
    else
        inet_pton(AF_INET,ip,&address->sin_addr.s_addr);

    return address;
}
