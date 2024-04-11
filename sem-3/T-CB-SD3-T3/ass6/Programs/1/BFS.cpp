#include <iostream>
#include "BFS.h"
 
Node* BFS::findNode(Node* node, int item)
{
    if(node == NULL)
    {
        return NULL;
    }

    if(node->data == item)
    {
        foundNode = node;
        return foundNode;
    }
    if(node->left != NULL)
    {
        findNode(node->left, item);
    }

    if (node->right != NULL)
    {
        findNode(node->right, item);
    }

    return foundNode;
}

Node* BFS::createNode(int data)
{
    Node *createdNode = new Node();
    createdNode->data = data;
    createdNode->left = NULL;
    createdNode->right = NULL;
    return createdNode;
}

int BFS::height(Node* node)
{
    if (node == NULL)
        return 0;
    else {
        /* compute the height of each subtree */
        int lheight = height(node->left);
        int rheight = height(node->right);

        /* use the larger one */
        if (lheight > rheight) {
            return (lheight + 1);
        }
        else {
            return (rheight + 1);
        }
    }
}