#include "page_node.h"

PageNode::PageNode(){
	this->wr = false;
	// this->id = id;
}

PageNode::PageNode(std::string st){
	this->name = st;
	this->wr = false;
	// this->id = id;
}

PageNode::~PageNode(){}

void PageNode::add_child(PageNode* x){
	this->sublinks.push_back(x);
}

void PageNode::written(){
	this->wr = true;
}
