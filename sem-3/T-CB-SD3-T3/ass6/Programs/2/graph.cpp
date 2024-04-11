#include <iostream>
#include "graph.h"

using namespace std;

Graph::Graph(int Nodes)
{
  amountOfNodes = Nodes;
}

int Graph::minDistance(int dist[],
                       bool sptSet[])
{
  int min = INT_MAX, min_index;

  for (int v = 0; v < amountOfNodes; v++)
    if (sptSet[v] == false &&
        dist[v] <= min)
      min = dist[v], min_index = v;

  return min_index;
}

int Graph::dijkstra(int **graph, int src)
{
  int *dist = new int[amountOfNodes];
  bool *sptSet = new bool[amountOfNodes];
  for (int i = 0; i < amountOfNodes; i++)
  {
    dist[i] = INT_MAX;
    sptSet[i] = false;
  }
  dist[src] = 0;

  for (int count = 0; count < amountOfNodes - 1; count++)
  {
    int u = minDistance(dist, sptSet);
    sptSet[u] = true;
    for (int v = 0; v < amountOfNodes; v++)
      if (!sptSet[v] && graph[u][v] &&
          dist[u] + graph[u][v] < dist[v])
      {
        dist[v] = dist[u] + graph[u][v];
      }
  }
  
  // A 1 needs to be subscrated from the amountOfNodes since
  // the dist[] starts at zero.
  int lengthPath = dist[amountOfNodes - 1];
  delete[] dist;
  delete[] sptSet;
  return lengthPath;
}
