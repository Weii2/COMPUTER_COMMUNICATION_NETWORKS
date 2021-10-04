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
{                           //  顯示錯誤訊息
    perror(msg);            //
    exit(1);                //
}                           //

int main(int argc, char *argv[])
{
    time_t now;                                                           	//用來取得目前時間

    int sockfd, newsockfd, portno , logperfive , log = 0 , logcount = 0;	//sockfd,newsockfd,portn,clilen 是建立連線的變數 
    socklen_t clilen; 														//logperfive , log , logcount 是記錄傳送進度 的變數 

    unsigned char sendbuf[512];                                             //傳送data 所用的buffer(udp)
    unsigned char recvbuf[512];                                           	//傳送data 所用的buffer(udp)
    unsigned char buffer[512];                                             	//傳送data 所用的buffer(tcp) 

    struct sockaddr_in serv_addr, cli_addr;                            		//建立連線所需變數 
    struct hostent *server;                                             	//建立連線所需變數 

    FILE *fp;                                                            	//file的pointer 

    if(!strcmp(argv[1],"tcp"))                                             	//判斷是否為TCP
    {
        if(!strcmp(argv[2],"send"))                                        	//判斷是否為傳送資料 
        {
            int n; 
            sockfd = socket(AF_INET, SOCK_STREAM, 0);                   	//做socket()
            if (sockfd < 0) error("ERROR opening socket\n");                //判斷socket開啟有無錯誤 

            bzero((char *) &serv_addr, sizeof(serv_addr));                  //bind() 所需的參數
            portno = atoi(argv[4]);                                         //bind() 所需的參數
            serv_addr.sin_family = AF_INET;                                 //bind() 所需的參數
            serv_addr.sin_addr.s_addr = INADDR_ANY;                         //bind() 所需的參數
            serv_addr.sin_port = htons(portno);                             //bind() 所需的參數
            if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
				error("ERROR on binding\n"); 								//bind() 並判斷有無錯誤 

            listen(sockfd,5);                                           	//做listen()

            clilen = sizeof(cli_addr);
            newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);	//accept() 同意發出請求的對象
            if (newsockfd < 0) error("ERROR on accept\n");                      //判斷accept有無錯誤 

            fp=fopen(argv[5],"rb");                                         //開檔
            if(!fp)
            	error("Error\n");
            	
			int sz=0, s=0;
			fseek(fp, 0L, SEEK_END);
			sz = ftell(fp);													//計算檔案大小 
			logperfive = sz*0.25 ;
			rewind(fp);
            n = write(newsockfd, &sz, sizeof(int));                         //將欲傳送的檔案大小 傳給接收端
            if (n < 0) error("ERROR writing to socket\n");					

			clock_t start = clock(), end;									//紀錄時間所需的變數 
            while(1) {
				bzero(buffer, 512);											//清空buffer 
                fread(buffer, sizeof(unsigned char), 512, fp);             	//讀取檔案放進buffer  每次512bytes

                n = write(newsockfd,buffer,512);                            //將buffer傳送給接收端
                if (n < 0) error("ERROR writing to socket\n");
                
				if(feof(fp)) break;											//讀完檔案就結束傳送
				
                logcount+=512;                                              //logcount每次加512
				if(log == 0) {
					time(&now);                                            	//取得目前時間
				    printf("%d%% %s" , log , ctime(&now)); 					//印出已傳送多少以及時間 
					log+=25;
				}
                if((logcount >= logperfive && log != 100) || log==100) {	//判斷logcount是否有到檔案大小的25%
                    logcount = 0;                                          	//logcount歸零
                    time(&now);
                    printf("%d%% %s" , log , ctime(&now));
					log+=25;
                }
            }
			end = clock();
			printf("Total trans time: %fms\n", (double)(end-start)/1000);	//印出花費時間 
			printf("file size : %dMB\n", sz/1000/1000);						//印出檔案大小 
		
            close(newsockfd);												//close socket
            close(sockfd);													//close socket
        }
        else if(!strcmp(argv[2],"recv"))                  					//判斷是否為接收資料 
        {
	    	int n;
            char file_name[512] = {'r','e','c','e','i','v','e','.','t','x','t'};
            
            sockfd = socket(AF_INET, SOCK_STREAM, 0);                       //做socket()
            if (sockfd < 0) error("ERROR opening socket\n");                //判斷socket開啟有無錯誤 

            portno = atoi(argv[4]);                                         //設定port
            server = gethostbyname(argv[3]);                              	//設定server IP
            if (server == NULL) {                                           //判斷取得有沒有錯誤 
                fprintf(stderr,"ERROR, no such host\n");
                exit(0);
            }

            bzero((char *) &serv_addr, sizeof(serv_addr));                 	//connect()所需的參數
            serv_addr.sin_family = AF_INET;                               	//connect()所需的參數
            bcopy((char *)server->h_addr , (char *)&serv_addr.sin_addr.s_addr,server->h_length);	//connect()所需的參數
            serv_addr.sin_port = htons(portno);                             //connect()所需的參數
            if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
				error("ERROR connecting\n");								//connect()並判斷是否有錯誤 

            bzero(buffer,512);                                            	//清空buffer

            fp = fopen(file_name,"wb");                                    	//開檔
            if(!fp)
            	error("Error\n");
            int sz=0, s=0;    
            n = read(sockfd, &sz, sizeof(int));                             //接收'傳送檔案'的大小
            if (n < 0) error("ERROR reading from socket\n");
            while(s <= sz) {                                             	//s每次加512 直到等於檔案大小 
				bzero(buffer, 512);											//清空buffer 
                n = read(sockfd , buffer , 512);                          	//接收server送來的資料
                if (n < 0) error("ERROR reading from socket\n");
				s+=512;
				if(s >= sz) {												//最後可能buffer不足512 只需寫buffer有的資料 
					for(int i=0; i<512; ++i) {
						if(!buffer[i]) {
							fwrite(buffer, sizeof(unsigned char), i, fp);
							break;
						}
					}
					break;
				}
                fwrite(buffer, sizeof(unsigned char), 512, fp);         	//將資料寫入檔案中
            }
            printf("Success\n");
            close(sockfd);													//close socket
        }
    }
    else if(!strcmp(argv[1],"udp"))                                        	//判斷是否為udp
    {
        if(!strcmp(argv[2],"send"))                                      	//判斷是否為傳送資料 
        {
			int request = 0;
            if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)				//做socket()並判斷開啟有無錯誤
				error("socket error");

            bzero((char *) &serv_addr, sizeof(serv_addr));              	//bind()所需的參數
            portno = atoi(argv[4]);                                        	//bind()所需的參數
            serv_addr.sin_family = AF_INET;                                	//bind()所需的參數
            serv_addr.sin_port = htons(portno);                            	//bind()所需的參數
            serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);                 	//bind()所需的參數
            if(bind (sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
				error("bind error");										//bind()並判斷是否有錯誤 

            clilen = sizeof(cli_addr);
			recvfrom(sockfd, &request, sizeof(int), 0, (struct sockaddr *)&cli_addr, &clilen);	//接收request
			if(request) {
	            fp=fopen(argv[5],"rb");                                   	//開檔
	            if(!fp)
            		error("Error\n");
	
				int sz=0, s=0;
				fseek(fp, 0L, SEEK_END);
				sz = ftell(fp);												//計算檔案大小 
				logperfive = sz*0.25 ;
				rewind(fp);
	
	            sendto(sockfd, &sz, sizeof(int), 0 , (struct sockaddr *)&cli_addr, clilen);	//將欲傳送的檔案大小傳給接收端
	            
				clock_t start = clock(), end;								//紀錄時間所需的變數 
	            while(1) {
					bzero(sendbuf,512);										//清空buffer 
	                fread(sendbuf, sizeof(unsigned char), 511 , fp);       	//讀取檔案放進buffer  每次512bytes
	                
	                sendto(sockfd,sendbuf,strlen(sendbuf), 0 , (struct sockaddr *)&cli_addr, clilen);	//將buffer傳送給接收端
	                if(feof(fp)) break;                                     //讀完檔案就結束傳送 
	                logcount+=511;                                         	//logcount每次加511 
					if(log == 0) {
						time(&now);                                       	//取得目前時間
					    printf("%d%% %s" , log , ctime(&now));				//印出已傳送多少以及時間 
						log+=25;
					}
	                if((logcount >= logperfive && log != 100) || log==100) {	//判斷logcount是否有到檔案大小的25%
	                    logcount = 0;                                      	//logcount歸零
	                    time(&now);
	                    printf("%d%% %s" , log , ctime(&now));
						log+=25;
	                }
	            }
				end = clock();
				printf("Total trans time: %fms\n", (double)(end-start)/1000);	//印出花費時間 
				printf("file size : %dMB\n", sz/1000/1000);						//印出檔案大小 
			}
			else
				printf("No request\n");
		
            close(sockfd);												//close socket
        }
        else if(!strcmp(argv[2],"recv"))                            	//判斷是否為接收檔案 
        {
            int ret , lose=0, request=1;
            struct timeval timeout={0,1};
            char file_name[512] = {'r','e','c','e','i','v','e','.','t','x','t'};
       
            if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)			//做socket()並判斷開啟有無錯誤 
				error("socket");

            bzero((char *) &serv_addr, sizeof(serv_addr));
            portno = atoi(argv[4]);                                     //設定UDP所需參數
            server = gethostbyname(argv[3]);                    		//設定UDP所需參數
            serv_addr.sin_family = AF_INET;                            	//設定UDP所需參數
            serv_addr.sin_port = htons(portno);                       	//設定UDP所需參數
            bcopy((char *)server->h_addr , (char *)&serv_addr.sin_addr.s_addr,server->h_length);	//設定UDP所需參數

            bzero(recvbuf,512);                                       	//清空buffer
			sendto(sockfd, &request, sizeof(int) , 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));	//傳送request

            fp = fopen(file_name,"wb");                                	//開檔
            if(!fp)
            	error("Error\n");
			int sz=0, s=0;
            recvfrom(sockfd, &sz, sizeof(int), 0, NULL , NULL);       	//接收'傳送檔案'的檔案大小
            setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));	//控制recv()的超時
            while(s <= sz) {											//s每次加512 直到等於檔案大小 
				bzero(recvbuf, 512);									//清空buffer
                ret=recvfrom(sockfd , recvbuf , sizeof(recvbuf)-1, 0, NULL , NULL);	//接收server送來的資料

                if(ret==-1) {
                    lose++;                                            	//計算lose
                    if(lose > 100)break;
                }
                else {
                    fwrite(recvbuf, sizeof(unsigned char), strlen(recvbuf), fp);	//將資料寫入檔案中
                    s+=strlen(recvbuf);
                }
            }
            if(!lose)													//如果沒有lose則完成接收檔案
				printf("Success\n");
            else														//如果有lose則印出檔案總packets及lose的packets數量 
				printf("Total packets: %d   lose %d packets\n", sz, sz-s);
            close(sockfd);												//close socket
        }
    }
    return 0;
}
