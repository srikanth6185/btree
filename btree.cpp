#include <iostream>
#include <stdio.h>
#include "str.h"
#include <fstream>
#include <sys/stat.h>
#include <time.h>
#include <windows.h>

using std::cerr;
using std::cin;
using std::cout;
using std::fstream;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::endl;
using std::flush;

#define MAX_NUM_KEYS 32
#define MAX_NUM_CHILD 33

typedef struct btree_node {
  int n;
  int key[ MAX_NUM_KEYS ];
  long child[ MAX_NUM_CHILD ];
}btree_node_t;

typedef struct btree_node_location {
	btree_node_t node;
	long location;
	int childIdx;
}btree_node_location_t;

typedef struct stack_node
{
	long offset;
	int childIdx;
	struct stack_node* next;
}stack_node_t;

typedef struct print_node
{
	btree_node_t node;
	long location;
	struct print_node* next;
}print_node_t;


string *pIndex_file_name;
int num_nodes;
long root_offset;
stack_node_t *pTop;
print_node_t *pLevel1 , *pLevel2;

void add_to_stack(long offset, int idx)
{
	stack_node_t *newNode = (stack_node_t*)malloc(sizeof(stack_node_t));
	newNode->offset = offset;
	newNode->childIdx = idx;
	newNode->next = NULL;
	if(pTop)
		newNode->next = pTop;
	pTop = newNode;
}
void remove_from_stack(void)
{
	stack_node_t *delNode;
	if(pTop)
	{
		delNode = pTop;
		pTop = pTop->next;
		free(delNode);
	}
}

stack_node_t* get_stack_top(void)
{
	return pTop;
}

void pop_all_element(void)
{
	stack_node_t *delNode , *currentNode;
	currentNode = pTop;
	while(currentNode)
	{
		delNode = currentNode;
		currentNode = currentNode->next;
		if(delNode)
			free(delNode);
	}
	pTop = NULL;
}

void print_stack(void)
{
	stack_node_t *current = pTop;
	while(current)
	{
		cout << " " << current->offset << endl;
		current = current->next;
	}
}
void shift_child_array(long *darr, long *sarr,int start_idx , long new_offset)
{
	int idx , sidx = 0;
	for(idx = 0; idx < MAX_NUM_CHILD+1; idx++)
	{
		if(idx == (start_idx+1))
			darr[idx] = new_offset;
		else
			darr[idx] = sarr[sidx++];			
	}
}
void sort_array(int n, int *arr)
{
	int i,j;
	for(i = 0; i < n-1; i++)
	{
		for(j = 0; j < n-1-i; j++)
		{
				if(arr[j] > arr[j+1])
				{
					int temp = arr[j];
					arr[j] = arr[j+1];
					arr[j+1] = temp;
				}			
		}
	}
}

void reset_keys(int val, int size, int *arr, int start_idx)
{
	int idx;
	for(idx = start_idx; idx < size; idx++)
		arr[idx] = val;
}
void reset_child(long val, int size, long *arr, int start_idx)
{
	int idx;
	for(idx = start_idx; idx < size; idx++)
		arr[idx] = val;
}

void intialize(void)
{
	struct 	stat st;
	fstream f;
	/*Remove the index file if it exists*/
	remove(pIndex_file_name->operator char *());
	num_nodes = 0;
	if(stat(pIndex_file_name->operator char *(),&st)!= 0)
	{
		f.open( pIndex_file_name->operator char *(), ios::out);
		f << flush;
		f.close();
	}
	pTop = NULL;
	pLevel1 = NULL;
	pLevel2 = NULL;
}
void read_node_from_file(btree_node_location_t *node)
{
	ifstream indexfile;
	indexfile.open(pIndex_file_name->operator char *(),ios::in|ios::out|ios::binary);
	if(indexfile.is_open())
	{
		indexfile.seekg(node->location,ios::beg);
		indexfile.read((char*)&node->node,sizeof(btree_node_t));
		indexfile.close();
	}
	else
		cout<< "Could not open file"<< endl; 	
}
void update_node(btree_node_location_t *node)
{
	ofstream indexfile;	
	indexfile.open(pIndex_file_name->operator char *(),ios::out|ios::in|ios::binary);
	if(indexfile.is_open())
	{
		indexfile.seekp(node->location,ios::beg);
		indexfile.write((char*)&node->node,sizeof(btree_node_t));
		indexfile << flush;
		indexfile.close();
	}
	else
		cout<< "Could not open file"<< endl; 
}
void add_node_to_file(btree_node_location_t *node , bool newNode)
{
	ofstream indexfile;
	
	if(!newNode)
	{
		update_node(node);
		return;
	}
	indexfile.open(pIndex_file_name->operator char *(),ios::app|ios::in|ios::binary);
	if(indexfile.is_open())
	{
		indexfile.seekp(0,ios::end);
		indexfile.write((char*)&node->node,sizeof(btree_node_t));
		indexfile << flush;
		num_nodes++;		
		indexfile.close();
	}
	else
		cout<< "Could not open file"<< endl; 
}
void load_level(btree_node_location *pRead_node,int level)
{
	print_node_t * pCurrent_node;
	print_node_t *newNode = (print_node_t *)malloc(sizeof(print_node_t));
	memcpy(&newNode->node,&pRead_node->node,sizeof(btree_node_t));
	newNode->location = pRead_node->location;
	newNode->next = NULL;
	
	if(level == 1)
		pCurrent_node = pLevel1;
	else
		pCurrent_node = pLevel2;

	if(pCurrent_node == NULL)
	{
		if(level == 1)
			pLevel1 = newNode;
		else
			pLevel2 = newNode;
	}
	else
	{
		while(pCurrent_node->next != NULL)
			pCurrent_node = pCurrent_node->next;
		pCurrent_node->next = newNode;
	}
}

void switch_levels(void)
{
	pLevel1 = pLevel2;
	pLevel2 = NULL;
}
void delete_level(void)
{
	print_node_t * pCurrent_node , *delNode;
	pCurrent_node = pLevel1;
	pLevel1 = NULL;
	while(pCurrent_node!= NULL)
	{		
		delNode = pCurrent_node;
		pCurrent_node = pCurrent_node->next;
		if(delNode)
			free(delNode);
	}
}
print_node_t* get_level_head(void)
{
	return pLevel1;
}

void print_level(int level)
{
	print_node_t *pCurrent = get_level_head();	
	cout << endl;
	cout << level << ":";
	while(pCurrent)
	{		
		cout << " ";
		for(int idx = 0; idx < pCurrent->node.n; idx++)
		{
			cout << pCurrent->node.key[idx];
			if((idx+1) < pCurrent->node.n)
				cout << ",";
		}
		cout << "/"<< pCurrent->location;
		pCurrent = pCurrent->next;
	}
}
void load_second_level(void)
{
	long current_offset;
	btree_node_location read_node;
	print_node_t *pCurrent = get_level_head();
	while(pCurrent)
	{
		for(int idx = 0; idx < (pCurrent->node.n+1);idx++)
		{	
			current_offset = pCurrent->node.child[idx];
			if(current_offset == -1)
				break;
			read_node.location = current_offset;
			read_node_from_file(&read_node);
			load_level(&read_node,2);
		}
		pCurrent = pCurrent->next;
	}
}
void print_tree(void)
{
	btree_node_location read_node;
	long current_offset = root_offset;
	int level = 1;
	read_node.location = current_offset;
	read_node_from_file(&read_node);
	load_level(&read_node,1);
	
	while(get_level_head() != NULL)
	{
		print_level(level);
		load_second_level();
		delete_level();
		switch_levels();
		level++;
	}
	cout << endl;
}

void find_insertion_point(int newVal , btree_node_location_t *out_node)
{
	int keyIdx , childIdx = 0;
	long current_offset = root_offset , next_node_location = 0;
	fstream indexfile;
	
	indexfile.open(pIndex_file_name->operator char *(),ios::in|ios::binary);
	if(indexfile.is_open())
	{
		indexfile.seekg(root_offset,ios::beg);
		while(1)
		{
			indexfile.read((char*)&out_node->node,sizeof(btree_node_t));
			for(keyIdx = 0; keyIdx < out_node->node.n; keyIdx++)
			{
				if(newVal == out_node->node.key[keyIdx])
				{
					//cout << "Value already exists!!" << endl;
					out_node->location = -1;
					indexfile.close();
					return;
				}
				else
				{
					if(out_node->node.key[keyIdx] > newVal) /*The node value is greater than new one.*/
					{
						next_node_location = out_node->node.child[keyIdx];
						break;
					}
				}
			}
			if(keyIdx >= out_node->node.n) /*Assign the last pointer as the next location.*/
			{
				next_node_location = out_node->node.child[out_node->node.n];			
				keyIdx = out_node->node.n;
			}
			/*If its a leaf node*/
			if(next_node_location == -1)
			{
				out_node->location = current_offset;
				indexfile.close();
				return;
			}
			/*Locate the parents child index*/
			childIdx = keyIdx;
			add_to_stack(current_offset,childIdx); /*Update the parent location and the child index used in stack*/
			indexfile.seekg(next_node_location,ios::beg);
			current_offset = indexfile.tellg();
		}
	}
	else
		cout<< "Could not open file"<< endl; 
	
}

void split_node(int newVal , btree_node_location_t *leaf_node)
{
	int key_array[MAX_NUM_KEYS+1];
	long child_array[MAX_NUM_CHILD+1];
	btree_node_location_t new_node , new_root , node_to_be_split;
	btree_node_location_t *node_to_split = &node_to_be_split;
	long new_node_location = 0;
	int value_to_be_shifted = 0 , childidx = -1;
	stack_node_t* parent;

	memcpy(node_to_split,leaf_node,sizeof(btree_node_location_t));
	reset_child(-1,MAX_NUM_CHILD+1,child_array,0);
	//remove_from_stack();
	if(get_stack_top() != NULL)
	{		
		while(1)
		{
			/*Copy all the keys and sort*/
			memcpy(key_array,node_to_split->node.key,node_to_split->node.n*sizeof(int));
			key_array[node_to_split->node.n] = newVal;
			sort_array(MAX_NUM_KEYS+1 , key_array);
			newVal = key_array[MAX_NUM_KEYS/2];
			/*Update node that has become full*/
			reset_keys(0,MAX_NUM_KEYS,node_to_split->node.key,0);
			memcpy(node_to_split->node.key,key_array,sizeof(int)*(MAX_NUM_KEYS/2));
			reset_child(-1,MAX_NUM_CHILD,node_to_split->node.child,0);
			memcpy(node_to_split->node.child,child_array,sizeof(long)*(MAX_NUM_KEYS/2+1));
			node_to_split->node.n = MAX_NUM_KEYS/2;
			add_node_to_file(node_to_split,false);

			/*Create the new node*/
			new_node.location = num_nodes*sizeof(btree_node_t);
			reset_child(-1,MAX_NUM_CHILD,new_node.node.child,0);
			memcpy(new_node.node.child,&child_array[MAX_NUM_KEYS/2+1],sizeof(long)*(MAX_NUM_KEYS/2+1));
			memcpy(new_node.node.key,&key_array[MAX_NUM_KEYS/2+1],sizeof(int)*(MAX_NUM_KEYS/2));
			new_node.node.n = MAX_NUM_KEYS/2;
			add_node_to_file(&new_node,true);

			 
			if((parent = get_stack_top()) != NULL)
			{
				node_to_split->location = parent->offset;
				read_node_from_file(node_to_split);
				if(node_to_split->node.n < MAX_NUM_KEYS)
				{
					node_to_split->node.key[node_to_split->node.n++] = newVal;
					sort_array(node_to_split->node.n,node_to_split->node.key);
					shift_child_array(child_array,node_to_split->node.child,parent->childIdx,new_node.location);
					memcpy(node_to_split->node.child,child_array,sizeof(long)*(MAX_NUM_CHILD));
					add_node_to_file(node_to_split,false);
					break;
				}
				else
				{
					shift_child_array(child_array,node_to_split->node.child,parent->childIdx,new_node.location);
				}
				remove_from_stack();
			}
			else
			{
				/*Create new node as root*/
				new_root.location = (num_nodes)*sizeof(btree_node_t);
				reset_child(-1,MAX_NUM_CHILD,new_root.node.child,0);
				memset(new_root.node.key,0,sizeof(int)*MAX_NUM_KEYS);
				new_root.node.key[0] = key_array[MAX_NUM_KEYS/2];
				new_root.node.child[0] = node_to_split->location;
				new_root.node.child[1] = new_node.location;
				new_root.node.n = 1;
				root_offset = new_root.location;
				add_node_to_file(&new_root,true);
				break;
			}
		}
	}
	else
	{
		memcpy(key_array,node_to_split->node.key,node_to_split->node.n*sizeof(int));
		key_array[node_to_split->node.n] = newVal;
		sort_array(MAX_NUM_KEYS+1 , key_array);

		/*First 16 remain in the same node*/
		memset(node_to_split->node.key,0,sizeof(int)*MAX_NUM_KEYS);
		memcpy(node_to_split->node.key,key_array,sizeof(int)*(MAX_NUM_KEYS/2));
		node_to_split->node.n = MAX_NUM_KEYS/2;
		add_node_to_file(node_to_split,false);

		/*New Node with the second half*/
		new_node.location = num_nodes*sizeof(btree_node_t);
		reset_child(-1,MAX_NUM_CHILD,new_node.node.child,0);
		memset(new_node.node.key,0,sizeof(int)*MAX_NUM_KEYS);
		memcpy(new_node.node.key,&key_array[MAX_NUM_KEYS/2+1],sizeof(int)*MAX_NUM_KEYS/2);
		new_node.node.n = MAX_NUM_KEYS/2;
		add_node_to_file(&new_node,true);
		
		/*Create new node as root*/
		new_root.location = (num_nodes)*sizeof(btree_node_t);
		reset_child(-1,MAX_NUM_CHILD,new_root.node.child,0);
		memset(new_root.node.key,0,sizeof(int)*MAX_NUM_KEYS);
		new_root.node.key[0] = key_array[MAX_NUM_KEYS/2];
		new_root.node.child[0] = node_to_split->location;
		new_root.node.child[1] = new_node.location;
		new_root.node.n = 1;
		root_offset = new_root.location;
		add_node_to_file(&new_root,true);
	}
}

void insert_at_location(btree_node_location_t *node,int newVal)
{
	
	if(node->node.n == MAX_NUM_KEYS)
	{
		split_node(newVal,node);
	}
	else
	{
		node->node.key[node->node.n++] = newVal;
		sort_array(node->node.n , node->node.key);
		add_node_to_file(node,false);
	}
}

void add_to_btree(int newVal)
{
	btree_node_location_t node;
	long location = 0;
	if(num_nodes > 0)
	{
		pop_all_element();
		find_insertion_point(newVal,&node);
		if(node.location == -1)
			return;
		else
			insert_at_location(&node,newVal);
	}
	else
	{
		node.node.key[0] = newVal;
		reset_child(-1,MAX_NUM_CHILD,node.node.child,0);
		node.node.n = 1;
		node.location = 0;
		add_node_to_file(&node,true);
		root_offset = 0;
	}
}
int main(int argc, char* argv[])
{
	string current_line;
	string command_token[2];	
	int num_commands = 0;
	double  duration = 0;
	clock_t start, finish;
	DWORD dw1;
	DWORD dw2;


	pIndex_file_name = new string(argv[1]);

	intialize();
	while(1)
	{
		cin >> current_line;
		current_line.token(command_token,2);

		if(command_token[0] == "add")
		{
			//cout << "Add command!!" << endl;
			add_to_btree(command_token[1].operator int());
		}
		else
		if(command_token[0] == "find")
		{
			btree_node_location_t find_node;

			num_commands++;
			//start = clock();
			dw1 = GetTickCount();
			find_insertion_point(command_token[1].operator int(),&find_node);
			if(find_node.location == -1)
			{
				cout << "Record "<< command_token[1].operator int() << " exists." << endl;
			}
			else
			{
				cout << "Record "<< command_token[1].operator int() <<" does not exist."<< endl;
			}
			//finish = clock();
			dw2 = GetTickCount();
			duration += (double)(dw2 - dw1)/1000;
		}
		else
		if(command_token[0] == "end")
		{
			//cout << "End command!!"<< endl;
			break;
		}
		else
		if(command_token[0] == "print")
		{
			//cout << "Print done!!"<< endl;
			print_tree();
		}
		else
		if(command_token[0] == "test")
		{
			int n = 5;
			int arr[] = {2,5,7,8,1};
			//cout << "Print done!!"<< endl;
			sort_array(n, arr);
		}
		else
		if(command_token[0] == "as")
		{
			add_to_stack(command_token[1].operator int(),0);
		}
		else
		if(command_token[0] == "rs")
		{
			remove_from_stack();
		}
		else
		if(command_token[0] == "ps")
		{
			print_stack();
		}
		else
		{
			cout << "Invalid Command!!"<< endl;
		}
	}
	printf( "Sum : %2.6f seconds\n", duration );
	printf( "Avg : %2.6f seconds\n", duration/num_commands);
	return 0;
}
