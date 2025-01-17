#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <pthread.h>
#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
using namespace std;
#define endl "\n"

#define PORT 22000 
#define PORT1 21000
#define BACKLOG 500
#define LENGTH 512*1024

struct fileinfo{
    string filename;
    string filesize;
    string hashstring;
    string clientsock;
};

vector<struct fileinfo> v;
vector<string> trackers;
string seederlist;
string logfile;
void recievefileinfofrompeer(int fl,int sockfd);
vector<string> split(string phrase, string delimiter){
    vector<string> list;
    string s = string(phrase);
    size_t pos = 0;
    string token;
    while ((pos = s.find(delimiter)) != string::npos) {
        token = s.substr(0, pos);
        list.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    list.push_back(s);
    return list;
}

void senddata(string fs_name,int nsockfd){
	freopen(logfile.c_str(),"a+",stderr);
	//pthread_mutex_lock(&lock);
	char sdbuf[LENGTH];
	bzero(sdbuf, LENGTH); 
	int fs_block_sz; 
	fprintf(stderr,"[Server] Sending %s to the Client...\n", fs_name);
	ifstream fp (seederlist.c_str());
	if(fp.fail()){
	    fprintf(stderr, "ERROR: File %s not found on server. (errno = %d)\n", fs_name.c_str(), errno);
		pthread_exit(NULL);
		exit(1);
	}
	string str;
	string ips="";
	if(fp.is_open()){
		while(fp>>str){
			if(str==fs_name){
				fp>>str;
				ips = ips + "#";
				ips = ips + str;
			}
		}
	}
	//cout<<ips<<endl;
	if(send(nsockfd, ips.c_str(), LENGTH, 0) < 0)
	{
		fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name.c_str(), errno);
		//pthread_exit(NULL);
		//exit(1);
	}
	fprintf(stderr,"Ok sent to client!\n");
	close(nsockfd);
	fprintf(stderr,"[Server] Connection with Client closed. Server will wait now...\n");
	//pthread_mutex_unlock(&lock);
	//pthread_exit(NULL);
}

/*void trackerserver(){
    int sockfd; 
	int nsockfd; 
	int num;
	int sin_size; 
	struct sockaddr_in addr_local; 
	struct sockaddr_in addr_remote; 
	char revbuf[LENGTH]; 

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor. (errno = %d)\n", errno);
		pthread_exit(NULL);
		//exit(1);
	}
	else 
		printf("[Server] Obtaining socket descriptor successfully.\n");
	addr_local.sin_family = AF_INET; 
	addr_local.sin_port = htons(PORT); 
	addr_local.sin_addr.s_addr = INADDR_ANY; 
	bzero(&(addr_local.sin_zero), 8); 

	if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 ){
		fprintf(stderr, "ERROR: Failed to bind Port. (errno = %d)\n", errno);
		pthread_exit(NULL);
		//exit(1);
	}
	else 
		printf("[Server] Binded tcp port %d in addr 127.0.0.1 sucessfully.\n",PORT);

	if(listen(sockfd,BACKLOG) == -1){
		fprintf(stderr, "ERROR: Failed to listen Port. (errno = %d)\n", errno);
		pthread_exit(NULL);
		//exit(1);
	}
	else
		printf ("[Server] Listening the port %d successfully.\n", PORT);

	int success = 0;
	std::vector<std::thread> threads;
	while(success == 0){
		sin_size = sizeof(struct sockaddr_in);
		if ((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, (socklen_t *)&sin_size)) == -1){
		    fprintf(stderr, "ERROR: Obtaining new Socket Despcritor. (errno = %d)\n", errno);
			pthread_exit(NULL);
			//exit(1);
		}
		else 
			printf("[Server] Server has got connected from %s.\n", inet_ntoa(addr_remote.sin_addr));
		/*string sock = "";
		sock.append(inet_ntoa(addr_remote.sin_addr));
        sock.append(":");
		sock.append(to_string(ntohs(addr_remote.sin_port)));
		printf("%s",sock.c_str());*/
        /*char buf[LENGTH];
        read(nsockfd,buf,LENGTH);
        cout<<buf<<endl;
		//string fs_name = "f5.png";
        for(int i=0;i<v.size();i++){
            //cout<<v[i].filename<<endl;
            if(v[i].hashstring==buf){
                cout<<v[i].filename<<endl;
                threads.push_back(std::thread(senddata, v[i].filename,nsockfd));
            }
        }
		success = 0;
	}
	for (auto& th : threads) th.join();  
}*/

void senddatatotracker(int nsockfd){
	freopen(logfile.c_str(),"a+",stderr);
	//pthread_mutex_lock(&lock);
	fprintf(stderr,"[Tracker] Sending data to the another tracker...\n");
	string str;
	int i=0;
	str = to_string(v.size());
	str = str + "#";
	while(i<v.size()){
			str = str + v[i].filename;
			str = str + "$";
			str = str + v[i].filesize;
			str = str + "$";
			str = str + v[i].hashstring;
			str = str + "$";
			str = str + v[i].clientsock;
			str = str+"#";
		i++;
	}
	if(send(nsockfd, str.c_str(), LENGTH, 0) < 0)
		{
			//cout<<v[i].filename<<endl;
		    fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n",errno);
			pthread_exit(NULL);
		    exit(1);
		}
	fprintf(stderr,"Ok sent to client!\n");
	close(nsockfd);
	fprintf(stderr,"[Server] Connection with Client closed. Server will wait now...\n");
	//pthread_mutex_unlock(&lock);
	pthread_exit(NULL);
}

void removedata(string str1, string str2){
	freopen(logfile.c_str(),"a+",stderr);
	for(int i=0;i<v.size();i++){
		//cout<<v[i].filename<<endl;
		if(strcmp(v[i].hashstring.c_str(),str1.c_str())==0  && (strcmp(v[i].clientsock.c_str(),str2.c_str())==0)){
			//cout<<v[i].filename<<endl;
			v.erase(v.begin()+i);		
		}
	}

	ifstream fin;
	fin.open("seederlist.txt",ios::in);
	string line;
	string deleteline = str1 + " ";
	deleteline = deleteline + str2;
	//cout<<deleteline<<endl;
	vector<string> cntnt;
	while (getline(fin,line))
	{
    	if(line==deleteline)
			continue;
		//cout<<line<<endl;
		cntnt.push_back(line);
	}
	fin.close();
	ofstream fo;
	fo.open("seederlist.txt",ios::out);
	for(int i=0;i<cntnt.size();i++){
		fo<<cntnt[i]<<endl;
	}
	fo.close();
	//cout<<v.size()<<endl;
	//for(int i=0;i<v.size();i++){
	//	cout<<v[i].filename<<endl;
//	}
}

void removepeer(string str){
	freopen(logfile.c_str(),"a+",stderr);
	for(int i=0;i<v.size();i++){
		//cout<<v[i].filename<<endl;
		if(strcmp(v[i].clientsock.c_str(),str.c_str())==0){
			//cout<<v[i].filename<<endl;
			v.erase(v.begin()+i);		
		}
	}

	ifstream fin;
	fin.open("seederlist.txt",ios::in);
	string line;
	string deleteline = str;
	//cout<<deleteline<<endl;
	vector<string> cntnt;
	while (getline(fin,line))
	{
    	vector<string> op = split(line," ");
    	if(op[1]!=deleteline)
			continue;
		//cout<<line<<endl;
		cntnt.push_back(line);
	}
	fin.close();
	ofstream fo;
	fo.open("seederlist.txt",ios::out);
	for(int i=0;i<cntnt.size();i++){
		fo<<cntnt[i]<<endl;
	}
	fo.close();
	/*cout<<v.size()<<endl;
	for(int i=0;i<v.size();i++){
		cout<<v[i].filename<<endl;
	}*/
}

void connecttootherserver(string ip){
	freopen(logfile.c_str(),"a+",stderr);
    int sockfd; 
	int nsockfd; 
	int num;
	int sin_size; 
	struct sockaddr_in addr_local; 
	struct sockaddr_in addr_remote, remote_addr; 
	char revbuf[LENGTH]; 
	vector<string> mysock = split(ip,":");
	vector<string> othersock = split(trackers[1],":");
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor. (errno = %d)\n", errno);
		pthread_exit(NULL);
		//exit(1);
	}
	else 
		fprintf(stderr,"[Tracker] Obtaining socket descriptor successfully.\n");
	addr_local.sin_family = AF_INET; 
	addr_local.sin_port = htons(stoi(mysock[1])); 
	addr_local.sin_addr.s_addr = INADDR_ANY; 
	bzero(&(addr_local.sin_zero), 8); 

	if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 ){
		fprintf(stderr, "ERROR: Failed to bind Port. (errno = %d)\n", errno);
		pthread_exit(NULL);
		//exit(1);
	}
	else 
		fprintf(stderr,"[Tracker] Binded tcp port %d in addr 127.0.0.1 sucessfully.\n",stoi(mysock[1]));

	if(listen(sockfd,BACKLOG) == -1){
		fprintf(stderr, "ERROR: Failed to listen Port. (errno = %d)\n", errno);
		pthread_exit(NULL);
		//exit(1);
	}
	else
		fprintf (stderr,"[Tracker] Listening the port %d successfully.\n", stoi(mysock[1]));

	int success = 0;
	std::vector<std::thread> threads;
	while(success == 0){
		sin_size = sizeof(struct sockaddr_in);
		if ((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, (socklen_t *)&sin_size)) == -1){
		    fprintf(stderr, "ERROR: Obtaining new Socket Despcritor. (errno = %d)\n", errno);
			//pthread_exit(NULL);
			//exit(1);
		}
		else 
			fprintf(stderr,"[Tracker] Server has got connected from %s.\n", inet_ntoa(addr_remote.sin_addr));
		/*string sock = "";
		sock.append(inet_ntoa(addr_remote.sin_addr));
        sock.append(":");
		sock.append(to_string(ntohs(addr_remote.sin_port)));
		printf("%s",sock.c_str());*/
        //char buf[LENGTH];
        //read(nsockfd,buf,LENGTH);
        //cout<<buf<<endl;
		//string fs_name = "f5.png";
		recv(nsockfd, revbuf, LENGTH, 0);
		//cout<<revbuf<<endl;
		string str = revbuf;
		vector<string> opr = split(str,"$");
		if(strcmp(opr[0].c_str(),"tracker")==0){
				//senddatatotracker(sockfd);
        		threads.push_back(std::thread(senddatatotracker, nsockfd));
		}
		else{
			if(strcmp(opr[0].c_str(),"get")==0){
				//string fs_name = "f5.png";
				for(int i=0;i<v.size();i++){
					//cout<<v[i].filename<<endl;
					if(strcmp(v[i].hashstring.c_str(),opr[1].c_str())==0){
						//cout<<v[i].filename<<endl;
						threads.push_back(std::thread(senddata, v[i].hashstring,nsockfd));
					}
				}
			}
			else if(strcmp(opr[0].c_str(),"share")==0){
				ofstream fp ("seederlist.txt",ios::app | ios::out);
				struct fileinfo f;
				f.filename = opr[1];
				f.filesize = opr[2];
				f.hashstring = opr[3];
				f.clientsock = opr[4];
				v.push_back(f); 
				if(fp.is_open()){
					fp<<f.hashstring;
					fp<<" ";
					fp<<f.clientsock;
					fp<<endl;
				}
				fp.close();
			}
			else if(strcmp(opr[0].c_str(),"remove")==0){
				//string fs_name = "f5.png";
				removedata(opr[1],opr[2]);
			}
			else if(strcmp(opr[0].c_str(),"close")==0){
				removepeer(opr[1]);
			}
		}

    }
	success = 0;
	for (auto& th : threads) th.join();  
}

void recievefileinfofrompeer(int sockfd){
	freopen(logfile.c_str(),"a+",stderr);
	char revbuf[LENGTH];
	bzero(revbuf, LENGTH); 
	int fr_block_sz = 0;
	struct fileinfo f;
	string str;
	if((fr_block_sz = recv(sockfd, revbuf, LENGTH, 0)) > 0){
			//cout<<revbuf<<endl;
			str = revbuf;
			bzero(revbuf, LENGTH); 
	}
	vector<string> data = split(str,"#");
	int i = stoi(data[0]);
	ofstream fp ("seederlist.txt",ios::app | ios::out);
	int j=0;
	while(j<i){
		vector<string> ni = split(data[j+1],"$");
		cout<<data[j+1]<<endl;
		f.filename = ni[0];
		f.filesize = ni[1];
		f.hashstring = ni[2];
		f.clientsock = ni[3];
		v.push_back(f); 
		if(fp.is_open()){
			fp<<f.hashstring;
			fp<<" ";
			fp<<f.clientsock;
			fp<<endl;
		}
		j++;
	}  
	fp.close();
	
}

void connection(string ip){
	freopen(logfile.c_str(),"a+",stderr);
	int sockfd; 
	int nsockfd;
	char revbuf[LENGTH]; 
	struct sockaddr_in remote_addr;
	int i=0;
	vector<thread> threads;
	vector<string> othersock = split(trackers[1],":");
	//while(i<5){
		i++;
		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			fprintf(stderr, "ERROR: Failed to obtain Socket Descriptor! (errno = %d)\n",errno);
			//pthread_exit(NULL);
			//exit(1);
		}
		remote_addr.sin_family = AF_INET; 
		remote_addr.sin_port = htons(stoi(othersock[1])); 
		inet_pton(AF_INET, othersock[0].c_str(), &remote_addr.sin_addr); 
		bzero(&(remote_addr.sin_zero), 8);
	
		if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
		{
			fprintf(stderr, "ERROR: Failed to connect to the host! (errno = %d)\n",errno);
			//pthread_exit(NULL);
			//exit(1);
		}
		else{ 
			fprintf(stderr,"[Client] Connected to server at port %d...ok!\n", stoi(othersock[1]));
			recievefileinfofrompeer(sockfd);
			//threads.push_back(std::thread(recievefileinfofrompeer, sockfd));
		}
//	}
	close (sockfd);
	fprintf(stderr,"[Client] Connection lost.\n");
	//for (auto& th : threads) th.join();  
	//cout<<v.size()<<endl;
			//for(int i=0;i<v.size();i++)
			//			cout<<v[i].filename<<" "<<v[i].filesize<<" "<<v[i].hashstring<<" "<<othersock[1]<<endl;
}


int main(int argc, char *argv[]){
	logfile = argv[argc-1];
   	freopen(logfile.c_str(),"a+",stderr);
    //printf("\033[H\033[J");
	//printf("\033[3J");
	for(int i=0;i<argc-2;i++)
		trackers.push_back(argv[i+1]);
	seederlist = argv[argc-2];
	thread t2(connection,trackers[1]);
	t2.join();
	thread t1(connecttootherserver,trackers[0]);
	t1.join();
	fclose(stderr);
	return 0;
}