#include <string>
#include <vector>

class PageNode{
public:
	std::vector<PageNode* > sublinks;
	std::string name;
	std::string parent;
	int id; /* every node (file) has an id k which means that current node
			id k-th node */
	bool wr; /* if the corresponding file was written to disk or not */

	PageNode();
	PageNode(std::string x);
	~PageNode();
	void add_child(PageNode* x);
	void written(); /* set the current file as written to the disk */
};