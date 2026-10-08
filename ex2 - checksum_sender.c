#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_MSG_LEN 1000
#define STREAM_SIZE 30000
#define BIT_FLAG "01111110"

typedef struct
{
    char url[50];
    char ip[20];
    char mac[20];
    int port;
} NetworkEntry;

void charToBinaryString(char ch, char outputStr[]);
void intTo16BitBinaryString(int num, char outputStr[]);
void ipToBinaryString(char ipStr[], char outputStr[]);
void macToBinaryString(char macStr[], char outputStr[]);


void initializeLookupTable(NetworkEntry table[]);
int resolveURL(NetworkEntry table[], char targetURL[], char destIP[], char destMAC[], int *destPort);
int readInputFile(char fileMessage[]);

// network Layers
void ApplicationLayer(char fileMessage[], int msgLength, char appStream[]);
void TransportLayer(char appStream[], int srcPort, int destPort, char binSrcPort[], char binDestPort[], char transportStream[]);
void NetworkLayer(char transportStream[], char srcIP[], char destIP[], char binSrcIP[], char binDestIP[], char networkStream[]);
void DataLinkLabel(char networkStream[], char srcMAC[], char destMAC[], char binSrcMAC[], char binDestMAC[], char finalStream[]);

// Error Detection & Framing
unsigned short computePayloadChecksum(const char *payloadStream);
void BitStuffing(char finalStream[], char stuffedStream[]);
void writeTransmissionFile(char stuffedStream[]);

int main()
{
    srand((unsigned int)time(NULL));

    char srcIP[20] = "192.168.1.5";
    char srcMAC[20] = "AA:BB:CC:DD:EE:11";
    int srcPort = 49152 + (rand() % (65535 - 49152 + 1));

    NetworkEntry table[3];
    initializeLookupTable(table);

    char inputURL[50];
    char destIP[20], destMAC[20];
    int destPort;

    printf("=====================================================================\n");
    printf("                        SENDER ENGINE START                          \n");
    printf("=====================================================================\n");
    printf("Enter Target URL (e.g. www.gmail.com, www.facebook.com): ");
    scanf("%49s", inputURL);

    if (resolveURL(table, inputURL, destIP, destMAC, &destPort) == -1)
    {
        printf("\nError: URL not found in lookup table.\n");
        return 1;
    }

    char fileMessage[MAX_MSG_LEN];
    int msgLength = readInputFile(fileMessage);
    if (msgLength <= 0) return 1;

    char appStream[STREAM_SIZE] = "";
    char transportStream[STREAM_SIZE] = "";
    char networkStream[STREAM_SIZE] = "";
    char finalStream[STREAM_SIZE] = "";
    char stuffedStream[STREAM_SIZE] = "";

    char binSrcPort[17], binDestPort[17];
    intTo16BitBinaryString(srcPort, binSrcPort);
    intTo16BitBinaryString(destPort, binDestPort);

    char binSrcIP[33], binDestIP[33];
    ipToBinaryString(srcIP, binSrcIP);
    ipToBinaryString(destIP, binDestIP);

    char binSrcMAC[49], binDestMAC[49];
    macToBinaryString(srcMAC, binSrcMAC);
    macToBinaryString(destMAC, binDestMAC);

    // Convert Payload to Binary
    ApplicationLayer(fileMessage, msgLength, appStream);

    // Compute Checksum strictly on Payload (with padding if needed)
    unsigned short payloadChksum = computePayloadChecksum(appStream);
    char binChecksum[17];
    intTo16BitBinaryString(payloadChksum, binChecksum);

    printf("=====================================================================\n");
    printf("                 PAYLOAD 16-BIT INTERNET CHECKSUM                    \n");
    printf("=====================================================================\n");
    printf("Payload Bits Length     : %d bits\n", (int)strlen(appStream));
    printf("Calculated Checksum     : 0x%04X -> Binary: %s\n\n", payloadChksum, binChecksum);

    // perform layer encapsulation
    TransportLayer(appStream, srcPort, destPort, binSrcPort, binDestPort, transportStream);
    NetworkLayer(transportStream, srcIP, destIP, binSrcIP, binDestIP, networkStream);
    DataLinkLabel(networkStream, srcMAC, destMAC, binSrcMAC, binDestMAC, finalStream);

    // append Checksum to the end of the final stream
    strcat(finalStream, binChecksum);

    // frame with Bit stuffing & write to file
    BitStuffing(finalStream, stuffedStream);
    writeTransmissionFile(stuffedStream);

    printf("\n>>> SENDER COMPLETE: Transmission frame saved to 'tran.txt' <<<\n");
    return 0;
}

// 16-Bit Checksum calculated on payload with padding
unsigned short computePayloadChecksum(const char *payloadStream)
{
    char paddedPayload[STREAM_SIZE];
    strcpy(paddedPayload, payloadStream);

    int len = (int)strlen(paddedPayload);
    int remainder = len % 16;

    if (remainder != 0)
    {
        int padBits = 16 - remainder;
        int k;
        for (k = 0; k < padBits; k++)
        {
            strcat(paddedPayload, "0");
        }
        len += padBits;
    }

    int i;
    unsigned int sum = 0;
    for (i = 0; i < len; i += 16)
    {
        unsigned short word = 0;   //16 bits
        int j;
        for (j = 0; j < 16; j++)
        {
            word = (word << 1) | (paddedPayload[i + j] - '0');
        }
        sum += word;
        if (sum > 0xFFFF)
        {
            sum = (sum & 0xFFFF) + 1;   //masking the lower 16 bits
        }
    }
    return (unsigned short)(~sum);
}

void BitStuffing(char finalStream[], char stuffedStream[])
{
    int len = (int)strlen(finalStream);
    int outIdx = 0;

    strcpy(stuffedStream, BIT_FLAG);
    outIdx = (int)strlen(stuffedStream);

    int consecutiveOnes = 0;

    int i;
    for (i = 0; i < len; i++)
    {
        char currentBit = finalStream[i];
        stuffedStream[outIdx++] = currentBit;

        if (currentBit == '1')
        {
            consecutiveOnes++;
            if (consecutiveOnes == 5)
            {
                stuffedStream[outIdx++] = '0';
                consecutiveOnes = 0;
            }
        }
        else
        {
            consecutiveOnes = 0;
        }
    }

    stuffedStream[outIdx] = '\0';
    strcat(stuffedStream, BIT_FLAG);
}

void writeTransmissionFile(char stuffedStream[])
{
    FILE *fOut = fopen("tran.txt", "w");
    if (fOut != NULL) {
        fprintf(fOut, "%s", stuffedStream);
        fclose(fOut);
    }
}

void initializeLookupTable(NetworkEntry table[])
{
    strcpy(table[0].url, "www.gmail.com");
    strcpy(table[0].ip, "142.250.190.46");
    strcpy(table[0].mac, "00:11:22:33:44:55");
    table[0].port = 443;

    strcpy(table[1].url, "www.facebook.com");
    strcpy(table[1].ip, "157.240.22.35");
    strcpy(table[1].mac, "66:77:88:99:AA:BB");
    table[1].port = 80;

    strcpy(table[2].url, "www.yahoo.com");
    strcpy(table[2].ip, "98.137.11.163");
    strcpy(table[2].mac, "CC:DD:EE:FF:00:11");
    table[2].port = 8080;
}

int resolveURL(NetworkEntry table[], char targetURL[], char destIP[], char destMAC[], int *destPort)
{
   int i;
    for (i = 0; i < 3; i++) {
        if (strcmp(table[i].url, targetURL) == 0) {
            strcpy(destIP, table[i].ip);
            strcpy(destMAC, table[i].mac);
            *destPort = table[i].port;
            return i;
        }
    }
    return -1;
}

int readInputFile(char fileMessage[])
{
    FILE *filePtr = fopen("inp.txt", "r");
    if (filePtr == NULL) {
        FILE *fWrite = fopen("inp.txt", "w");
        fprintf(fWrite, "hi hello");
        fclose(fWrite);
        filePtr = fopen("inp.txt", "r");
    }
    int msgLength = 0, ch;
    while ((ch = fgetc(filePtr)) != EOF && msgLength < MAX_MSG_LEN - 1) {
        fileMessage[msgLength++] = (char)ch;
    }
    fileMessage[msgLength] = '\0';
    fclose(filePtr);
    return msgLength;
}

void ApplicationLayer(char fileMessage[], int msgLength, char appStream[])
{
    appStream[0] = '\0';
    char tempBin[9];
    int i;
    for (i = 0; i < msgLength; i++)
    {
        charToBinaryString(fileMessage[i], tempBin);
        strcat(appStream, tempBin);
    }
}

void TransportLayer(char appStream[], int srcPort, int destPort, char binSrcPort[], char binDestPort[], char transportStream[])
{
    strcpy(transportStream, binSrcPort);
    strcat(transportStream, binDestPort);
    strcat(transportStream, appStream);
}

void NetworkLayer(char transportStream[], char srcIP[], char destIP[], char binSrcIP[], char binDestIP[], char networkStream[])
{
    strcpy(networkStream, binSrcIP);
    strcat(networkStream, binDestIP);
    strcat(networkStream, transportStream);
}

void DataLinkLabel(char networkStream[], char srcMAC[], char destMAC[], char binSrcMAC[], char binDestMAC[], char finalStream[])
{
    strcpy(finalStream, binSrcMAC);
    strcat(finalStream, binDestMAC);
    strcat(finalStream, networkStream);
}

void charToBinaryString(char ch, char outputStr[])
{
    unsigned char uCh = (unsigned char)ch;
    int i;
    for (i = 7; i >= 0; i--)
    {
        outputStr[7 - i] = ((uCh >> i) & 1) ? '1' : '0';
    }
    outputStr[8] = '\0';
}

void intTo16BitBinaryString(int num, char outputStr[])
{
   int i;
    for (i = 15; i >= 0; i--)
    {
        outputStr[15 - i] = ((num >> i) & 1) ? '1' : '0';
    }
    outputStr[16] = '\0';
}

void ipToBinaryString(char ipStr[], char outputStr[])
{
    int o1, o2, o3, o4;
    sscanf(ipStr, "%d.%d.%d.%d", &o1, &o2, &o3, &o4);
    char temp[9]; outputStr[0] = '\0';
    charToBinaryString((char)o1, temp);
    strcat(outputStr, temp);
    charToBinaryString((char)o2, temp);
    strcat(outputStr, temp);
    charToBinaryString((char)o3, temp);
    strcat(outputStr, temp);
    charToBinaryString((char)o4, temp);
    strcat(outputStr, temp);
}

void macToBinaryString(char macStr[], char outputStr[]) {
    int m1, m2, m3, m4, m5, m6;
    sscanf(macStr, "%x:%x:%x:%x:%x:%x", &m1, &m2, &m3, &m4, &m5, &m6);
    char temp[9]; outputStr[0] = '\0';
    charToBinaryString((char)m1, temp);
    strcat(outputStr, temp);
    charToBinaryString((char)m2, temp);
     strcat(outputStr, temp);
    charToBinaryString((char)m3, temp);
    strcat(outputStr, temp);
    charToBinaryString((char)m4, temp);
    strcat(outputStr, temp);
    charToBinaryString((char)m5, temp);
    strcat(outputStr, temp);
    charToBinaryString((char)m6, temp);
    strcat(outputStr, temp);
}
