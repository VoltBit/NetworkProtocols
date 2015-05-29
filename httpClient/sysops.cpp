#include "sysops.h"
using namespace std;

int Sysops::log_fd = -1;
int Sysops::aux_fd = -1;
char Sysops::log_filename[255] = "\0";

Sysops::Sysops(){
	Sysops::log_fd = -1;
	Sysops::aux_fd = -1;
	strcpy(Sysops::log_filename, "\0");
}

Sysops::~Sysops(){}

void Sysops::set_logfile(const char* filename){
	strcpy(log_filename, filename);
	log_fd = open(log_filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
	cout << endl << "Logfile: " << filename << " FD: " << Sysops::log_fd << endl;
}

void Sysops::close_logfile(){
	close(Sysops::log_fd);
}

void Sysops::close_stderr(){
	Sysops::log_fd = open("/dev/null", O_WRONLY | O_APPEND);
	Sysops::aux_fd = dup(STDERR_FILENO); /* make a copy of the file descriptor pointing to stderr */
	dup2(Sysops::log_fd, STDERR_FILENO); /* stderr output will go to /dev/null */
}

void Sysops::restore_stderr(){
	dup2(Sysops::aux_fd, STDERR_FILENO); /* restore stderr file descriptor */
	close(Sysops::aux_fd);
}

void Sysops::redirect_stderr(){
	Sysops::aux_fd = dup(STDERR_FILENO); /* make a copy of the file descriptor pointing to stderr */
	dup2(Sysops::log_fd, STDERR_FILENO);
}

int Sysops::cd(char* path){
	int ret = 0;
	char comm[255];
	string errmsg = string("cd command failed on call: ");
	sprintf(comm, "%s %s", "cd", path);
	// Sysops::redirect_stderr();
	if((ret = system(comm))){
		if(ret == 512){
			errmsg.append(comm);
			throw(errmsg);
		}
		else{
			errmsg.append(comm);
			throw(errmsg);
		}
		// Sysops::restore_stderr();
		return -1;
	}
	cout << "\nEntered directory: " << path << endl;
	// Sysops::restore_stderr();
	return 0;
}

int Sysops::mkdir(char* name){
	int ret = 0;
	char comm[BUFFLEN];
	Sysops::log_fd = open("/dev/null", O_WRONLY | O_APPEND);
	Sysops::aux_fd = dup(STDERR_FILENO); /* make a copy of the file descriptor pointing to stderr */
	dup2(Sysops::log_fd, STDERR_FILENO); /* stderr output will go to /dev/null */
	string errmsg1 = string("mkdir command error on call: ");
	sprintf(comm, "%s %s", "mkdir", name);
	if((ret = system(comm))){
	// 	DIR* dir = opendir(name);
	// 	if (dir){
	// 	    /* Directory already exists. */
	// 	    closedir(dir);
	// 	    return 0;
	// 	}
	// 	else if (ENOENT == errno){
	// 		 // Directory does not exist. It could not be created
	// 		errmsg1 = string("Directory could not be created");
	// 		cerr << "Directory could not be created\n";
	// 		throw(errmsg1);
	// 	}
	// 	else{
	// 		errmsg1 = string("Error while makeing directory");
	// 		cerr << "Error while makeing directory\n";
	// 		throw(errmsg1);
	// 	    /* opendir() failed for some other reason. */
	// 	}
	// 	return -1;
	}
	close(Sysops::log_fd);
	close(Sysops::aux_fd);
	return 0;	
}

int Sysops::mkdir_full(char* path, char* root){
	char pathc[BUFFLEN];
	char comp[BUFFLEN];
	if(path == NULL){
		cout << "NULL path. Exiting.";
		return -1;
	}
	memset(comp, 0, BUFFLEN);
	comp[0] = '\0';
	if(path[0] == '/'){
		// Sysops::cd(root);
		strcpy(comp, root);
	}
	strcpy(pathc, path);
	cout << path << endl;
	char* p = strtok(pathc, "/");
	char k[BUFFLEN];
	strcpy(k,p);
	while(p){
		if(p == NULL){
			break;
		}
		if(strlen(comp) == 0) 
			sprintf(comp, "%s", p);
		else 
			sprintf(comp, "%s/%s", comp, p);
		// cout << "Making directory: " << comp << endl;
		try{
			Sysops::mkdir(comp);
		}catch(string msg){
			cout << msg << endl;
		}
		strcpy(k,p);
		p = strtok(NULL, "/");
	}
	return 0;	
}