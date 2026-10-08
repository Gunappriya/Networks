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
