/*
    Copyright (C) 2020, Sathira Silva (e17331@eng.pdn.ac.lk).
    
    This program generates a bar chart showing statistics of words or characters (depending upon the mode which the program is 
    currently running on) occurred in the given input file(s) containing texts. The chart shows most 10 frequent words (in default, 
    it can be chainged by giving the relevant command line arguments) with their frequencies in non increasing order. If two words
    have the same frequency, the first occurred word is shown first.
    
    Approach:   Since there can be multiple files of huge amounts of words provided as inputs, a both space and time efficient Data
                Structure is needed. A naive algorithm would be to store the words and there frequencies in an array with there order
                of occurrence and then sort the array by both frequency and order of occurrence. But it's very inefficient. Therefore,
                this program uses the Binary Max Heap Data Structure. Most of the operations in heaps are of order O(log(n)) time.
                The heap is stored using an array so that additional informations of each vertex in the heap do not have to be stored
                and they can be computed on the fly. An extra priority parameter is added to the heap other than the frequency of a
                word which is the order of occurrence of the word.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAXSCREENWIDTH 80
#define CHAR_COUNT 36
#define max(a, b) (a > b) ? a : b

// Enumerated operating mode: Character mode or Word mode.
typedef enum _
{
    CHARACTER,
    WORD
} Mode_t;

// Preprocess the given word (Remove non-alphanumeric characters and convert to lowercase).
void preproccess(char *word)
{
    int i = 0, j = 0;
    char c;
    while ((c = word[i++]) != '\0')
    {
        if (isalnum(c))
        {
            if (isalpha(c))
                word[j++] = tolower(c);
            else
                word[j++] = c;
        }
    }
    word[j] = '\0';
}

void displayUsageMessage()
{
    printf("usage: freq [-l length] [-w | -c] [--scaled] filename1 filename2 ..\n");
}

/* Trie Data Structure */
typedef struct __
{
    bool isLeaf;
    int frequency, heapId;
    struct __ *children[CHAR_COUNT];
} trienode_t;

trienode_t *createNewNode()
{
    trienode_t *newNode = (trienode_t *)malloc(sizeof(trienode_t));
    newNode->isLeaf = false;
    newNode->frequency = 0;
    newNode->heapId = -1;
    for (int i = 0; i < CHAR_COUNT; i++)
        newNode->children[i] = NULL;
    return newNode;
}
/* End of Trie Data Structure */

/* Utilities for Array - based Binary Max Heap Data Structure (With the order of occurrence also as a priority value) */
typedef struct ___
{
    char *word; // String word as a key.
    trienode_t *root;
    int frequency, occurredAt; //Priority values.
} word_t;

/* Global variables to store the max heap Data Structure */
word_t *frequencyHeap;
long long capacity = 1, size = 0, totalWords = 0, uniqueOccurrences = 0;
/* End of the Global Variables */

// Returns the parent's index of the ith vertex from the complete binary tree.
int parent(int);
// Returns the left child's index of the ith vertex from the complete binary tree.
int leftChild(int);
// Returns the right child's index of the ith vertex from the complete binary tree.
int rightChild(int);
// Swaps two elements of a word_t array.
void swap(word_t *, word_t *);
// Sifts up the ith vertex of the binary heap.
void siftUp(int);
// Sifts down the ith vertex of the binary heap.
void siftDown(int);
// Inserts a new vertex to the binary heap.
void insert(trienode_t **, char *, char *);
// Extracts the root node of the binary max heap.
word_t extractMax();

int parent(int i)
{
    return (i - 1) / 2;
}

int leftChild(int i)
{
    return 2 * i + 1;
}

int rightChild(int i)
{
    return 2 * i + 2;
}

void swap(word_t *a, word_t *b)
{
    word_t temp = *b;
    *b = *a;
    *a = temp;
}

void siftUp(int i)
{
    while (i > 0 && (frequencyHeap[parent(i)].frequency < frequencyHeap[i].frequency || (frequencyHeap[parent(i)].frequency == frequencyHeap[i].frequency && frequencyHeap[parent(i)].occurredAt > frequencyHeap[i].occurredAt)))
    {
        frequencyHeap[parent(i)].root->heapId = i;
        frequencyHeap[i].root->heapId = parent(i);
        swap(&frequencyHeap[parent(i)], &frequencyHeap[i]);
        i = parent(i);
    }
}

void siftDown(int i)
{
    int maxIndex = i;
    int left = leftChild(i), right = rightChild(i);
    if (left > size || right > size)
        return;
    if (frequencyHeap[left].frequency > frequencyHeap[maxIndex].frequency || (frequencyHeap[left].frequency == frequencyHeap[maxIndex].frequency && frequencyHeap[left].occurredAt < frequencyHeap[maxIndex].occurredAt))
        maxIndex = left;
    if (frequencyHeap[right].frequency > frequencyHeap[maxIndex].frequency || (frequencyHeap[right].frequency == frequencyHeap[maxIndex].frequency && frequencyHeap[right].occurredAt < frequencyHeap[maxIndex].occurredAt))
        maxIndex = right;
    if (i != maxIndex)
    {
        swap(&frequencyHeap[i], &frequencyHeap[maxIndex]);
        siftDown(maxIndex);
    }
}

void insertToHeap(char *word, trienode_t **root)
{
    totalWords++;
    if ((*root)->heapId != -1)
    {
        (frequencyHeap[(*root)->heapId].frequency)++;
        siftUp((*root)->heapId);
        return;
    }
    if (size == capacity)
    {
        capacity *= 2;
        frequencyHeap = realloc(frequencyHeap, capacity * sizeof(word_t));
    }
    frequencyHeap[size].word = malloc((strlen(word) + 1) * sizeof(char));
    frequencyHeap[size].word = word;
    frequencyHeap[size].frequency = (*root)->frequency;
    frequencyHeap[size].root = *root;
    (*root)->heapId = size;
    frequencyHeap[size].occurredAt = uniqueOccurrences++;
    siftUp(size++);
}

void insert(trienode_t **root, char *word, char *tempWord)
{
    if (*root == NULL)
        *root = createNewNode();
    if (*word != '\0')
    {
        char c = *word;
        if (c >= 'a')
            c -= 'a';
        else
            c -= '0' - 26;
        insert(&((*root)->children[(int)c]), word + 1, tempWord);
    }
    else
    {
        if ((*root)->isLeaf)
            ((*root)->frequency)++;
        else
        {
            (*root)->isLeaf = true;
            (*root)->frequency = 1;
        }
        insertToHeap(tempWord, root);
    }
}

word_t extractMax()
{
    word_t result = frequencyHeap[0];
    frequencyHeap[0] = frequencyHeap[--size];
    siftDown(0);
    return result;
}
/* End of Utilities for Binary Max Heap Data Structure */

int main(int argc, char *argv[])
{
    int before = clock();
    frequencyHeap = (word_t *)malloc(capacity * sizeof(word_t)); // Allocate memory for the binary heap.
    trienode_t *root = (trienode_t *)malloc(sizeof(trienode_t));
    int length = 10, maxWordLength = 0, scale = 0;
    bool scaled = false;
    Mode_t mode = WORD;
    FILE *fPtr;
    if (argc == 1)
    {
        printf("No input files were given\n");
        displayUsageMessage();
        return 0;
    }
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-l") == 0)
            {
                if (i == argc - 1)
                {
                    printf("Not enough options for [%s]\n", argv[i]);
                    displayUsageMessage();
                    return 0;
                }
                if ((length = atoi(argv[++i])) == 0)
                {
                    printf("Invalid options for [%s]\n", argv[i - 1]);
                    displayUsageMessage();
                    return 0;
                }
            }
            else if (strcmp(argv[i], "-c") == 0)
                mode = CHARACTER;
            else if (strcmp(argv[i], "-w") == 0)
                mode = WORD;
            else if (strcmp(argv[i], "--scaled") == 0)
                scaled = true;
            else
            {
                printf("Invalid option [%s]\n", argv[i]);
                displayUsageMessage();
                return 0;
            }
        }
        else
        {
            if ((fPtr = fopen(argv[i], "r")) != NULL)
            {
                while (!feof(fPtr))
                {
                    char *word = malloc(128 * sizeof(char));
                    if (mode == WORD)
                        fscanf(fPtr, "%s", word);
                    else
                        fscanf(fPtr, "%c", word);
                    preproccess(word);
                    if (strlen(word))
                        insert(&root, word, word);
                }
                fclose(fPtr);
            }
            else
            {
                printf("Cannot open one or more given files\n");
                displayUsageMessage();
                return 0;
            }
        }
    }
    word_t *words = (word_t *)malloc(length * sizeof(word_t)); // Store the words to be displayed.
    for (int i = 0; i < length; i++)
    {
        words[i] = extractMax();
        maxWordLength = max(maxWordLength, strlen(words[i].word));
    }
    scale = totalWords;
    if (scaled)
        scale = words[0].frequency;
    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j <= maxWordLength; j++)
            printf(" ");
        printf("\u2502");
        for (int j = 0; j < (int)((MAXSCREENWIDTH - 5) * ((float)words[i].frequency / scale)); j++)
            printf("\u2591");
        printf("\n");
        printf("%s", words[i].word);
        for (int j = strlen(words[i].word); j <= maxWordLength; j++)
            printf(" ");
        printf("\u2502");
        for (int j = 0; j < (int)((MAXSCREENWIDTH - 5) * ((float)words[i].frequency / scale)); j++)
            printf("\u2591");
        printf("%.2f%%\n", 100 * (float)words[i].frequency / totalWords);
        for (int j = 0; j <= maxWordLength; j++)
            printf(" ");
        printf("\u2502");
        for (int j = 0; j < (int)((MAXSCREENWIDTH - 5) * ((float)words[i].frequency / scale)); j++)
            printf("\u2591");
        printf("\n");
        for (int j = 0; j <= maxWordLength; j++)
            printf(" ");
        printf("\u2502\n");
    }
    for (int i = 0; i <= maxWordLength; i++)
        printf(" ");
    printf("\u2514");
    for (int i = 0; i < 80; i++)
        printf("\u2500");
    printf("\n");
    printf("The solution took %lds to execute\n", (clock() - before) / 1000);
    return 0;
}
