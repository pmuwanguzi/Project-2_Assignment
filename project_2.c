/*
 * Dynamic Text Mining and Linguistic Pattern Analyzer
 * Student: Paul Muwanguzi
 *
 * Compile: gcc -o project_2.c -o project_2_Dynamic_Text_Mining_and_Linguistic_Pattern_Analyzer
 * Run: ./project_2_Dynamic_Text_Mining_and_Linguistic_Pattern_Analyzer
 *
 * How words are handled:
 *  - All letters are changed to lowercase, so "The" and "the" are the same word.
 *  - A word is made of letters and digits.
 *  - An apostrophe or hyphen is kept only if a letter/digit comes after it
 *    inside a word, e.g. "don't" and "well-known" are one word.
 *    "way--in" becomes "way" and "in".
 *  - All other punctuation and spaces separate words. Extra spaces are skipped.
 *  - Words longer than 39 characters are cut to 39 characters
 *    (the size of the name field) and a message is printed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STUDENT_ID "p.muwanguzi@alustudent.com"
#define MAX_LEN 39

struct wordStat {
    char name[40];
    int count;
};

/* global data used by all the analysis functions */
char *text = NULL;                 /* the paragraph (dynamic) */
struct wordStat *words = NULL;     /* word statistics (dynamic) */
int totalWords = 0;
int distinctWords = 0;
int capacity = 0;                  /* size of the words array */
char source[200];

const char *sampleSource =
    "A Tale of Two Cities by Charles Dickens (1859), Book 1, Chapter 1";

const char *sampleText =
    "It was the best of times, it was the worst of times, it was the age of "
    "wisdom, it was the age of foolishness, it was the epoch of belief, it was "
    "the epoch of incredulity, it was the season of Light, it was the season "
    "of Darkness, it was the spring of hope, it was the winter of despair, we "
    "had everything before us, we had nothing before us, we were all going "
    "direct to Heaven, we were all going direct the other way--in short, the "
    "period was so far like the present period, that some of its noisiest "
    "authorities insisted on its being received, for good or for evil, in the "
    "superlative degree of comparison only. "
    "There were a king with a large jaw and a queen with a plain face, on the "
    "throne of England; there were a king with a large jaw and a queen with a "
    "fair face, on the throne of France. In both countries it was clearer "
    "than crystal to the lords of the State preserves of loaves and fishes, "
    "that things in general were settled for ever.";

/* function pointer type for the menu operations */
typedef void (*Operation)(void);

/* reading the text */

/* removes the rest of the input line after scanf */
void clear_line(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void free_memory(void)
{
    free(text);
    free(words);
    text = NULL;
    words = NULL;
}

/* reads a paragraph of any length, stops at an empty line */
int read_text(void)
{
    int size = 100;
    int len = 0;
    int c, prev = 0;

    text = malloc(size);
    if (text == NULL) {
        printf("Memory allocation failed.\n");
        return 0;
    }

    while ((c = getchar()) != EOF) {
        if (c == '\n' && prev == '\n')   /* empty line = end of paragraph */
            break;
        if (len + 1 >= size) {           /* buffer is full, make it bigger */
            char *temp;
            size = size * 2;
            temp = realloc(text, size);
            if (temp == NULL) {
                printf("Memory allocation failed.\n");
                return 0;
            }
            text = temp;
            printf("[Text buffer grown to %d bytes]\n", size);
        }
        text[len] = (char)c;
        len++;
        prev = c;
    }
    text[len] = '\0';
    return 1;
}

int load_sample(void)
{
    text = malloc(strlen(sampleText) + 1);
    if (text == NULL) {
        printf("Memory allocation failed.\n");
        return 0;
    }
    strcpy(text, sampleText);
    strcpy(source, sampleSource);
    return 1;
}

/* building the word statistics */

int add_word(const char *w)
{
    int i;

    /* if the word already exists, increase its count */
    for (i = 0; i < distinctWords; i++) {
        if (strcmp(words[i].name, w) == 0) {
            words[i].count++;
            return 1;
        }
    }

    /* new word: make the array bigger if it is full */
    if (distinctWords == capacity) {
        struct wordStat *temp;
        capacity = capacity * 2;
        temp = realloc(words, capacity * sizeof(struct wordStat));
        if (temp == NULL) {
            printf("Memory allocation failed.\n");
            return 0;
        }
        words = temp;
        printf("[Word statistics array grown to %d entries]\n", capacity);
    }

    strcpy(words[distinctWords].name, w);   /* safe: w is at most 39 chars */
    words[distinctWords].count = 1;
    distinctWords++;
    return 1;
}

/* goes through the text with a pointer and extracts each word */
int extract_words(void)
{
    char *p = text;
    char word[MAX_LEN + 1];
    int len;

    capacity = 10;
    words = calloc(capacity, sizeof(struct wordStat));
    if (words == NULL) {
        printf("Memory allocation failed.\n");
        return 0;
    }

    while (*p != '\0') {
        /* skip spaces and punctuation */
        while (*p != '\0' && !isalnum((unsigned char)*p))
            p++;
        if (*p == '\0')
            break;

        /* copy the word, letter by letter */
        len = 0;
        while (*p != '\0' &&
               (isalnum((unsigned char)*p) ||
                ((*p == '\'' || *p == '-') && isalnum((unsigned char)*(p + 1))))) {
            if (len < MAX_LEN) {
                word[len] = (char)tolower((unsigned char)*p);
                len++;
            } else if (len == MAX_LEN) {
                printf("Note: a long word was cut to 39 characters.\n");
                len++;   /* so the message is printed only once */
            }
            p++;
        }
        if (len > MAX_LEN)
            len = MAX_LEN;
        word[len] = '\0';

        if (!add_word(word))
            return 0;
        totalWords++;
    }
    return 1;
}

/* helper functions */

int find_longest(void)
{
    int i, best = 0;
    for (i = 1; i < distinctWords; i++)
        if (strlen(words[i].name) > strlen(words[best].name))
            best = i;
    return best;
}

int find_most_frequent(void)
{
    int i, best = 0;
    for (i = 1; i < distinctWords; i++)
        if (words[i].count > words[best].count)
            best = i;
    return best;
}

double average_length(void)
{
    int i;
    long letters = 0;
    for (i = 0; i < distinctWords; i++)
        letters += (long)strlen(words[i].name) * words[i].count;
    return (double)letters / totalWords;
}

/* Custom analysis: Lexical Diversity Score = distinct words / total words */
double lexical_diversity(void)
{
    return (double)distinctWords / totalWords * 100.0;
}

int compare_alpha(const void *a, const void *b)
{
    const struct wordStat *x = a;
    const struct wordStat *y = b;
    return strcmp(x->name, y->name);
}

int compare_count(const void *a, const void *b)
{
    const struct wordStat *x = a;
    const struct wordStat *y = b;
    return y->count - x->count;   /* highest count first */
}

/* menu operations */

void count_words(void)
{
    printf("Total number of words: %d\n", totalWords);
}

void longest_word(void)
{
    int i = find_longest();
    printf("Longest word: %s (%d characters)\n",
           words[i].name, (int)strlen(words[i].name));
}

void most_frequent(void)
{
    int i = find_most_frequent();
    printf("Most frequent word: %s (%d times)\n", words[i].name, words[i].count);
}

void display_word_statistics(void)
{
    int i;
    printf("%-5s %-40s %s\n", "No.", "Word", "Count");
    for (i = 0; i < distinctWords; i++)
        printf("%-5d %-40s %d\n", i + 1, words[i].name, words[i].count);
}

void sort_word_statistics(void)
{
    int choice;
    printf("1. Sort alphabetically\n2. Sort by frequency\nChoice: ");
    if (scanf("%d", &choice) != 1)
        choice = 0;
    clear_line();

    if (choice == 1)
        qsort(words, distinctWords, sizeof(struct wordStat), compare_alpha);
    else if (choice == 2)
        qsort(words, distinctWords, sizeof(struct wordStat), compare_count);
    else {
        printf("Invalid choice.\n");
        return;
    }
    display_word_statistics();
}

void search_word(void)
{
    char input[100];
    char *p;
    int i;

    printf("Enter a word to search: ");
    if (fgets(input, sizeof(input), stdin) == NULL)
        return;

    /* remove the newline and make it lowercase */
    input[strcspn(input, "\n")] = '\0';
    for (p = input; *p != '\0'; p++)
        *p = (char)tolower((unsigned char)*p);
    input[MAX_LEN] = '\0';   /* same 39 character limit as the stored words */

    for (i = 0; i < distinctWords; i++) {
        if (strcmp(words[i].name, input) == 0) {
            printf("\"%s\" appears %d time(s).\n", input, words[i].count);
            return;
        }
    }
    printf("\"%s\" was not found.\n", input);
}

void custom_analysis(void)
{
    int i, repeated = 0;
    double score = lexical_diversity();

    for (i = 0; i < distinctWords; i++)
        if (words[i].count > 1)
            repeated++;

    printf("Lexical Diversity Score = distinct words / total words x 100\n");
    printf("                        = %d / %d x 100 = %.1f%%\n",
           distinctWords, totalWords, score);
    printf("Words used more than once: %d\n", repeated);

    if (score >= 70)
        printf("Result: high diversity - the writer rarely repeats words.\n");
    else if (score >= 50)
        printf("Result: medium diversity.\n");
    else
        printf("Result: low diversity - many words are repeated.\n");
}

void generate_text_summary(void)
{
    int l = find_longest();
    int m = find_most_frequent();

    printf("----- Text Summary -----\n");
    printf("Source:              %s\n", source);
    printf("Total words:         %d\n", totalWords);
    printf("Distinct words:      %d\n", distinctWords);
    printf("Average word length: %.2f characters\n", average_length());
    printf("Longest word:        %s (%d characters)\n",
           words[l].name, (int)strlen(words[l].name));
    printf("Most frequent word:  %s (%d times)\n", words[m].name, words[m].count);
    printf("Lexical diversity:   %.1f%%\n", lexical_diversity());
}

void memory_inspection(void)
{
    int m = find_longest();

    printf("Memory Inspection\n");
    printf("--------------------------\n");
    printf("Text buffer address:      %p (start of the paragraph)\n",
           (void *)text);
    printf("Word statistics array:    %p (first element, words[0])\n",
           (void *)words);
    printf("Selected word-stat entry: %p (words[%d] = \"%s\")\n",
           (void *)&words[m], m, words[m].name);
    printf("Each entry uses %d bytes, so words[%d] is %d bytes after words[0].\n",
           (int)sizeof(struct wordStat), m, m * (int)sizeof(struct wordStat));
}

/* main */

int main(void)
{
    Operation operations[] = {
        count_words, longest_word, most_frequent, display_word_statistics,
        sort_word_statistics, search_word, generate_text_summary,
        custom_analysis, memory_inspection
    };
    const char *names[] = {
        "Count words", "Longest word", "Most frequent word",
        "Display word statistics", "Sort word statistics", "Search for a word",
        "Generate text summary", "Lexical diversity (custom)",
        "Memory inspection"
    };
    int count = sizeof(operations) / sizeof(operations[0]);
    int choice, i;

    printf("1. Use the sample paragraph (A Tale of Two Cities)\n");
    printf("2. Enter / paste my own paragraph\n");
    printf("Choice: ");
    if (scanf("%d", &choice) != 1)
        choice = 1;
    clear_line();

    if (choice == 2) {
        printf("Enter the source (title and author): ");
        if (fgets(source, sizeof(source), stdin) == NULL)
            strcpy(source, "Unknown");
        source[strcspn(source, "\n")] = '\0';
        printf("Paste the paragraph, then press Enter on an empty line:\n");
        if (!read_text()) {
            free_memory();
            return 1;
        }
    } else if (!load_sample()) {
        return 1;
    }

    if (!extract_words()) {
        free_memory();
        return 1;
    }
    if (totalWords == 0) {
        printf("Error: the text is empty. No words to analyse.\n");
        free_memory();
        return 1;
    }

    printf("\nStudent ID: %s\n", STUDENT_ID);
    printf("Project: Dynamic Text Mining and Linguistic Pattern Analyzer\n");
    printf("Source: %s\n", source);
    printf("Custom Analysis: Lexical Diversity Score\n");

    while (1) {
        printf("\n===== MENU =====\n");
        for (i = 0; i < count; i++)
            printf("%d. %s\n", i + 1, names[i]);
        printf("0. Exit\nChoice: ");

        if (scanf("%d", &choice) != 1)
            choice = 0;   /* bad input or end of input: exit */
        clear_line();

        if (choice == 0)
            break;
        if (choice < 1 || choice > count) {
            printf("Invalid choice.\n");
            continue;
        }
        printf("\n");
        operations[choice - 1]();   /* call the function through the pointer */
    }

    free_memory();
    printf("Memory freed. Goodbye.\n");
    return 0;
}
