#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<ctype.h>
#include<time.h>

#include<errno.h>
#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

void error(const char *msg) //
{                           //  ��ܿ��~�T��
    perror(msg);            //
    exit(1);                //
}                           //

int main(int argc, char *argv[])
{
    time_t now;                                                           	//�ΨӨ��o�ثe�ɶ�

    int sockfd, newsockfd, portno , logperfive , log = 0 , logcount = 0;	//sockfd,newsockfd,portn,clilen �O�إ߳s�u���ܼ� 
    socklen_t clilen; 														//logperfive , log , logcount �O�O���ǰe�i�� ���ܼ� 

    unsigned char sendbuf[512];                                             //�ǰedata �ҥΪ�buffer(udp)
    unsigned char recvbuf[512];                                           	//�ǰedata �ҥΪ�buffer(udp)
    unsigned char buffer[512];                                             	//�ǰedata �ҥΪ�buffer(tcp) 

    struct sockaddr_in serv_addr, cli_addr;                            		//�إ߳s�u�һ��ܼ� 
    struct hostent *server;                                             	//�إ߳s�u�һ��ܼ� 

    FILE *fp;                                                            	//file��pointer 

    if(!strcmp(argv[1],"tcp"))                                             	//�P�_�O�_��TCP
    {
        if(!strcmp(argv[2],"send"))                                        	//�P�_�O�_���ǰe��� 
        {
            int n; 
            sockfd = socket(AF_INET, SOCK_STREAM, 0);                   	//��socket()
            if (sockfd < 0) error("ERROR opening socket\n");                //�P�_socket�}�Ҧ��L���~ 

            bzero((char *) &serv_addr, sizeof(serv_addr));                  //bind() �һݪ��Ѽ�
            portno = atoi(argv[4]);                                         //bind() �һݪ��Ѽ�
            serv_addr.sin_family = AF_INET;                                 //bind() �һݪ��Ѽ�
            serv_addr.sin_addr.s_addr = INADDR_ANY;                         //bind() �һݪ��Ѽ�
            serv_addr.sin_port = htons(portno);                             //bind() �һݪ��Ѽ�
            if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
				error("ERROR on binding\n"); 								//bind() �çP�_���L���~ 

            listen(sockfd,5);                                           	//��listen()

            clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);	//accept() �P�N�o�X�ШD����H
            if (newsockfd < 0) error("ERROR on accept\n");                      //�P�_accept���L���~ 

            fp=fopen(argv[5],"rb");                                         //�}��
            if(!fp)
            	error("Error\n");
            	
			int sz=0, s=0;
			fseek(fp, 0L, SEEK_END);
			sz = ftell(fp);													//�p���ɮפj�p 
			logperfive = sz*0.25 ;
			rewind(fp);
            n = write(newsockfd, &sz, sizeof(int));                         //�N���ǰe���ɮפj�p �ǵ�������
            if (n < 0) error("ERROR writing to socket\n");					

			clock_t start = clock(), end;									//�����ɶ��һݪ��ܼ� 
            while(1) {
				bzero(buffer, 512);											//�M��buffer 
                fread(buffer, sizeof(unsigned char), 512, fp);             	//Ū���ɮש�ibuffer  �C��512bytes

                n = write(newsockfd,buffer,512);                            //�Nbuffer�ǰe��������
                if (n < 0) error("ERROR writing to socket\n");
                
				if(feof(fp)) break;											//Ū���ɮ״N�����ǰe
				
                logcount+=512;                                              //logcount�C���[512
				if(log == 0) {
					time(&now);                                            	//���o�ثe�ɶ�
				    printf("%d%% %s" , log , ctime(&now)); 					//�L�X�w�ǰe�h�֥H�ήɶ� 
					log+=25;
				}
                if((logcount >= logperfive && log != 100) || log==100) {	//�P�_logcount�O�_�����ɮפj�p��25%
                    logcount = 0;                                          	//logcount�k�s
                    time(&now);
                    printf("%d%% %s" , log , ctime(&now));
					log+=25;
                }
            }
			end = clock();
			printf("Total trans time: %fms\n", (double)(end-start)/1000);	//�L�X��O�ɶ� 
			printf("file size : %dMB\n", sz/1000/1000);						//�L�X�ɮפj�p 
		
            close(newsockfd);												//close socket
            close(sockfd);													//close socket
        }
        else if(!strcmp(argv[2],"recv"))                  					//�P�_�O�_��������� 
        {
	    	int n;
            char file_name[512] = {'r','e','c','e','i','v','e','.','t','x','t'};
            
            sockfd = socket(AF_INET, SOCK_STREAM, 0);                       //��socket()
            if (sockfd < 0) error("ERROR opening socket\n");                //�P�_socket�}�Ҧ��L���~ 

            portno = atoi(argv[4]);                                         //�]�wport
            server = gethostbyname(argv[3]);                              	//�]�wserver IP
            if (server == NULL) {                                           //�P�_���o���S�����~ 
                fprintf(stderr,"ERROR, no such host\n");
                exit(0);
            }

            bzero((char *) &serv_addr, sizeof(serv_addr));                 	//connect()�һݪ��Ѽ�
            serv_addr.sin_family = AF_INET;                               	//connect()�һݪ��Ѽ�
            bcopy((char *)server->h_addr , (char *)&serv_addr.sin_addr.s_addr,server->h_length);	//connect()�һݪ��Ѽ�
            serv_addr.sin_port = htons(portno);                             //connect()�һݪ��Ѽ�
            if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
				error("ERROR connecting\n");								//connect()�çP�_�O�_�����~ 

            bzero(buffer,512);                                            	//�M��buffer

            fp = fopen(file_name,"wb");                                    	//�}��
            if(!fp)
            	error("Error\n");
            int sz=0, s=0;    
            n = read(sockfd, &sz, sizeof(int));                             //����'�ǰe�ɮ�'���j�p
            if (n < 0) error("ERROR reading from socket\n");
            while(s <= sz) {                                             	//s�C���[512 ���쵥���ɮפj�p 
				bzero(buffer, 512);											//�M��buffer 
                n = read(sockfd , buffer , 512);                          	//����server�e�Ӫ����
                if (n < 0) error("ERROR reading from socket\n");
				s+=512;
				if(s >= sz) {												//�̫�i��buffer����512 �u�ݼgbuffer������� 
					for(int i=0; i<512; ++i) {
						if(!buffer[i]) {
							fwrite(buffer, sizeof(unsigned char), i, fp);
							break;
						}
					}
					break;
				}
                fwrite(buffer, sizeof(unsigned char), 512, fp);         	//�N��Ƽg�J�ɮפ�
            }
            printf("Success\n");
            close(sockfd);													//close socket
        }
    }
    else if(!strcmp(argv[1],"udp"))                                        	//�P�_�O�_��udp
    {
        if(!strcmp(argv[2],"send"))                                      	//�P�_�O�_���ǰe��� 
        {
			int request = 0;
            if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)				//��socket()�çP�_�}�Ҧ��L���~
				error("socket error");

            bzero((char *) &serv_addr, sizeof(serv_addr));              	//bind()�һݪ��Ѽ�
            portno = atoi(argv[4]);                                        	//bind()�һݪ��Ѽ�
            serv_addr.sin_family = AF_INET;                                	//bind()�һݪ��Ѽ�
            serv_addr.sin_port = htons(portno);                            	//bind()�һݪ��Ѽ�
            serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);                 	//bind()�һݪ��Ѽ�
            if(bind (sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
				error("bind error");										//bind()�çP�_�O�_�����~ 

            clilen = sizeof(cli_addr);
			recvfrom(sockfd, &request, sizeof(int), 0, (struct sockaddr *)&cli_addr, &clilen);	//����request
			if(request) {
	            fp=fopen(argv[5],"rb");                                   	//�}��
	            if(!fp)
            		error("Error\n");
	
				int sz=0, s=0;
				fseek(fp, 0L, SEEK_END);
				sz = ftell(fp);												//�p���ɮפj�p 
				logperfive = sz*0.25 ;
				rewind(fp);
	
	            sendto(sockfd, &sz, sizeof(int), 0 , (struct sockaddr *)&cli_addr, clilen);	//�N���ǰe���ɮפj�p�ǵ�������
	            
				clock_t start = clock(), end;								//�����ɶ��һݪ��ܼ� 
	            while(1) {
					bzero(sendbuf,512);										//�M��buffer 
	                fread(sendbuf, sizeof(unsigned char), 511 , fp);       	//Ū���ɮש�ibuffer  �C��512bytes
	                
	                sendto(sockfd,sendbuf,strlen(sendbuf), 0 , (struct sockaddr *)&cli_addr, clilen);	//�Nbuffer�ǰe��������
	                if(feof(fp)) break;                                     //Ū���ɮ״N�����ǰe 
	                logcount+=511;                                         	//logcount�C���[511 
					if(log == 0) {
						time(&now);                                       	//���o�ثe�ɶ�
					    printf("%d%% %s" , log , ctime(&now));				//�L�X�w�ǰe�h�֥H�ήɶ� 
						log+=25;
					}
	                if((logcount >= logperfive && log != 100) || log==100) {	//�P�_logcount�O�_�����ɮפj�p��25%
	                    logcount = 0;                                      	//logcount�k�s
	                    time(&now);
	                    printf("%d%% %s" , log , ctime(&now));
						log+=25;
	                }
	            }
				end = clock();
				printf("Total trans time: %fms\n", (double)(end-start)/1000);	//�L�X��O�ɶ� 
				printf("file size : %dMB\n", sz/1000/1000);						//�L�X�ɮפj�p 
			}
			else
				printf("No request\n");
		
            close(sockfd);												//close socket
        }
        else if(!strcmp(argv[2],"recv"))                            	//�P�_�O�_�������ɮ� 
        {
            int ret , lose=0, request=1;
            struct timeval timeout={0,1};
            char file_name[512] = {'r','e','c','e','i','v','e','.','t','x','t'};
       
            if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)			//��socket()�çP�_�}�Ҧ��L���~ 
				error("socket");

            bzero((char *) &serv_addr, sizeof(serv_addr));
            portno = atoi(argv[4]);                                     //�]�wUDP�һݰѼ�
            server = gethostbyname(argv[3]);                    		//�]�wUDP�һݰѼ�
            serv_addr.sin_family = AF_INET;                            	//�]�wUDP�һݰѼ�
            serv_addr.sin_port = htons(portno);                       	//�]�wUDP�һݰѼ�
            bcopy((char *)server->h_addr , (char *)&serv_addr.sin_addr.s_addr,server->h_length);	//�]�wUDP�һݰѼ�

            bzero(recvbuf,512);                                       	//�M��buffer
			sendto(sockfd, &request, sizeof(int) , 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));	//�ǰerequest

            fp = fopen(file_name,"wb");                                	//�}��
            if(!fp)
            	error("Error\n");
			int sz=0, s=0;
            recvfrom(sockfd, &sz, sizeof(int), 0, NULL , NULL);       	//����'�ǰe�ɮ�'���ɮפj�p
            setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));	//����recv()���W��
            while(s <= sz) {											//s�C���[512 ���쵥���ɮפj�p 
				bzero(recvbuf, 512);									//�M��buffer
                ret=recvfrom(sockfd , recvbuf , sizeof(recvbuf)-1, 0, NULL , NULL);	//����server�e�Ӫ����

                if(ret==-1) {
                    lose++;                                            	//�p��lose
                    if(lose > 100)break;
                }
                else {
                    fwrite(recvbuf, sizeof(unsigned char), strlen(recvbuf), fp);	//�N��Ƽg�J�ɮפ�
                    s+=strlen(recvbuf);
                }
            }
            if(!lose)													//�p�G�S��lose�h���������ɮ�
				printf("Success\n");
            else														//�p�G��lose�h�L�X�ɮ��`packets��lose��packets�ƶq 
				printf("Total packets: %d   lose %d packets\n", sz, sz-s);
            close(sockfd);												//close socket
        }
    }
    return 0;
}
