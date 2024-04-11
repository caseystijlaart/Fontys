struct Node
{
    int data;
    struct Node *left;
    struct Node *right;
};

class BFS
{
    private:
    Node* foundNode = NULL;

    public:
    int height(Node* node);
    Node* createNode(int data);
    Node* findNode(Node* node, int data);
};