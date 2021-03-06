#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

struct edge
{
    int vertexIndex;
    int vertexWeight;
    struct edge *edgePtr;
} edge;

struct vertex
{
    int vertexKey;
    struct edge *edgePtr;
    struct vertex *vertexPtr;
} vertex;

typedef struct edge Edge;
typedef struct vertex Vertex;

void printList();
int dijkstra(int, int, int);
int cost(int ,int);
int allcompleted(int [], int);
void printPath(int, int, int[]);

Vertex *start;
int destination;
int source;

int main(int argc, char *argv[]) {

    /* sockets related */
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[1000];
    char rbuffer[256];

    listenfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); //setup a tcp socket
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, 0, sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET; //check slides
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //check slides
    serv_addr.sin_port = htons(6789); 

    char *graph = argv[1];
    FILE *filePtr = fopen(graph, "r");
    int numNodes = atoi(argv[2]);
        
    FILE *numElements = filePtr;
    int f = 0; int e = -1;
        
    if (filePtr == NULL)
    {
        printf("File not found. Terminating.\n");
        exit(0);
    }
    
    while (!feof(numElements))
    {
        fscanf(numElements, "%d", &e);
        f++;
    }
    fclose(numElements);
    
    if (e==-1 && f==1)
    {
        printf("This text file is empty.\n");
        exit(0);
    }
    
    if (numNodes == 1 && e >= 0 && f == 1)
    {
        printf("You entered only one node ");
        exit(0);
    }
 
       
    *filePtr = *fopen(graph, "r");
    
    if (destination >= numNodes)
    {
        printf("That destination node does not exist.\n");
        exit(0);
    }
    
    if (source >= numNodes)
    {
        printf("That source node does not exist.\n");
        exit(0);
    }
    
    Vertex *currvertex;
    Edge *curredge;
    
    if (filePtr == NULL)
    {
        printf("File does not exist...terminating program.\n");
        exit(0);
    }
    else
    {
        int i;
        for (i = 0; i < numNodes; i++)
        {
            Vertex *v = (struct vertex *)malloc(sizeof(struct vertex));
            v->vertexKey = i;
            v->edgePtr = NULL;
            v->vertexPtr = NULL;
            
            if (start==NULL)
            {
                start=v;
                currvertex=v;
            }
            else
            {
                currvertex->vertexPtr = v;
                currvertex=v;
            }
            int j;
            for (j=0; j<numNodes; j++)
            {
                Edge *e = (struct edge *)malloc(sizeof(struct edge));
                int cw;
                if (!feof(filePtr))
                {
                    fscanf(filePtr, "%d", &cw);
                }
                else {
                    printf("Too few numbers in the file!");
                    fclose(filePtr);
                    exit(0);
                }
                
                if (cw <= 0) {
                }
                else if (cw != 0 && i==j)
                {
                    printf("A node cannot have an edge to itself! \n");
                    fclose(filePtr);
                    exit(0);
                }
                else
                {
                    e->vertexIndex = j;
                    e->vertexWeight = cw;
                    e->edgePtr = NULL;
                
                
                	if (v->edgePtr == NULL && e != NULL)
                	{
                    v->edgePtr = e;
                    curredge = e;
                	}
                	else
                	{
                    curredge->edgePtr = e;
                    curredge = e;
                	}
		    }
            }
            curredge->edgePtr=NULL;
        }
        currvertex->vertexPtr=NULL;
    }
    fclose(filePtr);
    
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));  //Hint: server address structure
    listen(listenfd, 10);

   
    while (1)
    {
	int n;
	connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
	if (connfd < 0) 
    	{
        perror("ERROR on accept");
        exit(1);
    	}
    	
	memset(rbuffer, 0, sizeof(rbuffer)); 
    	n = read( connfd,rbuffer,255 );
    	if (n < 0)
    	{
        perror("ERROR reading from socket");
        exit(1);
    	}
    	printf("Here is the message from the client: %s\n",rbuffer);
	
	/* Read the string rbuffer and extract source and destination */
        /* Use function from lab1 to extract numbers from a string */
        /* Display error if string contains more than 2 numbers or if the source and estination are outside range */

	char * tokens;
	int numParse=0;
	tokens=strtok(rbuffer," ");
	while (tokens != NULL)
	  {
	    //printf ("Parsed %s\n",tokens);
            if (numParse==0)
	      {
		source=atoi(tokens);
	      }
	    else if (numParse==1)
	      {
		destination=atoi(tokens);
	      }
	    else if (numParse>=2)
	      {
		printf("ERROR: Too many values specified\n");
	      }
	    numParse++;
	    tokens = strtok (NULL, " ");
	  }
	if (source >=numNodes || destination >=numNodes)
	{
	  printf("ERROR: Node(s) specified greater then number of nodes\n");
	}
	else
	{
	  int g = dijkstra(source, destination, numNodes);
	  sprintf(sendBuff,"%d",g);
	  write(connfd, sendBuff, strlen(sendBuff)); 
	}
	  close(connfd);
	  sleep(1);
	
    }
    return 0;
}

void printList()
{
    Vertex *read;
    Edge *readedge;
    
    read = start;
    
    while (read != NULL)
    {
        printf("Node %d: ", read->vertexKey);
        readedge = read->edgePtr;
        while (readedge != NULL)
        {
            printf("%d ", readedge->vertexWeight);
            readedge=readedge->edgePtr;
        }
        printf("\n");
        read=read->vertexPtr;
    }

}

/*Dijkstra's short path algorithm.
 */
int dijkstra(int source, int destination, int numNodes)
{
    int completed[numNodes], distance[numNodes], prev[numNodes], z;
    
    
    for (z = 0; z < numNodes; z++) {
        if (z==source)
        {
            completed[z] = 1;
            distance[z] = 0;
            prev[z] = z;
        }
        else
        {
            completed[z] = 0;
            distance[z] = 5000000;
            prev[z] = -1;
        }
    }

    int current = source;
    int min = 0; int new = 0; int i; int k=-1;

    while (allcompleted(completed, numNodes) != 1)
    {
        min = 5000000;
        k=-1;
        for (i=0; i<numNodes; i++) {
            if (completed[i] == 0)
            {
                new = distance[current] + cost(current, i);
                if (new >= 5000000 && i == (numNodes-1) && min >= 5000000)
                {
                    completed[i]=1;
                }
                if (new < distance[i]) {
                    distance[i] = new;
                    prev[i] = current;
                }
                if (distance[i] < min) {
                    min = distance[i];
                    k = i;
                }
            }
        }
        if (k==-1)
        {
            break;
        }
        else
        {
            current = k;
            completed[current]=1;
        }
    }
    if(prev[destination] == -1) 
    {
    	printf("error: there is no path between source and destination");
	return -1;
    }else 
    {

	printPath(source, destination, prev);
    	printf(": %d\n", distance[destination]);
	return distance[destination];
    }

    
}


void printPath(int source, int dest, int prev[]){
  if(source==dest){
    printf("%d - ", source);
  } else{
    printPath(source, prev[dest], prev);
    printf("%d - ", dest);
  }
}




int allcompleted(int completed[], int numNodes)
{
    int i;
    for (i = 0; i<numNodes; i++)
    {
        if (completed[i] == 0)
        {
            return 0;
        }
    }
    return 1;
}

/*Travels through the adjacency list until it hits the current node of vertex index current
 *then it checks and see if it has the edge of vertex index i
 */
int cost(int current, int i)
{
    Vertex *currV; Edge *currE;

    currV = start;
    
    while (currV != NULL && currV->vertexKey != current)
    {
        currV=currV->vertexPtr;
    }
    
    if (currV != NULL)
    {
        currE = currV->edgePtr;
    }
    
    while (currE != NULL && currE->vertexIndex != i)
    {
        currE = currE->edgePtr;
    }
    
    int returned = 0;
    if (currE == NULL || currE->vertexWeight == 0)
    {
        returned = 5000000;
        return returned;
    }
    else
    {
        returned = currE->vertexWeight;
        return returned;
    }
}





