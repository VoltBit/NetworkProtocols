#include "sysops.h"
using namespace std;

char root[BUFFLEN];

map <string, vector<string*> > node_tree;

void display_tree(string* tabs, string* node) {
	cout << *tabs << *node << endl;
	tabs->append("\t");
	vector<string*>::iterator it;
	for(it = node_tree[*node].begin(); it != node_tree[*node].end(); it++) {
		display_tree(tabs, *it);
	}
}

/* receives a path and inserts the new nodes in the tree structure */
void get_structure(char* path) {
	string parent = string("/"); /* the root node */
	char* p = strtok(path, "/");
	if(node_tree.count(string(p)) == 0) {
		node_tree.insert(make_pair(string(p), *(new vector<string*>())));
		node_tree[parent].push_back(new string(p));
		parent = string(p);
	}
	while(p) {
		cout << p << endl;
		if(p != NULL && node_tree.count(string(p)) == 0) {
			node_tree[parent].push_back(new string(p));
		}
		parent = string(p);
		p = strtok(NULL, "/");
	}
}


void check_cd_effect(){
	char path[BUFFLEN], path2[BUFFLEN];
	strcpy(path, root);
	strcat(path, "/abcd/abb/ababab/kebab/shawarma/k");
	strcpy(path2, "abcd/abb/ababab/kebab/shawarma/l");
	Sysops::mkdir(path);

	// Sysops::cd(path);
	// Sysops::mkdir(path2);
}

char c_dir[BUFFLEN];

void gen_struct(char* path){
	char pathc[BUFFLEN];
	char comp[BUFFLEN];
	if(path == NULL){
		cout << "NULL path. Exiting.";
		return;
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
	do{
		if(p == NULL){
			cout << "Write " << k << " to disk\n";
			Sysops::cd(comp);
			break;
		}
		if(strlen(comp) == 0) 
			sprintf(comp, "%s", p);
		else 
			sprintf(comp, "%s/%s", comp, p);
		// strcat(comp, p);
		cout << "Making directory: " << comp << endl;
		try{
			Sysops::mkdir(comp);
		}catch(string msg){
			// cout << msg << endl;
		}
		p = strtok(NULL, "/");
		strcpy(k,p);
	}while(true);
}

void create_strucutre(char* path) {
	char* p = strtok(path, "/");
	char comm[100];
	int ret, fd_err, fd;
	memset(comm, 0, 100);
	// sprintf(comm, "%s %s", "cd", path);
	sprintf(comm, "%s", "~/Desktop/");

	try {
		Sysops::cd(comm);
	}catch (string msg) {
		cout << msg << endl;
	}

	try {
		Sysops::mkdir(path);
	}catch (string msg) {
		cout << msg << endl;
	}

	try {
		Sysops::mkdir(path);
	}catch (string msg) {
		cout << msg << endl;
	}
	// while(p) {
	// 	cout << p << endl;
	// 	p = strtok(NULL, "/");
	// }
}

int main(int argc, char* argv[]) {
	// Sysops::set_logfile("/home/smith/Dropbox/ACS/PC/4_TemaPC/logs");
	// Sysops::redirect_stderr();
	// cout << endl;
	// char path[BUFFLEN] = "/home/smith/Desktop/PCtests/test1/test2";
	// gen_struct(path);
	// strcpy(root,"/home/smith/Desktop/PCtests");
	// // check_cd_effect();
	// strcpy(path, "test1/dodost/file1");
	// gen_struct(path);
	// Sysops::restore_stderr();
	// Sysops::close_logfile();
	
	regex reg1("href");
	return 0;
}
	// create_strucutre(argv[1]);
	// get_structure(argv[1]);
	// node_tree.insert(make_pair(string("/"), *(new vector<string*>())));
	// string tabs = string("\t");
	// display_tree(&tabs, new string("/"));

	// get_structure("/test1/test2/blabla.pdf");
	// display_tree(&tabs, new string("/"));