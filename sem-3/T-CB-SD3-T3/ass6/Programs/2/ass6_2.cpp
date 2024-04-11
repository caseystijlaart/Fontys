#include <iostream>
#include <iomanip>
#include "graph.h"

using namespace std;

int main()
{
    int t, n, m;
    int route_count = 0;

    cin >> t;

    if (t < 1 || t > 20)
    {
        cout << "ERROR too many test cases" << endl;
        return -1;
    }

    for (int i = 0; i < t; i++)
    {
        cin >> n >> m;
        if (n < 1 || n > 10000)
        {
            cout << "ERROR n (" << n << ") value out of range" << endl;
            return -1;
        }

        if (m < 1 || m > 100000)
        {
            cout << "ERROR m (" << m << ") value out of range" << endl;
            return -1;
        }
        Graph gr(n);
        int **graph = new int *[n];
        for (int k = 0; k < n; k++)
        {
            graph[k] = new int[n];
            for (int l = 0; l < n; l++)
                graph[k][l] = 0;
        }

        for (int j = 0; j < m; j++)
        {
            int x, y;
            cin >> x >> y;

            if (x < 1 || x > n)
            {
                cout << "ERROR node (" << x << ") value out of range" << endl;
                return -1;
            }
            if (y < 1 || y > n)
            {
                cout << "ERROR node (" << y << ") value out of range" << endl;
                return -1;
            }

            // since an arr[][] starts at 0 and the node Nr at 1, a 1 needs 
            // to be substracted from the input.
            graph[x - 1][y - 1] = 1;
            graph[y - 1][x - 1] = 1;
        }
        route_count = gr.dijkstra(graph, 0);
        cout << route_count << endl;

        for (int i = 0; i < n; ++i)
        {
            delete [] graph[i];
        }
        delete [] graph;
    
    }
}
