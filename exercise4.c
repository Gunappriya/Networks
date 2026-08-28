
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 50
#define INF 999999

// ---------- Edge ----------
typedef struct Edge
{
    int source;
    int destination;
    int weight;
} Edge;

// ---------- Router ----------
typedef struct
{
    char name[20];
} Node;

// ---------- Graph ----------
typedef struct
{
    int vertices;
    int edges;

    Node node[MAX];
    Edge edge[MAX * MAX];

} Graph;


// Function declarations
void readGraph(Graph *g);
void displayGraph(Graph *g);

void bellmanFord(
    Graph *g,
    int source,
    int distance[],
    int parent[]
);

void printPath(
    Graph *g,
    int parent[],
    int destination
);

void printRoutingTable(
    Graph *g,
    int source,
    int distance[],
    int parent[]
);


// ==========================================================
// MAIN
// ==========================================================

int main()
{
    Graph graph;

    int source;

    int distance[MAX];

    int parent[MAX];


    // Read graph
    readGraph(&graph);


    // Display graph
    displayGraph(&graph);


    printf("\n\n");
    printf("====================================================\n");
    printf("             BELLMAN-FORD ALGORITHM\n");
    printf("====================================================\n");


    // Run Bellman-Ford for every router
    for (source = 0;
         source < graph.vertices;
         source++)
    {
        printf("\n\n");
        printf("####################################################\n");

        printf("SOURCE ROUTER = %s\n",
               graph.node[source].name);

        printf("####################################################\n");


        bellmanFord(
            &graph,
            source,
            distance,
            parent
        );


        printRoutingTable(
            &graph,
            source,
            distance,
            parent
        );
    }


    return 0;
}


// ==========================================================
// READ GRAPH
// ==========================================================

void readGraph(Graph *g)
{
    int i;

    int src;
    int dest;
    int cost;


    printf("Enter number of routers : ");
    scanf("%d", &g->vertices);


    // Initialize router names
    for (i = 0;
         i < g->vertices;
         i++)
    {
        printf("Router %d name : ", i);

        scanf("%19s",
              g->node[i].name);
    }


    printf("\nEnter number of links : ");
    scanf("%d", &g->edges);


    printf("\nRouter Numbers\n");

    for (i = 0;
         i < g->vertices;
         i++)
    {
        printf("%d --> %s\n",
               i,
               g->node[i].name);
    }


    // Read edges
    for (i = 0;
         i < g->edges;
         i++)
    {
        printf("\nLink %d\n",
               i + 1);


        printf("Source Router Number : ");
        scanf("%d", &src);


        printf("Destination Router Number : ");
        scanf("%d", &dest);


        printf("Cost : ");
        scanf("%d", &cost);


        if (src < 0 ||
            src >= g->vertices ||
            dest < 0 ||
            dest >= g->vertices)
        {
            printf("Invalid Router!\n");

            i--;

            continue;
        }


        g->edge[i].source = src;

        g->edge[i].destination = dest;

        g->edge[i].weight = cost;
    }
}


// ==========================================================
// DISPLAY GRAPH
// ==========================================================

void displayGraph(Graph *g)
{
    int i;


    printf("\n\n");
    printf("================ GRAPH ================\n");


    for (i = 0;
         i < g->edges;
         i++)
    {
        printf("%s -> %s (%d)\n",

               g->node[
                   g->edge[i].source
               ].name,

               g->node[
                   g->edge[i].destination
               ].name,

               g->edge[i].weight);
    }
}


// ==========================================================
// BELLMAN-FORD
// ==========================================================

void bellmanFord(
    Graph *g,
    int source,
    int distance[],
    int parent[])
{
    int i;
    int j;

    int u;
    int v;
    int weight;

    int newDistance;


    // ------------------------------------------
    // Step 1: Initialize
    // ------------------------------------------

    for (i = 0;
         i < g->vertices;
         i++)
    {
        distance[i] = INF;

        parent[i] = -1;
    }


    // Distance from source to itself = 0
    distance[source] = 0;


    printf("\nInitial distances:\n");

    for (i = 0;
         i < g->vertices;
         i++)
    {
        if (distance[i] == INF)
            printf("%s = INF\n",
                   g->node[i].name);
        else
            printf("%s = %d\n",
                   g->node[i].name,
                   distance[i]);
    }


    // ------------------------------------------
    // Step 2:
    // Relax all edges V-1 times
    // ------------------------------------------

    for (i = 1;
         i <= g->vertices - 1;
         i++)
    {
        printf("\n");
        printf("----------------------------------------\n");
        printf("Iteration %d\n", i);
        printf("----------------------------------------\n");


        for (j = 0;
             j < g->edges;
             j++)
        {
            u = g->edge[j].source;

            v = g->edge[j].destination;

            weight = g->edge[j].weight;


            // Relaxation
            if (distance[u] != INF)
            {
                newDistance =
                    distance[u] + weight;


                if (newDistance < distance[v])
                {
                    distance[v] = newDistance;

                    parent[v] = u;


                    printf(
                        "Updated %s = %d through %s\n",

                        g->node[v].name,

                        distance[v],

                        g->node[u].name
                    );
                }
            }
        }
    }


    // ------------------------------------------
    // Step 3:
    // Check negative cycle
    // ------------------------------------------

    for (j = 0;
         j < g->edges;
         j++)
    {
        u = g->edge[j].source;

        v = g->edge[j].destination;

        weight = g->edge[j].weight;


        if (distance[u] != INF &&
            distance[u] + weight < distance[v])
        {
            printf("\n");
            printf("WARNING: Negative weight cycle exists!\n");

            return;
        }
    }


    printf("\nBellman-Ford completed successfully.\n");
}


// ==========================================================
// PRINT PATH
// ==========================================================

void printPath(
    Graph *g,
    int parent[],
    int destination)
{
    if (parent[destination] == -1)
    {
        printf("%s",
               g->node[destination].name);

        return;
    }


    printPath(
        g,
        parent,
        parent[destination]
    );


    printf(" -> %s",
           g->node[destination].name);
}


// ==========================================================
// ROUTING TABLE
// ==========================================================

void printRoutingTable(
    Graph *g,
    int source,
    int distance[],
    int parent[])
{
    int i;


    printf("\n\n");

    printf("===============================================================\n");

    printf("ROUTING TABLE OF %s\n",
           g->node[source].name);

    printf("===============================================================\n");


    printf("Destination\tCost\tParent\t\tPath\n");

    printf("---------------------------------------------------------------\n");


    for (i = 0;
         i < g->vertices;
         i++)
    {
        printf("%s\t\t",
               g->node[i].name);


        // Destination unreachable
        if (distance[i] == INF)
        {
            printf("INF\t-\t\tNo Path\n");

            continue;
        }


        // Cost
        printf("%d\t",
               distance[i]);


        // Parent
        if (parent[i] == -1)
        {
            printf("-\t\t");
        }
        else
        {
            printf("%s\t\t",
                   g->node[
                       parent[i]
                   ].name);
        }


        // Path
        printPath(
            g,
            parent,
            i
        );


        printf("\n");
    }


    printf("===============================================================\n");
}
