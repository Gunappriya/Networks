#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_RECORDS 20
#define FRAME_SIZE 80

struct RoutingEntry
{
    char url[50];
    char sourceIP[20];
    char destinationIP[20];
    char sourceMAC[20];
    char destinationMAC[20];
};

struct RoutingEntry routingTable[MAX_RECORDS];

int totalRecords = 0;

/* Function Prototypes */

void addURL();
void deleteURL();
void displayTable();
void createFrame();

/*--------------------------------------------------*/
/* Add URL                                          */
/*--------------------------------------------------*/

void addURL()
{
    if(totalRecords >= MAX_RECORDS)
    {
        printf("\nRouting Table Full.\n");
        return;
    }

    printf("\nEnter URL : ");
    scanf("%s", routingTable[totalRecords].url);

    printf("Enter Source IP : ");
    scanf("%s", routingTable[totalRecords].sourceIP);

    printf("Enter Destination IP : ");
    scanf("%s", routingTable[totalRecords].destinationIP);

    printf("Enter Source MAC : ");
    scanf("%s", routingTable[totalRecords].sourceMAC);

    printf("Enter Destination MAC : ");
    scanf("%s", routingTable[totalRecords].destinationMAC);

    totalRecords++;

    printf("\nURL Added Successfully.\n");
}

/*--------------------------------------------------*/
/* Delete URL                                       */
/*--------------------------------------------------*/

void deleteURL()
{
    char deleteURL[50];
    int i;
    int j;
    int found = 0;

    printf("\nEnter URL to Delete : ");
    scanf("%s", deleteURL);

    for(i = 0; i < totalRecords; i++)
    {
        if(strcmp(routingTable[i].url, deleteURL) == 0)
        {
            found = 1;

            for(j = i; j < totalRecords - 1; j++)
            {
                routingTable[j] = routingTable[j + 1];
            }

            totalRecords--;

            printf("\nURL Deleted Successfully.\n");
            break;
        }
    }

    if(found == 0)
    {
        printf("\nURL Not Found.\n");
    }
}

/*--------------------------------------------------*/
/* Display Routing Table                            */
/*--------------------------------------------------*/

void displayTable()
{
    int i;

    if(totalRecords == 0)
    {
        printf("\nRouting Table is Empty.\n");
        return;
    }

    printf("\n================ ROUTING TABLE ================\n");

    printf("\n%-20s %-18s %-18s\n",
           "URL",
           "SOURCE IP",
           "DESTINATION IP");

    printf("---------------------------------------------------------------\n");

    for(i = 0; i < totalRecords; i++)
    {
        printf("%-20s %-18s %-18s\n",
               routingTable[i].url,
               routingTable[i].sourceIP,
               routingTable[i].destinationIP);

        printf("Source MAC      : %s\n",
               routingTable[i].sourceMAC);

        printf("Destination MAC : %s\n\n",
               routingTable[i].destinationMAC);
    }
}

/*==========================================================
                PART B
      APPLICATION & TRANSPORT LAYER
==========================================================*/

struct Message
{
    char data[500];
};

struct Message message;

char binaryMessage[5000];

int sourcePort;
int destinationPort;

char sourcePortBinary[20];
char destinationPortBinary[20];

/*----------------------------------------------------------
            FUNCTION PROTOTYPES
----------------------------------------------------------*/

void getMessage();
void saveMessage();
void readMessage();
void decimalToBinary(int number,char binary[]);
void convertMessageToBinary();
void generatePorts();
void applicationLayer();
void transportLayer();

/*----------------------------------------------------------
                GET MESSAGE
----------------------------------------------------------*/

void getMessage()
{
    getchar();

    printf("\nEnter Message : ");

    fgets(message.data,500,stdin);

    message.data[strcspn(message.data,"\n")]='\0';
}

/*----------------------------------------------------------
            STORE MESSAGE IN FILE
----------------------------------------------------------*/

void saveMessage()
{
    FILE *fp;

    fp=fopen("message.txt","w");

    if(fp==NULL)
    {
        printf("\nCannot Create File.\n");
        return;
    }

    fprintf(fp,"%s",message.data);

    fclose(fp);

    printf("\nMessage Stored Successfully.\n");
}

/*----------------------------------------------------------
            READ MESSAGE FROM FILE
----------------------------------------------------------*/

void readMessage()
{
    FILE *fp;

    fp=fopen("message.txt","r");

    if(fp==NULL)
    {
        printf("\nCannot Open File.\n");
        return;
    }

    fgets(message.data,500,fp);

    fclose(fp);

    printf("\nMessage Read Successfully.\n");
}

/*----------------------------------------------------------
            DECIMAL TO BINARY
----------------------------------------------------------*/

void decimalToBinary(int number,char binary[])
{
    int i;
    int j=0;

    for(i=7;i>=0;i--)
    {
        binary[j++]=((number>>i)&1)+'0';
    }

    binary[j]='\0';
}

/*----------------------------------------------------------
            MESSAGE TO BINARY
----------------------------------------------------------*/

void convertMessageToBinary()
{
    int i;

    char temp[9];

    binaryMessage[0]='\0';

    for(i=0;message.data[i]!='\0';i++)
    {
        decimalToBinary((unsigned char)message.data[i],temp);

        strcat(binaryMessage,temp);

        strcat(binaryMessage," ");
    }
}

/*----------------------------------------------------------
            GENERATE PORT NUMBERS
----------------------------------------------------------*/

void generatePorts()
{
    sourcePort=5000;

    destinationPort=8080;

    decimalToBinary(sourcePort%256,sourcePortBinary);

    decimalToBinary(destinationPort%256,destinationPortBinary);
}

/*----------------------------------------------------------
            APPLICATION LAYER
----------------------------------------------------------*/

void applicationLayer()
{
    printf("\n====================================");
    printf("\nAPPLICATION LAYER");
    printf("\n====================================\n");

    convertMessageToBinary();

    generatePorts();

    printf("Application Layer Processing Completed.\n");
}

/*----------------------------------------------------------
            TRANSPORT LAYER
----------------------------------------------------------*/

void transportLayer()
{
    printf("\n====================================");
    printf("\nTRANSPORT LAYER");
    printf("\n====================================\n");

    printf("Message            : %s\n",message.data);

    printf("Source Port        : %d\n",sourcePort);

    printf("Destination Port   : %d\n",destinationPort);

    printf("\nTransport Layer Processing Completed.\n");
}

/*==========================================================
                    PART C1
            NETWORK LAYER MODULE
==========================================================*/

char sourceIPBinary[100];
char destinationIPBinary[100];

char sourceMACBinary[200];
char destinationMACBinary[200];

char packets[20][FRAME_SIZE+1];
int totalPackets = 0;

/*----------------------------------------------------------
        CONVERT IP ADDRESS TO BINARY
----------------------------------------------------------*/

void convertIPToBinary(char ip[], char binary[])
{
    char temp[30];
    char *part;
    char binaryValue[9];

    binary[0] = '\0';

    strcpy(temp, ip);

    part = strtok(temp, ".");

    while(part != NULL)
    {
        decimalToBinary(atoi(part), binaryValue);

        strcat(binary, binaryValue);
        strcat(binary, " ");

        part = strtok(NULL, ".");
    }
}

/*----------------------------------------------------------
        CONVERT MAC ADDRESS TO BINARY
----------------------------------------------------------*/

void convertMACToBinary(char mac[], char binary[])
{
    char temp[30];
    char *part;
    char binaryValue[9];
    int value;

    binary[0] = '\0';

    strcpy(temp, mac);

    part = strtok(temp, ":");

    while(part != NULL)
    {
        value = (int)strtol(part, NULL, 16);

        decimalToBinary(value, binaryValue);

        strcat(binary, binaryValue);
        strcat(binary, " ");

        part = strtok(NULL, ":");
    }
}

/*----------------------------------------------------------
        CREATE PACKETS
----------------------------------------------------------*/

void createPackets()
{
    int length;
    int i;
    

    length = strlen(binaryMessage);

    totalPackets = 0;

    for(i = 0; i < length; i += FRAME_SIZE)
    {
        strncpy(packets[totalPackets], binaryMessage + i, FRAME_SIZE);

        packets[totalPackets][FRAME_SIZE] = '\0';

        totalPackets++;
    }

    printf("\nPackets Created : %d\n", totalPackets);
}

/*----------------------------------------------------------
        NETWORK LAYER
----------------------------------------------------------*/

void networkLayer()
{
    int index;

    printf("\n====================================");
    printf("\nNETWORK LAYER");
    printf("\n====================================\n");

    index = totalRecords - 1;

    convertIPToBinary(routingTable[index].sourceIP,
                      sourceIPBinary);

    convertIPToBinary(routingTable[index].destinationIP,
                      destinationIPBinary);

    convertMACToBinary(routingTable[index].sourceMAC,
                       sourceMACBinary);

    convertMACToBinary(routingTable[index].destinationMAC,
                       destinationMACBinary);

    createPackets();

    printf("Network Layer Processing Completed.\n");
}
/*==========================================================
                    PART C2
        DATA LINK LAYER & MAIN FUNCTION
==========================================================*/

char frames[20][FRAME_SIZE + 1];
int totalFrames = 0;

/*----------------------------------------------------------
        CREATE HEADER
----------------------------------------------------------*/

void createHeader(char header[])
{
    header[0]='\0';

    strcat(header,"SRC IP:");
    strcat(header,sourceIPBinary);

    strcat(header,"\nDEST IP:");
    strcat(header,destinationIPBinary);

    strcat(header,"\nSRC MAC:");
    strcat(header,sourceMACBinary);

    strcat(header,"\nDEST MAC:");
    strcat(header,destinationMACBinary);

    strcat(header,"\n");
}

/*----------------------------------------------------------
        CREATE PAYLOAD
----------------------------------------------------------*/

void createPayload(char payload[],int packetNumber)
{
    strcpy(payload,packets[packetNumber]);
}

/*----------------------------------------------------------
        CREATE TRAILER
----------------------------------------------------------*/

void createTrailer(char trailer[])
{
    strcpy(trailer,"00000000");
}

/*----------------------------------------------------------
        PAD LAST FRAME
----------------------------------------------------------*/

void appendZeros(char frame[])
{
    int length=strlen(frame);

    while(length<FRAME_SIZE)
    {
        frame[length]='0';
        length++;
    }

    frame[length]='\0';
}

/*----------------------------------------------------------
        CREATE FRAMES
----------------------------------------------------------*/

void createFrames()
{
    int i;

    totalFrames=totalPackets;

    for(i=0;i<totalPackets;i++)
    {
        strcpy(frames[i],packets[i]);

        if(strlen(frames[i])<FRAME_SIZE)
        {
            appendZeros(frames[i]);
        }
    }
}

/*----------------------------------------------------------
        DISPLAY FRAMES
----------------------------------------------------------*/

void displayFrames()
{
    char header[1000];
    char payload[FRAME_SIZE+1];
    char trailer[20];

    int i;

    printf("\n\n========== FRAMES ==========\n");

    for(i=0;i<totalFrames;i++)
    {
        createHeader(header);

        createPayload(payload,i);

        createTrailer(trailer);

        printf("\nFrame %d\n",i+1);

        printf("----------------------------------\n");

        printf("%s",header);

        printf("PAYLOAD : %s\n",payload);

        printf("TRAILER : %s\n",trailer);

        printf("----------------------------------\n");
    }
}

/*----------------------------------------------------------
        DATA LINK LAYER
----------------------------------------------------------*/

void dataLinkLayer()
{
    printf("\n================================");
    printf("\nDATA LINK LAYER");
    printf("\n================================\n");

    createFrames();

    displayFrames();

    printf("\nData Link Layer Completed.\n");
}

/*----------------------------------------------------------
        DISPLAY SUMMARY
----------------------------------------------------------*/

void displaySummary()
{
    printf("\n================================");
    printf("\nSUMMARY");
    printf("\n================================\n");

    printf("Packets Created : %d\n",totalPackets);

    printf("Frames Created  : %d\n",totalFrames);
}

/*----------------------------------------------------------
        CREATE FRAME
----------------------------------------------------------*/

void createFrame()
{
    if(totalRecords==0)
    {
        printf("\nNo Routing Table Available.\n");
        return;
    }

    getMessage();

    saveMessage();

    readMessage();

    applicationLayer();

    transportLayer();

    networkLayer();

    dataLinkLayer();

    displaySummary();
}

/*----------------------------------------------------------
        MAIN FUNCTION
----------------------------------------------------------*/

int main()
{
    int choice;

    while(1)
    {
        printf("\n=====================================\n");
        printf("      FRAMING PROTOCOL PROGRAM\n");
        printf("=====================================\n");

        printf("1. Add URL\n");
        printf("2. Delete URL\n");
        printf("3. Display URL Table\n");
        printf("4. Create Frame\n");
        printf("5. Exit\n");

        printf("\nEnter Choice : ");
        scanf("%d",&choice);

        switch(choice)
        {
            case 1:
                addURL();
                break;

            case 2:
                deleteURL();
                break;

            case 3:
                displayTable();
                break;

            case 4:
                createFrame();
                break;

            case 5:
                printf("\nProgram Ended.\n");
                exit(0);

            default:
                printf("\nInvalid Choice.\n");
        }
    }

    return 0;
}
