#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <malloc.h>


typedef struct AcceptedSocket
{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

typedef struct message{
	int funcId;
	char senderName[15];
	char senderSurname[25];
	char phNo[11];
	int senderId;
	int receiverId;
	char content[1024];
}message;

typedef struct kullanici{
	char name[15];
	char surname[25];
	char phNo[11];
	int id;
}kullanici;

typedef struct  contact{
	int id;
	char name[15];
	char surname[25];
	char phNo[11];
}contact;

typedef struct chat{
	int senderId;
	int receiverId;
	char content[1024];
}chat;


struct sockaddr_in* createIPv4Address(char *ip, int port);

int createTCPIpv4Socket();

struct AcceptedSocket * acceptIncomingConnection(int serverSocketFD);

void receiveAndProcess(int socketFD);

void startAcceptingIncomingConnections(int serverSocketFD);

void receiveAndProcessIncomingDataOnSeparateThread(struct AcceptedSocket *pSocket);

void addUserToUserList(message* mesaj, int socketFD);

void addUserToContactsList(message* mesaj);

void sendContactsList(message* mesaj, int socketFD);

void sendMessage(message* mesaj, int socketFD);

void deleteUserFromContactsList(message* mesaj);

void checkMessage(message* mesaj, int socketFD);

void listMessages(message* mesaj, int socketFD);



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


void startAcceptingIncomingConnections(int serverSocketFD) {

    while(true)
    {
        struct AcceptedSocket* clientSocket  = acceptIncomingConnection(serverSocketFD);

        receiveAndProcessIncomingDataOnSeparateThread(clientSocket);
    }
}


void receiveAndProcessIncomingDataOnSeparateThread(struct AcceptedSocket *pSocket) {

    pthread_t id;
    pthread_create(&id,NULL,receiveAndProcess,pSocket->acceptedSocketFD);
}

void addUserToUserList(message* mesaj, int socketFD){
    FILE *file = fopen("users.txt", "a+");

    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }
    
    kullanici tmp;
    
    while (fread(&tmp, sizeof(kullanici), 1, file) == 1) {
        if (tmp.id == mesaj->senderId) {      	
            return;
        }
    }

    kullanici yeniKullanici;
    yeniKullanici.id=mesaj->senderId;
    strcpy(yeniKullanici.name, mesaj->senderName);
    strcpy(yeniKullanici.surname, mesaj->senderSurname);
    strcpy(yeniKullanici.phNo, mesaj->phNo);

    fwrite(&yeniKullanici, sizeof(kullanici), 1, file);

    fclose(file);
}

void addUserToContactsList(message* mesaj){
    int userId = mesaj->senderId;

    char fileName[256];
    snprintf(fileName, sizeof(fileName), "%d_Contacts.txt", userId);

    FILE *file = fopen(fileName, "a+");

    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }
    
    contact tmp;
    
    while (fread(&tmp, sizeof(contact), 1, file) == 1) {
        if (tmp.id == mesaj->receiverId) {      	
            return;
        }
    }

    contact newContact;

    newContact.id = mesaj->receiverId;  // ID'yi tekrar sormaya gerek yok, zaten belirtilmiş.
    
    FILE *nameFile = fopen("users.txt", "r");
    
    if (nameFile == NULL) {
        printf("Error opening file.\n");
        fclose(file);
        return;
    }
    
    kullanici yeniKullanici;
    int flag=0;
    while (fread(&yeniKullanici, sizeof(kullanici), 1, nameFile) == 1 && flag==0) {
        if (yeniKullanici.id == mesaj->receiverId) {
            strcpy(newContact.name, yeniKullanici.name);
            strcpy(newContact.surname, yeniKullanici.surname);
            strcpy(newContact.phNo, yeniKullanici.phNo);
            flag=1;
        }
    }
    
    if(flag==0){
    	fclose(file);
    	return;
    }
    
    fwrite(&newContact, sizeof(contact), 1, file);

    fclose(file);
}

void sendContactsList(message* mesaj, int socketFD){
    int userId = mesaj->senderId;

    char fileName[256];
    snprintf(fileName, sizeof(fileName), "%d_Contacts.txt", userId);
    
    FILE *file = fopen(fileName, "a+");

    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }
    
    contact newContact;
    char buffer[1024];
    
    while (fread(&newContact, sizeof(contact), 1, file) == 1) {
    	sprintf(buffer,"Id: %d , Isim: %s Soyisim: %s TelNo: %s",newContact.id, newContact.name, newContact.surname, newContact.phNo);
    	ssize_t amountWasSent =  send(socketFD,
		                      buffer,
		                      sizeof(buffer), 0);
    }
    
    

    fclose(file);
    	
}

void listMessages(message* mesaj, int socketFD){
    int senderId = mesaj->senderId;
    
    char fileName[256];
    snprintf(fileName, sizeof(fileName), "%d_new.txt", senderId);
    FILE *file = fopen(fileName, "r");
    
    if(file == NULL) {
    	printf("Error opening file.\n");
    	return;
    }
    
    chat newChat;
    char buffer[1024];
    while (fread(&newChat, sizeof(chat), 1, file) == 1) {
    	if(newChat.senderId==mesaj->receiverId){
    		sprintf(buffer,"User %d: %s",newChat.senderId, newChat.content);
        	ssize_t amountWasSent =  send(socketFD,
		                      	      buffer,
		                      	      sizeof(buffer), 0);
    	}
        
    }
    
    fclose(file);
    
    
    //int receiverId = mesaj->receiverId;
    
    //char fileName[256];
    //snprintf(fileName, sizeof(fileName), "%d_new.txt", senderId);
    FILE *newFile = fopen(fileName, "r+");

    if (newFile == NULL) {
        printf("Error opening file.\n");
        return;
    }

    FILE *tempFile = fopen("tmp.txt", "w");

    if (tempFile == NULL) {
        printf("Error opening temporary file.\n");
        fclose(file);
        return;
    }

    //chat newChat;

    while (fread(&newChat, sizeof(chat), 1, newFile) == 1) {
    	if(newChat.senderId!=mesaj->receiverId){
    		fwrite(&newChat, sizeof(chat), 1, tempFile);
    	}
    }

    fclose(newFile);
    fclose(tempFile);

    remove(fileName);
    rename("tmp.txt", fileName);
    
}

void checkMessage(message* mesaj, int socketFD){
    int senderId = mesaj->senderId;
    
    char fileName[256];
    snprintf(fileName, sizeof(fileName), "%d_new.txt", senderId);
    FILE *file = fopen(fileName, "r");
    
    if(file == NULL) {
    	printf("Error opening file.\n");
    	return;
    }
    
    chat newChat;
    char buffer[1024];
    while (fread(&newChat, sizeof(chat), 1, file) == 1) {
        sprintf(buffer,"You have new message from User %d",newChat.senderId);
        ssize_t amountWasSent =  send(socketFD,
		                      buffer,
		                      sizeof(buffer), 0);
    }
    
    fclose(file);
    
}

void deleteMessage(message* mesaj, int socketFD){
    int senderId = mesaj->senderId;
    int receiverId = mesaj->receiverId;
    char fileName[256];
    
    if(senderId<receiverId){
    	snprintf(fileName, sizeof(fileName), "%d_%d.txt", senderId, receiverId);
    }else{
    	snprintf(fileName, sizeof(fileName), "%d_%d.txt", receiverId, senderId);
    }
    
    FILE *file = fopen(fileName, "r+");
    
    
    
    FILE *tempFile = fopen("temp.txt", "w");

    if (tempFile == NULL) {
        printf("Error opening temporary file.\n");
        fclose(file);
        return;
    }

    chat newChat;

    while (fread(&newChat, sizeof(chat), 1, file) == 1) {
        if (strcmp(newChat.content, mesaj->content)!=0) {
            fwrite(&newChat, sizeof(chat), 1, tempFile);
        }
    }

    fclose(file);
    fclose(tempFile);

    remove(fileName);
    rename("temp.txt", fileName);
}

void displayMessages(message* mesaj, int socketFD){
    int senderId = mesaj->senderId;
    int receiverId = mesaj->receiverId;
    char fileName[256];
    
    if(senderId<receiverId){
    	snprintf(fileName, sizeof(fileName), "%d_%d.txt", senderId, receiverId);
    }else{
    	snprintf(fileName, sizeof(fileName), "%d_%d.txt", receiverId, senderId);
    }
    
    FILE *file = fopen(fileName, "r");
    
    chat newChat;
    char buffer[1024];
    while (fread(&newChat, sizeof(chat), 1, file) == 1) {
    	if(senderId==newChat.senderId){
    		sprintf(buffer,"You: %s",newChat.content);
    	}else{
    		sprintf(buffer,"User %d: %s",newChat.senderId, newChat.content);
    	}
        
        ssize_t amountWasSent =  send(socketFD,
		                      buffer,
		                      sizeof(buffer), 0);
    }
    
    fclose(file);
}

void sendMessage(message* mesaj, int socketFD){
    int senderId = mesaj->senderId;
    int receiverId = mesaj->receiverId;
    

    char fileName[256];
    snprintf(fileName, sizeof(fileName), "%d_Contacts.txt", senderId);

    FILE *tempFile = fopen(fileName, "r");

    if (tempFile == NULL) {
        printf("Error opening file.\n");
        return;
    }
    
    contact tmpContact;
    int flag=0;
    while (fread(&tmpContact, sizeof(contact), 1, tempFile) == 1) {
        if (tmpContact.id == mesaj->receiverId) {      	
            flag=1;
        }
    }
    char buffer[1024];
    if(flag==0){
    	sprintf(buffer,"This user is not in your contact list.");
        ssize_t amountWasSent =  send(socketFD,
		                      buffer,
		                      sizeof(buffer), 0);
    	fclose(tempFile);
    	return;
    }
    fclose(tempFile);
    
    //char fileName[256];
     
    if(senderId<receiverId){
    	snprintf(fileName, sizeof(fileName), "%d_%d.txt", senderId, receiverId);
    }else{
    	snprintf(fileName, sizeof(fileName), "%d_%d.txt", receiverId, senderId);
    }

    FILE *file = fopen(fileName, "a+");

    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }
    
    chat newChat;
    newChat.senderId = mesaj->senderId;
    newChat.receiverId = mesaj->receiverId;
    strcpy(newChat.content, mesaj->content);
    
    fwrite(&newChat, sizeof(chat), 1, file);

    fclose(file);
    
    snprintf(fileName, sizeof(fileName), "%d_new.txt", receiverId);
    
    FILE *tmpfile = fopen(fileName, "a+");
    
    if(tmpfile == NULL) {
    	printf("Error opening file.\n");
    	return;
    }
    
    fwrite(&newChat, sizeof(chat), 1, tmpfile);
    
    fclose(tmpfile);
    
}

void deleteUserFromContactsList(message* mesaj){
    int receiverId = mesaj->receiverId;
    
    int senderId = mesaj->senderId;

    char fileName[256];
    snprintf(fileName, sizeof(fileName), "%d_Contacts.txt", senderId);

    FILE *file = fopen(fileName, "a+");

    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    FILE *tempFile = fopen("temp.txt", "w");

    if (tempFile == NULL) {
        printf("Error opening temporary file.\n");
        fclose(file);
        return;
    }

    contact newContact;

    while (fread(&newContact, sizeof(contact), 1, file) == 1) {
        if (newContact.id != receiverId) {
            fwrite(&newContact, sizeof(contact), 1, tempFile);
        }
    }

    fclose(file);
    fclose(tempFile);

    remove(fileName);
    rename("temp.txt", fileName);

    printf("User with ID %d deleted.\n", receiverId);
}



void receiveAndProcess(int socketFD) {
    message* mesaj = (message*)malloc(sizeof(message));

    while (true)
    {
        ssize_t  amountReceived = recv(socketFD,mesaj,sizeof(message),0);

        if(amountReceived>0)
        {
            printf("%d\n", mesaj->senderId);
            printf("%d\n", mesaj->receiverId);
            printf("%s\n", mesaj->content);	
            printf("func bu %d\n", mesaj->funcId);
            
            switch(mesaj->funcId){
            	case 0:
            		addUserToUserList(mesaj, socketFD);
            		break;
            	case 1:
            		sendContactsList(mesaj, socketFD);
            		break;
            	case 2:
            		addUserToContactsList(mesaj);
            		break;
            	case 3:
            		deleteUserFromContactsList(mesaj);
            		break;
            	case 4:
            		checkMessage(mesaj, socketFD);
            		break;
            	case 5:
            		sendMessage(mesaj, socketFD);
            		break;
            	case 6:
            		listMessages(mesaj, socketFD);
            		break;
            	case 7:
            		displayMessages(mesaj, socketFD);
            		break;
            	case 8:
            		deleteMessage(mesaj, socketFD);
            		break;
            	default:
            		printf("Invalid function Id\n");
            	
            }

        }

        if(amountReceived==0)
            break;
    }

    close(socketFD);
}

struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD) {
    struct sockaddr_in  clientAddress ;
    int clientAddressSize = sizeof (struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD,&clientAddress,&clientAddressSize);

    struct AcceptedSocket* acceptedSocket = malloc(sizeof (struct AcceptedSocket));
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = clientSocketFD>0;

    if(!acceptedSocket->acceptedSuccessfully)
        acceptedSocket->error = clientSocketFD;

    return acceptedSocket;
}


int main() {

    int serverSocketFD = createTCPIpv4Socket();
    struct sockaddr_in *serverAddress = createIPv4Address("",2000);

    int result = bind(serverSocketFD,serverAddress, sizeof(*serverAddress));
    if(result == 0)
        printf("socket was bound successfully\n");

    int listenResult = listen(serverSocketFD,10);

    startAcceptingIncomingConnections(serverSocketFD);

    shutdown(serverSocketFD,SHUT_RDWR);

    return 0;
}
