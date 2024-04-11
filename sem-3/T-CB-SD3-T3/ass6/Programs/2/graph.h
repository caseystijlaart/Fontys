#include <iostream>
#include <limits.h>

using namespace std;

class Graph
{
private:
int amountOfNodes;

private:
  int minDistance(int dist[],
                  bool sptSet[]);

public:
  Graph(int nrNodes);
  int dijkstra(int **graph, int src);
};
