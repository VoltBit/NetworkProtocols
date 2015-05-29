#include "networking.h"
class Sysops{
public:
	Sysops();
	~Sysops();
	static char log_filename[255];
	static int log_fd, aux_fd;
	static int cd(char* path);
	static int mkdir(char* name);
	static int mkdir_full(char* path, char* root); /* Creates the full path of directories. */
	static char* pwd(char* name);
	/* 
		The two functions close_stderr and open_stderr are used to supress
		error messages comming from system() function call. 
	*/
	static void close_stderr();
	static void restore_stderr();
	static void redirect_stderr();
	static void set_logfile(const char* filename);
	static void close_logfile();
};