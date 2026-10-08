#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MSG_LEN 1000
#define STREAM_SIZE 30000

void readTransmissionFile(char receivedStream[]);
void BitUnstuffing(char receivedStream[], char unstuffedStream[]);
unsigned short computeChecksum(const char *payloadAndChecksumStream);
int ReceiverProcess(char unstuffedStream[]);
unsigned char binaryStringToChar(const char bitStr[]);

int main()
{
    printf("=====================================================================\n");
    printf("                  RECEIVER ENGINE START (METHOD A)                   \n");
    printf("=====================================================================\n");

    char receivedStream[STREAM_SIZE] = "";
    char unstuffedStream[STREAM_SIZE] = "";

    readTransmissionFile(receivedStream);
    BitUnstuffing(receivedStream, unstuffedStream);

    // Initial Processing
    int errorDetected = ReceiverProcess(unstuffedStream);

    if (!errorDetected)
    {
        printf("\n=====================================================================\n");
        printf("                      INTERACTIVE ERROR INJECTION                    \n");
        printf("=====================================================================\n");

        char choice;
        printf("Do you want to modify a data bit in the payload? (y/n): ");
        scanf(" %c", &choice);

        if (choice == 'y' || choice == 'Y')
        {
            int charPos, bitPos;
            printf("Enter character index to modify (1-based index): ");
            scanf("%d", &charPos);
            printf("Enter bit position within character (1 to 8): ");
            scanf("%d", &bitPos);

            // calculating bit index offset past headers:
            // MACs (96) + IPs (64) + Ports (32) = 192 bits
            int HEADER_OFFSET = 192;
            int targetBitIdx = HEADER_OFFSET + ((charPos - 1) * 8) + (bitPos - 1);

            int totalBits = (int)strlen(unstuffedStream);
            if (targetBitIdx >= HEADER_OFFSET && targetBitIdx < (totalBits - 16))
            {
                // Complement (flip) the chosen bit
                unstuffedStream[targetBitIdx] = (unstuffedStream[targetBitIdx] == '0') ? '1' : '0';

                printf("\n[Simulator] Flipped bit at Character %d, Bit %d (Stream index %d).\n",
                       charPos, bitPos, targetBitIdx);
                printf("Re-running Method A Checksum Verification...\n\n");

                ReceiverProcess(unstuffedStream);
            }
            else
            {
                printf("\n[Error] Target bit position is out of valid payload bounds!\n");
            }
        }
        else
        {
            printf("\nExiting program cleanly. Data is intact!\n");
        }
    }

    return 0;
}

void readTransmissionFile(char receivedStream[])
{
    FILE *fIn = fopen("tran.txt", "r");
    if (fIn == NULL)
    {
        printf("Error: Could not open 'tran.txt' transmission file.\n");
        exit(1);
    }
    fscanf(fIn, "%s", receivedStream);
    fclose(fIn);
}

void BitUnstuffing(char receivedStream[], char unstuffedStream[])
{
    int totalLen = (int)strlen(receivedStream);
    int payloadStart = 8;          // droping opening flag
    int payloadEnd = totalLen - 8;   // droping closing flag

    int outIdx = 0;
    int consecutiveOnes = 0;

    int i;
    for (i = payloadStart; i < payloadEnd; i++)
    {
        char currentBit = receivedStream[i];
        unstuffedStream[outIdx++] = currentBit;

        if (currentBit == '1')
        {
            consecutiveOnes++;
            if (consecutiveOnes == 5)
            {
                i++; // Skip stuffed '0' bit
                consecutiveOnes = 0;
            }
        }
        else
        {
            consecutiveOnes = 0;
        }
    }
    unstuffedStream[outIdx] = '\0';
}

//Sums ALL 16-bit words (Payload Data + Received Checksum Word)
// Returns ~sum , If the result is 0x0000 (all 0s),then valid .
unsigned short computeChecksum(const char *payloadAndChecksumStream)
{
    char paddedStream[STREAM_SIZE];
    strcpy(paddedStream, payloadAndChecksumStream);

    int len = (int)strlen(paddedStream);
    int remainder = len % 16;

    // Apply 0-padding if required before processing 16-bit blocks
    if (remainder != 0)
    {
        int padBits = 16 - remainder;
        int k;
        for (k = 0; k < padBits; k++)
        {
            strcat(paddedStream, "0");
        }
        len += padBits;
    }

    unsigned int sum = 0;
    int i;
    for (i = 0; i < len; i += 16)
    {

         unsigned short word = 0;
        int j;
         for (j= 0; j < 16; j++)
        {
            word = (word << 1) | (paddedStream[i + j] - '0');
        }
        sum += word;
        if (sum > 0xFFFF)
        {
            sum = (sum & 0xFFFF) + 1; // Carry wrap-around
        }
    }

    return (unsigned short)(~sum); // Complement of the total sum
}

int ReceiverProcess(char unstuffedStream[])
{
    int totalLen = (int)strlen(unstuffedStream);
    int bitOffset = 0;

    //slicing Header Bits
    char recSrcMAC[49], recDestMAC[49], recSrcIP[33], recDestIP[33], recSrcPort[17], recDestPort[17];

    strncpy(recSrcMAC, &unstuffedStream[bitOffset], 48); recSrcMAC[48] = '\0'; bitOffset += 48;
    strncpy(recDestMAC, &unstuffedStream[bitOffset], 48); recDestMAC[48] = '\0'; bitOffset += 48;
    strncpy(recSrcIP, &unstuffedStream[bitOffset], 32); recSrcIP[32] = '\0'; bitOffset += 32;
    strncpy(recDestIP, &unstuffedStream[bitOffset], 32); recDestIP[32] = '\0'; bitOffset += 32;
    strncpy(recSrcPort, &unstuffedStream[bitOffset], 16); recSrcPort[16] = '\0'; bitOffset += 16;
    strncpy(recDestPort, &unstuffedStream[bitOffset], 16); recDestPort[16] = '\0'; bitOffset += 16;

    // slicing Payload + Checksum block
    char payloadAndChecksumBlock[STREAM_SIZE];
    strcpy(payloadAndChecksumBlock, &unstuffedStream[bitOffset]);

    // Adding (Data + Checksum) in 1's complement yields 0xFFFF,Compliment (~0xFFFF) = 0x0000.
    unsigned short checkResult = computeChecksum(payloadAndChecksumBlock);

    printf("---------------------------------------------------------------------\n");
    if (checkResult == 0x0000)
    {
        printf("[Checksum Status] PASSED -> 1's Complement Sum Inverted = 0x0000 (All 0s)!\n");
        printf("---------------------------------------------------------------------\n");

        printf("Extracted Src MAC  : %s\n", recSrcMAC);
        printf("Extracted Dest MAC : %s\n", recDestMAC);
        printf("Extracted Src IP   : %s\n", recSrcIP);
        printf("Extracted Dest IP  : %s\n", recDestIP);
        printf("Extracted Src Port : %s\n", recSrcPort);
        printf("Extracted Dest Port: %s\n", recDestPort);

        // Separate payload text from the final 16-bit checksum
        int payloadBitsLen = totalLen - bitOffset - 16;
        char finalMessageText[MAX_MSG_LEN] = "";
        char singleCharBits[9];
        int txtIndex = 0, pOffset = 0;

        while (pOffset < payloadBitsLen && txtIndex < MAX_MSG_LEN - 1)
        {
            strncpy(singleCharBits, &payloadAndChecksumBlock[pOffset], 8);
            singleCharBits[8] = '\0';
            finalMessageText[txtIndex++] = binaryStringToChar(singleCharBits);
            pOffset += 8;
        }
        finalMessageText[txtIndex] = '\0';

        printf("Parsed Message Text: \"%s\"\n", finalMessageText);

        FILE *fOut = fopen("out.txt", "w");
        if (fOut != NULL)
        {
            fprintf(fOut, "%s", finalMessageText);
            fclose(fOut);
            printf("Success: Decoded payload saved to 'out.txt'!\n");
        }
        return 0; // Success
    }
    else
    {
        printf("[Checksum Status] FAILED -> Residual = 0x%04X (Expected 0x0000)!\n", checkResult);
        printf("[Receiver Action] ERROR DETECTED IN PAYLOAD! Discarding frame.\n");
        return 1; // Error detected
    }
}


unsigned char binaryStringToChar(const char bitStr[])
{
    unsigned char val = 0;

    int i;
    for (i = 0; i < 8; i++)
    {
        // If the string is shorter than 8 characters, stop safely
        if (bitStr[i] == '\0') break;

        val <<= 1;
        if (bitStr[i] == '1')
        {
            val |= 1;
        }
    }

    return val;
}
