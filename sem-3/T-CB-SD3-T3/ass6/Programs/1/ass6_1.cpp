#include <iostream>
#include <string.h>
#include "BFS.h"

using namespace std;


int main()
{
	BFS bfs;
	int n;
	cin >> n;
	Node* root;
	if(n > 20 )
	{
		cout << "ERROR to many nodes"<< endl;
		return -1;
	}
	for (int i = 0; i < n; i++)
	{
		int nr, left, right;
		cin >> nr >> left >> right;

		if(i==0)
		{
			root = bfs.createNode(nr);
			root->left = bfs.createNode(left);
			root->right = bfs.createNode(right);
		} 
		else {
			Node* searchNode = bfs.findNode(root, nr);
			searchNode->left = bfs.createNode(left);
			searchNode->right = bfs.createNode(right);
		}
		
	}

	int depth = bfs.height(root) -1 ;

	cout << "depth is " << depth << endl;
	return 0;
}
