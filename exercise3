#include<stdio.h>
#include<string.h>
void convertToBinary(char msg[], int binary[], int *size)
{
    int i, j;
    *size = 0;
    printf("\nCharacter to Binary");
    for(i=0; msg[i]!='\0'; i++)
    {
        int value = msg[i];
        printf("%c : ", msg[i]);
        for(j=7; j>=0; j--)
        {
            binary[*size] = (value >> j) & 1;
            printf("%d", binary[*size]);
            (*size)++;
        }
        printf("\n");
    }
    printf("\nComplete Binary : ");
    for(i=0; i<*size; i++)
        printf("%d", binary[i]);
    printf("\n");
}
int parityBits(int m)
{
    int r = 0;
    while((1<<r) < (m+r+1))
        r++;
    return r;
}
void placeBits(int data[], int hamming[], int m, int r)
{
    int i;
    int k = 0;
    int total = m + r;
    for(i=1; i<=total; i++)
    {
        if((i & (i-1)) == 0)
            hamming[i] = 0;
        else
            hamming[i] = data[k++];
    }
    printf("\nPosition : ");
    for(i=total; i>=1; i--)
        printf("%3d", i);
    printf("\nBit : ");
    for(i=total; i>=1; i--)
        printf("%3d", hamming[i]);
    printf("\n");
}
int findParity(int hamming[], int position, int total)
{
    int i;
    int parity = 0;
    printf("\nCheck P%d : ", position);
    for(i=1; i<=total; i++)
    {
        if(i & position)
        {
            printf("%d ", i);
            parity ^= hamming[i];
        }
    }
    printf("\nParity = %d", parity);
    return parity;
}
int generateHamming(int data[], int hamming[], int size)
{
    int r, total, i;
    r = parityBits(size);
    total = size + r;
    printf("\nNumber of Parity Bits = %d", r);
    placeBits(data, hamming, size, r);
    printf("\nParity Bits");
    for(i=0; i<r; i++)
    {
        int p = 1 << i;
        hamming[p] = findParity(hamming, p, total);
    }
    return total;
}
int main()
{
    char message[100];
    int binary[500];
    int hamming[600];
    int size = 0;
    int total;
    int i;
    FILE *fp1, *fp2;
    printf("-----------------------\n");
    printf(" HAMMING CODE\n");
    printf("-----------------------\n");
    printf("Enter Message : ");
    scanf("%s", message);
    convertToBinary(message, binary, &size);
    fp1 = fopen("out.txt", "w");
    for(i=0; i<size; i++)
        fprintf(fp1, "%d", binary[i]);
    fclose(fp1);
    printf("\nBinary stored in out.txt");
    total = generateHamming(binary, hamming, size);
    printf("\nFinal Hamming Code");
    printf("\nPosition : ");
    for(i=total; i>=1; i--)
        printf("%3d", i);
    printf("\nBit : ");
    for(i=total; i>=1; i--)
        printf("%3d", hamming[i]);
    fp2 = fopen("out2.txt", "w");
    printf("\nHamming Code : ");
    for(i=total; i>=1; i--)
    {
        printf("%d", hamming[i]);
        fprintf(fp2, "%d", hamming[i]);
    }
    fclose(fp2);
    printf("\nHamming code stored in out2.txt\n");
    return 0;
}

cat hamrec.c
#include<stdio.h>
#include<string.h>
int checkParity(int hamming[], int position, int total)
{
    int i;
    int parity = 0;
    printf("\nCheck P%d : ", position);
    for(i=1; i<=total; i++)
    {
        if(i & position)
        {
            printf("%d ", i);
            parity ^= hamming[i];
        }
    }
    printf("\nParity = %d\n", parity);
    return parity;
}
void displayBits(int hamming[], int total)
{
    int i;
    printf("\nPosition : ");
    for(i=total; i>=1; i--)
        printf("%3d", i);
    printf("\nBit : ");
    for(i=total; i>=1; i--)
        printf("%3d", hamming[i]);
    printf("\n");
}
int main()
{
    char code[500];
    int hamming[600];
    int data[500];
    int total;
    int i, j;
    int r = 0;
    int syndrome = 0;
    FILE *fp;
    printf("----------------------\n");
    printf(" HAMMING CODE\n");
    printf("----------------------\n");
    fp = fopen("out2.txt","r");
    if(fp==NULL)
    {
        printf("out2.txt not found\n");
        return 0;
    }
    fscanf(fp,"%s",code);
    fclose(fp);
    total = strlen(code);
    j = 1;
    for(i=total-1; i>=0; i--)
        hamming[j++] = code[i]-'0';
    printf("\nReceived Hamming Code");
    displayBits(hamming,total);
    int choice,pos;
    printf("\nDo you want to change any bit? (1-Yes 0-No): ");
    scanf("%d",&choice);
    if(choice==1)
    {
        printf("Enter Bit Position (1-%d): ",total);
        scanf("%d",&pos);
        if(pos>=1 && pos<=total)
            hamming[pos]=!hamming[pos];
        printf("\nCode After Error");
        displayBits(hamming,total);
    }
    while((1<<r)<=total)
        r++;
    for(i=0; i<r; i++)
    {
        int p = 1 << i;
        if(checkParity(hamming,p,total)==1)
            syndrome += p;
    }
    if(syndrome==0)
    {
        printf("\nNo Error Detected\n");
    }
    else
    {
        printf("\nError Found at Position %d\n",syndrome);
        hamming[syndrome]=!hamming[syndrome];
        printf("Error Corrected Successfully\n");
    }
    printf("\nCorrected Hamming Code\n");
    displayBits(hamming,total);
    printf("\nCorrected Code : ");
    for(i=total;i>=1;i--)
        printf("%d",hamming[i]);
    printf("\n");
    printf("\nExtracting Data Bits");
    int index=0;
    for(i=1;i<=total;i++)
    {
        if((i&(i-1))!=0)
            data[index++]=hamming[i];
    }
    printf("Binary : ");
    for(i=0;i<index;i++)
        printf("%d",data[i]);
    printf("\n");
    printf("\nMessage : ");
    for(i=0;i<index;i+=8)
    {
        int value=0;
        for(j=0;j<8 && (i+j)<index;j++)
        {
            value=(value<<1)+data[i+j];
        }
        printf("%c",value);
    }
    printf("\n");
    return 0;
}
